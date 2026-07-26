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

void SemanticAnalyzer::declare(const std::string &name, bool isConst, const std::string &type)
{
    if (m_scopes.empty()) pushScope();
    m_scopes.back()[name] = Binding{isConst, type};
}

const SemanticAnalyzer::Binding *SemanticAnalyzer::lookup(const std::string &name) const
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    // Module-level variables remain in scope inside functions
    auto global = m_globals.find(name);
    if (global != m_globals.end()) return &global->second;
    return nullptr;
}

void SemanticAnalyzer::hoistDeclarations(const std::vector<std::unique_ptr<StatementNode>> &statements)
{
    for (const auto &stmt : statements) {
        if (auto *funcDecl = dynamic_cast<FunctionDeclarationNode*>(stmt.get())) {
            FunctionSignature signature;
            for (const auto &param : funcDecl->parameters) {
                signature.parameterTypes.push_back(param.type);
            }
            // A generic function's parameter types are its type variables, which
            // say nothing concrete — drop them so calls are not policed wrongly.
            if (!funcDecl->genericParams.empty()) {
                signature.parameterTypes.assign(funcDecl->parameters.size(), "");
                signature.returnType = "";
            } else {
                signature.returnType = funcDecl->returnType;
            }
            m_functions[funcDecl->functionName] = signature;
        } else if (auto *externDecl = dynamic_cast<ExternDeclarationNode*>(stmt.get())) {
            // Foreign functions are arity- and type-checked just like local ones
            FunctionSignature signature;
            for (const auto &param : externDecl->parameters) {
                signature.parameterTypes.push_back(param.type);
            }
            signature.returnType = externDecl->returnType;
            m_functions[externDecl->functionName] = signature;
        } else if (auto *classDecl = dynamic_cast<ClassDeclarationNode*>(stmt.get())) {
            m_types.insert(classDecl->className);
            if (!classDecl->parentClass.empty()) {
                m_classParents[classDecl->className] = classDecl->parentClass;
            }
            auto &fields = m_classFields[classDecl->className];
            for (const auto &prop : classDecl->objectTemplate->properties) {
                if (!prop.method && !prop.declaredType.empty()) {
                    fields[prop.key] = prop.declaredType;
                }
            }
        } else if (auto *interfaceDecl = dynamic_cast<InterfaceDeclarationNode*>(stmt.get())) {
            m_types.insert(interfaceDecl->interfaceName);
        } else if (auto *varDecl = dynamic_cast<VariableDeclarationNode*>(stmt.get())) {
            // Module-level variables are visible inside function bodies
            m_globals[varDecl->variableName] =
                Binding{varDecl->isConst, varDecl->typeName == "auto" ? "" : varDecl->typeName};
        } else if (auto *destructure = dynamic_cast<DestructuringDeclarationNode*>(stmt.get())) {
            for (const auto &name : destructure->bindings) {
                m_globals[name] = Binding{destructure->isConst, ""};
            }
        }
    }
}


// =============================================================================
// Types
// =============================================================================

std::string SemanticAnalyzer::elementTypeOf(const std::string &arrayType)
{
    if (arrayType.size() > 2 && arrayType.compare(arrayType.size() - 2, 2, "[]") == 0) {
        return arrayType.substr(0, arrayType.size() - 2);
    }
    return "";
}

SemanticAnalyzer::TypeCategory SemanticAnalyzer::categoryOf(const std::string &type) const
{
    if (type.empty() || type == "auto") return TypeCategory::Unknown;

    // A single upper-case letter is a generic parameter (T, K, V) — it could be
    // anything, so treat it as unknown rather than as a handle. Unless it is the
    // name of a real class or interface, which a one-letter name may well be.
    if (type.size() == 1 && type[0] >= 'A' && type[0] <= 'Z' && !m_types.count(type)) {
        return TypeCategory::Unknown;
    }

    if (type == "i32" || type == "i64" || type == "i8" || type == "u8" ||
        type == "f32" || type == "f64" || type == "number" || type == "boolean") {
        return TypeCategory::Numeric;
    }
    if (type == "string") return TypeCategory::Text;
    if (type == "void") return TypeCategory::Void;

    // json is dynamic: its members can be anything, so don't police it
    if (type == "json") return TypeCategory::Unknown;

    // ptr, object, class and interface names, arrays, Map/Set, closures
    return TypeCategory::Handle;
}

bool SemanticAnalyzer::isAssignable(const std::string &target, const std::string &source) const
{
    if (target.empty() || source.empty()) return true;   // unknown: stay quiet
    if (target == source) return true;

    // null fits anywhere a pointer does
    if (source == "null") {
        TypeCategory ct = categoryOf(target);
        return ct != TypeCategory::Numeric && ct != TypeCategory::Void;
    }

    TypeCategory ct = categoryOf(target);
    TypeCategory cs = categoryOf(source);
    if (ct == TypeCategory::Unknown || cs == TypeCategory::Unknown) return true;

    // Within a family codegen already coerces: i32<->f64, boolean as i32, and
    // string/ptr are both i8*. Only crossing families is a real mistake.
    if (ct == cs) return true;
    if (ct == TypeCategory::Text && cs == TypeCategory::Handle) return true;
    if (ct == TypeCategory::Handle && cs == TypeCategory::Text) return true;

    return false;
}

