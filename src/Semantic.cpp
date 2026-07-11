// src/Semantic.cpp - Semantic analysis pass
#include "Semantic.h"

#include <stdexcept>

void SemanticAnalyzer::fail(const ASTNode *node, const std::string &message) const
{
    std::string position;
    if (node && node->line > 0) {
        position = " at line " + std::to_string(node->line) +
                   ", column " + std::to_string(node->column);
    }
    throw std::runtime_error("Semantic Error: " + message + position);
}

void SemanticAnalyzer::pushScope() { m_scopes.emplace_back(); }
void SemanticAnalyzer::popScope() { m_scopes.pop_back(); }

void SemanticAnalyzer::declare(const std::string &name, bool isConst)
{
    m_scopes.back()[name] = Binding{isConst};
}

const SemanticAnalyzer::Binding *SemanticAnalyzer::lookup(const std::string &name) const
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    return nullptr;
}

void SemanticAnalyzer::hoistDeclarations(const std::vector<std::unique_ptr<StatementNode>> &statements)
{
    for (const auto &stmt : statements) {
        if (auto *funcDecl = dynamic_cast<FunctionDeclarationNode*>(stmt.get())) {
            m_functions[funcDecl->functionName] = funcDecl->parameters.size();
        } else if (auto *classDecl = dynamic_cast<ClassDeclarationNode*>(stmt.get())) {
            m_types.insert(classDecl->className);
        } else if (auto *interfaceDecl = dynamic_cast<InterfaceDeclarationNode*>(stmt.get())) {
            m_types.insert(interfaceDecl->interfaceName);
        }
    }
}

void SemanticAnalyzer::analyzeExpression(ExpressionNode *expr)
{
    if (!expr) return;

    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(expr)) {
        if (varExpr->name == "this") {
            if (!m_inMethod) fail(varExpr, "'this' can only be used inside a method");
            return;
        }
        if (!lookup(varExpr->name) && !m_functions.count(varExpr->name)) {
            fail(varExpr, "Use of undefined variable '" + varExpr->name + "'");
        }
    } else if (auto *binOp = dynamic_cast<BinaryExpressionNode*>(expr)) {
        analyzeExpression(binOp->left.get());
        analyzeExpression(binOp->right.get());
    } else if (auto *unaryOp = dynamic_cast<UnaryExpressionNode*>(expr)) {
        analyzeExpression(unaryOp->operand.get());
    } else if (auto *call = dynamic_cast<FunctionCallNode*>(expr)) {
        auto fnIt = m_functions.find(call->functionName);
        if (fnIt != m_functions.end() && call->arguments.size() != fnIt->second) {
            fail(call, "Function '" + call->functionName + "' expects " +
                       std::to_string(fnIt->second) + " argument(s), got " +
                       std::to_string(call->arguments.size()));
        }
        // Unknown names may be closure variables or external C++ functions
        for (const auto &arg : call->arguments) analyzeExpression(arg.get());
    } else if (auto *method = dynamic_cast<MethodCallNode*>(expr)) {
        analyzeExpression(method->object.get());
        for (const auto &arg : method->arguments) analyzeExpression(arg.get());
    } else if (auto *arrAccess = dynamic_cast<ArrayAccessNode*>(expr)) {
        analyzeExpression(arrAccess->array.get());
        analyzeExpression(arrAccess->index.get());
    } else if (auto *objAccess = dynamic_cast<ObjectAccessNode*>(expr)) {
        analyzeExpression(objAccess->object.get());
    } else if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(expr)) {
        for (const auto &element : arrLit->elements) analyzeExpression(element.get());
    } else if (auto *objLit = dynamic_cast<ObjectLiteralNode*>(expr)) {
        for (const auto &prop : objLit->properties) {
            if (prop.method) {
                analyzeFunctionBody(prop.method->parameters, prop.method->bodyStatements, true);
            } else {
                analyzeExpression(prop.value.get());
            }
        }
    } else if (auto *newExpr = dynamic_cast<NewExpressionNode*>(expr)) {
        for (const auto &arg : newExpr->arguments) analyzeExpression(arg.get());
    } else if (auto *arrowFn = dynamic_cast<ArrowFunctionNode*>(expr)) {
        // Arrows capture the enclosing scope, so analyze inside the current scopes
        pushScope();
        for (const auto &param : arrowFn->parameters) declare(param.name, false);
        int savedLoopDepth = m_loopDepth;
        int savedSwitchDepth = m_switchDepth;
        m_loopDepth = 0;
        m_switchDepth = 0;
        analyzeStatementList(arrowFn->bodyStatements);
        m_loopDepth = savedLoopDepth;
        m_switchDepth = savedSwitchDepth;
        popScope();
    }
}

