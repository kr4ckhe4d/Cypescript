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

const Token &Parser::consume(TokenType expectedType, const std::string &errorMessage)
{
    if (peek().type == expectedType)
    {
        return advance();
    }
    std::string errorMsg = "Parse Error: " + errorMessage + ". Found " +
                           tokenTypeToString(peek().type) + " ('" + peek().value + "') instead.";
    std::cerr << errorMsg << std::endl;
    // Ideally, add line/column info here from the token
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
    if (peek().type == TOK_LET || peek().type == TOK_CONST)
    {
        return parseVariableDeclarationStatement();
    }
    else if (peek().type == TOK_FUNCTION)
    {
        return parseFunctionDeclaration();
    }
    else if (peek().type == TOK_TYPE)
    {
        return parseTypeAliasStatement();
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
    else if (peek().type == TOK_IDENTIFIER)
    {
        // Look ahead to see what kind of statement this is
        if (peek(1).type == TOK_LPAREN) {
            // Function call as expression statement
            auto expr = parseExpression(); // This will parse the function call
            consume(TOK_SEMICOLON, "Expected ';' after function call");
            return std::make_unique<ExpressionStatementNode>(std::move(expr));
        }
        else if (peek(1).type == TOK_EQUAL) {
            // Simple variable assignment: identifier = expression
            return parseAssignmentStatement();
        }
        else if (peek(1).type == TOK_LBRACKET) {
            // Could be array assignment: identifier[index] = value
            size_t pos = 2;
            int bracketDepth = 1;
            while (pos < m_tokens.size() && bracketDepth > 0) {
                if (m_tokens[m_currentPos + pos].type == TOK_LBRACKET) bracketDepth++;
                else if (m_tokens[m_currentPos + pos].type == TOK_RBRACKET) bracketDepth--;
                pos++;
            }
            if (pos < m_tokens.size() && m_tokens[m_currentPos + pos].type == TOK_EQUAL) {
                return parseArrayAssignmentStatement();
            } else {
                auto expr = parseExpression();
                consume(TOK_SEMICOLON, "Expected ';' after array access statement");
                return std::make_unique<ExpressionStatementNode>(std::move(expr));
            }
        }
        else if (peek(1).type == TOK_DOT) {
            auto expr = parseExpression();
            consume(TOK_SEMICOLON, "Expected ';' after method call");
            return std::make_unique<ExpressionStatementNode>(std::move(expr));
        }
        else {
            std::string errorMsg = std::string("Unexpected token at start of statement: ") + 
                                 tokenTypeToString(peek().type) + " ('" + peek().value + "')";
            throw std::runtime_error(errorMsg);
        }
    }
    else
    {
        std::string errorMsg = std::string("Parsing failed: Unexpected token at start of statement: ") +
                               tokenTypeToString(peek().type) + " ('" + peek().value + "')";
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
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
        consume(TOK_LBRACE, "Expected '{'");
        while (peek().type != TOK_RBRACE && !isAtEnd()) {
            ifNode->elseStatements.push_back(parseStatement());
        }
        consume(TOK_RBRACE, "Expected '}'");
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
    if ((peek().type == TOK_LET || peek().type == TOK_CONST) && peek(2).type == TOK_OF) {
        bool isConst = (peek().type == TOK_CONST);
        advance();
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
    if (peek().type == TOK_LET || peek().type == TOK_CONST) initialization = parseVariableDeclarationStatement();
    else if (peek().type == TOK_IDENTIFIER && peek(1).type == TOK_EQUAL) initialization = parseAssignmentStatement();
    else if (peek().type != TOK_SEMICOLON) throw std::runtime_error("Expected init in for loop");
    if (!initialization) consume(TOK_SEMICOLON, "Expected ';'");
    std::unique_ptr<ExpressionNode> condition = nullptr;
    if (peek().type != TOK_SEMICOLON) condition = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';'");
    std::unique_ptr<StatementNode> increment = nullptr;
    if (peek().type != TOK_RPAREN) {
        if (peek().type == TOK_IDENTIFIER && peek(1).type == TOK_EQUAL) {
            const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name");
            consume(TOK_EQUAL, "Expected '='");
            increment = std::make_unique<AssignmentStatementNode>(varNameToken.value, parseExpression());
        } else throw std::runtime_error("Expected assignment in for increment");
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
    auto expr = parseComparisonExpression();
    while (peek().type == TOK_AND) {
        advance();
        expr = std::make_unique<BinaryExpressionNode>(BinaryExpressionNode::LOGICAL_AND, std::move(expr), parseComparisonExpression());
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseComparisonExpression()
{
    auto expr = parseAdditionExpression();
    while (peek().type == TOK_EQUAL_EQUAL || peek().type == TOK_NOT_EQUAL ||
           peek().type == TOK_LESS || peek().type == TOK_LESS_EQUAL ||
           peek().type == TOK_GREATER || peek().type == TOK_GREATER_EQUAL) {
        TokenType operatorType = peek().type;
        advance();
        auto right = parseAdditionExpression();
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
    if (peek().type == TOK_BANG || peek().type == TOK_MINUS) {
        TokenType operatorType = peek().type;
        advance();
        return std::make_unique<UnaryExpressionNode>(operatorType == TOK_BANG ? UnaryExpressionNode::NOT : UnaryExpressionNode::MINUS, parseUnaryExpression());
    }
    return parsePrimaryExpression();
}

std::unique_ptr<ExpressionNode> Parser::parsePrimaryExpression()
{
    std::unique_ptr<ExpressionNode> expr = nullptr;
    if (peek().type == TOK_STRING_LITERAL) expr = parseStringLiteral();
    else if (peek().type == TOK_INT_LITERAL) expr = parseIntegerLiteral();
    else if (peek().type == TOK_TRUE || peek().type == TOK_FALSE) expr = parseBooleanLiteral();
    else if (peek().type == TOK_IDENTIFIER) expr = parseVariableExpression();
    else if (peek().type == TOK_LPAREN) {
        advance();
        expr = parseExpression();
        consume(TOK_RPAREN, "Expected ')'");
    }
    else if (peek().type == TOK_NEW) expr = parseNewExpression();
    else if (peek().type == TOK_LBRACKET) expr = parseArrayLiteral();
    else if (peek().type == TOK_LBRACE) expr = parseObjectLiteral();
    else throw std::runtime_error("Parsing failed: Expected an expression");
    while (peek().type == TOK_BANG) advance();
    return expr;
}

std::unique_ptr<StringLiteralNode> Parser::parseStringLiteral() { return std::make_unique<StringLiteralNode>(consume(TOK_STRING_LITERAL, "Expected string literal").value); }

std::unique_ptr<IntegerLiteralNode> Parser::parseIntegerLiteral() { return std::make_unique<IntegerLiteralNode>(std::stoll(consume(TOK_INT_LITERAL, "Expected integer literal").value)); }

std::unique_ptr<BooleanLiteralNode> Parser::parseBooleanLiteral() {
    bool val = (peek().type == TOK_TRUE);
    advance();
    return std::make_unique<BooleanLiteralNode>(val);
}

std::unique_ptr<ExpressionNode> Parser::parseVariableExpression()
{
    const Token &varToken = consume(TOK_IDENTIFIER, "Expected variable name.");
    
    // Generic function call: func<T>(...)
    if (peek().type == TOK_LESS) {
        advance(); // consume '<'
        std::vector<std::string> genericArgs;
        while (peek().type != TOK_GREATER) {
            genericArgs.push_back(parseType());
            if (peek().type == TOK_COMMA) advance();
        }
        consume(TOK_GREATER, "Expected '>'");
        
        if (peek().type == TOK_LPAREN) {
            auto callNode = std::make_unique<FunctionCallNode>(varToken.value);
            // We could store genericArgs in FunctionCallNode if needed
            consume(TOK_LPAREN, "Expected '('");
            while (peek().type != TOK_RPAREN) {
                callNode->arguments.push_back(parseExpression());
                if (peek().type == TOK_COMMA) advance();
            }
            consume(TOK_RPAREN, "Expected ')'");
            return callNode;
        }
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
        m_currentPos--;
        const Token &funcNameToken = consume(TOK_IDENTIFIER, "Expected function name");
        auto callNode = std::make_unique<FunctionCallNode>(funcNameToken.value);
        consume(TOK_LPAREN, "Expected '('");
        while (peek().type != TOK_RPAREN) {
            callNode->arguments.push_back(parseExpression());
            if (peek().type == TOK_COMMA) advance();
        }
        consume(TOK_RPAREN, "Expected ')'");
        return callNode;
    } else {
        return parseArrayOrObjectAccess(std::make_unique<VariableExpressionNode>(varToken.value));
    }
}

std::string Parser::parseType() {
    std::string typeName = advance().value;
    if (peek().type == TOK_LESS) {
        advance(); typeName += "<";
        while (peek().type != TOK_GREATER) {
            typeName += parseType();
            if (peek().type == TOK_COMMA) { advance(); typeName += ","; }
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
        consume(TOK_COLON, "Expected ':'");
        objectNode->properties.emplace_back(key, parseExpression());
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
                while (peek().type != TOK_RPAREN) {
                    methodCall->arguments.push_back(parseExpression());
                    if (peek().type == TOK_COMMA) advance();
                }
                consume(TOK_RPAREN, "Expected ')'");
                base = std::move(methodCall);
            } else base = std::make_unique<ObjectAccessNode>(std::move(base), property);
        }
    }
    return base;
}

std::unique_ptr<FunctionDeclarationNode> Parser::parseFunctionDeclaration()
{
    consume(TOK_FUNCTION, "Expected 'function'");
    const Token &nameToken = consume(TOK_IDENTIFIER, "Expected name");
    auto funcNode = std::make_unique<FunctionDeclarationNode>(nameToken.value, "void");
    if (peek().type == TOK_LESS) {
        advance();
        while (peek().type != TOK_GREATER) {
            funcNode->genericParams.push_back(consume(TOK_IDENTIFIER, "Expected generic parameter name").value);
            if (peek().type == TOK_COMMA) advance();
        }
        consume(TOK_GREATER, "Expected '>'");
    }
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

std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement()
{
    consume(TOK_RETURN, "Expected 'return'");
    std::unique_ptr<ExpressionNode> expression = nullptr;
    if (peek().type != TOK_SEMICOLON) expression = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';'");
    return std::make_unique<ReturnStatementNode>(std::move(expression));
}
