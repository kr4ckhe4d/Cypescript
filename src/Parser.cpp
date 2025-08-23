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
    if (peek().type == TOK_LET)
    {
        return parseVariableDeclarationStatement();
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
    else if (peek().type == TOK_IDENTIFIER && isKnownFunction(peek().value))
    { // Handle function calls as expression statements
        auto expr = parseExpression(); // This will parse the function call
        consume(TOK_SEMICOLON, "Expected ';' after function call");
        return std::make_unique<ExpressionStatementNode>(std::move(expr));
    }
    else if (peek().type == TOK_IDENTIFIER)
    {
        // Look ahead to see what kind of statement this is
        if (peek(1).type == TOK_EQUAL) {
            // Simple variable assignment: identifier = expression
            return parseAssignmentStatement();
        }
        else if (peek(1).type == TOK_LBRACKET) {
            // Could be array assignment: identifier[index] = value
            // Look further ahead to check for assignment pattern
            size_t pos = 2;
            int bracketDepth = 1;
            
            // Skip through the array index expression to find the closing bracket
            while (pos < m_tokens.size() && bracketDepth > 0) {
                if (m_tokens[m_currentPos + pos].type == TOK_LBRACKET) {
                    bracketDepth++;
                } else if (m_tokens[m_currentPos + pos].type == TOK_RBRACKET) {
                    bracketDepth--;
                }
                pos++;
            }
            
            // Check if we have an assignment after the closing bracket
            if (pos < m_tokens.size() && m_tokens[m_currentPos + pos].type == TOK_EQUAL) {
                return parseArrayAssignmentStatement();
            } else {
                // Not an assignment, this is an unexpected statement
                std::string errorMsg = std::string("Unexpected token at start of statement: ") + 
                                     tokenTypeToString(peek().type) + " ('" + peek().value + "')";
                throw std::runtime_error(errorMsg);
            }
        }
        else {
            // Could be other expression statements in the future
            std::string errorMsg = std::string("Unexpected token at start of statement: ") + 
                                 tokenTypeToString(peek().type) + " ('" + peek().value + "')";
            throw std::runtime_error(errorMsg);
        }
    }
    // Add other statement types here (while, return, expression statements etc.)
    else
    {
        // Corrected string concatenation
        std::string errorMsg = std::string("Parsing failed: Unexpected token at start of statement: ") +
                               tokenTypeToString(peek().type) + " ('" + peek().value + "')";
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
}

std::unique_ptr<VariableDeclarationNode> Parser::parseVariableDeclarationStatement()
{
    consume(TOK_LET, "Expected 'let' keyword");

    const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name after 'let'");
    std::string varName = varNameToken.value;

    std::string typeName;
    
    // Check if there's a type annotation
    if (peek().type == TOK_COLON) {
        consume(TOK_COLON, "Expected ':' after variable name for type annotation");

        // Handle both old identifier-based types and new specific type tokens
        TokenType currentType = peek().type;
        
        if (currentType == TOK_TYPE_STRING || currentType == TOK_TYPE_I32 || 
            currentType == TOK_TYPE_F64 || currentType == TOK_TYPE_BOOLEAN ||
            currentType == TOK_TYPE_NUMBER) {
            // New specific type tokens
            const Token &typeToken = advance();
            typeName = typeToken.value;
        } else if (currentType == TOK_IDENTIFIER) {
            // Legacy identifier-based types (for backward compatibility)
            const Token &typeNameToken = consume(TOK_IDENTIFIER, "Expected type name after ':'");
            typeName = typeNameToken.value;
        } else {
            std::string errorMsg = "Expected type name after ':'. Found " + 
                                  std::string(tokenTypeToString(peek().type)) + " ('" + peek().value + "') instead.";
            std::cerr << errorMsg << std::endl;
            throw std::runtime_error(errorMsg);
        }
        
        // Check for array type syntax: i32[]
        if (peek().type == TOK_LBRACKET) {
            advance(); // consume '['
            consume(TOK_RBRACKET, "Expected ']' after '[' in array type");
            typeName += "[]"; // Mark as array type
        }
    } else {
        // No explicit type annotation - infer from initializer
        typeName = "auto"; // We'll infer the type from the initializer
    }

    consume(TOK_EQUAL, "Expected '=' for variable initialization");

    std::unique_ptr<ExpressionNode> initializer = parseExpression();

    consume(TOK_SEMICOLON, "Expected ';' after variable declaration statement");

    return std::make_unique<VariableDeclarationNode>(varName, typeName, std::move(initializer));
}

std::unique_ptr<IfStatementNode> Parser::parseIfStatement()
{
    consume(TOK_IF, "Expected 'if' keyword");
    consume(TOK_LPAREN, "Expected '(' after 'if'");
    
    std::unique_ptr<ExpressionNode> condition = parseExpression();
    
    consume(TOK_RPAREN, "Expected ')' after if condition");
    consume(TOK_LBRACE, "Expected '{' to start if body");
    
    auto ifNode = std::make_unique<IfStatementNode>(std::move(condition));
    
    // Parse then statements
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        ifNode->thenStatements.push_back(parseStatement());
    }
    
    consume(TOK_RBRACE, "Expected '}' to end if body");
    
    // Check for optional else clause
    if (peek().type == TOK_ELSE) {
        advance(); // consume 'else'
        consume(TOK_LBRACE, "Expected '{' to start else body");
        
        // Parse else statements
        while (peek().type != TOK_RBRACE && !isAtEnd()) {
            ifNode->elseStatements.push_back(parseStatement());
        }
        
        consume(TOK_RBRACE, "Expected '}' to end else body");
    }
    
    return ifNode;
}

