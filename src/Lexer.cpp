// src/Lexer.cpp - Improved lexical analyzer
#include "Lexer.h"
#include <cctype>
#include <string>
#include <iostream>
#include <unordered_map>

// Constructor
Lexer::Lexer(std::string_view source) : m_source(source), m_currentPos(0) {}

// --- Private Helper Methods ---

bool Lexer::isAtEnd() const {
    return m_currentPos >= m_source.length();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return m_source[m_currentPos];
}

char Lexer::advance() {
    if (!isAtEnd()) {
        m_currentPos++;
    }
    return m_source[m_currentPos - 1];
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        
        if (std::isspace(c)) {
            advance();
        }
        // Single-line comments: //
        else if (c == '/' && m_currentPos + 1 < m_source.length() && m_source[m_currentPos + 1] == '/') {
            // Skip until end of line
            while (peek() != '\n' && !isAtEnd()) {
                advance();
            }
        }
        // Multi-line comments: /* */
        else if (c == '/' && m_currentPos + 1 < m_source.length() && m_source[m_currentPos + 1] == '*') {
            advance(); // consume '/'
            advance(); // consume '*'
            
            while (!isAtEnd()) {
                if (peek() == '*' && m_currentPos + 1 < m_source.length() && m_source[m_currentPos + 1] == '/') {
                    advance(); // consume '*'
                    advance(); // consume '/'
                    break;
                }
                advance();
            }
        }
        else {
            return;
        }
    }
}

Token Lexer::makeIdentifier() {
    const size_t startPos = m_currentPos;
    
    // First character must be letter or underscore
    if (!std::isalpha(peek()) && peek() != '_') {
        return errorToken("Invalid identifier start");
    }
    
    // Continue with alphanumeric characters and underscores
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    
    std::string_view value = m_source.substr(startPos, m_currentPos - startPos);
    
    // Keywords lookup table
    static const std::unordered_map<std::string_view, TokenType> keywords = {
        {"let", TOK_LET},
        {"const", TOK_CONST},
        {"var", TOK_VAR},
        {"function", TOK_FUNCTION},
        {"if", TOK_IF},
        {"else", TOK_ELSE},
        {"while", TOK_WHILE},
        {"for", TOK_FOR},
        {"return", TOK_RETURN},
        {"true", TOK_TRUE},
        {"false", TOK_FALSE},
        {"null", TOK_NULL},
        {"undefined", TOK_UNDEFINED},
        {"number", TOK_TYPE_NUMBER},
        {"string", TOK_TYPE_STRING},
        {"boolean", TOK_TYPE_BOOLEAN},
        {"i32", TOK_TYPE_I32},
        {"f64", TOK_TYPE_F64}
    };
    
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token(it->second, std::string(value));
    }
    
    return Token(TOK_IDENTIFIER, std::string(value));
}

Token Lexer::makeStringLiteral() {
    advance(); // consume opening quote
    
    const size_t startPos = m_currentPos;
    std::string result;
    
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\\') {
            advance(); // consume backslash
            if (isAtEnd()) {
                return errorToken("Unterminated string literal");
            }
            
            // Handle escape sequences
            char escaped = peek();
            switch (escaped) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                case '0': result += '\0'; break;
                default:
                    result += escaped; // Keep unknown escapes as-is
                    break;
            }
            advance();
        } else {
            result += peek();
            advance();
        }
    }
    
    if (isAtEnd()) {
        return errorToken("Unterminated string literal");
    }
    
    advance(); // consume closing quote
    return Token(TOK_STRING_LITERAL, result);
}

