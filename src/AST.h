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
class BooleanLiteralNode;
class VariableExpressionNode;
class FunctionCallNode;
class FunctionDeclarationNode;
class UnaryExpressionNode;
class MethodCallNode;
class NewExpressionNode;
class ReturnStatementNode;
class ArrayLiteralNode;
class ArrayAccessNode;
class ObjectLiteralNode;
class ObjectAccessNode;
class VariableDeclarationNode;
class TypeAliasNode;
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
    int line = 0;   // 1-based source position; 0 = unknown
    int column = 0;

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

class VariableDeclarationNode : public StatementNode
{
public:
    std::string variableName;
    std::string typeName;
    std::unique_ptr<ExpressionNode> initializer;
    bool isConst;

    VariableDeclarationNode(std::string varName, std::string type, std::unique_ptr<ExpressionNode> init, bool isConstVal = false)
        : variableName(std::move(varName)), typeName(std::move(type)), initializer(std::move(init)), isConst(isConstVal) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "VariableDeclarationNode: " << (isConst ? "const " : "let ") << variableName << " : " << typeName << " =\n";
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

class TypeAliasNode : public StatementNode
{
public:
    std::string aliasName;
    std::vector<std::string> genericParams;
    std::string targetType;

    TypeAliasNode(std::string name, std::string target)
        : aliasName(std::move(name)), targetType(std::move(target)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "TypeAliasNode: type " << aliasName;
        if (!genericParams.empty()) {
            os << "<";
            for (size_t i = 0; i < genericParams.size(); ++i) {
                if (i > 0) os << ", ";
                os << genericParams[i];
            }
            os << ">";
        }
        os << " = " << targetType << "\n";
    }
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

class BooleanLiteralNode : public ExpressionNode
{
public:
    bool value;
    explicit BooleanLiteralNode(bool val) : value(val) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "BooleanLiteralNode: " << (value ? "true" : "false") << "\n";
    }
};

class FloatLiteralNode : public ExpressionNode
{
public:
    double value;
    explicit FloatLiteralNode(double val) : value(val) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "FloatLiteralNode: " << value << "\n";
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
        GREATER_EQUAL,  // >=
        
        // Logical operators
        LOGICAL_AND,    // &&
        LOGICAL_OR,     // ||

        // Bitwise operators (integer operands only)
        BIT_AND,        // &
        BIT_OR,         // |
        BIT_XOR,        // ^
        SHIFT_LEFT,     // <<
        SHIFT_RIGHT     // >> (arithmetic)
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
            case LOGICAL_AND: return "&&";
            case LOGICAL_OR: return "||";
            case BIT_AND: return "&";
            case BIT_OR: return "|";
            case BIT_XOR: return "^";
            case SHIFT_LEFT: return "<<";
            case SHIFT_RIGHT: return ">>";
            default: return "UNKNOWN";
        }
    }
};

// `null` / `undefined` — a null pointer. Mostly used for optional `ptr` handles
// coming back from C (an unloaded texture, a sound that failed to load).
class NullLiteralNode : public ExpressionNode
{
public:
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "NullLiteralNode\n";
    }
};

class UnaryExpressionNode : public ExpressionNode
{
public:
    enum Operator { NOT, MINUS, BIT_NOT };
    Operator op;
    std::unique_ptr<ExpressionNode> operand;

    UnaryExpressionNode(Operator op, std::unique_ptr<ExpressionNode> operand)
        : op(op), operand(std::move(operand)) {}

    const char* operatorToString(Operator op) const
    {
        switch (op)
        {
            case NOT: return "!";
            case MINUS: return "-";
            case BIT_NOT: return "~";
            default: return "unknown";
        }
    }

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "UnaryExpressionNode: " << operatorToString(op) << "\n";
        if (operand) operand->printNode(os, indent + 1);
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
    // For `a[i] += v`: the operator, with `value` holding only the right-hand
    // side. Carrying it here rather than desugaring to `a[i] = a[i] + v` is what
    // keeps the array and index expressions evaluated exactly once.
    bool isCompound = false;
    BinaryExpressionNode::Operator compoundOp = BinaryExpressionNode::ADD;

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

// For-of statement (for (const item of array))
class ForOfStatementNode : public StatementNode
{
public:
    std::unique_ptr<VariableDeclarationNode> iteratorVariable;
    std::unique_ptr<ExpressionNode> iterable;
    std::vector<std::unique_ptr<StatementNode>> bodyStatements;