std::unique_ptr<WhileStatementNode> Parser::parseWhileStatement()
{
    consume(TOK_WHILE, "Expected 'while' keyword");
    consume(TOK_LPAREN, "Expected '(' after 'while'");
    
    std::unique_ptr<ExpressionNode> condition = parseExpression();
    
    consume(TOK_RPAREN, "Expected ')' after while condition");
    consume(TOK_LBRACE, "Expected '{' to start while body");
    
    auto whileNode = std::make_unique<WhileStatementNode>(std::move(condition));
    
    // Parse body statements
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        whileNode->bodyStatements.push_back(parseStatement());
    }
    
    consume(TOK_RBRACE, "Expected '}' to end while body");
    
    return whileNode;
}

std::unique_ptr<StatementNode> Parser::parseAssignmentStatement()
{
    const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name");
    consume(TOK_EQUAL, "Expected '=' in assignment");
    
    std::unique_ptr<ExpressionNode> value = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';' after assignment");
    
    return std::make_unique<AssignmentStatementNode>(varNameToken.value, std::move(value));
}

std::unique_ptr<StatementNode> Parser::parseArrayAssignmentStatement()
{
    // Parse array[index] = value;
    
    // Parse the array identifier
    const Token &arrayNameToken = consume(TOK_IDENTIFIER, "Expected array name");
    std::unique_ptr<ExpressionNode> arrayExpr = std::make_unique<VariableExpressionNode>(arrayNameToken.value);
    
    // Parse [index]
    consume(TOK_LBRACKET, "Expected '[' for array access");
    std::unique_ptr<ExpressionNode> indexExpr = parseExpression();
    consume(TOK_RBRACKET, "Expected ']' after array index");
    
    // Parse = value
    consume(TOK_EQUAL, "Expected '=' in array assignment");
    std::unique_ptr<ExpressionNode> valueExpr = parseExpression();
    consume(TOK_SEMICOLON, "Expected ';' after array assignment");
    
    return std::make_unique<ArrayAssignmentStatementNode>(std::move(arrayExpr), std::move(indexExpr), std::move(valueExpr));
}