Token Lexer::makeIntegerLiteral() {
    const size_t startPos = m_currentPos;
    
    // Handle different number formats
    if (peek() == '0' && m_currentPos + 1 < m_source.length()) {
        char next = m_source[m_currentPos + 1];
        if (next == 'x' || next == 'X') {
            // Hexadecimal
            advance(); // consume '0'
            advance(); // consume 'x'
            while (std::isxdigit(peek())) {
                advance();
            }
        } else if (next == 'b' || next == 'B') {
            // Binary
            advance(); // consume '0'
            advance(); // consume 'b'
            while (peek() == '0' || peek() == '1') {
                advance();
            }
        } else if (std::isdigit(next)) {
            // Octal
            while (peek() >= '0' && peek() <= '7') {
                advance();
            }
        } else {
            advance(); // just '0'
        }
    } else {
        // Decimal
        while (std::isdigit(peek())) {
            advance();
        }
        
        // Check for floating point
        if (peek() == '.' && m_currentPos + 1 < m_source.length() && std::isdigit(m_source[m_currentPos + 1])) {
            advance(); // consume '.'
            while (std::isdigit(peek())) {
                advance();
            }
            
            // Check for scientific notation
            if (peek() == 'e' || peek() == 'E') {
                advance();
                if (peek() == '+' || peek() == '-') {
                    advance();
                }
                while (std::isdigit(peek())) {
                    advance();
                }
            }
            
            std::string_view value = m_source.substr(startPos, m_currentPos - startPos);
            return Token(TOK_FLOAT_LITERAL, std::string(value));
        }
    }
    
    std::string_view value = m_source.substr(startPos, m_currentPos - startPos);
    return Token(TOK_INT_LITERAL, std::string(value));
}

Token Lexer::makeToken(TokenType type, const char* start, size_t length) {
    return Token(type, std::string(start, length));
}

Token Lexer::errorToken(const char* message) {
    return Token(TOK_UNKNOWN, std::string("Error: ") + message);
}

// --- Public Method ---

Token Lexer::getNextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TOK_EOF, "");
    }
    
    char c = peek();
    
    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return makeIdentifier();
    }
    
    // Numbers
    if (std::isdigit(c)) {
        return makeIntegerLiteral();
    }
    
    // String literals
    if (c == '"') {
        return makeStringLiteral();
    }
    
    // Two-character operators
    if (m_currentPos + 1 < m_source.length()) {
        char next = m_source[m_currentPos + 1];
        
        switch (c) {
            case '=':
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_EQUAL_EQUAL, "==");
                }
                if (next == '>') {
                    advance(); advance();
                    return Token(TOK_ARROW, "=>");
                }
                break;
            case '!':
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_NOT_EQUAL, "!=");
                }
                break;
            case '<':
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_LESS_EQUAL, "<=");
                }
                break;
            case '>':
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_GREATER_EQUAL, ">=");
                }
                break;
            case '&':
                if (next == '&') {
                    advance(); advance();
                    return Token(TOK_AND, "&&");
                }
                break;
            case '|':
                if (next == '|') {
                    advance(); advance();
                    return Token(TOK_OR, "||");
                }
                break;
            case '+':
                if (next == '+') {
                    advance(); advance();
                    return Token(TOK_PLUS_PLUS, "++");
                }
                break;
            case '-':
                if (next == '-') {
                    advance(); advance();
                    return Token(TOK_MINUS_MINUS, "--");
                }
                break;
        }
    }
    
    // Single-character tokens
    TokenType type = TOK_UNKNOWN;
    std::string value(1, c);
    
    switch (c) {
        case '(': type = TOK_LPAREN; break;
        case ')': type = TOK_RPAREN; break;
        case '{': type = TOK_LBRACE; break;
        case '}': type = TOK_RBRACE; break;
        case '[': type = TOK_LBRACKET; break;
        case ']': type = TOK_RBRACKET; break;
        case ';': type = TOK_SEMICOLON; break;
        case ':': type = TOK_COLON; break;
        case ',': type = TOK_COMMA; break;
        case '.': type = TOK_DOT; break;
        case '=': type = TOK_EQUAL; break;
        case '+': type = TOK_PLUS; break;
        case '-': type = TOK_MINUS; break;
        case '*': type = TOK_STAR; break;
        case '/': type = TOK_SLASH; break;
        case '%': type = TOK_PERCENT; break;
        case '!': type = TOK_BANG; break;
        case '<': type = TOK_LESS; break;
        case '>': type = TOK_GREATER; break;
        case '?': type = TOK_QUESTION; break;
        case '&': type = TOK_AMPERSAND; break;
        case '|': type = TOK_PIPE; break;
        case '^': type = TOK_CARET; break;
        case '~': type = TOK_TILDE; break;
    }
    
    advance();
    
    if (type == TOK_UNKNOWN) {
        std::cerr << "Warning: Unknown character '" << c << "' at position " << m_currentPos - 1 << std::endl;
    }
    
    return Token(type, value);
}
