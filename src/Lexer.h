// src/Lexer.h
#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <string_view>

class Lexer {
private:
    std::string_view m_source; // View into the source code string
    size_t m_currentPos = 0;   // Current position in the source string
    // Add line/column tracking later

    // Helper methods (private)
    char peek() const;            // Look at the current character without consuming
    char advance();           // Consume the current character and return it
    bool isAtEnd() const;
    void skipWhitespace();    // Skips spaces, tabs, newlines
    Token makeIdentifier();
    Token makeStringLiteral();
    Token makeToken(TokenType type, const char* start, size_t length);
    Token errorToken(const char* message);


public:
    // Constructor takes the source code as input
    explicit Lexer(std::string_view source);

    // The main method to get the next token from the stream
    Token getNextToken();
};

#endif // LEXER_H