bool Parser::isKnownFunction(const std::string& name)
{
    // Built-in functions
    if (name == "print" || name == "println") {
        return true;
    }
    
    // External C++ functions - Math
    if (name == "math_sqrt" || name == "math_pow" || name == "math_abs_f64" || name == "math_abs_i32" ||
        name == "math_sin" || name == "math_cos" || name == "math_tan" || 
        name == "math_log" || name == "math_exp") {
        return true;
    }
    
    // Custom C++ functions - Advanced Math
    if (name == "math_gcd" || name == "math_lcm" || name == "math_is_prime" ||
        name == "math_fibonacci" || name == "math_factorial") {
        return true;
    }
    
    // Custom C++ functions - Statistics
    if (name == "stats_mean" || name == "stats_median" || name == "stats_stddev") {
        return true;
    }
    
    // Custom C++ functions - Geometry
    if (name == "geom_distance" || name == "geom_circle_area" || 
        name == "geom_rectangle_area" || name == "geom_triangle_area") {
        return true;
    }
    
    // External C++ functions - String
    if (name == "string_reverse" || name == "string_upper" || name == "string_lower" ||
        name == "string_length" || name == "string_substring" || name == "string_find" ||
        name == "string_concat") {
        return true;
    }
    
    // External C++ functions - Array
    if (name == "array_sum_i32" || name == "array_max_i32" || name == "array_min_i32" ||
        name == "array_sort_i32" || name == "array_reverse_i32") {
        return true;
    }
    
    // External C++ functions - File I/O
    if (name == "file_read" || name == "file_write" || name == "file_exists") {
        return true;
    }
    
    // External C++ functions - Utility
    if (name == "sleep_ms" || name == "random_int" || name == "random_double" || 
        name == "random_seed" || name == "free_string") {
        return true;
    }
    
    return false;
}

std::unique_ptr<ForStatementNode> Parser::parseForStatement()
{
    consume(TOK_FOR, "Expected 'for' keyword");
    consume(TOK_LPAREN, "Expected '(' after 'for'");
    
    // Parse initialization (can be variable declaration or assignment)
    std::unique_ptr<StatementNode> initialization = nullptr;
    if (peek().type == TOK_LET) {
        initialization = parseVariableDeclarationStatement();
    } else if (peek().type == TOK_IDENTIFIER && peek(1).type == TOK_EQUAL) {
        initialization = parseAssignmentStatement();
    } else if (peek().type != TOK_SEMICOLON) {
        throw std::runtime_error("Expected variable declaration or assignment in for loop initialization");
    }
    
    // If no initialization, consume the semicolon
    if (!initialization) {
        consume(TOK_SEMICOLON, "Expected ';' after for loop initialization");
    }
    
    // Parse condition
    std::unique_ptr<ExpressionNode> condition = nullptr;
    if (peek().type != TOK_SEMICOLON) {
        condition = parseExpression();
    }
    consume(TOK_SEMICOLON, "Expected ';' after for loop condition");
    
    // Parse increment
    std::unique_ptr<StatementNode> increment = nullptr;
    if (peek().type != TOK_RPAREN) {
        if (peek().type == TOK_IDENTIFIER && peek(1).type == TOK_EQUAL) {
            // Parse assignment without semicolon for increment
            const Token &varNameToken = consume(TOK_IDENTIFIER, "Expected variable name");
            consume(TOK_EQUAL, "Expected '=' in assignment");
            std::unique_ptr<ExpressionNode> value = parseExpression();
            increment = std::make_unique<AssignmentStatementNode>(varNameToken.value, std::move(value));
        } else {
            throw std::runtime_error("Expected assignment in for loop increment");
        }
    }
    
    consume(TOK_RPAREN, "Expected ')' after for loop header");
    consume(TOK_LBRACE, "Expected '{' to start for loop body");
    
    auto forNode = std::make_unique<ForStatementNode>(std::move(initialization), std::move(condition), std::move(increment));
    
    // Parse body statements
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        forNode->bodyStatements.push_back(parseStatement());
    }
    
    consume(TOK_RBRACE, "Expected '}' to end for loop body");
    
    return forNode;
}

