// src/AST.h
#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory> // For smart pointers (good practice for AST nodes)

// Forward Declarations (if needed, maybe not for this simple MVP)
class StringLiteralNode;
class FunctionCallNode;
class StatementNode; // Base for statements
class ExpressionNode; // Base for expressions


// --- Base Node Types (Optional but recommended for structure) ---

// Base class for all AST nodes (can add common stuff like position later)
class ASTNode {
public:
    virtual ~ASTNode() = default; // Virtual destructor is important for base classes
    // We could add methods for things like pretty-printing the AST later
    // virtual void print(int indent = 0) const = 0;
};

// Base class for anything that can be an expression (evaluates to a value)
class ExpressionNode : public ASTNode {
public:
    // virtual void print(int indent = 0) const override = 0;
};

// Base class for anything that can be a statement (performs an action)
class StatementNode : public ASTNode {
public:
    // virtual void print(int indent = 0) const override = 0;
};


// --- Concrete Node Types for MVP ---

// Represents a string literal expression, e.g., "Hello, World!"
class StringLiteralNode : public ExpressionNode {
public:
    std::string value; // The content of the string (quotes removed)

    explicit StringLiteralNode(std::string val) : value(std::move(val)) {}

    // void print(int indent = 0) const override { /* Print logic */ }
};

// Represents a function call expression/statement, e.g., print(...)
// For MVP, we'll treat the print call as a Statement
class FunctionCallNode : public StatementNode { // Can be ExpressionNode too if fn returns value
public:
    std::string functionName;
    // Arguments are expressions. Using unique_ptr to manage memory.
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

    explicit FunctionCallNode(std::string name) : functionName(std::move(name)) {}

    // void print(int indent = 0) const override { /* Print logic */ }
};

// Represents the whole program or a block of statements
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StatementNode>> statements;

    // void print(int indent = 0) const override { /* Print logic */ }
};


#endif // AST_H

// What is an Abstract Syntax Tree (AST) and what does it do?

// Think about the tokens from the lexer: IDENTIFIER("print"), LPAREN, STRING_LITERAL("Hello, World!"), RPAREN, SEMICOLON, EOF. This is just a flat list, like a sequence of words. It doesn't inherently tell us how these pieces relate to each other structurally.

// The Abstract Syntax Tree (AST) is a hierarchical tree structure that represents the syntactic structure of the source code, based on the language's grammar. It's called "Abstract" because it usually omits details that are syntactically important but don't affect the meaning or structure (like the exact placement of parentheses or semicolons, although they guide the creation of the tree).

// What does it do?

// Captures Structure: It takes the flat token stream and builds a tree that shows the relationships. For print("Hello, World!");, the AST would represent something like:

// A ProgramNode (the root)1
// Contains a list of statements:
// A FunctionCallNode
// Function Name: "print"
// Arguments List:
// A StringLiteralNode with Value: "Hello, World!"