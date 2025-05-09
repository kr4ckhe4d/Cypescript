// src/AST.h
#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory> // For smart pointers (good practice for AST nodes)

// Forward Declarations
class StringLiteralNode;
class IntegerLiteralNode; // New
class VariableExpressionNode; // New
class FunctionCallNode;
class VariableDeclarationNode; // New
class StatementNode;
class ExpressionNode;
class ProgramNode;


// --- Base Node Types ---

class ASTNode {
public:
    virtual ~ASTNode() = default;
    // virtual void print(int indent = 0) const = 0; // For AST debugging
};

class ExpressionNode : public ASTNode {
public:
    // Type information can be added here later during semantic analysis
    // virtual llvm::Type* type;
};

class StatementNode : public ASTNode {
public:
};


// --- Concrete Expression Node Types ---

class StringLiteralNode : public ExpressionNode {
public:
    std::string value;
    explicit StringLiteralNode(std::string val) : value(std::move(val)) {}
};

class IntegerLiteralNode : public ExpressionNode {
public:
    long long value; // Using long long for flexibility
    explicit IntegerLiteralNode(long long val) : value(val) {}
};

class VariableExpressionNode : public ExpressionNode {
public:
    std::string name;
    explicit VariableExpressionNode(std::string varName) : name(std::move(varName)) {}
};


// --- Concrete Statement Node Types ---

class FunctionCallNode : public StatementNode { // Could also be an ExpressionNode if functions return values
public:
    std::string functionName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    explicit FunctionCallNode(std::string name) : functionName(std::move(name)) {}
};

class VariableDeclarationNode : public StatementNode {
public:
    std::string variableName;
    std::string typeName; // e.g., "string", "i32". Could be a TypeNode later.
    std::unique_ptr<ExpressionNode> initializer; // The expression on the RHS of '='

    VariableDeclarationNode(std::string varName, std::string type, std::unique_ptr<ExpressionNode> init)
        : variableName(std::move(varName)), typeName(std::move(type)), initializer(std::move(init)) {}
};


// --- Program Node ---

class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StatementNode>> statements;
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