std::unique_ptr<DoWhileStatementNode> Parser::parseDoWhileStatement()
{
    consume(TOK_DO, "Expected 'do' keyword");
    consume(TOK_LBRACE, "Expected '{' to start do-while body");
    
    auto doWhileNode = std::make_unique<DoWhileStatementNode>(nullptr);
    
    // Parse body statements
    while (peek().type != TOK_RBRACE && !isAtEnd()) {
        doWhileNode->bodyStatements.push_back(parseStatement());
    }
    
    consume(TOK_RBRACE, "Expected '}' to end do-while body");
    consume(TOK_WHILE, "Expected 'while' after do-while body");
    consume(TOK_LPAREN, "Expected '(' after 'while'");
    
    std::unique_ptr<ExpressionNode> condition = parseExpression();
    doWhileNode->condition = std::move(condition);
    
    consume(TOK_RPAREN, "Expected ')' after do-while condition");
    consume(TOK_SEMICOLON, "Expected ';' after do-while statement");
    
    return doWhileNode;
}

std::unique_ptr<FunctionCallNode> Parser::parseFunctionCallStatement()
{
    const Token &funcNameToken = consume(TOK_IDENTIFIER, "Expected function name"); // e.g. "print"
    auto callNode = std::make_unique<FunctionCallNode>(funcNameToken.value);

    consume(TOK_LPAREN, "Expected '(' after function name");

    if (peek().type != TOK_RPAREN)
    { // If there are arguments
        // For now, only one argument
        callNode->arguments.push_back(parseExpression());
        // Later, handle multiple comma-separated arguments:
        // while (peek().type != TOK_RPAREN) {
        //     callNode->arguments.push_back(parseExpression());
        //     if (!match(TOK_COMMA)) break;
        // }
    }

    consume(TOK_RPAREN, "Expected ')' after function arguments");
    consume(TOK_SEMICOLON, "Expected ';' after function call statement");

    return callNode;
}

// Expression Parsing with operator precedence
// Grammar:
// expression -> comparison
// comparison -> addition ( ( "==" | "!=" | "<" | "<=" | ">" | ">=" ) addition )*
// addition -> multiplication ( ( "+" | "-" ) multiplication )*
// multiplication -> primary ( ( "*" | "/" | "%" ) primary )*
// primary -> NUMBER | STRING | IDENTIFIER | "(" expression ")"

std::unique_ptr<ExpressionNode> Parser::parseExpression()
{
    return parseComparisonExpression();
}

