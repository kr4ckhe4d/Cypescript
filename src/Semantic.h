// src/Semantic.h - Semantic analysis pass
// Runs between parsing and codegen, catching errors with source positions:
//   - use of undefined variables
//   - reassignment of const bindings
//   - break/continue outside loops/switch
//   - wrong argument counts for user-defined functions
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
    };

    std::vector<std::map<std::string, Binding>> m_scopes;
    std::map<std::string, size_t> m_functions; // user function name -> arity
    std::set<std::string> m_types;             // class/interface names (not values)
    // Module-level variables. Unlike enclosing locals these stay visible inside
    // function bodies, matching the globals CodeGen promotes for the same names.
    std::map<std::string, Binding> m_globals;
    int m_loopDepth = 0;
    int m_switchDepth = 0;
    bool m_inMethod = false;

    void pushScope();
    void popScope();
    void declare(const std::string &name, bool isConst);
    const Binding *lookup(const std::string &name) const;

    void hoistDeclarations(const std::vector<std::unique_ptr<StatementNode>> &statements);
    void analyzeStatement(StatementNode *stmt);
    void analyzeStatementList(const std::vector<std::unique_ptr<StatementNode>> &statements);
    void analyzeExpression(ExpressionNode *expr);
    void analyzeFunctionBody(const std::vector<FunctionDeclarationNode::Parameter> &params,
                             const std::vector<std::unique_ptr<StatementNode>> &body,
                             bool isMethod);

    [[noreturn]] void fail(const ASTNode *node, const std::string &message) const;
};

#endif // SEMANTIC_H
