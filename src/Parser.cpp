// src/Parser.cpp
#include "Parser.h"
#include "Token.h"
#include "AST.h"
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>

// --- Constructor ---
Parser::Parser(const std::vector<Token> &tokens) : m_tokens(tokens), m_currentPos(0) {}

// --- Private Helper Methods ---

const Token &Parser::peek(int offset) const
{
    static Token eofToken(TOK_EOF, "");
    size_t index = m_currentPos + offset;
    if (index >= m_tokens.size())
    {
        return eofToken;
    }
    return m_tokens[index];
}

const Token &Parser::advance()
{
    if (!isAtEnd())
    {
        m_currentPos++;
    }
    return peek(-1);
}

bool Parser::isAtEnd() const
{
    return peek().type == TOK_EOF;
}

bool Parser::match(TokenType expectedType)
{
    if (isAtEnd() || peek().type != expectedType)
    {
        return false;
    }
    advance();
    return true;
}

// Formats a token's source position for error messages
static std::string tokenPosition(const Token &tok)
{
    if (tok.line <= 0) return "";
    return " at line " + std::to_string(tok.line) + ", column " + std::to_string(tok.column);
}

const Token &Parser::consume(TokenType expectedType, const std::string &errorMessage)
{
    if (peek().type == expectedType)
    {
        return advance();
    }
    std::string errorMsg = "Parse Error: " + errorMessage + ". Found " +
                           tokenTypeToString(peek().type) + " ('" + peek().value + "') instead" +
                           tokenPosition(peek()) + ".";
    std::cerr << errorMsg << std::endl;
    throw std::runtime_error(errorMsg);
}

// --- Parsing Methods for Grammar Rules ---

std::unique_ptr<ProgramNode> Parser::parseProgram()
{
    auto programNode = std::make_unique<ProgramNode>();
    while (!isAtEnd())
    {
        programNode->statements.push_back(parseStatement());
    }
    return programNode;
}

std::unique_ptr<StatementNode> Parser::parseStatement()
{
    int line = peek().line;
    int column = peek().column;
    auto stmt = parseStatementInner();
    if (stmt && stmt->line == 0) {
        stmt->line = line;
        stmt->column = column;
    }
    return stmt;
}

std::unique_ptr<StatementNode> Parser::parseStatementInner()
{
    if (peek().type == TOK_LET || peek().type == TOK_CONST)
    {
        if (peek(1).type == TOK_LBRACE)
        {
            return parseDestructuringDeclaration();
        }
        return parseVariableDeclarationStatement();
    }
    else if (peek().type == TOK_EXPORT)
    {
        // Modules are inlined at compile time; `export` just marks the declaration
        advance();
        return parseStatement();
    }
    else if (peek().type == TOK_IMPORT)
    {
        throw std::runtime_error("Parse Error: import statements are resolved by the compiler driver. "
                                 "Compile the importing file with cscript so imports are inlined.");
    }
    else if (peek().type == TOK_FUNCTION)
    {
        return parseFunctionDeclaration();
    }
    else if (isExternDeclarationAhead())
    {
        return parseExternDeclaration();
    }
    else if (isLinkDirectiveAhead())
    {
        return parseLinkDirective();
    }
    else if (peek().type == TOK_TYPE)
    {
        return parseTypeAliasStatement();
    }
    else if (peek().type == TOK_INTERFACE)
    {
        return parseInterfaceDeclaration();
    }
    else if (peek().type == TOK_CLASS)
    {
        return parseClassDeclaration();
    }
    else if (peek().type == TOK_RETURN)
    {
        return parseReturnStatement();
    }
    else if (peek().type == TOK_IF)
    {
        return parseIfStatement();
    }
    else if (peek().type == TOK_WHILE)
    {
        return parseWhileStatement();
    }
    else if (peek().type == TOK_FOR)
    {
        return parseForStatement();
    }
    else if (peek().type == TOK_DO)
    {
        return parseDoWhileStatement();
    }
    else if (peek().type == TOK_SWITCH)
    {
        return parseSwitchStatement();
    }
    else if (peek().type == TOK_TRY)
    {
        return parseTryStatement();
    }
    else if (peek().type == TOK_THROW)
    {
        return parseThrowStatement();
    }
    else if (peek().type == TOK_BREAK)
    {
        advance();
        consume(TOK_SEMICOLON, "Expected ';' after 'break'");
        return std::make_unique<BreakStatementNode>();
    }
    else if (peek().type == TOK_CONTINUE)
    {
        advance();
        consume(TOK_SEMICOLON, "Expected ';' after 'continue'");
        return std::make_unique<ContinueStatementNode>();
    }
    else if (peek().type == TOK_IDENTIFIER || peek().type == TOK_THIS)
    {
        return parseExpressionOrAssignmentStatement(true);
    }
    else
    {
        std::string errorMsg = std::string("Parsing failed: Unexpected token at start of statement: ") +
                               tokenTypeToString(peek().type) + " ('" + peek().value + "')" +
                               tokenPosition(peek());
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
}

std::unique_ptr<StatementNode> Parser::makeAssignmentStatement(std::unique_ptr<ExpressionNode> target,
                                                               std::unique_ptr<ExpressionNode> value)
{
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(target.get())) {
        return std::make_unique<AssignmentStatementNode>(varExpr->name, std::move(value));
    }
    if (dynamic_cast<ArrayAccessNode*>(target.get())) {
        auto *arrAccess = static_cast<ArrayAccessNode*>(target.release());
        std::unique_ptr<ArrayAccessNode> owned(arrAccess);
        return std::make_unique<ArrayAssignmentStatementNode>(std::move(owned->array),
                                                              std::move(owned->index),
                                                              std::move(value));
    }
    if (dynamic_cast<ObjectAccessNode*>(target.get())) {
        auto *objAccess = static_cast<ObjectAccessNode*>(target.release());
        std::unique_ptr<ObjectAccessNode> owned(objAccess);
        return std::make_unique<ObjectPropertyAssignmentNode>(std::move(owned->object),
                                                              owned->property,
                                                              std::move(value));
    }
    throw std::runtime_error("Parse Error: Invalid assignment target");
}

// Builds `target op= value`. For an array element or an object property the
// operator is carried on the assignment node, so codegen evaluates the target
// once and does a load-modify-store. Desugaring these to `a[i] = a[i] + v`
// used to evaluate the index twice — with a side-effecting index that both
// stepped it twice and read from a different slot than it wrote to.
//
// A plain variable target has nothing to evaluate twice, so it keeps the
// simpler desugared form, which reuses the ordinary assignment path.
std::unique_ptr<StatementNode> Parser::makeCompoundAssignmentStatement(
    std::unique_ptr<ExpressionNode> target, BinaryExpressionNode::Operator op,
    std::unique_ptr<ExpressionNode> rhs, size_t targetStart)
{
    if (dynamic_cast<ArrayAccessNode*>(target.get())) {
        auto *access = static_cast<ArrayAccessNode*>(target.release());
        std::unique_ptr<ArrayAccessNode> owned(access);
        auto node = std::make_unique<ArrayAssignmentStatementNode>(
            std::move(owned->array), std::move(owned->index), std::move(rhs));
        node->isCompound = true;
        node->compoundOp = op;
        return node;
    }
    if (dynamic_cast<ObjectAccessNode*>(target.get())) {
        auto *access = static_cast<ObjectAccessNode*>(target.release());
        std::unique_ptr<ObjectAccessNode> owned(access);
        auto node = std::make_unique<ObjectPropertyAssignmentNode>(
            std::move(owned->object), owned->property, std::move(rhs));
        node->isCompound = true;
        node->compoundOp = op;
        return node;
    }

    // Plain variable: re-read the target tokens for the left operand
    size_t save = m_currentPos;
    m_currentPos = targetStart;
    auto readCopy = parseExpression();
    m_currentPos = save;

    auto value = std::make_unique<BinaryExpressionNode>(op, std::move(readCopy), std::move(rhs));
    return makeAssignmentStatement(std::move(target), std::move(value));
}