    ForOfStatementNode(std::unique_ptr<VariableDeclarationNode> itVar,
                       std::unique_ptr<ExpressionNode> iter)
        : iteratorVariable(std::move(itVar)), iterable(std::move(iter)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ForOfStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Iterator Variable:\n";
        if (iteratorVariable) {
            iteratorVariable->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Iterable:\n";
        if (iterable) {
            iterable->printNode(os, indent + 2);
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

// Break statement (loops and switch)
class BreakStatementNode : public StatementNode
{
public:
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "BreakStatementNode\n";
    }
};

// Continue statement (loops)
class ContinueStatementNode : public StatementNode
{
public:
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ContinueStatementNode\n";
    }
};

// Switch statement with case/default clauses (fallthrough semantics, break exits)
class SwitchStatementNode : public StatementNode
{
public:
    struct CaseClause {
        std::unique_ptr<ExpressionNode> value; // null => default clause
        std::vector<std::unique_ptr<StatementNode>> statements;
    };

    std::unique_ptr<ExpressionNode> condition;
    std::vector<CaseClause> cases;

    explicit SwitchStatementNode(std::unique_ptr<ExpressionNode> cond)
        : condition(std::move(cond)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "SwitchStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Condition:\n";
        if (condition) condition->printNode(os, indent + 2);
        for (const auto &clause : cases) {
            printIndent(os, indent + 1);
            os << (clause.value ? "Case:\n" : "Default:\n");
            if (clause.value) clause.value->printNode(os, indent + 2);
            for (const auto &stmt : clause.statements) {
                if (stmt) stmt->printNode(os, indent + 2);
            }
        }
    }
};

// Interface declaration: compile-time structural type
class InterfaceDeclarationNode : public StatementNode
{
public:
    struct Member {
        std::string name;
        std::string type;
        Member(std::string n, std::string t) : name(std::move(n)), type(std::move(t)) {}
    };

    std::string interfaceName;
    std::string parentInterface; // from "extends", empty if none
    std::vector<Member> members;

    explicit InterfaceDeclarationNode(std::string name) : interfaceName(std::move(name)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "InterfaceDeclarationNode: " << interfaceName;
        if (!parentInterface.empty()) os << " extends " << parentInterface;
        os << "\n";
        for (const auto &m : members) {
            printIndent(os, indent + 1);
            os << m.name << ": " << m.type << "\n";
        }
    }
};

// Foreign function declaration: declare function rl_rect(x: f64, y: f64): void;
// The body lives in a C library; codegen emits an LLVM declaration and calls it
// directly, so the compiler never needs to know which library it came from.
class ExternDeclarationNode : public StatementNode
{
public:
    struct Parameter {
        std::string name;
        std::string type;
        Parameter(std::string n, std::string t) : name(std::move(n)), type(std::move(t)) {}
    };

    std::string functionName;
    std::vector<Parameter> parameters;
    std::string returnType;
    // The C symbol to call. Defaults to functionName, but `= "cyps_rect"` lets a
    // declaration expose a natural Cypescript name over a C-style symbol.
    std::string symbolName;

    ExternDeclarationNode(std::string name, std::string retType)
        : functionName(std::move(name)), returnType(std::move(retType)),
          symbolName(functionName) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ExternDeclarationNode: " << functionName << "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) os << ", ";
            os << parameters[i].name << ": " << parameters[i].type;
        }
        os << "): " << returnType;
        if (symbolName != functionName) os << " = \"" << symbolName << "\"";
        os << "\n";
    }
};

// Linker directive: link "raylib"; link framework "Cocoa"; link path "/usr/local/lib";
// An optional platform qualifier restricts it to one OS, which is what lets a
// single source file describe libraries that differ per platform:
//     link macos framework "Cocoa";
//     link linux "GL";
//     link windows "opengl32";
// `link source "mylib.c";` compiles a C/C++ file alongside the program, so using
// your own native code needs no separate build step or library.
// Collected by the driver and turned into flags on the final clang++ invocation.
class LinkDirectiveNode : public StatementNode
{
public:
    enum class Kind { Library, Framework, SearchPath, Source };
    enum class Platform { Any, MacOS, Linux, Windows };

    Kind kind;
    Platform platform;
    std::string value;

    LinkDirectiveNode(Kind k, std::string v, Platform p = Platform::Any)
        : kind(k), platform(p), value(std::move(v)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        const char *kindName = kind == Kind::Library ? "library"
                             : kind == Kind::Framework ? "framework"
                             : kind == Kind::Source ? "source" : "path";
        const char *platformName = platform == Platform::Any ? ""
                                 : platform == Platform::MacOS ? "macos "
                                 : platform == Platform::Linux ? "linux " : "windows ";
        os << "LinkDirectiveNode: " << platformName << kindName << " \"" << value << "\"\n";
    }
};

