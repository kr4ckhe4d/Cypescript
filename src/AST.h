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
class ArrayLiteralNode;
class ArrayAccessNode;
class ObjectLiteralNode;
class ObjectAccessNode;
class VariableDeclarationNode;
class AssignmentStatementNode;
class ArrayAssignmentStatementNode;
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

// Binary expression for arithmetic and comparison operations
class BinaryExpressionNode : public ExpressionNode
{
public:
    enum Operator {
        // Arithmetic operators
        ADD,        // +
        SUBTRACT,   // -
        MULTIPLY,   // *
        DIVIDE,     // /
        MODULO,     // %
        
        // Comparison operators
        EQUAL,          // ==
        NOT_EQUAL,      // !=
        LESS_THAN,      // <
        LESS_EQUAL,     // <=
        GREATER_THAN,   // >
        GREATER_EQUAL   // >=
    };
    
    Operator op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
    
    BinaryExpressionNode(Operator operation, 
                        std::unique_ptr<ExpressionNode> leftExpr, 
                        std::unique_ptr<ExpressionNode> rightExpr)
        : op(operation), left(std::move(leftExpr)), right(std::move(rightExpr)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "BinaryExpressionNode: " << operatorToString(op) << "\n";
        printIndent(os, indent + 1);
        os << "Left:\n";
        if (left) {
            left->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Right:\n";
        if (right) {
            right->printNode(os, indent + 2);
        }
    }
    
private:
    const char* operatorToString(Operator op) const {
        switch (op) {
            case ADD: return "+";
            case SUBTRACT: return "-";
            case MULTIPLY: return "*";
            case DIVIDE: return "/";
            case MODULO: return "%";
            case EQUAL: return "==";
            case NOT_EQUAL: return "!=";
            case LESS_THAN: return "<";
            case LESS_EQUAL: return "<=";
            case GREATER_THAN: return ">";
            case GREATER_EQUAL: return ">=";
            default: return "UNKNOWN";
        }
    }
};

// --- Concrete Statement Node Types ---

// If statement for control flow
class IfStatementNode : public StatementNode
{
public:
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::unique_ptr<StatementNode>> thenStatements;
    std::vector<std::unique_ptr<StatementNode>> elseStatements; // Optional
    
    explicit IfStatementNode(std::unique_ptr<ExpressionNode> cond)
        : condition(std::move(cond)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "IfStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Condition:\n";
        if (condition) {
            condition->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Then:\n";
        for (const auto &stmt : thenStatements) {
            if (stmt) {
                stmt->printNode(os, indent + 2);
            }
        }
        if (!elseStatements.empty()) {
            printIndent(os, indent + 1);
            os << "Else:\n";
            for (const auto &stmt : elseStatements) {
                if (stmt) {
                    stmt->printNode(os, indent + 2);
                }
            }
        }
    }
};

// While statement for loops
class WhileStatementNode : public StatementNode
{
public:
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::unique_ptr<StatementNode>> bodyStatements;
    
    explicit WhileStatementNode(std::unique_ptr<ExpressionNode> cond)
        : condition(std::move(cond)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "WhileStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Condition:\n";
        if (condition) {
            condition->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto &stmt : bodyStatements) {
            if (stmt) {
                stmt->printNode(os, indent + 2);
            }
        }
    }
};

// Assignment statement for variable updates
class AssignmentStatementNode : public StatementNode
{
public:
    std::string variableName;
    std::unique_ptr<ExpressionNode> value;
    
    AssignmentStatementNode(std::string varName, std::unique_ptr<ExpressionNode> val)
        : variableName(std::move(varName)), value(std::move(val)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "AssignmentStatementNode: " << variableName << " =\n";
        if (value) {
            value->printNode(os, indent + 1);
        }
    }
};

// Array assignment statement for array element updates (arr[index] = value)
class ArrayAssignmentStatementNode : public StatementNode
{
public:
    std::unique_ptr<ExpressionNode> array;
    std::unique_ptr<ExpressionNode> index;
    std::unique_ptr<ExpressionNode> value;
    