std::unique_ptr<StatementNode> Parser::parseExpressionOrAssignmentStatement(bool consumeSemicolon)
{
    size_t exprStart = m_currentPos;
    auto expr = parseExpression();
    TokenType next = peek().type;

    std::unique_ptr<StatementNode> stmt;
    if (next == TOK_EQUAL) {
        advance();
        auto value = parseExpression();
        stmt = makeAssignmentStatement(std::move(expr), std::move(value));
    } else if (next == TOK_PLUS_EQUAL || next == TOK_MINUS_EQUAL || next == TOK_STAR_EQUAL ||
               next == TOK_SLASH_EQUAL || next == TOK_PERCENT_EQUAL ||
               next == TOK_PLUS_PLUS || next == TOK_MINUS_MINUS) {
        advance();

        BinaryExpressionNode::Operator op;
        std::unique_ptr<ExpressionNode> rhs;
        switch (next) {
            case TOK_PLUS_EQUAL:    op = BinaryExpressionNode::ADD;      break;
            case TOK_MINUS_EQUAL:   op = BinaryExpressionNode::SUBTRACT; break;
            case TOK_STAR_EQUAL:    op = BinaryExpressionNode::MULTIPLY; break;
            case TOK_SLASH_EQUAL:   op = BinaryExpressionNode::DIVIDE;   break;
            case TOK_PERCENT_EQUAL: op = BinaryExpressionNode::MODULO;   break;
            case TOK_PLUS_PLUS:     op = BinaryExpressionNode::ADD;      break;
            default:                op = BinaryExpressionNode::SUBTRACT; break;
        }
        if (next == TOK_PLUS_PLUS || next == TOK_MINUS_MINUS) {
            rhs = std::make_unique<IntegerLiteralNode>(1);
        } else {
            rhs = parseExpression();
        }

        stmt = makeCompoundAssignmentStatement(std::move(expr), op, std::move(rhs), exprStart);
    } else {
        stmt = std::make_unique<ExpressionStatementNode>(std::move(expr));
    }

    if (consumeSemicolon) {
        consume(TOK_SEMICOLON, "Expected ';' after statement");
    }
    return stmt;
}

std::unique_ptr<StatementNode> Parser::parseDestructuringDeclaration()
{
    bool isConst = (peek().type == TOK_CONST);
    advance(); // consume let/const
    consume(TOK_LBRACE, "Expected '{' in destructuring declaration");
    std::vector<std::string> names;
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        names.push_back(consume(TOK_IDENTIFIER, "Expected binding name in destructuring").value);
        if (peek().type == TOK_COMMA) advance();
        else break;
    }
    consume(TOK_RBRACE, "Expected '}' in destructuring declaration");
    consume(TOK_EQUAL, "Expected '=' in destructuring declaration");
    auto initializer = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';' after destructuring declaration");
    return std::make_unique<DestructuringDeclarationNode>(std::move(names), std::move(initializer), isConst);
}

std::unique_ptr<StatementNode> Parser::parseSwitchStatement()
{
    consume(TOK_SWITCH, "Expected 'switch'");
    consume(TOK_LPAREN, "Expected '(' after 'switch'");
    auto switchNode = std::make_unique<SwitchStatementNode>(parseExpression());
    consume(TOK_RPAREN, "Expected ')' after switch condition");
    consume(TOK_LBRACE, "Expected '{' after switch condition");

    bool sawDefault = false;
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        SwitchStatementNode::CaseClause clause;
        if (peek().type == TOK_CASE) {
            advance();
            clause.value = parseExpression();
            consume(TOK_COLON, "Expected ':' after case value");
        } else if (peek().type == TOK_DEFAULT) {
            if (sawDefault) throw std::runtime_error("Parse Error: Multiple 'default' clauses in switch");
            sawDefault = true;
            advance();
            consume(TOK_COLON, "Expected ':' after 'default'");
        } else {
            throw std::runtime_error("Parse Error: Expected 'case' or 'default' in switch body");
        }
        while (peek().type != TOK_CASE && peek().type != TOK_DEFAULT &&
               peek().type != TOK_RBRACE && !isAtEnd()) {
            clause.statements.push_back(parseStatement());
        }
        switchNode->cases.push_back(std::move(clause));
    }
    consume(TOK_RBRACE, "Expected '}' after switch body");
    return switchNode;
}

std::unique_ptr<StatementNode> Parser::parseInterfaceDeclaration()
{
    consume(TOK_INTERFACE, "Expected 'interface'");
    const Token &nameToken = consume(TOK_IDENTIFIER, "Expected interface name");
    auto interfaceNode = std::make_unique<InterfaceDeclarationNode>(nameToken.value);
    if (peek().type == TOK_EXTENDS) {
        advance();
        interfaceNode->parentInterface = consume(TOK_IDENTIFIER, "Expected parent interface name").value;
    }
    consume(TOK_LBRACE, "Expected '{' after interface name");
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        const Token &memberName = consume(TOK_IDENTIFIER, "Expected interface member name");
        if (peek().type == TOK_LPAREN) {
            // Method signature: name(params): returnType — recorded but not enforced
            advance();
            while (peek().type != TOK_RPAREN && !isAtEnd()) advance();
            consume(TOK_RPAREN, "Expected ')' in interface method signature");
            std::string retType = "void";
            if (peek().type == TOK_COLON) { advance(); retType = parseType(); }
            interfaceNode->members.emplace_back(memberName.value, "method:" + retType);
        } else {
            consume(TOK_COLON, "Expected ':' after interface member name");
            interfaceNode->members.emplace_back(memberName.value, parseType());
        }
        if (peek().type == TOK_SEMICOLON || peek().type == TOK_COMMA) advance();
    }
    consume(TOK_RBRACE, "Expected '}' after interface body");
    return interfaceNode;
}

