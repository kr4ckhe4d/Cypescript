// src/Optimizer.h - AST-level optimizations (Phase 3 roadmap)
// Constant folding and dead-branch elimination run before LLVM codegen,
// complementing the -O2 passes applied at native compile time.
#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "AST.h"
#include <memory>
#include <vector>

class ASTOptimizer
{
public:
    struct Stats {
        int foldedExpressions = 0;
        int eliminatedBranches = 0;
    };

    // Runs all AST optimizations in place and returns what was done
    Stats optimize(ProgramNode *program);

private:
    Stats m_stats;

    // Recursively folds constants inside an expression; may replace the node
    void optimizeExpression(std::unique_ptr<ExpressionNode> &expr);
    // Recurses into a statement's expressions and nested statement lists
    void optimizeStatement(StatementNode *stmt);
    // Optimizes a statement list, eliminating dead if/while branches
    void optimizeStatementList(std::vector<std::unique_ptr<StatementNode>> &stmts);

    // Returns true and sets `value` if the expression is a literal with a known truthiness
    bool literalTruthiness(ExpressionNode *expr, bool &value);
};

#endif // OPTIMIZER_H
