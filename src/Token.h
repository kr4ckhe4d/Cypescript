// src/Token.h
#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <string_view> // Use string_view for efficiency if possible

// Enum for all possible token types in our language (expand later)
enum TokenType
{
    // Single-character tokens
    TOK_LPAREN,    // (
    TOK_RPAREN,    // )
    TOK_SEMICOLON, // ;
    TOK_COLON,     // :
    TOK_EQUAL,     // =

    // Keywords
    TOK_LET, // let

    // Literals
    TOK_IDENTIFIER,     // e.g., print, variableName, typeName (string, i32)
    TOK_STRING_LITERAL, // e.g., "Hello, World!"
    TOK_INT_LITERAL,    // e.g., 123, 0, 42

    // Special Tokens
    TOK_EOF,    // End of input
    TOK_UNKNOWN // For errors or unrecognized characters
};

// Function to get a string representation of a TokenType (for debugging)
inline const char *tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TOK_LPAREN:
        return "LPAREN";
    case TOK_RPAREN:
        return "RPAREN";
    case TOK_SEMICOLON:
        return "SEMICOLON";
    case TOK_IDENTIFIER:
        return "IDENTIFIER";
    case TOK_STRING_LITERAL:
        return "STRING_LITERAL";
    case TOK_LET:
        return "LET";
    case TOK_INT_LITERAL:
        return "INT_LITERAL";
    case TOK_COLON:
        return "COLON";
    case TOK_EQUAL:
        return "EQUAL";
    case TOK_EOF:
        return "EOF";
    case TOK_UNKNOWN:
        return "UNKNOWN";
    default:
        return "INVALID_TOKEN_TYPE";
    }
}

// Structure to represent a Token
struct Token
{
    TokenType type;
    std::string value; // The actual text of the token (e.g., "print", "\"Hello, World!\"")
    // Add line/column numbers later for better error messages
    // int line;
    // int column;

    // Optional: Constructor for convenience
    Token(TokenType t, std::string v) : type(t), value(std::move(v)) {}
    Token() : type(TOK_UNKNOWN) {} // Default constructor
};

#endif // TOKEN_H