std::unique_ptr<StatementNode> Parser::parseClassDeclaration()
{
    consume(TOK_CLASS, "Expected 'class'");
    const Token &nameToken = consume(TOK_IDENTIFIER, "Expected class name");
    auto classNode = std::make_unique<ClassDeclarationNode>(nameToken.value);
    if (peek().type == TOK_EXTENDS) {
        advance();
        classNode->parentClass = consume(TOK_IDENTIFIER, "Expected parent class name").value;
    }
    consume(TOK_LBRACE, "Expected '{' after class name");

    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        std::string memberName;
        if (peek().type == TOK_IDENTIFIER) memberName = advance().value;
        else throw std::runtime_error("Parse Error: Expected class member name" );

        if (peek().type == TOK_LPAREN) {
            // Method (or constructor): name(params): ret { body }
            auto method = parseFunctionRest(memberName);
            if (memberName == "constructor") classNode->hasConstructor = true;
            classNode->objectTemplate->properties.emplace_back(memberName, std::move(method));
        } else {
            // Field: name: type;  or  name: type = default;
            consume(TOK_COLON, "Expected ':' after class field name");
            std::string fieldType = parseType();
            std::unique_ptr<ExpressionNode> defaultValue;
            if (peek().type == TOK_EQUAL) {
                advance();
                defaultValue = parseExpression();
            } else {
                // Zero-value defaults by declared type
                if (fieldType == "string") defaultValue = std::make_unique<StringLiteralNode>("");
                else if (fieldType == "f64") defaultValue = std::make_unique<FloatLiteralNode>(0.0);
                else if (fieldType == "boolean") defaultValue = std::make_unique<BooleanLiteralNode>(false);
                else defaultValue = std::make_unique<IntegerLiteralNode>(0);
            }
            consume(TOK_SEMICOLON, "Expected ';' after class field");
            classNode->objectTemplate->properties.emplace_back(memberName, std::move(defaultValue));
            classNode->objectTemplate->properties.back().declaredType = fieldType;
        }
    }
    consume(TOK_RBRACE, "Expected '}' after class body");
    return classNode;
}

std::unique_ptr<StatementNode> Parser::parseTryStatement()
{
    consume(TOK_TRY, "Expected 'try'");
    consume(TOK_LBRACE, "Expected '{' after 'try'");
    auto tryNode = std::make_unique<TryCatchStatementNode>();
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        tryNode->tryStatements.push_back(parseStatement());
    }
    consume(TOK_RBRACE, "Expected '}' after try block");

    if (peek().type == TOK_CATCH) {
        advance();
        if (peek().type == TOK_LPAREN) {
            advance();
            tryNode->errorVariable = consume(TOK_IDENTIFIER, "Expected catch variable name").value;
            if (peek().type == TOK_COLON) { advance(); parseType(); } // optional annotation, ignored
            consume(TOK_RPAREN, "Expected ')' after catch variable");
        }
        consume(TOK_LBRACE, "Expected '{' after 'catch'");
        while (peek().type != TOK_RBRACE && !isAtEnd()) {
            tryNode->catchStatements.push_back(parseStatement());
        }
        consume(TOK_RBRACE, "Expected '}' after catch block");
    } else if (peek().type != TOK_FINALLY) {
        throw std::runtime_error("Parse Error: 'try' requires a 'catch' or 'finally' block");
    }

    if (peek().type == TOK_FINALLY) {
        advance();
        consume(TOK_LBRACE, "Expected '{' after 'finally'");
        while (peek().type != TOK_RBRACE && !isAtEnd()) {
            tryNode->finallyStatements.push_back(parseStatement());
        }
        consume(TOK_RBRACE, "Expected '}' after finally block");
    }
    return tryNode;
}

std::unique_ptr<StatementNode> Parser::parseThrowStatement()
{
    consume(TOK_THROW, "Expected 'throw'");
    auto expr = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';' after throw expression");
    return std::make_unique<ThrowStatementNode>(std::move(expr));
}

std::unique_ptr<TypeAliasNode> Parser::parseTypeAliasStatement() {
    consume(TOK_TYPE, "Expected 'type' keyword");
    const Token &nameToken = consume(TOK_IDENTIFIER, "Expected alias name after 'type'");
    auto typeAlias = std::make_unique<TypeAliasNode>(nameToken.value, "");
    if (peek().type == TOK_LESS) {
        advance();
        while (peek().type != TOK_GREATER) {
            typeAlias->genericParams.push_back(consume(TOK_IDENTIFIER, "Expected generic parameter name").value);
            if (peek().type == TOK_COMMA) advance();
        }
        consume(TOK_GREATER, "Expected '>' after generic parameters");
    }
    consume(TOK_EQUAL, "Expected '=' in type alias");
    typeAlias->targetType = parseType();
    consume(TOK_SEMICOLON, "Expected ';' after type alias");
    return typeAlias;
}

std::unique_ptr<VariableDeclarationNode> Parser::parseVariableDeclarationStatement()
{
    bool isConst = false;
    if (peek().type == TOK_CONST) {
        consume(TOK_CONST, "Expected 'const' keyword");
        isConst = true;
    } else {
        consume(TOK_LET, "Expected 'let' keyword");
    }
    const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name");
    std::string varName = varNameToken.value;
    std::string typeName = "auto";
    if (peek().type == TOK_COLON) {
        consume(TOK_COLON, "Expected ':'");
        typeName = parseType();
    }
    consume(TOK_EQUAL, "Expected '='");
    std::unique_ptr<ExpressionNode> initializer = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';'");
    return std::make_unique<VariableDeclarationNode>(varName, typeName, std::move(initializer), isConst);
}

std::unique_ptr<IfStatementNode> Parser::parseIfStatement()
{
    consume(TOK_IF, "Expected 'if' keyword");
    consume(TOK_LPAREN, "Expected '('");
    std::unique_ptr<ExpressionNode> condition = parseExpression();
    consume(TOK_RPAREN, "Expected ')'");
    consume(TOK_LBRACE, "Expected '{'");
    auto ifNode = std::make_unique<IfStatementNode>(std::move(condition));
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        ifNode->thenStatements.push_back(parseStatement());
    }
    consume(TOK_RBRACE, "Expected '}'");
    if (peek().type == TOK_ELSE) {
        advance();
        if (peek().type == TOK_IF) {
            // else-if chain: parse the nested if as the sole else statement
            ifNode->elseStatements.push_back(parseIfStatement());
        } else {
            consume(TOK_LBRACE, "Expected '{'");
            while (peek().type != TOK_RBRACE && !isAtEnd()) {
                ifNode->elseStatements.push_back(parseStatement());
            }
            consume(TOK_RBRACE, "Expected '}'");
        }
    }
    return ifNode;
}

std::unique_ptr<WhileStatementNode> Parser::parseWhileStatement()
{
    consume(TOK_WHILE, "Expected 'while' keyword");
    consume(TOK_LPAREN, "Expected '('");
    std::unique_ptr<ExpressionNode> condition = parseExpression();
    consume(TOK_RPAREN, "Expected ')'");
    consume(TOK_LBRACE, "Expected '{'");
    auto whileNode = std::make_unique<WhileStatementNode>(std::move(condition));
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        whileNode->bodyStatements.push_back(parseStatement());
    }
    consume(TOK_RBRACE, "Expected '}'");
    return whileNode;
}

std::unique_ptr<StatementNode> Parser::parseAssignmentStatement()
{
    const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name");
    consume(TOK_EQUAL, "Expected '='");
    std::unique_ptr<ExpressionNode> value = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';'");
    return std::make_unique<AssignmentStatementNode>(varNameToken.value, std::move(value));
}

std::unique_ptr<StatementNode> Parser::parseArrayAssignmentStatement()
{
    const Token &arrayNameToken = consume(TOK_IDENTIFIER, "Expected array name");
    std::unique_ptr<ExpressionNode> arrayExpr = std::make_unique<VariableExpressionNode>(arrayNameToken.value);
    consume(TOK_LBRACKET, "Expected '['");
    std::unique_ptr<ExpressionNode> indexExpr = parseExpression();
    consume(TOK_RBRACKET, "Expected ']'");
    consume(TOK_EQUAL, "Expected '='");
    std::unique_ptr<ExpressionNode> valueExpr = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';'");
    return std::make_unique<ArrayAssignmentStatementNode>(std::move(arrayExpr), std::move(indexExpr), std::move(valueExpr));
}

