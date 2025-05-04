// src/Lexer.cpp
#include "Lexer.h"
#include <cctype>   // For isspace, isalpha, isalnum
#include <string>   // For creating string in Token value
#include <iostream> // For potential debug messages (optional)

// Constructor
Lexer::Lexer(std::string_view source) : m_source(source), m_currentPos(0) {}

// --- Private Helper Methods ---

bool Lexer::isAtEnd() const {
    return m_currentPos >= m_source.length();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0'; // Null character signifies end
    return m_source[m_currentPos];
}

char Lexer::advance() {
    if (!isAtEnd()) {
        m_currentPos++;
    }
    // Return the character *before* advancing (the one consumed)
    return m_source[m_currentPos - 1];
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        // Basic whitespace check
        if (std::isspace(c)) {
            advance();
        }
        // Basic single-line comment support (optional, can be added later)
        // else if (c == '/' && m_currentPos + 1 < m_source.length() && m_source[m_currentPos + 1] == '/') {
        //     while (peek() != '\n' && !isAtEnd()) {
        //         advance();
        //     }
        // }
        else {
            // Not whitespace or comment start, stop skipping
            return;
        }
    }
}

Token Lexer::makeIdentifier() {
    // Assumes the current character (peek()) is the start of the identifier
    const size_t startPos = m_currentPos;
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string_view value = m_source.substr(startPos, m_currentPos - startPos);

    // Basic keyword check can be added later:
    // if (value == "print") return Token(TOK_KEYWORD_PRINT, std::string(value));
    // ... other keywords

    return Token(TOK_IDENTIFIER, std::string(value));
}

Token Lexer::makeStringLiteral() {
    // Assumes the current character (peek()) is the opening quote "
    advance(); // Consume the opening quote

    const size_t startPos = m_currentPos;
    while (peek() != '"' && !isAtEnd()) {
        // Add escape sequence handling later (e.g., \", \\, \n)
        advance();
    }

    if (isAtEnd()) {
        // Unterminated string
        return Token(TOK_UNKNOWN, "Unterminated string literal.");
    }

    // Found the closing quote
    std::string_view value = m_source.substr(startPos, m_currentPos - startPos);
    advance(); // Consume the closing quote

    return Token(TOK_STRING_LITERAL, std::string(value)); // Value excludes quotes
}

// --- Public Method ---

Token Lexer::getNextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return Token(TOK_EOF, ""); // End of file token
    }

    char c = peek();

    // Check for identifiers or keywords
    if (std::isalpha(c) || c == '_') {
        return makeIdentifier();
    }

    // Check for string literals
    if (c == '"') {
        return makeStringLiteral();
    }

    // Check for single-character tokens
    TokenType type = TOK_UNKNOWN;
    std::string value(1, c); // Create string with the single char

    switch (c) {
        case '(': type = TOK_LPAREN; break;
        case ')': type = TOK_RPAREN; break;
        case ';': type = TOK_SEMICOLON; break;
        // Add other single-char tokens later: {, }, [, ], ,, ., +, -, *, /, etc.
    }

    // Consume the character *after* checking it
    advance();

    if (type == TOK_UNKNOWN) {
        // Keep the value as the unrecognized character
        // Optionally add error message? "Unexpected character: " + value
    }

    return Token(type, value);
}