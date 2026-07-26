// src/Semantic.h - Semantic analysis pass
// Runs between parsing and codegen, catching errors with source positions:
//   - use of undefined variables
//   - reassignment of const bindings
//   - break/continue outside loops/switch
//   - wrong argument counts for user-defined functions
//   - type mismatches at declarations, assignments, returns and call arguments
//
// TYPE CHECKING POLICY: report an error only when both sides are confidently
// known and definitely incompatible. Cypescript has plenty of places where a
// type genuinely is not known at this stage — generic parameters, `json` values,
// `Map`/`Set` contents, closures, destructured bindings — and an over-eager
// checker would reject programs that compile and run correctly today. An unknown
// type (the empty string) therefore silences the check rather than failing it.
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "AST.h"
#include <map>
#include <set>
#include <string>
#include <vector>

class SemanticAnalyzer
{
public:
    // Throws std::runtime_error with a positioned message on the first problem
    void analyze(ProgramNode *program);

private:
    struct Binding {
        bool isConst = false;
        std::string type;   // canonical type name; empty means unknown
    };

    struct FunctionSignature {
        std::vector<std::string> parameterTypes;
        std::string returnType;
    };

    // Broad families a type belongs to. Assignment between different families is
    // what the checker rejects; within a family it stays quiet, because codegen
    // already coerces (i32<->f64, boolean as i32, string and ptr both being i8*).
    enum class TypeCategory { Unknown, Numeric, Text, Handle, Void };

    std::vector<std::map<std::string, Binding>> m_scopes;
    std::map<std::string, FunctionSignature> m_functions; // user + declared foreign
    std::set<std::string> m_types;             // class/interface names (not values)
    // Module-level variables. Unlike enclosing locals these stay visible inside
    // function bodies, matching the globals CodeGen promotes for the same names.
    std::map<std::string, Binding> m_globals;
    // Field name -> declared type, per class, for checking property access
    std::map<std::string, std::map<std::string, std::string>> m_classFields;
    // Class name -> parent class name, from `extends`
    std::map<std::string, std::string> m_classParents;
    int m_loopDepth = 0;
    int m_switchDepth = 0;
    bool m_inMethod = false;
    // Class whose method is being analyzed, so `super` can be validated
    std::string m_currentClass;
    // Declared return type of the function being analyzed ("" outside one)
    std::string m_currentReturnType;
    bool m_inFunction = false;

    void pushScope();
    void popScope();
    void declare(const std::string &name, bool isConst, const std::string &type = "");
    const Binding *lookup(const std::string &name) const;

    // --- Types ---
    // Best-effort static type of an expression; "" when not confidently known
    std::string typeOf(ExpressionNode *expr);
    TypeCategory categoryOf(const std::string &type) const;
    // Would assigning a `source`-typed value to a `target`-typed slot be wrong?
    bool isAssignable(const std::string &target, const std::string &source) const;
    // Reports a mismatch at `node` unless the two types are compatible
    void checkAssignable(const ASTNode *node, const std::string &target,
                         const std::string &source, const std::string &context);
    // Element type of an array type ("i32[]" -> "i32"), or "" if not an array
    static std::string elementTypeOf(const std::string &arrayType);

    void hoistDeclarations(const std::vector<std::unique_ptr<StatementNode>> &statements);
    void analyzeStatement(StatementNode *stmt);
    void analyzeStatementList(const std::vector<std::unique_ptr<StatementNode>> &statements);
    void analyzeExpression(ExpressionNode *expr);
    void analyzeFunctionBody(const std::vector<FunctionDeclarationNode::Parameter> &params,
                             const std::vector<std::unique_ptr<StatementNode>> &body,
                             bool isMethod, const std::string &returnType = "",
                             const std::string &className = "");

    [[noreturn]] void fail(const ASTNode *node, const std::string &message) const;
};

#endif // SEMANTIC_H