bool Parser::isKnownFunction(const std::string& name)
{
    if (name == "print" || name == "println") return true;
    if (name == "math_sqrt" || name == "math_pow" || name == "math_abs_f64" || name == "math_abs_i32" ||
        name == "math_sin" || name == "math_cos" || name == "math_tan" || 
        name == "math_log" || name == "math_exp") return true;
    if (name == "JSON.stringify" || name == "JSON.parse") return true;
    return false;
}

std::unique_ptr<StatementNode> Parser::parseForStatement()
{
    consume(TOK_FOR, "Expected 'for'");
    consume(TOK_LPAREN, "Expected '('");
    if ((peek().type == TOK_LET || peek().type == TOK_CONST) && (peek(1).type == TOK_IDENTIFIER && peek(2).type == TOK_OF)) {
        bool isConst = (peek().type == TOK_CONST);
        advance(); // consume let/const
        const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name");
        auto iterVar = std::make_unique<VariableDeclarationNode>(varNameToken.value, "auto", nullptr, isConst);
        consume(TOK_OF, "Expected 'of'");
        auto iterable = parseExpression();
        consume(TOK_RPAREN, "Expected ')'");
        consume(TOK_LBRACE, "Expected '{'");
        auto forOfNode = std::make_unique<ForOfStatementNode>(std::move(iterVar), std::move(iterable));
        while (peek().type != TOK_RBRACE && !isAtEnd()) {
            forOfNode->bodyStatements.push_back(parseStatement());
        }
        consume(TOK_RBRACE, "Expected '}'");
        return forOfNode;
    }
    std::unique_ptr<StatementNode> initialization = nullptr;
    if (peek().type == TOK_LET || peek().type == TOK_CONST) {
        // Parse variable declaration inline (without consuming semicolon)
        bool isConst = false;
        if (peek().type == TOK_CONST) { consume(TOK_CONST, "Expected 'const'"); isConst = true; }
        else { consume(TOK_LET, "Expected 'let'"); }
        const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name");
        std::string typeName = "auto";
        if (peek().type == TOK_COLON) { consume(TOK_COLON, "Expected ':'"); typeName = parseType(); }
        consume(TOK_EQUAL, "Expected '='");
        auto initExpr = parseExpression();
        initialization = std::make_unique<VariableDeclarationNode>(varNameToken.value, typeName, std::move(initExpr), isConst);
    } else if (peek().type == TOK_IDENTIFIER || peek().type == TOK_THIS) {
        // Parse assignment/expression inline (without consuming semicolon)
        initialization = parseExpressionOrAssignmentStatement(false);
    }
    consume(TOK_SEMICOLON, "Expected ';' after for-loop init");
    std::unique_ptr<ExpressionNode> condition = nullptr;
    if (peek().type != TOK_SEMICOLON) condition = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';'");
    std::unique_ptr<StatementNode> increment = nullptr;
    if (peek().type != TOK_RPAREN) {
        // Supports i = i + 1, i++, i += 2, arr[i] = v, obj.prop = v, ...
        increment = parseExpressionOrAssignmentStatement(false);
    }
    consume(TOK_RPAREN, "Expected ')'");
    consume(TOK_LBRACE, "Expected '{'");
    auto forNode = std::make_unique<ForStatementNode>(std::move(initialization), std::move(condition), std::move(increment));
    while (peek().type != TOK_RBRACE && !isAtEnd()) forNode->bodyStatements.push_back(parseStatement());
    consume(TOK_RBRACE, "Expected '}'");
    return forNode;
}

std::unique_ptr<DoWhileStatementNode> Parser::parseDoWhileStatement()
{
    consume(TOK_DO, "Expected 'do'");
    consume(TOK_LBRACE, "Expected '{'");
    auto doWhileNode = std::make_unique<DoWhileStatementNode>(nullptr);
    while (peek().type != TOK_RBRACE && !isAtEnd()) doWhileNode->bodyStatements.push_back(parseStatement());
    consume(TOK_RBRACE, "Expected '}'");
    consume(TOK_WHILE, "Expected 'while'");
    consume(TOK_LPAREN, "Expected '('");
    doWhileNode->condition = parseExpression();
    consume(TOK_RPAREN, "Expected ')'");
    consume(TOK_SEMICOLON, "Expected ';'");
    return doWhileNode;
}

