// src/Lexer.cpp - Improved lexical analyzer
#include "Lexer.h"
#include <cctype>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>

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
        char c = m_source[m_currentPos];
        m_currentPos++;
        if (c == '\n') {
            m_line++;
            m_col = 1;
        } else {
            m_col++;
        }
        return c;
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
        {"do", TOK_DO},
        {"return", TOK_RETURN},
        {"true", TOK_TRUE},
        {"false", TOK_FALSE},
        {"null", TOK_NULL},
        {"undefined", TOK_UNDEFINED},
        {"of", TOK_OF},
        {"new", TOK_NEW},
        {"type", TOK_TYPE},
        {"break", TOK_BREAK},
        {"continue", TOK_CONTINUE},
        {"switch", TOK_SWITCH},
        {"case", TOK_CASE},
        {"default", TOK_DEFAULT},
        {"interface", TOK_INTERFACE},
        {"extends", TOK_EXTENDS},
        {"this", TOK_THIS},
        {"import", TOK_IMPORT},
        {"export", TOK_EXPORT},
        {"from", TOK_FROM},
        {"try", TOK_TRY},
        {"catch", TOK_CATCH},
        {"finally", TOK_FINALLY},
        {"throw", TOK_THROW},
        {"class", TOK_CLASS},
        {"enum", TOK_ENUM},
        {"number", TOK_TYPE_NUMBER},
        {"string", TOK_TYPE_STRING},
        {"boolean", TOK_TYPE_BOOLEAN},
        {"i32", TOK_TYPE_I32},
        {"f64", TOK_TYPE_F64},
        {"void", TOK_TYPE_VOID}
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

Token Lexer::makeTemplateLiteral() {
    // All expansion tokens report the template's start position
    int tplLine = m_line;
    int tplCol = m_col;
    advance(); // consume opening backtick

    // Collected pieces: literal string parts and embedded expression sources
    struct Piece { bool isExpr; std::string text; };
    std::vector<Piece> pieces;
    std::string current;

    while (!isAtEnd() && peek() != '`') {
        char ch = peek();
        if (ch == '\\') {
            advance();
            if (isAtEnd()) return errorToken("Unterminated template literal");
            char escaped = peek();
            switch (escaped) {
                case 'n': current += '\n'; break;
                case 't': current += '\t'; break;
                case 'r': current += '\r'; break;
                case '\\': current += '\\'; break;
                case '`': current += '`'; break;
                case '$': current += '$'; break;
                default: current += escaped; break;
            }
            advance();
        } else if (ch == '$' && m_currentPos + 1 < m_source.length() && m_source[m_currentPos + 1] == '{') {
            advance(); // consume '$'
            advance(); // consume '{'
            pieces.push_back({false, current});
            current.clear();

            // Capture the expression source until the matching '}'
            std::string exprSrc;
            int braceDepth = 1;
            bool inString = false;
            while (!isAtEnd() && braceDepth > 0) {
                char ec = peek();
                if (inString) {
                    if (ec == '\\') { exprSrc += advance(); if (!isAtEnd()) exprSrc += advance(); continue; }
                    if (ec == '"') inString = false;
                } else {
                    if (ec == '"') inString = true;
                    else if (ec == '{') braceDepth++;
                    else if (ec == '}') { braceDepth--; if (braceDepth == 0) { advance(); break; } }
                }
                exprSrc += advance();
            }
            if (braceDepth > 0) return errorToken("Unterminated ${...} in template literal");
            pieces.push_back({true, exprSrc});
        } else {
            current += advance();
        }
    }

    if (isAtEnd()) return errorToken("Unterminated template literal");
    advance(); // consume closing backtick
    pieces.push_back({false, current});

    // Desugar into a parenthesized concatenation:  ("part" + (expr) + "part")
    std::vector<Token> expansion;
    expansion.emplace_back(TOK_LPAREN, "(");
    bool first = true;
    for (const auto& piece : pieces) {
        // Skip empty literal parts unless the whole template is empty
        if (!piece.isExpr && piece.text.empty() && !(pieces.size() == 1)) continue;
        if (!first) expansion.emplace_back(TOK_PLUS, "+");
        first = false;
        if (piece.isExpr) {
            expansion.emplace_back(TOK_LPAREN, "(");
            Lexer subLexer(piece.text);
            Token sub;
            while ((sub = subLexer.getNextToken()).type != TOK_EOF) {
                if (sub.type == TOK_UNKNOWN) return errorToken("Invalid expression in template literal");
                expansion.push_back(sub);
            }
            expansion.emplace_back(TOK_RPAREN, ")");
        } else {
            expansion.emplace_back(TOK_STRING_LITERAL, piece.text);
        }
    }
    if (first) {
        // Template contained nothing printable; produce an empty string
        expansion.emplace_back(TOK_STRING_LITERAL, "");
    }
    expansion.emplace_back(TOK_RPAREN, ")");

    for (auto& tok : expansion) {
        tok.line = tplLine;
        tok.column = tplCol;
    }
    Token head = expansion.front();
    for (size_t i = 1; i < expansion.size(); ++i) m_pending.push_back(expansion[i]);
    return head;
}

Token Lexer::makeIntegerLiteral() {
    const size_t startPos = m_currentPos;
    
    // Handle different number formats
    if (peek() == '0' && m_currentPos + 1 < m_source.length() &&
        m_source[m_currentPos + 1] != '.') {
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
    // Serve tokens queued by template-literal desugaring first
    if (!m_pending.empty()) {
        Token tok = m_pending.front();
        m_pending.pop_front();
        return tok;
    }

    skipWhitespace();
    int startLine = m_line;
    int startCol = m_col;
    Token tok = scanToken();
    if (tok.line == 0) {
        tok.line = startLine;
        tok.column = startCol;
    }
    return tok;
}

Token Lexer::scanToken() {
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

    // Template literals: `text ${expr} text`
    if (c == '`') {
        return makeTemplateLiteral();
    }

    // Two-character operators
    if (m_currentPos + 1 < m_source.length()) {
        char next = m_source[m_currentPos + 1];
        
        switch (c) {
            case '=':
                if (next == '=') {
                    advance(); advance();
                    if (peek() == '=') advance(); // === treated as ==
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
                    if (peek() == '=') advance(); // !== treated as !=
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
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_PLUS_EQUAL, "+=");
                }
                break;
            case '-':
                if (next == '-') {
                    advance(); advance();
                    return Token(TOK_MINUS_MINUS, "--");
                }
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_MINUS_EQUAL, "-=");
                }
                break;
            case '*':
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_STAR_EQUAL, "*=");
                }
                break;
            case '/':
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_SLASH_EQUAL, "/=");
                }
                break;
            case '%':
                if (next == '=') {
                    advance(); advance();
                    return Token(TOK_PERCENT_EQUAL, "%=");
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