void SemanticAnalyzer::checkAssignable(const ASTNode *node, const std::string &target,
                                       const std::string &source, const std::string &context)
{
    if (isAssignable(target, source)) return;
    fail(node, "Type mismatch " + context + ": expected '" + target +
               "', got '" + source + "'");
}

std::string SemanticAnalyzer::typeOf(ExpressionNode *expr)
{
    if (!expr) return "";

    if (dynamic_cast<IntegerLiteralNode*>(expr))  return "i32";
    if (dynamic_cast<FloatLiteralNode*>(expr))    return "f64";
    if (dynamic_cast<StringLiteralNode*>(expr))   return "string";
    if (dynamic_cast<BooleanLiteralNode*>(expr))  return "boolean";
    if (dynamic_cast<NullLiteralNode*>(expr))     return "null";

    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(expr)) {
        if (varExpr->name == "this") return "";
        const Binding *binding = lookup(varExpr->name);
        return binding ? binding->type : "";
    }
    if (auto *newExpr = dynamic_cast<NewExpressionNode*>(expr)) {
        // Map/Set and other generics stay unknown; a class instance is its class
        if (m_types.count(newExpr->className)) return newExpr->className;
        return "";
    }
    if (auto *call = dynamic_cast<FunctionCallNode*>(expr)) {
        auto it = m_functions.find(call->functionName);
        if (it != m_functions.end()) return it->second.returnType;
        return "";
    }
    if (auto *unary = dynamic_cast<UnaryExpressionNode*>(expr)) {
        if (unary->op == UnaryExpressionNode::NOT) return "boolean";
        return typeOf(unary->operand.get());
    }
    if (auto *binary = dynamic_cast<BinaryExpressionNode*>(expr)) {
        switch (binary->op) {
            case BinaryExpressionNode::EQUAL:
            case BinaryExpressionNode::NOT_EQUAL:
            case BinaryExpressionNode::LESS_THAN:
            case BinaryExpressionNode::LESS_EQUAL:
            case BinaryExpressionNode::GREATER_THAN:
            case BinaryExpressionNode::GREATER_EQUAL:
                return "boolean";
            case BinaryExpressionNode::LOGICAL_AND:
            case BinaryExpressionNode::LOGICAL_OR:
                // Value-returning: `a || b` yields one of the operands
                return "";
            case BinaryExpressionNode::BIT_AND:
            case BinaryExpressionNode::BIT_OR:
            case BinaryExpressionNode::BIT_XOR:
            case BinaryExpressionNode::SHIFT_LEFT:
            case BinaryExpressionNode::SHIFT_RIGHT:
                return "i32";
            default: break;
        }
        std::string left = typeOf(binary->left.get());
        std::string right = typeOf(binary->right.get());
        if (binary->op == BinaryExpressionNode::ADD &&
            (left == "string" || right == "string")) {
            return "string";   // concatenation
        }
        if (left == "f64" || right == "f64" || left == "number" || right == "number") {
            return "f64";
        }
        if (left == "i32" && right == "i32") return "i32";
        return "";
    }
    if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(expr)) {
        if (!arrLit->elementType.empty()) return arrLit->elementType + "[]";
        return "";
    }
    if (auto *arrAccess = dynamic_cast<ArrayAccessNode*>(expr)) {
        return elementTypeOf(typeOf(arrAccess->array.get()));
    }
    if (auto *objAccess = dynamic_cast<ObjectAccessNode*>(expr)) {
        if (objAccess->property == "length") return "i32";
        // A class field's declared type is known; anything else is not
        std::string objectType = typeOf(objAccess->object.get());
        auto classIt = m_classFields.find(objectType);
        if (classIt != m_classFields.end()) {
            auto fieldIt = classIt->second.find(objAccess->property);
            if (fieldIt != classIt->second.end()) return fieldIt->second;
        }
        return "";
    }

    // Method calls, object literals, arrows: not modelled yet
    return "";
}