std::unique_ptr<FunctionCallNode> Parser::parseFunctionCallStatement()
{
    const Token &funcNameToken = consume(TOK_IDENTIFIER, "Expected function name");
    auto callNode = std::make_unique<FunctionCallNode>(funcNameToken.value);
    consume(TOK_LPAREN, "Expected '('");
    if (peek().type != TOK_RPAREN) callNode->arguments.push_back(parseExpression());
    consume(TOK_RPAREN, "Expected ')'");
    consume(TOK_SEMICOLON, "Expected ';'");
    return callNode;
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() { return parseLogicalOrExpression(); }

std::unique_ptr<ExpressionNode> Parser::parseLogicalOrExpression()
{
    auto expr = parseLogicalAndExpression();
    while (peek().type == TOK_OR) {
        advance();
        expr = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::LOGICAL_OR, std::move(expr), parseLogicalAndExpression());
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalAndExpression()
{
    auto expr = parseBitOrExpression();
    while (peek().type == TOK_AND) {
        advance();
        expr = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::LOGICAL_AND, std::move(expr), parseBitOrExpression());
    }
    return expr;
}

// Bitwise precedence follows TypeScript/C: | below ^ below &, all below comparison.
std::unique_ptr<ExpressionNode> Parser::parseBitOrExpression()
{
    auto expr = parseBitXorExpression();
    while (peek().type == TOK_PIPE) {
        advance();
        expr = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::BIT_OR, std::move(expr), parseBitXorExpression());
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseBitXorExpression()
{
    auto expr = parseBitAndExpression();
    while (peek().type == TOK_CARET) {
        advance();
        expr = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::BIT_XOR, std::move(expr), parseBitAndExpression());
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseBitAndExpression()
{
    auto expr = parseComparisonExpression();
    while (peek().type == TOK_AMPERSAND) {
        advance();
        expr = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::BIT_AND, std::move(expr), parseComparisonExpression());
    }
    return expr;
}

// True when the next two tokens are the same angle bracket written with no
// space between them ("<<"), as opposed to the ">>" that ends nested generics.
bool Parser::isShiftAhead(TokenType half) const
{
    return peek().type == half && peek(1).type == half &&
           peek(1).line == peek().line && peek(1).column == peek().column + 1;
}

std::unique_ptr<ExpressionNode> Parser::parseShiftExpression()
{
    auto expr = parseAdditionExpression();
    while (isShiftAhead(TOK_LESS) || isShiftAhead(TOK_GREATER)) {
        BinaryExpressionNode::Operator op = (peek().type == TOK_LESS)
            ? BinaryExpressionNode::SHIFT_LEFT
            : BinaryExpressionNode::SHIFT_RIGHT;
        advance();
        advance();
        expr = std::make_unique<BinaryExpressionNode>(op, std::move(expr), parseAdditionExpression());
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseComparisonExpression()
{
    auto expr = parseShiftExpression();
    while (peek().type == TOK_EQUAL_EQUAL || peek().type == TOK_NOT_EQUAL ||
           peek().type == TOK_LESS || peek().type == TOK_LESS_EQUAL ||
           peek().type == TOK_GREATER || peek().type == TOK_GREATER_EQUAL) {
        // A "<<" or ">>" here belongs to the shift level, not comparison
        if (isShiftAhead(TOK_LESS) || isShiftAhead(TOK_GREATER)) break;
        TokenType operatorType = peek().type;
        advance();
        auto right = parseShiftExpression();
        BinaryExpressionNode::Operator op;
        switch (operatorType) {
            case TOK_EQUAL_EQUAL: op = BinaryExpressionNode::EQUAL; break;
            case TOK_NOT_EQUAL: op = BinaryExpressionNode::NOT_EQUAL; break;
            case TOK_LESS: op = BinaryExpressionNode::LESS_THAN; break;
            case TOK_LESS_EQUAL: op = BinaryExpressionNode::LESS_EQUAL; break;
            case TOK_GREATER: op = BinaryExpressionNode::GREATER_THAN; break;
            case TOK_GREATER_EQUAL: op = BinaryExpressionNode::GREATER_EQUAL; break;
            default: throw std::runtime_error("Unknown comparison operator");
        }
        expr = std::make_unique<BinaryExpressionNode>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseAdditionExpression()
{
    auto expr = parseMultiplicationExpression();
    while (peek().type == TOK_PLUS || peek().type == TOK_MINUS) {
        TokenType operatorType = peek().type;
        advance();
        auto right = parseMultiplicationExpression();
        expr = std::make_unique<BinaryExpressionNode>(operatorType == TOK_PLUS ? BinaryExpressionNode::ADD : BinaryExpressionNode::SUBTRACT, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicationExpression()
{
    auto expr = parseUnaryExpression();
    while (peek().type == TOK_STAR || peek().type == TOK_SLASH || peek().type == TOK_PERCENT) {
        TokenType operatorType = peek().type;
        advance();
        auto right = parseUnaryExpression();
        BinaryExpressionNode::Operator op;
        if (operatorType == TOK_STAR) op = BinaryExpressionNode::MULTIPLY;
        else if (operatorType == TOK_SLASH) op = BinaryExpressionNode::DIVIDE;
        else op = BinaryExpressionNode::MODULO;
        expr = std::make_unique<BinaryExpressionNode>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseUnaryExpression()
{
    if (peek().type == TOK_BANG || peek().type == TOK_MINUS || peek().type == TOK_TILDE) {
        TokenType operatorType = peek().type;
        advance();
        UnaryExpressionNode::Operator op = UnaryExpressionNode::MINUS;
        if (operatorType == TOK_BANG) op = UnaryExpressionNode::NOT;
        else if (operatorType == TOK_TILDE) op = UnaryExpressionNode::BIT_NOT;
        return std::make_unique<UnaryExpressionNode>(op, parseUnaryExpression());
    }
    return parsePrimaryExpression();
}

std::unique_ptr<ExpressionNode> Parser::parsePrimaryExpression()
{
    std::unique_ptr<ExpressionNode> expr = nullptr;
    if (peek().type == TOK_STRING_LITERAL) expr = parseStringLiteral();
    else if (peek().type == TOK_INT_LITERAL) expr = parseIntegerLiteral();
    else if (peek().type == TOK_FLOAT_LITERAL) {
        expr = std::make_unique<FloatLiteralNode>(std::stod(advance().value));
    }
    else if (peek().type == TOK_TRUE || peek().type == TOK_FALSE) expr = parseBooleanLiteral();
    else if (peek().type == TOK_NULL || peek().type == TOK_UNDEFINED) {
        // A null pointer literal — mainly for optional `ptr` handles from C.
        advance();
        expr = std::make_unique<NullLiteralNode>();
    }
    else if (peek().type == TOK_THIS) {
        advance();
        expr = parseArrayOrObjectAccess(std::make_unique<VariableExpressionNode>("this"));
    }
    else if (peek().type == TOK_IDENTIFIER) {
        if (peek(1).type == TOK_ARROW) expr = parseArrowFunction(); // x => ...
        else expr = parseVariableExpression();
    }
    else if (peek().type == TOK_LPAREN) {
        if (isArrowFunctionAhead()) {
            expr = parseArrowFunction();
        } else {
            advance();
            expr = parseExpression();
            consume(TOK_RPAREN, "Expected ')'");
        }
    }
    else if (peek().type == TOK_NEW) expr = parseNewExpression();
    else if (peek().type == TOK_LBRACKET) expr = parseArrayLiteral();
    else if (peek().type == TOK_LBRACE) expr = parseObjectLiteral();
    else throw std::runtime_error("Parsing failed: Expected an expression, found " +
                                  std::string(tokenTypeToString(peek().type)) + " ('" + peek().value + "')" +
                                  tokenPosition(peek()));
    // Anything can be followed by `.prop`, `[i]` or `.method()`. Chaining here
    // rather than in each producer means it works uniformly for call results,
    // `new` expressions, literals and parenthesised expressions — `f().prop`
    // used to be a parse error because only the plain-variable path chained.
    // The loop interleaves with `!` so `map.get(k)!.length` parses too.
    while (true) {
        expr = parseArrayOrObjectAccess(std::move(expr));
        if (peek().type == TOK_BANG) { advance(); continue; }
        break;
    }
    return expr;
}

// Looks ahead from a '(' for "...) =>" or "...): type =>" to disambiguate
// arrow functions from parenthesized expressions.
bool Parser::isArrowFunctionAhead() const
{
    if (peek().type != TOK_LPAREN) return false;
    int depth = 1;
    int i = 1;
    while (depth > 0) {
        TokenType t = peek(i).type;
        if (t == TOK_EOF) return false;
        if (t == TOK_LPAREN) depth++;
        else if (t == TOK_RPAREN) depth--;
        i++;
    }
    if (peek(i).type == TOK_ARROW) return true;
    if (peek(i).type == TOK_COLON) {
        // Skip a return-type annotation: only type-ish tokens may appear before '=>'
        i++;
        while (true) {
            TokenType t = peek(i).type;
            if (t == TOK_ARROW) return true;
            if (t == TOK_IDENTIFIER || t == TOK_TYPE_STRING || t == TOK_TYPE_I32 ||
                t == TOK_TYPE_F64 || t == TOK_TYPE_BOOLEAN || t == TOK_TYPE_NUMBER ||
                t == TOK_TYPE_VOID || t == TOK_LESS || t == TOK_GREATER ||
                t == TOK_COMMA || t == TOK_LBRACKET || t == TOK_RBRACKET) {
                i++;
                continue;
            }
            return false;
        }
    }
    return false;
}

std::unique_ptr<ExpressionNode> Parser::parseArrowFunction()
{
    auto arrowNode = std::make_unique<ArrowFunctionNode>();

    if (peek().type == TOK_IDENTIFIER) {
        // Single untyped parameter: x => ...   (untyped params default to i32)
        arrowNode->parameters.emplace_back(advance().value, "i32");
    } else {
        consume(TOK_LPAREN, "Expected '(' in arrow function");
        while (peek().type != TOK_RPAREN && !isAtEnd()) {
            const Token &paramName = consume(TOK_IDENTIFIER, "Expected arrow function parameter name");
            std::string paramType = "i32";
            if (peek().type == TOK_COLON) {
                advance();
                paramType = parseType();
            }
            arrowNode->parameters.emplace_back(paramName.value, paramType);
            if (peek().type == TOK_COMMA) advance();
            else break;
        }
        consume(TOK_RPAREN, "Expected ')' in arrow function");
        if (peek().type == TOK_COLON) {
            advance();
            arrowNode->returnType = parseType();
        }
    }

    consume(TOK_ARROW, "Expected '=>' in arrow function");

    if (peek().type == TOK_LBRACE) {
        advance();
        while (peek().type != TOK_RBRACE && !isAtEnd()) {
            arrowNode->bodyStatements.push_back(parseStatement());
        }
        consume(TOK_RBRACE, "Expected '}' after arrow function body");
    } else {
        // Expression body: implicit return
        arrowNode->bodyStatements.push_back(
            std::make_unique<ReturnStatementNode>(parseExpression()));
    }
    return arrowNode;
}

std::unique_ptr<StringLiteralNode> Parser::parseStringLiteral() { return std::make_unique<StringLiteralNode>(consume(TOK_STRING_LITERAL, "Expected string literal").value); }

// Parses the literal forms the lexer accepts: 255, 0xFF, 0b1010, 0755.
// (std::stoll defaults to base 10, which silently truncated "0xFF" to 0.)
static long long parseIntegerLiteralValue(const std::string &text) {
    if (text.size() > 2 && text[0] == '0') {
        if (text[1] == 'x' || text[1] == 'X') return std::stoll(text.substr(2), nullptr, 16);
        if (text[1] == 'b' || text[1] == 'B') return std::stoll(text.substr(2), nullptr, 2);
    }
    if (text.size() > 1 && text[0] == '0' && text.find_first_not_of("01234567", 1) == std::string::npos) {
        return std::stoll(text.substr(1), nullptr, 8);
    }
    return std::stoll(text, nullptr, 10);
}

std::unique_ptr<IntegerLiteralNode> Parser::parseIntegerLiteral() {
    return std::make_unique<IntegerLiteralNode>(
        parseIntegerLiteralValue(consume(TOK_INT_LITERAL, "Expected integer literal").value));
}

std::unique_ptr<BooleanLiteralNode> Parser::parseBooleanLiteral() {
    bool val = (peek().type == TOK_TRUE);
    advance();
    return std::make_unique<BooleanLiteralNode>(val);
}

std::unique_ptr<ExpressionNode> Parser::parseVariableExpression()
{
    const Token &varToken = consume(TOK_IDENTIFIER, "Expected variable name.");
    
    // Generic function call: func<T>(...)
    // Only treat '<' as generic if followed by a type name, not a number/expression
    auto isTypeToken = [](TokenType t) {
        return t == TOK_IDENTIFIER || t == TOK_TYPE_STRING || t == TOK_TYPE_I32 ||
               t == TOK_TYPE_F64 || t == TOK_TYPE_BOOLEAN || t == TOK_TYPE_NUMBER;
    };
    if (peek().type == TOK_LESS && isTypeToken(peek(1).type)) {
        // Save position in case this isn't actually a generic call
        size_t savedPos = m_currentPos;
        advance(); // consume '<'
        std::vector<std::string> genericArgs;
        bool isGeneric = true;
        while (peek().type != TOK_GREATER && !isAtEnd()) {
            genericArgs.push_back(parseType());
            if (peek().type == TOK_COMMA) advance();
            else if (peek().type != TOK_GREATER) { isGeneric = false; break; }
        }
        if (isGeneric && peek().type == TOK_GREATER) {
            consume(TOK_GREATER, "Expected '>'");
            if (peek().type == TOK_LPAREN) {
                auto callNode = std::make_unique<FunctionCallNode>(varToken.value);
                consume(TOK_LPAREN, "Expected '('");
                while (peek().type != TOK_RPAREN && !isAtEnd()) {
                    callNode->arguments.push_back(parseExpression());
                    if (peek().type == TOK_COMMA) advance();
                    else break;
                }
                consume(TOK_RPAREN, "Expected ')'");
                return callNode;
            }
        }
        // Not a generic call, backtrack
        m_currentPos = savedPos;
    }

    // TypeScript compatibility: console.log/error/warn/info map to println,
    // multiple arguments are joined with spaces like in JS.
    if (varToken.value == "console" && peek().type == TOK_DOT) {
        advance();
        const Token &methodToken = consume(TOK_IDENTIFIER, "Expected console method");
        if (methodToken.value != "log" && methodToken.value != "error" &&
            methodToken.value != "warn" && methodToken.value != "info") {
            throw std::runtime_error("Parse Error: Unsupported console method 'console." + methodToken.value + "'");
        }
        consume(TOK_LPAREN, "Expected '(' after console." + methodToken.value);
        std::vector<std::unique_ptr<ExpressionNode>> args;
        while (peek().type != TOK_RPAREN && !isAtEnd()) {
            args.push_back(parseExpression());
            if (peek().type == TOK_COMMA) advance();
            else break;
        }
        consume(TOK_RPAREN, "Expected ')'");

        auto callNode = std::make_unique<FunctionCallNode>("println");
        if (args.empty()) {
            callNode->arguments.push_back(std::make_unique<StringLiteralNode>(""));
        } else if (args.size() == 1) {
            callNode->arguments.push_back(std::move(args[0]));
        } else {
            // Join arguments with spaces: a + " " + b + " " + c
            std::unique_ptr<ExpressionNode> joined = std::move(args[0]);
            for (size_t i = 1; i < args.size(); ++i) {
                joined = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::ADD,
                    std::move(joined), std::make_unique<StringLiteralNode>(" "));
                joined = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::ADD,
                    std::move(joined), std::move(args[i]));
            }
            callNode->arguments.push_back(std::move(joined));
        }
        return callNode;
    }

    // TypeScript compatibility: Math.* maps to the native math stdlib
    if (varToken.value == "Math" && peek().type == TOK_DOT) {
        advance();
        const Token &methodToken = consume(TOK_IDENTIFIER, "Expected Math method");

        // Math.PI is a constant, not a call
        if (methodToken.value == "PI") {
            return std::make_unique<FloatLiteralNode>(3.14159265358979323846);
        }
        if (methodToken.value == "E") {
            return std::make_unique<FloatLiteralNode>(2.71828182845904523536);
        }

        std::string target;
        if (methodToken.value == "sqrt") target = "math_sqrt";
        else if (methodToken.value == "pow") target = "math_pow";
        else if (methodToken.value == "abs") target = "math_abs_f64";
        else if (methodToken.value == "floor") target = "math_floor";
        else if (methodToken.value == "sin") target = "math_sin";
        else if (methodToken.value == "cos") target = "math_cos";
        else if (methodToken.value == "log") target = "math_log";
        else if (methodToken.value == "exp") target = "math_exp";
        else if (methodToken.value == "min") target = "math_min";
        else if (methodToken.value == "max") target = "math_max";
        else if (methodToken.value == "round") target = "math_round";
        else if (methodToken.value == "ceil") target = "math_ceil";
        else if (methodToken.value == "atan2") target = "math_atan2";
        else if (methodToken.value == "random") target = "math_random";
        else throw std::runtime_error("Parse Error: Unsupported Math method 'Math." + methodToken.value + "'");

        auto callNode = std::make_unique<FunctionCallNode>(target);
        consume(TOK_LPAREN, "Expected '(' after Math." + methodToken.value);
        while (peek().type != TOK_RPAREN && !isAtEnd()) {
            callNode->arguments.push_back(parseExpression());
            if (peek().type == TOK_COMMA) advance();
            else break;
        }
        consume(TOK_RPAREN, "Expected ')'");
        return callNode;
    }

    if (varToken.value == "JSON" && peek().type == TOK_DOT) {
        advance();
        const Token &methodToken = consume(TOK_IDENTIFIER, "Expected method");
        auto callNode = std::make_unique<FunctionCallNode>("JSON." + methodToken.value);
        consume(TOK_LPAREN, "Expected '('");
        if (peek().type != TOK_RPAREN) {
            do {
                callNode->arguments.push_back(parseExpression());
                if (peek().type == TOK_COMMA) advance();
            } while (peek().type != TOK_RPAREN && !isAtEnd());
        }
        consume(TOK_RPAREN, "Expected ')'");
        return callNode;
    }
    if (peek().type == TOK_LPAREN) {
        auto callNode = std::make_unique<FunctionCallNode>(varToken.value);
        callNode->line = varToken.line;
        callNode->column = varToken.column;
        consume(TOK_LPAREN, "Expected '('");
        while (peek().type != TOK_RPAREN && !isAtEnd()) {
            callNode->arguments.push_back(parseExpression());
            if (peek().type == TOK_COMMA) advance();
            else break;
        }
        consume(TOK_RPAREN, "Expected ')'");
        return callNode;
    }
 else {
        auto varNode = std::make_unique<VariableExpressionNode>(varToken.value);
        varNode->line = varToken.line;
        varNode->column = varToken.column;
        return parseArrayOrObjectAccess(std::move(varNode));
    }
}

std::string Parser::parseType() {
    // Function type: (i32, string) => i32  (parameter names allowed and ignored)
    // Canonical form: "closure(i32,string)=>i32"
    if (peek().type == TOK_LPAREN) {
        advance();
        std::vector<std::string> argTypes;
        while (peek().type != TOK_RPAREN && !isAtEnd()) {
            if (peek().type == TOK_IDENTIFIER && peek(1).type == TOK_COLON) {
                advance(); advance(); // skip "name:"
            }
            argTypes.push_back(parseType());
            if (peek().type == TOK_COMMA) advance();
            else break;
        }
        consume(TOK_RPAREN, "Expected ')' in function type");
        consume(TOK_ARROW, "Expected '=>' in function type");
        std::string retType = parseType();
        std::string typeName = "closure(";
        for (size_t i = 0; i < argTypes.size(); ++i) {
            if (i > 0) typeName += ",";
            typeName += argTypes[i];
        }
        return typeName + ")=>" + retType;
    }

    std::string typeName = advance().value;
    if (peek().type == TOK_LESS) {
        advance(); typeName += "<";
        while (peek().type != TOK_GREATER && !isAtEnd()) {
            typeName += parseType();
            if (peek().type == TOK_COMMA) { advance(); typeName += ","; }
            else if (peek().type != TOK_GREATER) break;
        }
        consume(TOK_GREATER, "Expected '>'"); typeName += ">";
    }
    while (peek().type == TOK_LBRACKET) { advance(); consume(TOK_RBRACKET, "Expected ']'"); typeName += "[]"; }
    return typeName;
}

std::unique_ptr<NewExpressionNode> Parser::parseNewExpression() {
    consume(TOK_NEW, "Expected 'new'");
    const Token &nameToken = consume(TOK_IDENTIFIER, "Expected class name");
    auto newNode = std::make_unique<NewExpressionNode>(nameToken.value);
    if (peek().type == TOK_LESS) {
        advance();
        while (peek().type != TOK_GREATER) {
            newNode->genericTypes.push_back(parseType());
            if (peek().type == TOK_COMMA) advance();
        }
        consume(TOK_GREATER, "Expected '>'");
    }
    consume(TOK_LPAREN, "Expected '('");
    if (peek().type != TOK_RPAREN) {
        do {
            newNode->arguments.push_back(parseExpression());
            if (peek().type == TOK_COMMA) advance();
        } while (peek().type != TOK_RPAREN && !isAtEnd());
    }
    consume(TOK_RPAREN, "Expected ')'");
    return newNode;
}

std::unique_ptr<ProgramNode> Parser::parse() { try { return parseProgram(); } catch (const std::runtime_error &e) { std::cerr << "Parser Error: " << e.what() << std::endl; return nullptr; } }

std::unique_ptr<ArrayLiteralNode> Parser::parseArrayLiteral()
{
    consume(TOK_LBRACKET, "Expected '['");
    if (peek().type == TOK_RBRACKET) { advance(); return std::make_unique<ArrayLiteralNode>("i32"); }
    auto firstElement = parseExpression();
    std::string elementType = "i32";
    if (dynamic_cast<StringLiteralNode*>(firstElement.get())) elementType = "string";
    else if (dynamic_cast<FloatLiteralNode*>(firstElement.get())) elementType = "f64";
    auto arrayNode = std::make_unique<ArrayLiteralNode>(elementType);
    arrayNode->elements.push_back(std::move(firstElement));
    while (peek().type == TOK_COMMA) { advance(); if (peek().type == TOK_RBRACKET) break; arrayNode->elements.push_back(parseExpression()); }
    consume(TOK_RBRACKET, "Expected ']'");
    return arrayNode;
}

std::unique_ptr<ObjectLiteralNode> Parser::parseObjectLiteral()
{
    consume(TOK_LBRACE, "Expected '{'");
    auto objectNode = std::make_unique<ObjectLiteralNode>();
    if (peek().type == TOK_RBRACE) { advance(); return objectNode; }
    do {
        std::string key;
        if (peek().type == TOK_IDENTIFIER || peek().type == TOK_STRING_LITERAL) { key = peek().value; advance(); }
        else throw std::runtime_error("Expected property name");
        if (peek().type == TOK_LPAREN) {
            // Shorthand method: greet(name: string): void { ... }
            objectNode->properties.emplace_back(key, parseFunctionRest(key));
        } else {
            consume(TOK_COLON, "Expected ':'");
            if (peek().type == TOK_FUNCTION) {
                // Method property: add: function(x: i32): i32 { ... }
                advance();
                objectNode->properties.emplace_back(key, parseFunctionRest(key));
            } else {
                objectNode->properties.emplace_back(key, parseExpression());
            }
        }
        if (peek().type == TOK_COMMA) advance();
        else break;
    } while (peek().type != TOK_RBRACE && !isAtEnd());
    consume(TOK_RBRACE, "Expected '}'");
    return objectNode;
}

std::unique_ptr<ExpressionNode> Parser::parseArrayOrObjectAccess(std::unique_ptr<ExpressionNode> base)
{
    while (peek().type == TOK_LBRACKET || peek().type == TOK_DOT) {
        if (peek().type == TOK_LBRACKET) {
            advance(); auto index = parseExpression(); consume(TOK_RBRACKET, "Expected ']'");
            base = std::make_unique<ArrayAccessNode>(std::move(base), std::move(index));
        } else {
            advance(); if (peek().type != TOK_IDENTIFIER) throw std::runtime_error("Expected property name");
            std::string property = peek().value; advance();
            if (peek().type == TOK_LPAREN) {
                auto methodCall = std::make_unique<MethodCallNode>(std::move(base), property);
                advance();
                while (peek().type != TOK_RPAREN && !isAtEnd()) {
                    methodCall->arguments.push_back(parseExpression());
                    if (peek().type == TOK_COMMA) advance();
                    else break;
                }
                consume(TOK_RPAREN, "Expected ')'");
                base = std::move(methodCall);
            } else base = std::make_unique<ObjectAccessNode>(std::move(base), property);
        }
    }
    return base;
}

std::unique_ptr<FunctionDeclarationNode> Parser::parseFunctionRest(const std::string& name)
{
    auto funcNode = std::make_unique<FunctionDeclarationNode>(name, "void");
    consume(TOK_LPAREN, "Expected '('");
    if (peek().type != TOK_RPAREN) {
        do {
            const Token &paramName = consume(TOK_IDENTIFIER, "Expected param name");
            consume(TOK_COLON, "Expected ':'");
            funcNode->parameters.emplace_back(paramName.value, parseType());
            if (peek().type == TOK_COMMA) advance();
        } while (peek().type != TOK_RPAREN);
    }
    consume(TOK_RPAREN, "Expected ')'");
    if (peek().type == TOK_COLON) { advance(); funcNode->returnType = parseType(); }
    consume(TOK_LBRACE, "Expected '{'");
    while (peek().type != TOK_RBRACE && !isAtEnd()) funcNode->bodyStatements.push_back(parseStatement());
    consume(TOK_RBRACE, "Expected '}'");
    return funcNode;
}

// --- Foreign function interface ---
// `declare` and `link` are contextual: they are only keywords in these exact
// shapes, so a program that already uses either as an identifier keeps working.

bool Parser::isExternDeclarationAhead() const
{
    return peek().type == TOK_IDENTIFIER && peek().value == "declare" &&
           peek(1).type == TOK_FUNCTION;
}

bool Parser::isLinkDirectiveAhead() const
{
    if (peek().type != TOK_IDENTIFIER || peek().value != "link") return false;
    // link "raylib";
    if (peek(1).type == TOK_STRING_LITERAL) return true;
    if (peek(1).type != TOK_IDENTIFIER) return false;

    auto isQualifier = [](const std::string &word) {
        return word == "framework" || word == "path" || word == "source" ||
               word == "include" ||
               word == "macos" || word == "linux" || word == "windows";
    };
    if (!isQualifier(peek(1).value)) return false;

    // link framework "Cocoa";  /  link macos "raylib";
    if (peek(2).type == TOK_STRING_LITERAL) return true;
    // link macos framework "Cocoa";
    return peek(2).type == TOK_IDENTIFIER && isQualifier(peek(2).value) &&
           peek(3).type == TOK_STRING_LITERAL;
}

// declare function rl_rect(x: f64, y: f64, w: f64, h: f64, color: i32): void;
std::unique_ptr<StatementNode> Parser::parseExternDeclaration()
{
    advance(); // "declare"
    consume(TOK_FUNCTION, "Expected 'function' after 'declare'");
    const Token &nameToken = consume(TOK_IDENTIFIER, "Expected function name in declare");

    auto node = std::make_unique<ExternDeclarationNode>(nameToken.value, "void");
    consume(TOK_LPAREN, "Expected '(' in declare");
    if (peek().type != TOK_RPAREN) {
        do {
            const Token &paramName = consume(TOK_IDENTIFIER, "Expected parameter name in declare");
            consume(TOK_COLON, "Expected ':' — declared parameters need explicit types");
            node->parameters.emplace_back(paramName.value, parseType());
            if (peek().type == TOK_COMMA) advance();
        } while (peek().type != TOK_RPAREN && !isAtEnd());
    }
    consume(TOK_RPAREN, "Expected ')' in declare");
    if (peek().type == TOK_COLON) { advance(); node->returnType = parseType(); }

    // Optional symbol binding: declare function drawRect(...): void = "cyps_rect";
    // Lets the Cypescript-facing name differ from the C symbol it calls.
    if (peek().type == TOK_EQUAL) {
        advance();
        node->symbolName = consume(TOK_STRING_LITERAL,
            "Expected a C symbol name in quotes after '=' in declare").value;
    }

    consume(TOK_SEMICOLON, "Expected ';' after declare (foreign functions have no body)");
    return node;
}

// link "raylib";              link framework "Cocoa";     link path "/opt/homebrew/lib";
// link linux "GL";            link macos framework "IOKit";
// link source "mylib.c";      link linux source "posix_bits.c";
std::unique_ptr<StatementNode> Parser::parseLinkDirective()
{
    advance(); // "link"

    LinkDirectiveNode::Kind kind = LinkDirectiveNode::Kind::Library;
    LinkDirectiveNode::Platform platform = LinkDirectiveNode::Platform::Any;

    // Qualifiers may appear in either order, and either may be omitted
    while (peek().type == TOK_IDENTIFIER) {
        const std::string &word = peek().value;
        if (word == "framework")      kind = LinkDirectiveNode::Kind::Framework;
        else if (word == "path")      kind = LinkDirectiveNode::Kind::SearchPath;
        else if (word == "source")    kind = LinkDirectiveNode::Kind::Source;
        else if (word == "include")   kind = LinkDirectiveNode::Kind::IncludePath;
        else if (word == "macos")     platform = LinkDirectiveNode::Platform::MacOS;
        else if (word == "linux")     platform = LinkDirectiveNode::Platform::Linux;
        else if (word == "windows")   platform = LinkDirectiveNode::Platform::Windows;
        else break;
        advance();
    }

    const Token &valueToken = consume(TOK_STRING_LITERAL, "Expected a string after 'link'");
    consume(TOK_SEMICOLON, "Expected ';' after link directive");
    return std::make_unique<LinkDirectiveNode>(kind, valueToken.value, platform);
}

std::unique_ptr<FunctionDeclarationNode> Parser::parseFunctionDeclaration()
{
    consume(TOK_FUNCTION, "Expected 'function'");
    const Token &nameToken = consume(TOK_IDENTIFIER, "Expected name");
    std::vector<std::string> genericParams;
    if (peek().type == TOK_LESS) {
        advance();
        while (peek().type != TOK_GREATER) {
            genericParams.push_back(consume(TOK_IDENTIFIER, "Expected generic parameter name").value);
            if (peek().type == TOK_COMMA) advance();
        }
        consume(TOK_GREATER, "Expected '>'");
    }
    auto funcNode = parseFunctionRest(nameToken.value);
    funcNode->genericParams = std::move(genericParams);
    return funcNode;
}

std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement()
{
    consume(TOK_RETURN, "Expected 'return'");
    std::unique_ptr<ExpressionNode> expression = nullptr;
    if (peek().type != TOK_SEMICOLON) expression = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';'");
    return std::make_unique<ReturnStatementNode>(std::move(expression));
}
