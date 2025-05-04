// src/Parser.cpp
#include "Parser.h"
#include "Token.h" // Include Token definitions
#include "AST.h"   // Include AST node definitions
#include <vector>
#include <memory>   // For std::make_unique
#include <stdexcept> // For reporting errors (can refine later)
#include <iostream>  // For error messages

// --- Constructor ---
Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens), m_currentPos(0) {}

// --- Private Helper Methods ---

// Look at the token at the current position + offset, without consuming
const Token& Parser::peek(int offset) const {
    // Static EOF token to return if index is out of bounds
    static Token eofToken(TOK_EOF, "");

    size_t index = m_currentPos + offset;
    if (index >= m_tokens.size()) {
        return eofToken; // Return the static EOF if out of bounds
    }
    return m_tokens[index];
}

// Consume the current token and advance position
const Token& Parser::advance() {
    if (!isAtEnd()) {
        m_currentPos++;
    }
    return peek(-1); // Return the token *before* the new currentPos (the one consumed)
}

// Check if we've consumed all tokens (except potentially the final EOF)
bool Parser::isAtEnd() const {
    // We consider EOF as a token, so end is when current token is EOF
    return peek().type == TOK_EOF;
}

// Check if the current token matches the expected type
bool Parser::match(TokenType expectedType) {
    if (isAtEnd()) return false;
    if (peek().type == expectedType) {
        advance(); // Consume the token if it matches
        return true;
    }
    return false;
}

// Consume the next token if it matches, otherwise report error
const Token& Parser::consume(TokenType expectedType, const std::string& errorMessage) {
    if (peek().type == expectedType) {
        return advance(); // Consume and return the matched token
    }
    // Error: Unexpected token
    // More sophisticated error handling needed later (line/col numbers)
    std::cerr << "Parse Error: " << errorMessage << ". Found "
              << tokenTypeToString(peek().type) << " ('" << peek().value << "') instead.\n";
    // For now, throw an exception to halt parsing. Refine later.
    throw std::runtime_error("Parsing failed: " + errorMessage);
}

// --- Parsing Methods for Grammar Rules ---

// Parse the entire program (currently just one statement)
std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto programNode = std::make_unique<ProgramNode>();
    while (!isAtEnd()) {
        programNode->statements.push_back(parseStatement());
    }
    return programNode;
}

// Parse a single statement (currently only function calls like print(...))
std::unique_ptr<StatementNode> Parser::parseStatement() {
    // For MVP, we only expect 'print' function call statement
    if (peek().type == TOK_IDENTIFIER && peek().value == "print") {
        return parseFunctionCallStatement();
    } else {
        // Error: Unexpected token at start of statement
        throw std::runtime_error("Parsing failed: Expected 'print' statement.");
    }
}

// Parse the specific print("..."); statement
std::unique_ptr<FunctionCallNode> Parser::parseFunctionCallStatement() {
    // Assumes current token is IDENTIFIER("print")
    std::string functionName = advance().value; // Consume identifier
    auto callNode = std::make_unique<FunctionCallNode>(functionName);

    consume(TOK_LPAREN, "Expected '(' after function name");

    // Parse arguments (only one string literal for MVP)
    if (peek().type != TOK_RPAREN) { // Check if there are arguments
         // Currently, only expect a string literal
         if (peek().type == TOK_STRING_LITERAL) {
             callNode->arguments.push_back(parseStringLiteral());
         } else {
              throw std::runtime_error("Parsing failed: Expected string literal argument for 'print'.");
         }
         // Add logic for more arguments later (e.g., comma separation)
    } // else: no arguments between () - handle later if needed


    consume(TOK_RPAREN, "Expected ')' after function arguments");
    consume(TOK_SEMICOLON, "Expected ';' after statement");

    return callNode;
}

// Parse an expression (only string literals for MVP)
std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    if (peek().type == TOK_STRING_LITERAL) {
        return parseStringLiteral();
    } else {
        throw std::runtime_error("Parsing failed: Expected an expression (only string literals supported).");
    }
}

// Parse a string literal token into a StringLiteralNode
std::unique_ptr<StringLiteralNode> Parser::parseStringLiteral() {
    // Assumes current token is TOK_STRING_LITERAL
    const Token& stringToken = consume(TOK_STRING_LITERAL, "Expected string literal.");
    return std::make_unique<StringLiteralNode>(stringToken.value);
}


// --- Public Main Parsing Method ---

// Start the parsing process
std::unique_ptr<ProgramNode> Parser::parse() {
    try {
        return parseProgram();
    } catch (const std::runtime_error& e) {
        // Basic error catching
        std::cerr << "Caught parsing exception: " << e.what() << std::endl;
        return nullptr; // Indicate failure
    }
}