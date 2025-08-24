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
    std::unique_ptr<FunctionDeclarationNode> parseFunctionDeclaration(); // For function definitions
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();    // For return statements
    std::unique_ptr<IfStatementNode> parseIfStatement();            // For if/else statements
    std::unique_ptr<WhileStatementNode> parseWhileStatement();      // For while loops
    std::unique_ptr<ForStatementNode> parseForStatement();          // For traditional for loops
    std::unique_ptr<DoWhileStatementNode> parseDoWhileStatement();  // For do-while loops
    std::unique_ptr<StatementNode> parseAssignmentStatement();      // For variable assignments
    std::unique_ptr<StatementNode> parseArrayAssignmentStatement(); // For array element assignments
    
    // Helper methods
    bool isKnownFunction(const std::string& name);                  // Check if function is known
    
    // Expression parsing with operator precedence
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parseComparisonExpression();    // == != < <= > >=
    std::unique_ptr<ExpressionNode> parseAdditionExpression();      // + -
    std::unique_ptr<ExpressionNode> parseMultiplicationExpression(); // * / %
    std::unique_ptr<ExpressionNode> parsePrimaryExpression();       // literals, variables, (expr)
    
    std::unique_ptr<StringLiteralNode> parseStringLiteral();
    std::unique_ptr<IntegerLiteralNode> parseIntegerLiteral();
    std::unique_ptr<BooleanLiteralNode> parseBooleanLiteral();
    std::unique_ptr<ExpressionNode> parseVariableExpression();
    std::unique_ptr<VariableDeclarationNode> parseVariableDeclarationStatement();
    
    // Array and object parsing
    std::unique_ptr<ArrayLiteralNode> parseArrayLiteral();
    std::unique_ptr<ObjectLiteralNode> parseObjectLiteral();
    std::unique_ptr<ExpressionNode> parseArrayOrObjectAccess(std::unique_ptr<ExpressionNode> base);

public:
    // Constructor takes the vector of tokens
    explicit Parser(const std::vector<Token> &tokens);

    // The main method that initiates parsing
    // Returns the root of the AST (a ProgramNode)
    std::unique_ptr<ProgramNode> parse();
};

#endif // PARSER_H