// Object property assignment: obj.prop = value (also this.prop = value)
class ObjectPropertyAssignmentNode : public StatementNode
{
public:
    std::unique_ptr<ExpressionNode> object;
    std::string property;
    std::unique_ptr<ExpressionNode> value;
    // For `obj.prop += v` — see ArrayAssignmentStatementNode for why
    bool isCompound = false;
    BinaryExpressionNode::Operator compoundOp = BinaryExpressionNode::ADD;

    ObjectPropertyAssignmentNode(std::unique_ptr<ExpressionNode> obj, std::string prop,
                                 std::unique_ptr<ExpressionNode> val)
        : object(std::move(obj)), property(std::move(prop)), value(std::move(val)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ObjectPropertyAssignmentNode: ." << property << " =\n";
        printIndent(os, indent + 1);
        os << "Object:\n";
        if (object) object->printNode(os, indent + 2);
        printIndent(os, indent + 1);
        os << "Value:\n";
        if (value) value->printNode(os, indent + 2);
    }
};

// Object destructuring: let { a, b } = obj;
class DestructuringDeclarationNode : public StatementNode
{
public:
    std::vector<std::string> bindings;
    std::unique_ptr<ExpressionNode> initializer;
    bool isConst;

    DestructuringDeclarationNode(std::vector<std::string> names,
                                 std::unique_ptr<ExpressionNode> init, bool isConstVal)
        : bindings(std::move(names)), initializer(std::move(init)), isConst(isConstVal) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "DestructuringDeclarationNode: " << (isConst ? "const {" : "let {");
        for (size_t i = 0; i < bindings.size(); ++i) {
            if (i > 0) os << ", ";
            os << bindings[i];
        }
        os << "} =\n";
        if (initializer) initializer->printNode(os, indent + 1);
    }
};

// Throw statement
class ThrowStatementNode : public StatementNode
{
public:
    std::unique_ptr<ExpressionNode> expression;

    explicit ThrowStatementNode(std::unique_ptr<ExpressionNode> expr)
        : expression(std::move(expr)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ThrowStatementNode:\n";
        if (expression) expression->printNode(os, indent + 1);
    }
};

// Try/catch/finally statement
class TryCatchStatementNode : public StatementNode
{
public:
    std::vector<std::unique_ptr<StatementNode>> tryStatements;
    std::string errorVariable; // catch (e) — empty if no binding
    std::vector<std::unique_ptr<StatementNode>> catchStatements;
    std::vector<std::unique_ptr<StatementNode>> finallyStatements;

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "TryCatchStatementNode:\n";
        printIndent(os, indent + 1);
        os << "Try:\n";
        for (const auto &stmt : tryStatements) if (stmt) stmt->printNode(os, indent + 2);
        printIndent(os, indent + 1);
        os << "Catch (" << errorVariable << "):\n";
        for (const auto &stmt : catchStatements) if (stmt) stmt->printNode(os, indent + 2);
        if (!finallyStatements.empty()) {
            printIndent(os, indent + 1);
            os << "Finally:\n";
            for (const auto &stmt : finallyStatements) if (stmt) stmt->printNode(os, indent + 2);
        }
    }
};

// Expression Statement Node - for expressions used as statements (like function calls)
class ExpressionStatementNode : public StatementNode
{
public:
    std::unique_ptr<ExpressionNode> expression;
    explicit ExpressionStatementNode(std::unique_ptr<ExpressionNode> expr) : expression(std::move(expr)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ExpressionStatementNode:\n";
        if (expression) {
            expression->printNode(os, indent + 1);
        }
    }
};

// Function Declaration Node - for function definitions
class FunctionDeclarationNode : public StatementNode
{
public:
    struct Parameter {
        std::string name;
        std::string type;
        Parameter(std::string n, std::string t) : name(std::move(n)), type(std::move(t)) {}
    };
    
    std::string functionName;
    std::vector<std::string> genericParams;
    std::vector<Parameter> parameters;
    std::string returnType;
    std::vector<std::unique_ptr<StatementNode>> bodyStatements;

    FunctionDeclarationNode(std::string name, std::string retType)
        : functionName(std::move(name)), returnType(std::move(retType)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "FunctionDeclarationNode: " << functionName;
        if (!genericParams.empty()) {
            os << "<";
            for (size_t i = 0; i < genericParams.size(); ++i) {
                if (i > 0) os << ", ";
                os << genericParams[i];
            }
            os << ">";
        }
        os << "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) os << ", ";
            os << parameters[i].name << ": " << parameters[i].type;
        }
        os << "): " << returnType << "\n";
        
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto &stmt : bodyStatements) {
            if (stmt) {
                stmt->printNode(os, indent + 2);
            }
        }
    }
};

