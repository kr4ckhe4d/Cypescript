// src/Lexer.h
#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <string_view>
#include <deque>

class Lexer {
private:
    std::string_view m_source; // View into the source code string
    size_t m_currentPos = 0;   // Current position in the source string
    std::deque<Token> m_pending; // Tokens queued by multi-token constructs (template literals)
    int m_line = 1;            // Current line (1-based)
    int m_col = 1;             // Current column (1-based)

    // Scans the next raw token (positions are stamped by getNextToken)
    Token scanToken();

    // Helper methods (private)
    char peek() const;            // Look at the current character without consuming
    char advance();           // Consume the current character and return it
    bool isAtEnd() const;
    void skipWhitespace();    // Skips spaces, tabs, newlines
    Token makeIdentifier();
    Token makeStringLiteral();
    Token makeIntegerLiteral();
    Token makeTemplateLiteral(); // Desugars `a ${x} b` into ("a" + (x) + " b")
    Token makeToken(TokenType type, const char* start, size_t length);
    Token errorToken(const char* message);


public:
    // Constructor takes the source code as input
    explicit Lexer(std::string_view source);

    // The main method to get the next token from the stream
    Token getNextToken();
};

#endif // LEXER_H