std::unique_ptr<ExpressionNode> Parser::parseComparisonExpression()
{
    std::unique_ptr<ExpressionNode> expr = parseAdditionExpression();
    
    while (peek().type == TOK_EQUAL_EQUAL || peek().type == TOK_NOT_EQUAL ||
           peek().type == TOK_LESS || peek().type == TOK_LESS_EQUAL ||
           peek().type == TOK_GREATER || peek().type == TOK_GREATER_EQUAL) {
        
        TokenType operatorType = peek().type;
        advance(); // consume operator
        
        std::unique_ptr<ExpressionNode> right = parseAdditionExpression();
        
        BinaryExpressionNode::Operator op;
        switch (operatorType) {
            case TOK_EQUAL_EQUAL: op = BinaryExpressionNode::EQUAL; break;
            case TOK_NOT_EQUAL: op = BinaryExpressionNode::NOT_EQUAL; break;
            case TOK_LESS: op = BinaryExpressionNode::LESS_THAN; break;
            case TOK_LESS_EQUAL: op = BinaryExpressionNode::LESS_EQUAL; break;
            case TOK_GREATER: op = BinaryExpressionNode::GREATER_THAN; break;
            case TOK_GREATER_EQUAL: op = BinaryExpressionNode::GREATER_EQUAL; break;
            default:
                throw std::runtime_error("Unknown comparison operator");
        }
        
        expr = std::make_unique<BinaryExpressionNode>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseAdditionExpression()
{
    std::unique_ptr<ExpressionNode> expr = parseMultiplicationExpression();
    
    while (peek().type == TOK_PLUS || peek().type == TOK_MINUS) {
        TokenType operatorType = peek().type;
        advance(); // consume operator
        
        std::unique_ptr<ExpressionNode> right = parseMultiplicationExpression();
        
        BinaryExpressionNode::Operator op;
        if (operatorType == TOK_PLUS) {
            op = BinaryExpressionNode::ADD;
        } else {
            op = BinaryExpressionNode::SUBTRACT;
        }
        
        expr = std::make_unique<BinaryExpressionNode>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicationExpression()
{
    std::unique_ptr<ExpressionNode> expr = parsePrimaryExpression();
    
    while (peek().type == TOK_STAR || peek().type == TOK_SLASH || peek().type == TOK_PERCENT) {
        TokenType operatorType = peek().type;
        advance(); // consume operator
        
        std::unique_ptr<ExpressionNode> right = parsePrimaryExpression();
        
        BinaryExpressionNode::Operator op;
        if (operatorType == TOK_STAR) {
            op = BinaryExpressionNode::MULTIPLY;
        } else if (operatorType == TOK_SLASH) {
            op = BinaryExpressionNode::DIVIDE;
        } else {
            op = BinaryExpressionNode::MODULO;
        }
        
        expr = std::make_unique<BinaryExpressionNode>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parsePrimaryExpression()
{
    if (peek().type == TOK_STRING_LITERAL)
    {
        return parseStringLiteral();
    }
    else if (peek().type == TOK_INT_LITERAL)
    {
        return parseIntegerLiteral();
    }
    else if (peek().type == TOK_IDENTIFIER)
    {
        return parseVariableExpression();
    }
    else if (peek().type == TOK_LPAREN)
    {
        // Parenthesized expression
        advance(); // consume '('
        std::unique_ptr<ExpressionNode> expr = parseExpression();
        consume(TOK_RPAREN, "Expected ')' after expression");
        return expr;
    }
    else if (peek().type == TOK_LBRACKET)
    {
        // Array literal [1, 2, 3]
        return parseArrayLiteral();
    }
    else if (peek().type == TOK_LBRACE)
    {
        // Object literal { key: value }
        return parseObjectLiteral();
    }
    // Add other primary expressions: ( expression ), function calls if they are expressions, etc.
    else
    {
        // Corrected string concatenation
        std::string errorMsg = std::string("Parsing failed: Expected an expression (literal or identifier), found ") +
                               tokenTypeToString(peek().type) + " ('" + peek().value + "')";
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
}

std::unique_ptr<StringLiteralNode> Parser::parseStringLiteral()
{
    const Token &stringToken = consume(TOK_STRING_LITERAL, "Expected string literal.");
    return std::make_unique<StringLiteralNode>(stringToken.value);
}

std::unique_ptr<IntegerLiteralNode> Parser::parseIntegerLiteral()
{
    const Token &intToken = consume(TOK_INT_LITERAL, "Expected integer literal.");
    try
    {
        long long val = std::stoll(intToken.value); // string to long long
        return std::make_unique<IntegerLiteralNode>(val);
    }
    catch (const std::out_of_range &oor)
    {
        std::string errorMsg = "Parse Error: Integer literal out of range: " + intToken.value;
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
    catch (const std::invalid_argument &ia)
    {
        std::string errorMsg = "Parse Error: Invalid integer literal: " + intToken.value;
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
}

std::unique_ptr<ExpressionNode> Parser::parseVariableExpression()
{
    const Token &varToken = consume(TOK_IDENTIFIER, "Expected variable name.");
    
    // Check if this is a function call
    if (peek().type == TOK_LPAREN && isKnownFunction(varToken.value)) {
        // This is a function call in an expression context
        // Backtrack and parse as function call
        m_currentPos--; // Go back to the identifier
        
        const Token &funcNameToken = consume(TOK_IDENTIFIER, "Expected function name");
        auto callNode = std::make_unique<FunctionCallNode>(funcNameToken.value);
        
        consume(TOK_LPAREN, "Expected '(' after function name");
        
        // Parse arguments
        while (peek().type != TOK_RPAREN) {
            callNode->arguments.push_back(parseExpression());
            
            if (peek().type == TOK_COMMA) {
                advance(); // consume ','
            } else if (peek().type != TOK_RPAREN) {
                throw std::runtime_error("Expected ',' or ')' in function call");
            }
        }
        
        consume(TOK_RPAREN, "Expected ')' after function arguments");
        return callNode;
    } else {
        // This is a regular variable
        auto baseExpr = std::make_unique<VariableExpressionNode>(varToken.value);
        
        // Check for array access [index] or object access .property
        return parseArrayOrObjectAccess(std::move(baseExpr));
    }
}

// --- Public Main Parsing Method ---
std::unique_ptr<ProgramNode> Parser::parse()
{
    try
    {
        return parseProgram();
    }
    catch (const std::runtime_error &e)
    {
        // Error already printed by consume() or other parse methods
        // std::cerr << "Caught parsing exception: " << e.what() << std::endl;
        return nullptr; // Indicate failure
    }
}

// Array literal parsing: [1, 2, 3]
std::unique_ptr<ArrayLiteralNode> Parser::parseArrayLiteral()
{
    consume(TOK_LBRACKET, "Expected '[' to start array literal");
    
    // Handle empty array []
    if (peek().type == TOK_RBRACKET) {
        advance(); // consume ']'
        // Default to i32 for empty arrays
        return std::make_unique<ArrayLiteralNode>("i32");
    }
    
    // Parse first element to infer type
    auto firstElement = parseExpression();
    std::string elementType = "i32"; // Default
    
    // Try to infer type from first element
    if (auto *strLit = dynamic_cast<StringLiteralNode*>(firstElement.get())) {
        elementType = "string";
    } else if (auto *intLit = dynamic_cast<IntegerLiteralNode*>(firstElement.get())) {
        elementType = "i32";
    }
    // Add more type inference as needed
    
    auto arrayNode = std::make_unique<ArrayLiteralNode>(elementType);
    arrayNode->elements.push_back(std::move(firstElement));
    
    // Parse remaining elements
    while (peek().type == TOK_COMMA) {
        advance(); // consume ','
        
        if (peek().type == TOK_RBRACKET) {
            // Trailing comma is allowed
            break;
        }
        
        arrayNode->elements.push_back(parseExpression());
    }
    
    consume(TOK_RBRACKET, "Expected ']' to close array literal");
    return arrayNode;
}

// Object literal parsing: { name: "Alice", age: 25 }
std::unique_ptr<ObjectLiteralNode> Parser::parseObjectLiteral()
{
    consume(TOK_LBRACE, "Expected '{' to start object literal");
    
    auto objectNode = std::make_unique<ObjectLiteralNode>();
    
    // Handle empty object {}
    if (peek().type == TOK_RBRACE) {
        advance(); // consume '}'
        return objectNode;
    }
    
    // Parse object properties
    do {
        // Parse property key (must be identifier or string)
        std::string key;
        if (peek().type == TOK_IDENTIFIER) {
            key = peek().value;
            advance();
        } else if (peek().type == TOK_STRING_LITERAL) {
            key = peek().value;
            advance();
        } else {
            throw std::runtime_error("Expected property name in object literal");
        }
        
        consume(TOK_COLON, "Expected ':' after property name");
        
        // Parse property value
        auto value = parseExpression();
        
        // Add property to object
        objectNode->properties.emplace_back(key, std::move(value));
        
        if (peek().type == TOK_COMMA) {
            advance(); // consume ','
        } else {
            break;
        }
    } while (peek().type != TOK_RBRACE && !isAtEnd());
    
    consume(TOK_RBRACE, "Expected '}' to close object literal");
    return objectNode;
}

// Parse array access arr[index] or object access obj.property
std::unique_ptr<ExpressionNode> Parser::parseArrayOrObjectAccess(std::unique_ptr<ExpressionNode> base)
{
    while (peek().type == TOK_LBRACKET || peek().type == TOK_DOT) {
        if (peek().type == TOK_LBRACKET) {
            // Array access: base[index]
            advance(); // consume '['
            auto index = parseExpression();
            consume(TOK_RBRACKET, "Expected ']' after array index");
            base = std::make_unique<ArrayAccessNode>(std::move(base), std::move(index));
        } else if (peek().type == TOK_DOT) {
            // Object property access: base.property
            advance(); // consume '.'
            if (peek().type != TOK_IDENTIFIER) {
                throw std::runtime_error("Expected property name after '.'");
            }
            std::string property = peek().value;
            advance(); // consume property name
            base = std::make_unique<ObjectAccessNode>(std::move(base), property);
        }
    }
    return base;
}