void SemanticAnalyzer::analyzeFunctionBody(
    const std::vector<FunctionDeclarationNode::Parameter> &params,
    const std::vector<std::unique_ptr<StatementNode>> &body, bool isMethod)
{
    // Function/method bodies do NOT see enclosing local scopes
    std::vector<std::map<std::string, Binding>> savedScopes;
    savedScopes.swap(m_scopes);
    pushScope();

    bool savedInMethod = m_inMethod;
    int savedLoopDepth = m_loopDepth;
    int savedSwitchDepth = m_switchDepth;
    m_inMethod = isMethod;
    m_loopDepth = 0;
    m_switchDepth = 0;

    for (const auto &param : params) declare(param.name, false);
    analyzeStatementList(body);

    m_inMethod = savedInMethod;
    m_loopDepth = savedLoopDepth;
    m_switchDepth = savedSwitchDepth;
    m_scopes.swap(savedScopes);
}

void SemanticAnalyzer::analyzeStatement(StatementNode *stmt)
{
    if (!stmt) return;

    if (auto *varDecl = dynamic_cast<VariableDeclarationNode*>(stmt)) {
        analyzeExpression(varDecl->initializer.get());
        declare(varDecl->variableName, varDecl->isConst);
    } else if (auto *destruct = dynamic_cast<DestructuringDeclarationNode*>(stmt)) {
        analyzeExpression(destruct->initializer.get());
        for (const auto &name : destruct->bindings) declare(name, destruct->isConst);
    } else if (auto *assign = dynamic_cast<AssignmentStatementNode*>(stmt)) {
        const Binding *binding = lookup(assign->variableName);
        if (!binding) {
            fail(assign, "Assignment to undefined variable '" + assign->variableName + "'");
        }
        if (binding->isConst) {
            fail(assign, "Cannot reassign const variable '" + assign->variableName + "'");
        }
        analyzeExpression(assign->value.get());
    } else if (auto *arrAssign = dynamic_cast<ArrayAssignmentStatementNode*>(stmt)) {
        analyzeExpression(arrAssign->array.get());
        analyzeExpression(arrAssign->index.get());
        analyzeExpression(arrAssign->value.get());
    } else if (auto *propAssign = dynamic_cast<ObjectPropertyAssignmentNode*>(stmt)) {
        analyzeExpression(propAssign->object.get());
        analyzeExpression(propAssign->value.get());
    } else if (auto *exprStmt = dynamic_cast<ExpressionStatementNode*>(stmt)) {
        analyzeExpression(exprStmt->expression.get());
    } else if (auto *ifStmt = dynamic_cast<IfStatementNode*>(stmt)) {
        analyzeExpression(ifStmt->condition.get());
        pushScope();
        analyzeStatementList(ifStmt->thenStatements);
        popScope();
        pushScope();
        analyzeStatementList(ifStmt->elseStatements);
        popScope();
    } else if (auto *whileStmt = dynamic_cast<WhileStatementNode*>(stmt)) {
        analyzeExpression(whileStmt->condition.get());
        pushScope();
        m_loopDepth++;
        analyzeStatementList(whileStmt->bodyStatements);
        m_loopDepth--;
        popScope();
    } else if (auto *doWhile = dynamic_cast<DoWhileStatementNode*>(stmt)) {
        pushScope();
        m_loopDepth++;
        analyzeStatementList(doWhile->bodyStatements);
        m_loopDepth--;
        popScope();
        analyzeExpression(doWhile->condition.get());
    } else if (auto *forStmt = dynamic_cast<ForStatementNode*>(stmt)) {
        pushScope();
        if (forStmt->initialization) analyzeStatement(forStmt->initialization.get());
        analyzeExpression(forStmt->condition.get());
        if (forStmt->increment) analyzeStatement(forStmt->increment.get());
        m_loopDepth++;
        analyzeStatementList(forStmt->bodyStatements);
        m_loopDepth--;
        popScope();
    } else if (auto *forOf = dynamic_cast<ForOfStatementNode*>(stmt)) {
        analyzeExpression(forOf->iterable.get());
        pushScope();
        declare(forOf->iteratorVariable->variableName, forOf->iteratorVariable->isConst);
        m_loopDepth++;
        analyzeStatementList(forOf->bodyStatements);
        m_loopDepth--;
        popScope();
    } else if (auto *switchStmt = dynamic_cast<SwitchStatementNode*>(stmt)) {
        analyzeExpression(switchStmt->condition.get());
        pushScope();
        m_switchDepth++;
        for (const auto &clause : switchStmt->cases) {
            analyzeExpression(clause.value.get());
            analyzeStatementList(clause.statements);
        }
        m_switchDepth--;
        popScope();
    } else if (auto *tryStmt = dynamic_cast<TryCatchStatementNode*>(stmt)) {
        pushScope();
        analyzeStatementList(tryStmt->tryStatements);
        popScope();
        pushScope();
        if (!tryStmt->errorVariable.empty()) declare(tryStmt->errorVariable, true);
        analyzeStatementList(tryStmt->catchStatements);
        popScope();
        pushScope();
        analyzeStatementList(tryStmt->finallyStatements);
        popScope();
    } else if (auto *retStmt = dynamic_cast<ReturnStatementNode*>(stmt)) {
        analyzeExpression(retStmt->expression.get());
    } else if (auto *throwStmt = dynamic_cast<ThrowStatementNode*>(stmt)) {
        analyzeExpression(throwStmt->expression.get());
    } else if (dynamic_cast<BreakStatementNode*>(stmt)) {
        if (m_loopDepth == 0 && m_switchDepth == 0) {
            fail(stmt, "'break' used outside of a loop or switch");
        }
    } else if (dynamic_cast<ContinueStatementNode*>(stmt)) {
        if (m_loopDepth == 0) {
            fail(stmt, "'continue' used outside of a loop");
        }
    } else if (auto *funcDecl = dynamic_cast<FunctionDeclarationNode*>(stmt)) {
        m_functions[funcDecl->functionName] = funcDecl->parameters.size();
        analyzeFunctionBody(funcDecl->parameters, funcDecl->bodyStatements, false);
    } else if (auto *classDecl = dynamic_cast<ClassDeclarationNode*>(stmt)) {
        m_types.insert(classDecl->className);
        for (const auto &prop : classDecl->objectTemplate->properties) {
            if (prop.method) {
                analyzeFunctionBody(prop.method->parameters, prop.method->bodyStatements, true);
            } else {
                analyzeExpression(prop.value.get());
            }
        }
    }
    // TypeAliasNode / InterfaceDeclarationNode: compile-time only, nothing to check
}

void SemanticAnalyzer::analyzeStatementList(const std::vector<std::unique_ptr<StatementNode>> &statements)
{
    for (const auto &stmt : statements) analyzeStatement(stmt.get());
}

void SemanticAnalyzer::analyze(ProgramNode *program)
{
    if (!program) return;
    m_scopes.clear();
    m_functions.clear();
    m_types.clear();
    m_loopDepth = 0;
    m_switchDepth = 0;
    m_inMethod = false;

    pushScope(); // global scope
    hoistDeclarations(program->statements);
    analyzeStatementList(program->statements);
    popScope();
}
