// src/Optimizer.cpp - AST-level constant folding and dead-branch elimination
#include "Optimizer.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace {

// Matches cyps_f64_to_string in the runtime so folded strings equal runtime output
std::string formatDouble(double value)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}

bool isNumericLiteral(ExpressionNode *expr, double &out, bool &isFloat)
{
    if (auto *intLit = dynamic_cast<IntegerLiteralNode *>(expr)) {
        out = static_cast<double>(intLit->value);
        isFloat = false;
        return true;
    }
    if (auto *floatLit = dynamic_cast<FloatLiteralNode *>(expr)) {
        out = floatLit->value;
        isFloat = true;
        return true;
    }
    if (auto *boolLit = dynamic_cast<BooleanLiteralNode *>(expr)) {
        out = boolLit->value ? 1.0 : 0.0;
        isFloat = false;
        return true;
    }
    return false;
}

// Renders a literal as it would appear after string concatenation at runtime
bool literalAsString(ExpressionNode *expr, std::string &out)
{
    if (auto *strLit = dynamic_cast<StringLiteralNode *>(expr)) {
        out = strLit->value;
        return true;
    }
    if (auto *intLit = dynamic_cast<IntegerLiteralNode *>(expr)) {
        out = std::to_string(intLit->value);
        return true;
    }
    if (auto *floatLit = dynamic_cast<FloatLiteralNode *>(expr)) {
        out = formatDouble(floatLit->value);
        return true;
    }
    if (auto *boolLit = dynamic_cast<BooleanLiteralNode *>(expr)) {
        out = boolLit->value ? "1" : "0";
        return true;
    }
    return false;
}

} // namespace

bool ASTOptimizer::literalTruthiness(ExpressionNode *expr, bool &value)
{
    if (auto *boolLit = dynamic_cast<BooleanLiteralNode *>(expr)) {
        value = boolLit->value;
        return true;
    }
    if (auto *intLit = dynamic_cast<IntegerLiteralNode *>(expr)) {
        value = intLit->value != 0;
        return true;
    }
    if (auto *floatLit = dynamic_cast<FloatLiteralNode *>(expr)) {
        value = floatLit->value != 0.0;
        return true;
    }
    if (dynamic_cast<StringLiteralNode *>(expr)) {
        value = true; // non-null pointer
        return true;
    }
    return false;
}

