// src/Token.h - Token definitions for Cypescript
#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <string_view>

// Enum for all possible token types in Cypescript
enum TokenType {
    // Literals
    TOK_IDENTIFIER,     // variable names, function names
    TOK_STRING_LITERAL, // "hello world"
    TOK_INT_LITERAL,    // 123, 0xFF, 0b1010
    TOK_FLOAT_LITERAL,  // 3.14, 1.0e-5
    
    // Keywords
    TOK_LET,            // let
    TOK_CONST,          // const
    TOK_VAR,            // var
    TOK_FUNCTION,       // function
    TOK_IF,             // if
    TOK_ELSE,           // else
    TOK_WHILE,          // while
    TOK_FOR,            // for
    TOK_DO,             // do
    TOK_RETURN,         // return
    TOK_TRUE,           // true
    TOK_FALSE,          // false
    TOK_NULL,           // null
    TOK_UNDEFINED,      // undefined
    
    // Type keywords
    TOK_TYPE_NUMBER,    // number
    TOK_TYPE_STRING,    // string
    TOK_TYPE_BOOLEAN,   // boolean
    TOK_TYPE_I32,       // i32
    TOK_TYPE_F64,       // f64
    TOK_TYPE_VOID,      // void
    
    // Punctuation
    TOK_LPAREN,         // (
    TOK_RPAREN,         // )
    TOK_LBRACE,         // {
    TOK_RBRACE,         // }
    TOK_LBRACKET,       // [
    TOK_RBRACKET,       // ]
    TOK_SEMICOLON,      // ;
    TOK_COLON,          // :
    TOK_COMMA,          // ,
    TOK_DOT,            // .
    TOK_QUESTION,       // ?
    
    // Operators
    TOK_EQUAL,          // =
    TOK_EQUAL_EQUAL,    // ==
    TOK_NOT_EQUAL,      // !=
    TOK_LESS,           // <
    TOK_LESS_EQUAL,     // <=
    TOK_GREATER,        // >
    TOK_GREATER_EQUAL,  // >=
    TOK_PLUS,           // +
    TOK_MINUS,          // -
    TOK_STAR,           // *
    TOK_SLASH,          // /
    TOK_PERCENT,        // %
    TOK_BANG,           // !
    TOK_AND,            // &&
    TOK_OR,             // ||
    TOK_AMPERSAND,      // &
    TOK_PIPE,           // |
    TOK_CARET,          // ^
    TOK_TILDE,          // ~
    TOK_PLUS_PLUS,      // ++
    TOK_MINUS_MINUS,    // --
    TOK_ARROW,          // =>
    
    // Special tokens
    TOK_EOF,            // End of file
    TOK_UNKNOWN         // Error/unknown token
};

// Function to get string representation of TokenType (for debugging)
inline const char* tokenTypeToString(TokenType type) {
    switch (type) {
        // Literals
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_STRING_LITERAL: return "STRING_LITERAL";
        case TOK_INT_LITERAL: return "INT_LITERAL";
        case TOK_FLOAT_LITERAL: return "FLOAT_LITERAL";
        
        // Keywords
        case TOK_LET: return "LET";
        case TOK_CONST: return "CONST";
        case TOK_VAR: return "VAR";
        case TOK_FUNCTION: return "FUNCTION";
        case TOK_IF: return "IF";
        case TOK_ELSE: return "ELSE";
        case TOK_WHILE: return "WHILE";
        case TOK_FOR: return "FOR";
        case TOK_DO: return "DO";
        case TOK_RETURN: return "RETURN";
        case TOK_TRUE: return "TRUE";
        case TOK_FALSE: return "FALSE";
        case TOK_NULL: return "NULL";
        case TOK_UNDEFINED: return "UNDEFINED";
        
        // Type keywords
        case TOK_TYPE_NUMBER: return "TYPE_NUMBER";
        case TOK_TYPE_STRING: return "TYPE_STRING";
        case TOK_TYPE_BOOLEAN: return "TYPE_BOOLEAN";
        case TOK_TYPE_I32: return "TYPE_I32";
        case TOK_TYPE_F64: return "TYPE_F64";
        case TOK_TYPE_VOID: return "TYPE_VOID";
        
        // Punctuation
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_LBRACE: return "LBRACE";
        case TOK_RBRACE: return "RBRACE";
        case TOK_LBRACKET: return "LBRACKET";
        case TOK_RBRACKET: return "RBRACKET";
        case TOK_SEMICOLON: return "SEMICOLON";
        case TOK_COLON: return "COLON";
        case TOK_COMMA: return "COMMA";
        case TOK_DOT: return "DOT";
        case TOK_QUESTION: return "QUESTION";
        
        // Operators
        case TOK_EQUAL: return "EQUAL";
        case TOK_EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TOK_NOT_EQUAL: return "NOT_EQUAL";
        case TOK_LESS: return "LESS";
        case TOK_LESS_EQUAL: return "LESS_EQUAL";
        case TOK_GREATER: return "GREATER";
        case TOK_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOK_PLUS: return "PLUS";
        case TOK_MINUS: return "MINUS";
        case TOK_STAR: return "STAR";
        case TOK_SLASH: return "SLASH";
        case TOK_PERCENT: return "PERCENT";
        case TOK_BANG: return "BANG";
        case TOK_AND: return "AND";
        case TOK_OR: return "OR";
        case TOK_AMPERSAND: return "AMPERSAND";
        case TOK_PIPE: return "PIPE";
        case TOK_CARET: return "CARET";
        case TOK_TILDE: return "TILDE";
        case TOK_PLUS_PLUS: return "PLUS_PLUS";
        case TOK_MINUS_MINUS: return "MINUS_MINUS";
        case TOK_ARROW: return "ARROW";
        
        // Special tokens
        case TOK_EOF: return "EOF";
        case TOK_UNKNOWN: return "UNKNOWN";
        
        default: return "INVALID_TOKEN_TYPE";
    }
}

// Structure to represent a Token
struct Token {
    TokenType type;
    std::string value;
    // Future: Add line/column information for better error reporting
    // int line = 1;
    // int column = 1;
    
    // Constructors
    Token(TokenType t, std::string v) : type(t), value(std::move(v)) {}
    Token() : type(TOK_UNKNOWN) {}
    
    // Utility methods
    bool isLiteral() const {
        return type == TOK_STRING_LITERAL || 
               type == TOK_INT_LITERAL || 
               type == TOK_FLOAT_LITERAL ||
               type == TOK_TRUE ||
               type == TOK_FALSE ||
               type == TOK_NULL;
    }
    
    bool isKeyword() const {
        return type >= TOK_LET && type <= TOK_UNDEFINED;
    }
    
    bool isOperator() const {
        return type >= TOK_EQUAL && type <= TOK_ARROW;
    }
    
    bool isType() const {
        return type >= TOK_TYPE_NUMBER && type <= TOK_TYPE_VOID;
    }
};

#endif // TOKEN_H