// src/Parser.h
#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "AST.h" // Include our AST node definitions
#include <vector>
#include <memory> // For unique_ptr

class Parser
{
private:
    const std::vector<Token> &m_tokens; // Reference to the token vector from Lexer
    size_t m_currentPos = 0;            // Current position in the token vector

    // Helper methods (private)
    const Token &peek(int offset = 0) const; // Look ahead/behind
    const Token &advance();                  // Consume current token and return it
    bool isAtEnd() const;
    // Tries to consume the next token if it matches the expected type.
    // Returns true on success, false otherwise. Optionally reports error.
    bool match(TokenType expectedType);
    // Consumes the next token if it matches, otherwise reports error and potentially throws/exits.
    const Token &consume(TokenType expectedType, const std::string &errorMessage);

    // Parsing methods corresponding to grammar rules
    // These return unique_ptr to the created AST node
    std::unique_ptr<ProgramNode> parseProgram();
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<FunctionCallNode> parseFunctionCallStatement(); // Specific for print(...)
    std::unique_ptr<ExpressionNode> parseExpression(); // We only need string literals for MVP
    std::unique_ptr<StringLiteralNode> parseStringLiteral();
    std::unique_ptr<IntegerLiteralNode> parseIntegerLiteral();
    std::unique_ptr<VariableExpressionNode> parseVariableExpression();
    std::unique_ptr<ExpressionNode> parsePrimaryExpression();
    std::unique_ptr<VariableDeclarationNode> parseVariableDeclarationStatement();

public:
    // Constructor takes the vector of tokens
    explicit Parser(const std::vector<Token> &tokens);

    // The main method that initiates parsing
    // Returns the root of the AST (a ProgramNode)
    std::unique_ptr<ProgramNode> parse();
};

#endif // PARSER_H