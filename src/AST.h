// src/AST.h
#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>                     // For smart pointers
#include <iostream>                   // For std::ostream
#include <iomanip>                    // For std::setw, std::left (used by llvm::raw_ostream formatting)
#include "llvm/Support/raw_ostream.h" // For llvm::outs()

// Forward Declarations
class StringLiteralNode;
class IntegerLiteralNode;
class VariableExpressionNode;
class FunctionCallNode;
class VariableDeclarationNode;
class StatementNode;
class ExpressionNode;
class ProgramNode;

// Helper for indentation
inline void printIndent(llvm::raw_ostream &os, int indent)
{
    for (int i = 0; i < indent; ++i)
    {
        os << "  "; // Two spaces per indent level
    }
}

// --- Base Node Types ---

class ASTNode
{
public:
    virtual ~ASTNode() = default;
    // Pure virtual function for printing the node
    virtual void printNode(llvm::raw_ostream &os, int indent = 0) const = 0;
};

class ExpressionNode : public ASTNode
{
public:
    // Inherits printNode
};

class StatementNode : public ASTNode
{
public:
    // Inherits printNode
};

// --- Concrete Expression Node Types ---

class StringLiteralNode : public ExpressionNode
{
public:
    std::string value;
    explicit StringLiteralNode(std::string val) : value(std::move(val)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "StringLiteralNode: \"" << value << "\"\n";
    }
};

class IntegerLiteralNode : public ExpressionNode
{
public:
    long long value;
    explicit IntegerLiteralNode(long long val) : value(val) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "IntegerLiteralNode: " << value << "\n";
    }
};

class VariableExpressionNode : public ExpressionNode
{
public:
    std::string name;
    explicit VariableExpressionNode(std::string varName) : name(std::move(varName)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "VariableExpressionNode: " << name << "\n";
    }
};

// --- Concrete Statement Node Types ---

class FunctionCallNode : public StatementNode
{
public:
    std::string functionName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    explicit FunctionCallNode(std::string name) : functionName(std::move(name)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "FunctionCallNode: " << functionName << "(\n";
        for (const auto &arg : arguments)
        {
            if (arg)
            {
                arg->printNode(os, indent + 1);
            }
            else
            {
                printIndent(os, indent + 1);
                os << "NullArgumentNode\n";
            }
        }
        printIndent(os, indent);
        os << ")\n";
    }
};

class VariableDeclarationNode : public StatementNode
{
public:
    std::string variableName;
    std::string typeName;
    std::unique_ptr<ExpressionNode> initializer;

    VariableDeclarationNode(std::string varName, std::string type, std::unique_ptr<ExpressionNode> init)
        : variableName(std::move(varName)), typeName(std::move(type)), initializer(std::move(init)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "VariableDeclarationNode: " << variableName << " : " << typeName << " =\n";
        if (initializer)
        {
            initializer->printNode(os, indent + 1);
        }
        else
        {
            printIndent(os, indent + 1);
            os << "NullInitializerNode\n";
        }
    }
};

// --- Program Node ---

class ProgramNode : public ASTNode
{
public:
    std::vector<std::unique_ptr<StatementNode>> statements;

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ProgramNode:\n";
        for (const auto &stmt : statements)
        {
            if (stmt)
            {
                stmt->printNode(os, indent + 1);
            }
            else
            {
                printIndent(os, indent + 1);
                os << "NullStatementNode\n";
            }
        }
    }
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