void ASTOptimizer::optimizeExpression(std::unique_ptr<ExpressionNode> &expr)
{
    if (!expr) return;

    if (auto *binOp = dynamic_cast<BinaryExpressionNode *>(expr.get())) {
        optimizeExpression(binOp->left);
        optimizeExpression(binOp->right);

        ExpressionNode *left = binOp->left.get();
        ExpressionNode *right = binOp->right.get();

        // Short-circuit folding when the left side's truthiness is known
        if (binOp->op == BinaryExpressionNode::LOGICAL_AND ||
            binOp->op == BinaryExpressionNode::LOGICAL_OR) {
            bool truthy;
            if (literalTruthiness(left, truthy)) {
                bool takeLeft = (binOp->op == BinaryExpressionNode::LOGICAL_OR) ? truthy : !truthy;
                expr = takeLeft ? std::move(binOp->left) : std::move(binOp->right);
                m_stats.foldedExpressions++;
            }
            return;
        }

        // String concatenation folding
        if (binOp->op == BinaryExpressionNode::ADD &&
            (dynamic_cast<StringLiteralNode *>(left) || dynamic_cast<StringLiteralNode *>(right))) {
            std::string lhs, rhs;
            if (literalAsString(left, lhs) && literalAsString(right, rhs)) {
                expr = std::make_unique<StringLiteralNode>(lhs + rhs);
                m_stats.foldedExpressions++;
            }
            return;
        }

        // Numeric folding
        double lval, rval;
        bool lFloat, rFloat;
        if (!isNumericLiteral(left, lval, lFloat) || !isNumericLiteral(right, rval, rFloat)) {
            return;
        }
        bool isFloat = lFloat || rFloat;

        auto makeNumber = [&](double v) -> std::unique_ptr<ExpressionNode> {
            m_stats.foldedExpressions++;
            if (isFloat) return std::make_unique<FloatLiteralNode>(v);
            return std::make_unique<IntegerLiteralNode>(static_cast<long long>(v));
        };
        auto makeBool = [&](bool v) -> std::unique_ptr<ExpressionNode> {
            m_stats.foldedExpressions++;
            return std::make_unique<BooleanLiteralNode>(v);
        };

        switch (binOp->op) {
            case BinaryExpressionNode::ADD: expr = makeNumber(lval + rval); break;
            case BinaryExpressionNode::SUBTRACT: expr = makeNumber(lval - rval); break;
            case BinaryExpressionNode::MULTIPLY: expr = makeNumber(lval * rval); break;
            case BinaryExpressionNode::DIVIDE:
                if (rval == 0.0) return; // leave division by zero for runtime
                if (isFloat) expr = makeNumber(lval / rval);
                else expr = makeNumber(static_cast<double>(
                    static_cast<long long>(lval) / static_cast<long long>(rval)));
                break;
            case BinaryExpressionNode::MODULO:
                if (rval == 0.0) return;
                if (isFloat) expr = makeNumber(std::fmod(lval, rval));
                else expr = makeNumber(static_cast<double>(
                    static_cast<long long>(lval) % static_cast<long long>(rval)));
                break;
            case BinaryExpressionNode::EQUAL: expr = makeBool(lval == rval); break;
            case BinaryExpressionNode::NOT_EQUAL: expr = makeBool(lval != rval); break;
            case BinaryExpressionNode::LESS_THAN: expr = makeBool(lval < rval); break;
            case BinaryExpressionNode::LESS_EQUAL: expr = makeBool(lval <= rval); break;
            case BinaryExpressionNode::GREATER_THAN: expr = makeBool(lval > rval); break;
            case BinaryExpressionNode::GREATER_EQUAL: expr = makeBool(lval >= rval); break;
            default: break;
        }
        return;
    }

    if (auto *unaryOp = dynamic_cast<UnaryExpressionNode *>(expr.get())) {
        optimizeExpression(unaryOp->operand);
        ExpressionNode *operand = unaryOp->operand.get();
        if (unaryOp->op == UnaryExpressionNode::MINUS) {
            if (auto *intLit = dynamic_cast<IntegerLiteralNode *>(operand)) {
                expr = std::make_unique<IntegerLiteralNode>(-intLit->value);
                m_stats.foldedExpressions++;
            } else if (auto *floatLit = dynamic_cast<FloatLiteralNode *>(operand)) {
                expr = std::make_unique<FloatLiteralNode>(-floatLit->value);
                m_stats.foldedExpressions++;
            }
        } else if (unaryOp->op == UnaryExpressionNode::NOT) {
            bool truthy;
            if (literalTruthiness(operand, truthy)) {
                expr = std::make_unique<BooleanLiteralNode>(!truthy);
                m_stats.foldedExpressions++;
            }
        }
        return;
    }

    if (auto *funcCall = dynamic_cast<FunctionCallNode *>(expr.get())) {
        for (auto &arg : funcCall->arguments) optimizeExpression(arg);
        return;
    }
    if (auto *methodCall = dynamic_cast<MethodCallNode *>(expr.get())) {
        optimizeExpression(methodCall->object);
        for (auto &arg : methodCall->arguments) optimizeExpression(arg);
        return;
    }
    if (auto *newExpr = dynamic_cast<NewExpressionNode *>(expr.get())) {
        for (auto &arg : newExpr->arguments) optimizeExpression(arg);
        return;
    }
    if (auto *arrLit = dynamic_cast<ArrayLiteralNode *>(expr.get())) {
        for (auto &element : arrLit->elements) optimizeExpression(element);
        return;
    }
    if (auto *objLit = dynamic_cast<ObjectLiteralNode *>(expr.get())) {
        for (auto &prop : objLit->properties) {
            if (prop.value) optimizeExpression(prop.value);
            if (prop.method) optimizeStatementList(prop.method->bodyStatements);
        }
        return;
    }
    if (auto *arrAccess = dynamic_cast<ArrayAccessNode *>(expr.get())) {
        optimizeExpression(arrAccess->array);
        optimizeExpression(arrAccess->index);
        return;
    }
    if (auto *objAccess = dynamic_cast<ObjectAccessNode *>(expr.get())) {
        optimizeExpression(objAccess->object);
        return;
    }
}