// Return Statement Node - for return statements
class ReturnStatementNode : public StatementNode
{
public:
    std::unique_ptr<ExpressionNode> expression; // Optional - can be null for void returns
    
    explicit ReturnStatementNode(std::unique_ptr<ExpressionNode> expr = nullptr) 
        : expression(std::move(expr)) {}
    
    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ReturnStatementNode:\n";
        if (expression) {
            printIndent(os, indent + 1);
            os << "Expression:\n";
            expression->printNode(os, indent + 2);
        } else {
            printIndent(os, indent + 1);
            os << "Void return\n";
        }
    }
};

// Arrow function expression: (a: i32, b: i32): i32 => a + b, or x => { ... }
// Captured outer variables are snapshotted by value when the closure is created.
class ArrowFunctionNode : public ExpressionNode
{
public:
    std::vector<FunctionDeclarationNode::Parameter> parameters;
    std::string returnType; // "auto" = inferred
    std::vector<std::unique_ptr<StatementNode>> bodyStatements;

    ArrowFunctionNode() : returnType("auto") {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ArrowFunctionNode: (";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) os << ", ";
            os << parameters[i].name << ": " << parameters[i].type;
        }
        os << ") => " << returnType << "\n";
        for (const auto &stmt : bodyStatements) {
            if (stmt) stmt->printNode(os, indent + 1);
        }
    }
};

class FunctionCallNode : public ExpressionNode
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
        std::unique_ptr<FunctionDeclarationNode> method; // non-null for method properties
        // Set for class fields, which are declared with an explicit type. Lets
        // a class's struct layout be computed before any code is generated.
        std::string declaredType;

        Property(std::string k, std::unique_ptr<ExpressionNode> v)
            : key(std::move(k)), value(std::move(v)) {}
        Property(std::string k, std::unique_ptr<FunctionDeclarationNode> m)
            : key(std::move(k)), method(std::move(m)) {}
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
            os << (prop.method ? "Method:\n" : "Value:\n");
            if (prop.method) {
                prop.method->printNode(os, indent + 3);
            } else if (prop.value) {
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

class MethodCallNode : public ExpressionNode
{
public:
    std::unique_ptr<ExpressionNode> object;
    std::string methodName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

    MethodCallNode(std::unique_ptr<ExpressionNode> obj, std::string method)
        : object(std::move(obj)), methodName(std::move(method)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "MethodCallNode: ." << methodName << "(\n";
        printIndent(os, indent + 1);
        os << "Object:\n";
        if (object) {
            object->printNode(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Arguments:\n";
        for (const auto &arg : arguments)
        {
            if (arg) arg->printNode(os, indent + 2);
        }
        printIndent(os, indent);
        os << ")\n";
    }
};

// New expression (new Map<string, string[]>())
class NewExpressionNode : public ExpressionNode
{
public:
    std::string className;
    std::vector<std::string> genericTypes;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

    NewExpressionNode(std::string name) : className(std::move(name)) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "NewExpressionNode: new " << className;
        if (!genericTypes.empty()) {
            os << "<";
            for (size_t i = 0; i < genericTypes.size(); ++i) {
                if (i > 0) os << ", ";
                os << genericTypes[i];
            }
            os << ">";
        }
        os << "(\n";
        printIndent(os, indent + 1);
        os << "Arguments:\n";
        for (const auto &arg : arguments)
        {
            if (arg) arg->printNode(os, indent + 2);
        }
        printIndent(os, indent);
        os << ")\n";
    }
};

// Class declaration. At parse time the fields and methods are synthesized
// into an ObjectLiteralNode template; `new ClassName(args)` instantiates the
// template and then invokes the constructor method (if any) on it.
class ClassDeclarationNode : public StatementNode
{
public:
    std::string className;
    std::unique_ptr<ObjectLiteralNode> objectTemplate; // fields (with defaults) + methods
    bool hasConstructor = false;

    explicit ClassDeclarationNode(std::string name)
        : className(std::move(name)), objectTemplate(std::make_unique<ObjectLiteralNode>()) {}

    void printNode(llvm::raw_ostream &os, int indent = 0) const override
    {
        printIndent(os, indent);
        os << "ClassDeclarationNode: " << className << "\n";
        if (objectTemplate) objectTemplate->printNode(os, indent + 1);
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