// A subclass can use everything its ancestors declare, so fold their fields in
// once the whole file has been hoisted (a class may extend one declared later).
static void foldInheritedFields(
    const std::string &className,
    const std::map<std::string, std::string> &parents,
    std::map<std::string, std::map<std::string, std::string>> &fields,
    std::set<std::string> &visiting)
{
    auto parentIt = parents.find(className);
    if (parentIt == parents.end()) return;
    if (!visiting.insert(className).second) return;   // cycle: leave it to codegen

    foldInheritedFields(parentIt->second, parents, fields, visiting);

    const auto &parentFields = fields[parentIt->second];
    auto &ownFields = fields[className];
    for (const auto &field : parentFields) {
        ownFields.insert(field);   // a redeclared field keeps the subclass's type
    }
    visiting.erase(className);
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
        if (fnIt != m_functions.end() &&
            call->arguments.size() != fnIt->second.parameterTypes.size()) {
            fail(call, "Function '" + call->functionName + "' expects " +
                       std::to_string(fnIt->second.parameterTypes.size()) + " argument(s), got " +
                       std::to_string(call->arguments.size()));
        }
        // Unknown names may be closure variables or external C++ functions
        for (const auto &arg : call->arguments) analyzeExpression(arg.get());

        // Each argument must fit its declared parameter type
        if (fnIt != m_functions.end()) {
            const auto &parameterTypes = fnIt->second.parameterTypes;
            for (size_t i = 0; i < call->arguments.size() && i < parameterTypes.size(); ++i) {
                checkAssignable(call->arguments[i].get(), parameterTypes[i],
                                typeOf(call->arguments[i].get()),
                                "in argument " + std::to_string(i + 1) + " of '" +
                                call->functionName + "'");
            }
        }
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
                analyzeFunctionBody(prop.method->parameters, prop.method->bodyStatements, true,
                                    prop.method->returnType);
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
    const std::vector<std::unique_ptr<StatementNode>> &body, bool isMethod,
    const std::string &returnType)
{
    // Function/method bodies do NOT see enclosing local scopes
    std::vector<std::map<std::string, Binding>> savedScopes;
    savedScopes.swap(m_scopes);
    pushScope();

    bool savedInMethod = m_inMethod;
    int savedLoopDepth = m_loopDepth;
    int savedSwitchDepth = m_switchDepth;
    std::string savedReturnType = m_currentReturnType;
    bool savedInFunction = m_inFunction;
    m_inMethod = isMethod;
    m_loopDepth = 0;
    m_switchDepth = 0;
    m_currentReturnType = returnType;
    m_inFunction = true;

    for (const auto &param : params) declare(param.name, false, param.type);
    analyzeStatementList(body);

    m_inMethod = savedInMethod;
    m_loopDepth = savedLoopDepth;
    m_switchDepth = savedSwitchDepth;
    m_currentReturnType = savedReturnType;
    m_inFunction = savedInFunction;
    m_scopes.swap(savedScopes);
}

void SemanticAnalyzer::analyzeStatement(StatementNode *stmt)
{
    if (!stmt) return;

    if (auto *varDecl = dynamic_cast<VariableDeclarationNode*>(stmt)) {
        analyzeExpression(varDecl->initializer.get());
        std::string declaredType = (varDecl->typeName == "auto") ? "" : varDecl->typeName;
        std::string initializerType = typeOf(varDecl->initializer.get());
        if (!declaredType.empty() && varDecl->initializer) {
            checkAssignable(varDecl->initializer.get(), declaredType, initializerType,
                            "in declaration of '" + varDecl->variableName + "'");
        }
        // Without an annotation the initializer's type is the variable's type
        declare(varDecl->variableName, varDecl->isConst,
                declaredType.empty() ? initializerType : declaredType);
    } else if (auto *destruct = dynamic_cast<DestructuringDeclarationNode*>(stmt)) {
        analyzeExpression(destruct->initializer.get());
        for (const auto &name : destruct->bindings) declare(name, destruct->isConst, "");
    } else if (auto *assign = dynamic_cast<AssignmentStatementNode*>(stmt)) {
        const Binding *binding = lookup(assign->variableName);
        if (!binding) {
            fail(assign, "Assignment to undefined variable '" + assign->variableName + "'");
        }
        if (binding->isConst) {
            fail(assign, "Cannot reassign const variable '" + assign->variableName + "'");
        }
        analyzeExpression(assign->value.get());
        checkAssignable(assign->value.get(), binding->type, typeOf(assign->value.get()),
                        "in assignment to '" + assign->variableName + "'");
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
        if (m_inFunction && retStmt->expression && m_currentReturnType != "void") {
            checkAssignable(retStmt->expression.get(), m_currentReturnType,
                            typeOf(retStmt->expression.get()), "in return value");
        }
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
        // A generic function's annotations are type variables, so nothing to check
        std::string returnType = funcDecl->genericParams.empty() ? funcDecl->returnType : "";
        analyzeFunctionBody(funcDecl->parameters, funcDecl->bodyStatements, false, returnType);
    } else if (auto *classDecl = dynamic_cast<ClassDeclarationNode*>(stmt)) {
        m_types.insert(classDecl->className);
        for (const auto &prop : classDecl->objectTemplate->properties) {
            if (prop.method) {
                analyzeFunctionBody(prop.method->parameters, prop.method->bodyStatements, true,
                                    prop.method->returnType);
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
    m_globals.clear();
    m_classFields.clear();
    m_classParents.clear();
    m_currentReturnType.clear();
    m_inFunction = false;
    m_loopDepth = 0;
    m_switchDepth = 0;
    m_inMethod = false;

    pushScope(); // global scope
    hoistDeclarations(program->statements);

    for (const auto &entry : m_classParents) {
        std::set<std::string> visiting;
        foldInheritedFields(entry.first, m_classParents, m_classFields, visiting);
    }
    analyzeStatementList(program->statements);
    popScope();
}