void ASTOptimizer::optimizeStatement(StatementNode *stmt)
{
    if (!stmt) return;

    if (auto *varDecl = dynamic_cast<VariableDeclarationNode *>(stmt)) {
        optimizeExpression(varDecl->initializer);
    } else if (auto *assign = dynamic_cast<AssignmentStatementNode *>(stmt)) {
        optimizeExpression(assign->value);
    } else if (auto *arrAssign = dynamic_cast<ArrayAssignmentStatementNode *>(stmt)) {
        optimizeExpression(arrAssign->array);
        optimizeExpression(arrAssign->index);
        optimizeExpression(arrAssign->value);
    } else if (auto *propAssign = dynamic_cast<ObjectPropertyAssignmentNode *>(stmt)) {
        optimizeExpression(propAssign->object);
        optimizeExpression(propAssign->value);
    } else if (auto *exprStmt = dynamic_cast<ExpressionStatementNode *>(stmt)) {
        optimizeExpression(exprStmt->expression);
    } else if (auto *ifStmt = dynamic_cast<IfStatementNode *>(stmt)) {
        optimizeExpression(ifStmt->condition);
        optimizeStatementList(ifStmt->thenStatements);
        optimizeStatementList(ifStmt->elseStatements);
    } else if (auto *whileStmt = dynamic_cast<WhileStatementNode *>(stmt)) {
        optimizeExpression(whileStmt->condition);
        optimizeStatementList(whileStmt->bodyStatements);
    } else if (auto *forStmt = dynamic_cast<ForStatementNode *>(stmt)) {
        if (forStmt->initialization) optimizeStatement(forStmt->initialization.get());
        optimizeExpression(forStmt->condition);
        if (forStmt->increment) optimizeStatement(forStmt->increment.get());
        optimizeStatementList(forStmt->bodyStatements);
    } else if (auto *forOfStmt = dynamic_cast<ForOfStatementNode *>(stmt)) {
        optimizeExpression(forOfStmt->iterable);
        optimizeStatementList(forOfStmt->bodyStatements);
    } else if (auto *doWhileStmt = dynamic_cast<DoWhileStatementNode *>(stmt)) {
        optimizeExpression(doWhileStmt->condition);
        optimizeStatementList(doWhileStmt->bodyStatements);
    } else if (auto *funcDecl = dynamic_cast<FunctionDeclarationNode *>(stmt)) {
        optimizeStatementList(funcDecl->bodyStatements);
    } else if (auto *retStmt = dynamic_cast<ReturnStatementNode *>(stmt)) {
        optimizeExpression(retStmt->expression);
    } else if (auto *throwStmt = dynamic_cast<ThrowStatementNode *>(stmt)) {
        optimizeExpression(throwStmt->expression);
    } else if (auto *tryStmt = dynamic_cast<TryCatchStatementNode *>(stmt)) {
        optimizeStatementList(tryStmt->tryStatements);
        optimizeStatementList(tryStmt->catchStatements);
        optimizeStatementList(tryStmt->finallyStatements);
    } else if (auto *switchStmt = dynamic_cast<SwitchStatementNode *>(stmt)) {
        optimizeExpression(switchStmt->condition);
        for (auto &clause : switchStmt->cases) {
            optimizeExpression(clause.value);
            optimizeStatementList(clause.statements);
        }
    } else if (auto *destruct = dynamic_cast<DestructuringDeclarationNode *>(stmt)) {
        optimizeExpression(destruct->initializer);
    }
}

void ASTOptimizer::optimizeStatementList(std::vector<std::unique_ptr<StatementNode>> &stmts)
{
    for (size_t i = 0; i < stmts.size(); ++i) {
        optimizeStatement(stmts[i].get());

        // Dead-branch elimination: if (constant) { ... } else { ... }
        if (auto *ifStmt = dynamic_cast<IfStatementNode *>(stmts[i].get())) {
            bool condValue;
            if (ifStmt->condition && literalTruthiness(ifStmt->condition.get(), condValue)) {
                auto &liveBranch = condValue ? ifStmt->thenStatements : ifStmt->elseStatements;
                std::vector<std::unique_ptr<StatementNode>> replacement = std::move(liveBranch);
                stmts.erase(stmts.begin() + i);
                for (size_t j = 0; j < replacement.size(); ++j) {
                    stmts.insert(stmts.begin() + i + j, std::move(replacement[j]));
                }
                m_stats.eliminatedBranches++;
                i--; // re-visit the spliced statements
                continue;
            }
        }

        // while (false) { ... } never runs
        if (auto *whileStmt = dynamic_cast<WhileStatementNode *>(stmts[i].get())) {
            bool condValue;
            if (whileStmt->condition && literalTruthiness(whileStmt->condition.get(), condValue) && !condValue) {
                stmts.erase(stmts.begin() + i);
                m_stats.eliminatedBranches++;
                i--;
                continue;
            }
        }
    }
}

ASTOptimizer::Stats ASTOptimizer::optimize(ProgramNode *program)
{
    m_stats = Stats();
    if (program) {
        optimizeStatementList(program->statements);
    }
    return m_stats;
}