    ArrayAssignmentStatementNode(std::unique_ptr<ExpressionNode> arr, 
                                std::unique_ptr<ExpressionNode> idx, 
                                std::unique_ptr<ExpressionNode> val)
        : array(std::move(arr)), index(std::move(idx)), value(std::move(val)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ArrayAssignmentStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Array:\n";
        if (array) {
            array->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Index:\n";
        if (index) {
            index->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Value:\n";
        if (value) {
            value->printNode(os, indent + 2);
        }
    }
};

// For statement (traditional C-style for loop)
class ForStatementNode : public StatementNode
{
public:
    std::unique_ptr<StatementNode> initialization; // let i: i32 = 0 or i = 0
    std::unique_ptr<ExpressionNode> condition;     // i < 10
    std::unique_ptr<StatementNode> increment;      // i = i + 1
    std::vector<std::unique_ptr<StatementNode>> bodyStatements;
    
    ForStatementNode(std::unique_ptr<StatementNode> init,
                     std::unique_ptr<ExpressionNode> cond,
                     std::unique_ptr<StatementNode> incr)
        : initialization(std::move(init)), condition(std::move(cond)), increment(std::move(incr)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ForStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Initialization:\n";
        if (initialization) {
            initialization->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Condition:\n";
        if (condition) {
            condition->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Increment:\n";
        if (increment) {
            increment->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto &stmt : bodyStatements) {
            if (stmt) {
                stmt->printNode(os, indent + 2);
            }
        }
    }
};

// Do-while statement (post-condition loop)
class DoWhileStatementNode : public StatementNode
{
public:
    std::vector<std::unique_ptr<StatementNode>> bodyStatements;
    std::unique_ptr<ExpressionNode> condition;
    
    explicit DoWhileStatementNode(std::unique_ptr<ExpressionNode> cond)
        : condition(std::move(cond)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "DoWhileStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto &stmt : bodyStatements) {
            if (stmt) {
                stmt->printNode(os, indent + 2);
            }
        }
        printIndent(os, indent + 1);
        os << "Condition:\n";
        if (condition) {
            condition->printNode(os, indent + 2);
        }
    }
};

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

// Array literal node for [1, 2, 3]
class ArrayLiteralNode : public ExpressionNode
{
public:
    std::vector<std::unique_ptr<ExpressionNode>> elements;
    std::string elementType; // "i32", "string", etc.
    
    explicit ArrayLiteralNode(std::string elemType) : elementType(std::move(elemType)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ArrayLiteralNode: " << elementType << "[]\n";
        printIndent(os, indent + 1);
        os << "Elements:\n";
        for (const auto &elem : elements) {
            if (elem) {
                elem->printNode(os, indent + 2);
            }
        }
    }
};

// Array access node for arr[index]
class ArrayAccessNode : public ExpressionNode
{
public:
    std::unique_ptr<ExpressionNode> array;
    std::unique_ptr<ExpressionNode> index;
    
    ArrayAccessNode(std::unique_ptr<ExpressionNode> arr, std::unique_ptr<ExpressionNode> idx)
        : array(std::move(arr)), index(std::move(idx)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ArrayAccessNode:\n";
        printIndent(os, indent + 1);
        os << "Array:\n";
        if (array) {
            array->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Index:\n";
        if (index) {
            index->printNode(os, indent + 2);
        }
    }
};

// Object literal node for { name: "Alice", age: 25 }
class ObjectLiteralNode : public ExpressionNode
{
public:
    struct Property {
        std::string key;
        std::unique_ptr<ExpressionNode> value;
        
        Property(std::string k, std::unique_ptr<ExpressionNode> v)
            : key(std::move(k)), value(std::move(v)) {}
    };
    
    std::vector<Property> properties;
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ObjectLiteralNode:\n";
        printIndent(os, indent + 1);
        os << "Properties:\n";
        for (const auto &prop : properties) {
            printIndent(os, indent + 2);
            os << "Key: " << prop.key << "\n";
            printIndent(os, indent + 2);
            os << "Value:\n";
            if (prop.value) {
                prop.value->printNode(os, indent + 3);
            }
        }
    }
};

// Object access node for obj.property
class ObjectAccessNode : public ExpressionNode
{
public:
    std::unique_ptr<ExpressionNode> object;
    std::string property;
    
    ObjectAccessNode(std::unique_ptr<ExpressionNode> obj, std::string prop)
        : object(std::move(obj)), property(std::move(prop)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ObjectAccessNode:\n";
        printIndent(os, indent + 1);
        os << "Object:\n";
        if (object) {
            object->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Property: " << property << "\n";
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