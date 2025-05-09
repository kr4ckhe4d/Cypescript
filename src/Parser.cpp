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
    else if (peek().type == TOK_IDENTIFIER && peek().value == "print")
    { // Assuming print is not a general expression start
        return parseFunctionCallStatement();
    }
    // Add other statement types here (if, while, return, expression statements etc.)
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

    consume(TOK_COLON, "Expected ':' after variable name for type annotation");

    // For now, type names are just identifiers (e.g., "string", "i32")
    const Token &typeNameToken = consume(TOK_IDENTIFIER, "Expected type name after ':'");
    std::string typeName = typeNameToken.value;

    consume(TOK_EQUAL, "Expected '=' for variable initialization");

    std::unique_ptr<ExpressionNode> initializer = parseExpression();

    consume(TOK_SEMICOLON, "Expected ';' after variable declaration statement");

    return std::make_unique<VariableDeclarationNode>(varName, typeName, std::move(initializer));
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

// Expression Parsing (currently very simple, will need operator precedence later)
std::unique_ptr<ExpressionNode> Parser::parseExpression()
{
    // For now, our expressions are just primary expressions (literals or identifiers)
    // This will be expanded for arithmetic, logical ops, etc.
    return parsePrimaryExpression();
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

std::unique_ptr<VariableExpressionNode> Parser::parseVariableExpression()
{
    const Token &varToken = consume(TOK_IDENTIFIER, "Expected variable name.");
    return std::make_unique<VariableExpressionNode>(varToken.value);
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
