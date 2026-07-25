// src/CodeGen.cpp - Fixed for LLVM 20+ compatibility
#include "CodeGen.h"
#include "AST.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include <iostream>
#include <vector>
#include <stdexcept>

// --- Constructor ---
CodeGen::CodeGen(llvm::LLVMContext &context) : m_context(context),
                                               m_builder(context)
{
    m_module = std::make_unique<llvm::Module>("CypescriptModule", m_context);
}

llvm::Value *CodeGen::ensureI1(llvm::Value *val)
{
    if (!val) return nullptr;
    if (val->getType()->isIntegerTy(1)) return val;

    if (val->getType()->isPointerTy()) {
        return m_builder.CreateICmpNE(val, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(val->getType())), "truthy_ptr");
    } else if (val->getType()->isDoubleTy()) {
        return m_builder.CreateFCmpONE(val, llvm::ConstantFP::get(val->getType(), 0.0), "truthy_f64");
    } else {
        return m_builder.CreateICmpNE(val, llvm::ConstantInt::get(val->getType(), 0), "truthy_val");
    }
}

llvm::Value *CodeGen::coerceValue(llvm::Value *val, llvm::Type *targetType)
{
    if (!val || !targetType || val->getType() == targetType) return val;

    llvm::Type *srcType = val->getType();

    // Integer <-> integer (booleans are i1 internally, i32 everywhere else;
    // i8/i64 show up at foreign-function boundaries)
    if (srcType->isIntegerTy() && targetType->isIntegerTy()) {
        unsigned srcBits = srcType->getIntegerBitWidth();
        unsigned dstBits = targetType->getIntegerBitWidth();
        if (srcBits == dstBits) return val;
        if (dstBits == 1) {
            return m_builder.CreateICmpNE(val, llvm::ConstantInt::get(srcType, 0), "bool_trunc");
        }
        if (srcBits == 1) {
            return m_builder.CreateZExt(val, targetType, "bool_ext");
        }
        return srcBits < dstBits ? m_builder.CreateSExt(val, targetType, "int_ext")
                                 : m_builder.CreateTrunc(val, targetType, "int_trunc");
    }
    // Integer -> floating point (f32 or f64)
    if (srcType->isIntegerTy() && targetType->isFloatingPointTy()) {
        return m_builder.CreateSIToFP(val, targetType, "int_to_fp");
    }
    // Floating point -> integer
    if (srcType->isFloatingPointTy() && targetType->isIntegerTy()) {
        return m_builder.CreateFPToSI(val, targetType, "fp_to_int");
    }
    // f64 <-> f32
    if (srcType->isFloatingPointTy() && targetType->isFloatingPointTy()) {
        return srcType->isDoubleTy() ? m_builder.CreateFPTrunc(val, targetType, "f64_to_f32")
                                     : m_builder.CreateFPExt(val, targetType, "f32_to_f64");
    }
    return val;
}

llvm::Value *CodeGen::toStringValue(llvm::Value *val)
{
    if (!val) return nullptr;
    llvm::Type *type = val->getType();
    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);

    if (type->isPointerTy()) {
        return val; // already a string pointer
    }
    if (type->isDoubleTy()) {
        llvm::FunctionCallee toStr = m_module->getOrInsertFunction("cyps_f64_to_string",
            charPtr, llvm::Type::getDoubleTy(m_context));
        return m_builder.CreateCall(toStr, {val}, "f64_str");
    }
    // Integers (including i1 booleans)
    if (type->isIntegerTy(1)) {
        val = m_builder.CreateZExt(val, llvm::Type::getInt32Ty(m_context), "bool_ext");
    }
    llvm::FunctionCallee toStr = m_module->getOrInsertFunction("cyps_i32_to_string",
        charPtr, llvm::Type::getInt32Ty(m_context));
    return m_builder.CreateCall(toStr, {val}, "i32_str");
}

// setjmp cannot be wrapped in a helper — its stack frame is what returns twice —
// so generated code has to call the C symbol directly. The name differs by
// platform, and must stay paired with the longjmp that cyps_throw() calls in
// cypescript_stdlib.cpp (see CYPS_LONGJMP there).
#if defined(_WIN32)
#define CYPS_SETJMP_SYMBOL "_setjmp"
#else
#define CYPS_SETJMP_SYMBOL "_setjmp"
#endif

llvm::FunctionCallee CodeGen::getOrDeclareSetjmp()
{
    llvm::Function *fn = m_module->getFunction(CYPS_SETJMP_SYMBOL);
    if (!fn) {
        llvm::FunctionType *fnType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(m_context),
            {llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0)},
            false);
        fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage,
                                    CYPS_SETJMP_SYMBOL, m_module.get());
        fn->addFnAttr(llvm::Attribute::ReturnsTwice);
    }
    return llvm::FunctionCallee(fn->getFunctionType(), fn);
}

void CodeGen::emitTryPops(int count)
{
    if (count <= 0) return;
    llvm::FunctionCallee popFn = m_module->getOrInsertFunction("cyps_try_pop",
        llvm::Type::getVoidTy(m_context));
    for (int i = 0; i < count; ++i) {
        m_builder.CreateCall(popFn, {});
    }
}

void CodeGen::branchAndSealBlock(llvm::BasicBlock *target, const std::string& deadName)
{
    m_builder.CreateBr(target);
    // Statements after break/continue/throw land in an unreachable block so
    // codegen can continue without emitting into a terminated block.
    llvm::Function *fn = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *deadBlock = llvm::BasicBlock::Create(m_context, deadName, fn);
    m_builder.SetInsertPoint(deadBlock);
}

// --- Helper Methods ---

llvm::Type *CodeGen::getLLVMType(const std::string &typeName)
{
    if (typeName == "string" || typeName == "json")
    {
        // In LLVM 20+, use PointerType::get instead of getInt8PtrTy
        return llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    }
    else if (typeName == "i32")
    {
        return llvm::Type::getInt32Ty(m_context);
    }
    else if (typeName == "f64")
    {
        return llvm::Type::getDoubleTy(m_context);
    }
    else if (typeName == "f32")
    {
        return llvm::Type::getFloatTy(m_context);
    }
    else if (typeName == "i64")
    {
        return llvm::Type::getInt64Ty(m_context);
    }
    else if (typeName == "i8" || typeName == "u8")
    {
        return llvm::Type::getInt8Ty(m_context);
    }
    else if (typeName == "ptr")
    {
        // Opaque handle to foreign memory (a Texture*, a Sound*, ...). The
        // compiler deliberately knows nothing about what it points at.
        return llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    }
    else if (typeName == "boolean")
    {
        // Booleans are represented as i32 throughout (literals, printing, C interop)
        return llvm::Type::getInt32Ty(m_context);
    }
    else if (typeName == "number")
    {
        return llvm::Type::getDoubleTy(m_context);
    }
    else if (typeName == "void")
    {
        return llvm::Type::getVoidTy(m_context);
    }
    else if (typeName == "object" || typeName == "closure" ||
             typeName.rfind("closure(", 0) == 0 ||
             interfaces.count(typeName) || classes.count(typeName))
    {
        // Objects, closures, interface- and class-typed values are opaque pointers
        return llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    }
    else if (typeName.find('<') != std::string::npos || typeName.length() == 1)
    {
        // Generic types (Map, Set, Graph, etc.) or type parameters (T, K, V) are opaque pointers
        return llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    }
    else if (typeName.length() > 2 && typeName.substr(typeName.length() - 2) == "[]")
    {
        // For dynamic arrays implemented via cypescript_stdlib, it's an opaque pointer
        return llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    }    else if (typeName == "auto")
    {
        // Type inference - for now, default to i32
        // In a full implementation, we'd analyze the initializer
        return llvm::Type::getInt32Ty(m_context);
    }
    
    std::cerr << "Codegen Error: Unknown type name '" << typeName << "'\n";
    throw std::runtime_error("Unknown type name in getLLVMType: " + typeName);
}

llvm::FunctionCallee CodeGen::getOrDeclarePuts()
{
    if (auto *func = m_module->getFunction("puts"))
    {
        return llvm::FunctionCallee(func->getFunctionType(), func);
    }
    llvm::Type *returnType = llvm::Type::getInt32Ty(m_context);
    llvm::Type *argType = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argType, false);
    llvm::Function *func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "puts", m_module.get());
    return llvm::FunctionCallee(funcType, func);
}

llvm::FunctionCallee CodeGen::getOrDeclarePrintf()
{
    if (auto *func = m_module->getFunction("printf"))
    {
        return llvm::FunctionCallee(func->getFunctionType(), func);
    }
    
    llvm::Type *returnType = llvm::Type::getInt32Ty(m_context);
    llvm::Type *formatArgType = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, formatArgType, true); // true for vararg

    llvm::Function *func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "printf", m_module.get());
    return llvm::FunctionCallee(funcType, func);
}

// --- Visitor Methods ---

void CodeGen::visit(ProgramNode *node)
{
    // Clear symbol table for each new program generation
    namedValues.clear();
    variableTypes.clear();
    arraySizes.clear();
    declaredFunctions.clear();
    interfaces.clear();
    objectMethods.clear();
    methodFunctions.clear();
    loopTargets.clear();
    loopTargetTryDepths.clear();
    tryDepth = 0;
    variableToArrow.clear();
    arrowFunctions.clear();
    arrowEnvTypes.clear();
    arrowCaptures.clear();
    arrowCounter = 0;

    classes.clear();
    externFunctions.clear();
    globalValues.clear();
    atModuleLevel = false;

    // Which module-level variables do functions reach for? Only those get promoted.
    collectFunctionReferencedNames(node);

    // Pass 0: Register interfaces, classes and foreign declarations so they are
    // usable everywhere, including above their point of declaration
    for (const auto &stmt : node->statements)
    {
        if (auto *interfaceNode = dynamic_cast<InterfaceDeclarationNode *>(stmt.get()))
        {
            interfaces[interfaceNode->interfaceName] = interfaceNode;
        }
        else if (auto *classNode = dynamic_cast<ClassDeclarationNode *>(stmt.get()))
        {
            classes[classNode->className] = classNode;
        }
        else if (auto *externNode = dynamic_cast<ExternDeclarationNode *>(stmt.get()))
        {
            externFunctions[externNode->functionName] = externNode;
        }
    }

    // Class struct layouts are computed from declared field types before any
    // code is generated, so a class-typed value knows its layout no matter
    // where it came from
    for (auto &entry : classes) {
        registerClassLayout(entry.second);
    }

    // First pass: declare every function's signature, but generate no bodies
    // yet. main is emitted before the bodies because it is what creates the
    // module-level globals those bodies may reference.
    std::vector<FunctionDeclarationNode *> functionDecls;
    for (const auto &stmt : node->statements)
    {
        if (auto *funcDeclNode = dynamic_cast<FunctionDeclarationNode *>(stmt.get()))
        {
            declareFunctionSignature(funcDeclNode);
            functionDecls.push_back(funcDeclNode);
        }
    }

    // Second pass: Create main function for non-function statements
    std::vector<StatementNode*> mainStatements;
    for (const auto &stmt : node->statements)
    {
        if (!dynamic_cast<FunctionDeclarationNode *>(stmt.get()) &&
            !dynamic_cast<InterfaceDeclarationNode *>(stmt.get()) &&
            !dynamic_cast<ClassDeclarationNode *>(stmt.get()) &&
            !dynamic_cast<ExternDeclarationNode *>(stmt.get()) &&
            !dynamic_cast<LinkDirectiveNode *>(stmt.get()))
        {
            mainStatements.push_back(stmt.get());
        }
    }

    // Only create main function if there are non-function statements
    if (!mainStatements.empty())
    {
        llvm::FunctionType *mainFuncType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), false);
        llvm::Function *mainFunc = llvm::Function::Create(
            mainFuncType, llvm::Function::ExternalLinkage, "main", m_module.get());

        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(m_context, "entry", mainFunc);
        m_builder.SetInsertPoint(entryBlock);
        
        // Set current function context for main
        currentFunction = mainFunc;

        // Process main statements. Declarations directly at this level (not
        // nested inside a block) are the ones eligible to become globals.
        for (StatementNode* stmt : mainStatements)
        {
            atModuleLevel = dynamic_cast<VariableDeclarationNode *>(stmt) != nullptr;
            visit(stmt);
        }
        atModuleLevel = false;

        m_builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));
        
        // Reset current function context
        currentFunction = nullptr;

        if (llvm::verifyFunction(*mainFunc, &llvm::errs()))
        {
            std::cerr << "Error: main function verification failed!\n";
        }
    }

    // Third pass: generate the function bodies, now that module-level globals
    // exist and their recorded types are known
    for (FunctionDeclarationNode *funcDeclNode : functionDecls)
    {
        visit(funcDeclNode);
    }
}

void CodeGen::visit(StatementNode *node)
{
    if (auto *declNode = dynamic_cast<VariableDeclarationNode *>(node))
    {
        visit(declNode);
    }
    else if (auto *funcDeclNode = dynamic_cast<FunctionDeclarationNode *>(node))
    {
        visit(funcDeclNode);
    }
    else if (auto *typeAliasNode = dynamic_cast<TypeAliasNode *>(node))
    {
        visit(typeAliasNode);
    }
    else if (auto *returnNode = dynamic_cast<ReturnStatementNode *>(node))
    {
        visit(returnNode);
    }
    else if (auto *exprStmtNode = dynamic_cast<ExpressionStatementNode *>(node))
    {
        visit(exprStmtNode);
    }
    else if (auto *ifNode = dynamic_cast<IfStatementNode *>(node))
    {
        visit(ifNode);
    }
    else if (auto *whileNode = dynamic_cast<WhileStatementNode *>(node))
    {
        visit(whileNode);
    }
    else if (auto *forNode = dynamic_cast<ForStatementNode *>(node))
    {
        visit(forNode);
    }
    else if (auto *forOfNode = dynamic_cast<ForOfStatementNode *>(node))
    {
        visit(forOfNode);
    }
    else if (auto *doWhileNode = dynamic_cast<DoWhileStatementNode *>(node))
    {
        visit(doWhileNode);
    }
    else if (auto *assignNode = dynamic_cast<AssignmentStatementNode *>(node))
    {
        visit(assignNode);
    }
    else if (auto *arrayAssignNode = dynamic_cast<ArrayAssignmentStatementNode *>(node))
    {
        visit(arrayAssignNode);
    }
    else if (auto *breakNode = dynamic_cast<BreakStatementNode *>(node))
    {
        visit(breakNode);
    }
    else if (auto *continueNode = dynamic_cast<ContinueStatementNode *>(node))
    {
        visit(continueNode);
    }
    else if (auto *switchNode = dynamic_cast<SwitchStatementNode *>(node))
    {
        visit(switchNode);
    }
    else if (auto *interfaceNode = dynamic_cast<InterfaceDeclarationNode *>(node))
    {
        visit(interfaceNode);
    }
    else if (auto *classNode = dynamic_cast<ClassDeclarationNode *>(node))
    {
        visit(classNode);
    }
    else if (auto *propAssignNode = dynamic_cast<ObjectPropertyAssignmentNode *>(node))
    {
        visit(propAssignNode);
    }
    else if (auto *destructNode = dynamic_cast<DestructuringDeclarationNode *>(node))
    {
        visit(destructNode);
    }
    else if (auto *tryNode = dynamic_cast<TryCatchStatementNode *>(node))
    {
        visit(tryNode);
    }
    else if (auto *throwNode = dynamic_cast<ThrowStatementNode *>(node))
    {
        visit(throwNode);
    }
    else if (dynamic_cast<ExternDeclarationNode *>(node) ||
             dynamic_cast<LinkDirectiveNode *>(node))
    {
        // Declarations only: externs are registered in pass 0 and emitted lazily
        // at their first call site; link directives are consumed by the driver.
    }
    else
    {
        std::cerr << "Codegen Error: Unsupported statement type.\n";
        throw std::runtime_error("Unsupported statement type in codegen.");
    }
}

void CodeGen::visit(VariableDeclarationNode *node)
{
    // Structural type check when declaring against an interface
    if (interfaces.count(node->typeName)) {
        if (auto *objLit = dynamic_cast<ObjectLiteralNode*>(node->initializer.get())) {
            checkInterfaceConformance(node->typeName, objLit, node->variableName);
        }
    }

    // An explicit array annotation drives the literal's element type
    // (e.g. let a: f64[] = [1, 2] pushes doubles, not i32s)
    if (node->typeName.length() > 2 && node->typeName.substr(node->typeName.length() - 2) == "[]") {
        if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(node->initializer.get())) {
            arrLit->elementType = node->typeName.substr(0, node->typeName.length() - 2);
        }
    }

    // Claim the module-level flag immediately: generating the initializer may
    // recurse into nested bodies (an arrow function, say), and those
    // declarations must not be mistaken for module-level ones.
    const bool isModuleLevelDecl = atModuleLevel;
    atModuleLevel = false;

    // Generate the initializer first so its value can drive type inference
    llvm::Value *initVal = nullptr;
    if (node->initializer) {
        initVal = visit(node->initializer.get());
        if (!initVal) {
            throw std::runtime_error("Codegen Error: Failed to generate initializer for variable " + node->variableName);
        }
    }

    // Determine the recorded type name for this variable
    std::string typeToStore = node->typeName;
    if (auto *objLit = dynamic_cast<ObjectLiteralNode*>(node->initializer.get())) {
        // Native object: track the layout key regardless of the declared type
        typeToStore = "object";
        variableToObjectKey[node->variableName] =
            "opt_obj_" + std::to_string(reinterpret_cast<uintptr_t>(objLit));
    } else if (auto *arrowLit = dynamic_cast<ArrowFunctionNode*>(node->initializer.get())) {
        // Closure: track the arrow node so calls through this variable bind statically
        typeToStore = "closure";
        variableToArrow[node->variableName] = arrowLit;
    } else if (dynamic_cast<NewExpressionNode*>(node->initializer.get()) &&
               classes.count(static_cast<NewExpressionNode*>(node->initializer.get())->className)) {
        // Class instance: record the class as the variable's type, so its layout
        // can be found from the type alone, and keep the direct binding too
        auto *newExpr = static_cast<NewExpressionNode*>(node->initializer.get());
        typeToStore = newExpr->className;
        variableToObjectKey[node->variableName] = "opt_obj_" +
            std::to_string(reinterpret_cast<uintptr_t>(classes[newExpr->className]->objectTemplate.get()));
    } else if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(node->initializer.get())) {
        if (node->typeName == "auto") typeToStore = arrLit->elementType + "[]";
        arraySizes[node->variableName] = arrLit->elements.size();
    } else if (node->typeName == "auto") {
        if (auto *callNode = dynamic_cast<FunctionCallNode*>(node->initializer.get())) {
            if (callNode->functionName == "JSON.parse") typeToStore = "json";
            else if (callNode->functionName == "JSON.stringify") typeToStore = "string";
            else {
                // A declared foreign function knows its own return type — without
                // this an opaque `ptr` handle would be mistaken for a string.
                auto externIt = externFunctions.find(callNode->functionName);
                if (externIt != externFunctions.end()) {
                    typeToStore = externIt->second->returnType;
                } else {
                    // A user function's declared return type carries object
                    // identity, so `let v = make();` knows v's layout.
                    auto retIt = functionReturnTypes.find(callNode->functionName);
                    typeToStore = (retIt != functionReturnTypes.end()) ? retIt->second : "";
                    if (typeToStore == "void") typeToStore = "";
                }
            }
        } else if (auto *newExpr = dynamic_cast<NewExpressionNode*>(node->initializer.get())) {
            // e.g. new Map<string, string[]>() -> "Map<string,string[]>"
            typeToStore = newExpr->className;
            if (!newExpr->genericTypes.empty()) {
                typeToStore += "<";
                for (size_t i = 0; i < newExpr->genericTypes.size(); ++i) {
                    if (i > 0) typeToStore += ",";
                    typeToStore += newExpr->genericTypes[i];
                }
                typeToStore += ">";
            }
        } else if (auto *varRef = dynamic_cast<VariableExpressionNode*>(node->initializer.get())) {
            // Aliasing another variable: copy its recorded type and object/closure binding
            auto typeIt = variableTypes.find(varRef->name);
            typeToStore = (typeIt != variableTypes.end()) ? typeIt->second : "";
            auto keyIt = variableToObjectKey.find(varRef->name);
            if (keyIt != variableToObjectKey.end()) {
                variableToObjectKey[node->variableName] = keyIt->second;
            }
            auto arrowIt = variableToArrow.find(varRef->name);
            if (arrowIt != variableToArrow.end()) {
                variableToArrow[node->variableName] = arrowIt->second;
            }
        } else if (auto *methodCall = dynamic_cast<MethodCallNode*>(node->initializer.get())) {
            // e.g. numbers.map(...) -> i32[]/string[], arr.filter(...) -> source type
            typeToStore = inferMethodCallTypeName(methodCall);
        } else if (auto *arrAccess = dynamic_cast<ArrayAccessNode*>(node->initializer.get())) {
            // let e = entities[i]; takes the array's element type, so an object
            // element keeps its layout
            if (auto *arrVar = dynamic_cast<VariableExpressionNode*>(arrAccess->array.get())) {
                auto arrTypeIt = variableTypes.find(arrVar->name);
                if (arrTypeIt != variableTypes.end()) {
                    const std::string &arrType = arrTypeIt->second;
                    if (arrType.size() > 2 && arrType.substr(arrType.size() - 2) == "[]") {
                        typeToStore = arrType.substr(0, arrType.size() - 2);
                    }
                }
            }
        } else {
            typeToStore = "";
        }
        if (typeToStore.empty()) {
            // Fall back to the LLVM type of the generated value
            if (!initVal) typeToStore = "i32";
            else if (initVal->getType()->isPointerTy()) typeToStore = "string";
            else if (initVal->getType()->isDoubleTy()) typeToStore = "f64";
            else if (initVal->getType()->isIntegerTy(1)) typeToStore = "boolean";
            else typeToStore = "i32";
        }
    }

    // Determine the LLVM storage type
    llvm::Type *varLLVMType = (node->typeName == "auto" || typeToStore == "object")
        ? getLLVMType(typeToStore)
        : getLLVMType(node->typeName);

    // A module-level variable that some function reads becomes an LLVM global so
    // it is reachable from outside main. The initializer still runs here, in
    // declaration order, so observable behaviour is unchanged.
    llvm::Value *storage = nullptr;
    if (isModuleLevelDecl && namesUsedByFunctions.count(node->variableName)) {
        auto *global = new llvm::GlobalVariable(
            *m_module, varLLVMType, /*isConstant=*/false,
            llvm::GlobalValue::InternalLinkage,
            llvm::Constant::getNullValue(varLLVMType),
            node->variableName);
        globalValues[node->variableName] = global;
        namedValues.erase(node->variableName);
        storage = global;
    } else {
        // Create alloca at the beginning of the function
        llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
        llvm::IRBuilder<> TmpB(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
        llvm::AllocaInst *allocaInst = TmpB.CreateAlloca(varLLVMType, nullptr, node->variableName);
        namedValues[node->variableName] = allocaInst;
        storage = allocaInst;
    }

    variableTypes[node->variableName] = typeToStore;
    constVariables[node->variableName] = node->isConst;

    if (initVal) {
        if (initVal->getType()->isPointerTy() && varLLVMType->isPointerTy()) {
            // Pointers (strings, arrays, objects, collections) store directly
            if (initVal->getType() != varLLVMType) {
                initVal = m_builder.CreateBitCast(initVal, varLLVMType, "ptr_cast");
            }
        } else {
            initVal = coerceValue(initVal, varLLVMType);
        }
        m_builder.CreateStore(initVal, storage);
    }
}

llvm::Value *CodeGen::visit(ExpressionNode *node)
{
    if (auto *strNode = dynamic_cast<StringLiteralNode *>(node))
    {
        return visit(strNode);
    }
    else if (auto *intNode = dynamic_cast<IntegerLiteralNode *>(node))
    {
        return visit(intNode);
    }
    else if (auto *boolNode = dynamic_cast<BooleanLiteralNode *>(node))
    {
        return visit(boolNode);
    }
    else if (auto *floatNode = dynamic_cast<FloatLiteralNode *>(node))
    {
        return visit(floatNode);
    }
    else if (dynamic_cast<NullLiteralNode *>(node))
    {
        return llvm::ConstantPointerNull::get(
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (auto *varNode = dynamic_cast<VariableExpressionNode *>(node))
    {
        return visit(varNode);
    }
    else if (auto *binOpNode = dynamic_cast<BinaryExpressionNode *>(node))
    {
        return visit(binOpNode);
    }
    else if (auto *unaryOpNode = dynamic_cast<UnaryExpressionNode *>(node))
    {
        return visit(unaryOpNode);
    }
    else if (auto *arrLitNode = dynamic_cast<ArrayLiteralNode *>(node))
    {
        return visit(arrLitNode);
    }
    else if (auto *objLitNode = dynamic_cast<ObjectLiteralNode *>(node))
    {
        return visit(objLitNode);
    }
    else if (auto *arrAccNode = dynamic_cast<ArrayAccessNode *>(node))
    {
        return visit(arrAccNode);
    }
    else if (auto *objAccNode = dynamic_cast<ObjectAccessNode *>(node))
    {
        return visit(objAccNode);
    }
    else if (auto *methodCallNode = dynamic_cast<MethodCallNode *>(node))
    {
        return visit(methodCallNode);
    }
    else if (auto *newNode = dynamic_cast<NewExpressionNode *>(node))
    {
        return visit(newNode);
    }
    else if (auto *arrowNode = dynamic_cast<ArrowFunctionNode *>(node))
    {
        return visit(arrowNode);
    }
    else if (auto *funcCallNode = dynamic_cast<FunctionCallNode *>(node))
    {
        return visit(funcCallNode);
    }
    
    std::cerr << "Codegen Error: Unsupported expression type in visit(ExpressionNode*).\n";
    throw std::runtime_error("Unsupported expression type in codegen.");
}

llvm::Value *CodeGen::visit(StringLiteralNode *node)
{
    // Use CreateGlobalString instead of deprecated CreateGlobalStringPtr
    return m_builder.CreateGlobalString(node->value, ".str_literal");
}

llvm::Value *CodeGen::visit(IntegerLiteralNode *node)
{
    // Create i32 constant for integer literals
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), node->value, true);
}

llvm::Value *CodeGen::visit(BooleanLiteralNode *node)
{
    // Create i32 constant for boolean literals (1 for true, 0 for false)
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), node->value ? 1 : 0, false);
}

llvm::Value *CodeGen::visit(FloatLiteralNode *node)
{
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(m_context), node->value);
}

// Walks every function, method and arrow body and records the identifiers they
// reference. A module-level `let`/`const` is promoted to an LLVM global only if
// it appears here; anything else keeps its alloca in main so that top-level hot
// loops stay register-allocated exactly as before.
void CodeGen::collectFunctionReferencedNames(ProgramNode *node)
{
    namesUsedByFunctions.clear();

    for (const auto &stmt : node->statements) {
        std::set<std::string> bound;
        std::set<std::string> free;

        if (auto *funcDecl = dynamic_cast<FunctionDeclarationNode *>(stmt.get())) {
            for (const auto &param : funcDecl->parameters) bound.insert(param.name);
            for (const auto &bodyStmt : funcDecl->bodyStatements) {
                collectFreeVars(bodyStmt.get(), bound, free);
            }
        } else if (auto *classDecl = dynamic_cast<ClassDeclarationNode *>(stmt.get())) {
            for (const auto &prop : classDecl->objectTemplate->properties) {
                if (!prop.method) continue;
                std::set<std::string> methodBound;
                std::set<std::string> methodFree;
                methodBound.insert("this");
                for (const auto &param : prop.method->parameters) methodBound.insert(param.name);
                for (const auto &bodyStmt : prop.method->bodyStatements) {
                    collectFreeVars(bodyStmt.get(), methodBound, methodFree);
                }
                free.insert(methodFree.begin(), methodFree.end());
            }
        } else {
            continue;
        }

        namesUsedByFunctions.insert(free.begin(), free.end());
    }
}

llvm::Value *CodeGen::variableStorage(const std::string &name, llvm::Type **outType)
{
    auto local = namedValues.find(name);
    if (local != namedValues.end() && local->second) {
        if (outType) *outType = local->second->getAllocatedType();
        return local->second;
    }
    auto global = globalValues.find(name);
    if (global != globalValues.end() && global->second) {
        if (outType) *outType = global->second->getValueType();
        return global->second;
    }
    return nullptr;
}

llvm::Value *CodeGen::visit(VariableExpressionNode *node)
{
    llvm::Type *storedType = nullptr;
    llvm::Value *storage = variableStorage(node->name, &storedType);
    if (!storage)
    {
        throw std::runtime_error("Codegen Error: Unknown variable name: " + node->name);
    }

    // Load the value from the memory location
    return m_builder.CreateLoad(storedType, storage, node->name + "_val");
}

llvm::Value *CodeGen::visit(UnaryExpressionNode *node)
{
    llvm::Value *operand = visit(node->operand.get());
    if (!operand) return nullptr;

    switch (node->op) {
        case UnaryExpressionNode::NOT:
            if (operand->getType()->isPointerTy()) {
                // Null check
                llvm::Value *isNull = m_builder.CreateICmpEQ(operand, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(operand->getType())), "isnull");
                return m_builder.CreateZExt(isNull, llvm::Type::getInt32Ty(m_context), "nottmp");
            } else {
                llvm::Value *isZero = m_builder.CreateICmpEQ(operand, llvm::ConstantInt::get(operand->getType(), 0), "iszero");
                return m_builder.CreateZExt(isZero, llvm::Type::getInt32Ty(m_context), "nottmp");
            }
        case UnaryExpressionNode::MINUS:
            if (operand->getType()->isDoubleTy()) {
                return m_builder.CreateFNeg(operand, "fnegtmp");
            }
            return m_builder.CreateNeg(operand, "negtmp");
        case UnaryExpressionNode::BIT_NOT:
            if (!operand->getType()->isIntegerTy()) {
                throw std::runtime_error("Codegen Error: '~' requires an integer operand");
            }
            operand = coerceValue(operand, llvm::Type::getInt32Ty(m_context));
            return m_builder.CreateNot(operand, "bitnottmp");
        default:
            throw std::runtime_error("Unknown unary operator");
    }
}

llvm::Value *CodeGen::visit(BinaryExpressionNode *node)
{
    // Special handling for logical OR/AND (short-circuiting)
    if (node->op == BinaryExpressionNode::LOGICAL_OR || node->op == BinaryExpressionNode::LOGICAL_AND) {
        llvm::Value *leftVal = visit(node->left.get());
        if (!leftVal) return nullptr;

        llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *rightBlock = llvm::BasicBlock::Create(m_context, "log_right", currentFunction);
        llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(m_context, "log_merge", currentFunction);

        llvm::Value *isTruthy;
        if (leftVal->getType()->isPointerTy()) {
            isTruthy = m_builder.CreateICmpNE(leftVal, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(leftVal->getType())), "ptr_truthy");
        } else {
            isTruthy = m_builder.CreateICmpNE(leftVal, llvm::ConstantInt::get(leftVal->getType(), 0), "val_truthy");
        }

        if (node->op == BinaryExpressionNode::LOGICAL_OR) {
            m_builder.CreateCondBr(isTruthy, mergeBlock, rightBlock);
        } else { // LOGICAL_AND
            m_builder.CreateCondBr(isTruthy, rightBlock, mergeBlock);
        }

        // Left block end (it already branched)
        llvm::BasicBlock *leftEndBlock = m_builder.GetInsertBlock();

        // Right block
        m_builder.SetInsertPoint(rightBlock);
        llvm::Value *rightVal = visit(node->right.get());

        // The two sides can produce different LLVM types — most often an i32
        // boolean field on one side and an i1 comparison on the other. Both
        // arms of the phi must agree, so reconcile them here. Integers widen to
        // the larger width, which keeps `a || b` returning a usable value (the
        // pointer-valued idiom `map.get(k) || []` still yields a pointer).
        if (leftVal->getType() != rightVal->getType()) {
            llvm::Type *leftType = leftVal->getType();
            llvm::Type *rightType = rightVal->getType();
            if (leftType->isIntegerTy() && rightType->isIntegerTy()) {
                llvm::Type *wide =
                    leftType->getIntegerBitWidth() >= rightType->getIntegerBitWidth()
                        ? leftType : rightType;
                if (wide->isIntegerTy(1)) wide = llvm::Type::getInt32Ty(m_context);
                rightVal = coerceValue(rightVal, wide);
                // The left value is produced in a different block, so widen it there
                llvm::IRBuilder<> leftBuilder(leftEndBlock->getTerminator());
                if (leftVal->getType() != wide) {
                    leftVal = leftBuilder.CreateZExt(leftVal, wide, "log_left_ext");
                }
            } else {
                rightVal = coerceValue(rightVal, leftType);
            }
        }

        m_builder.CreateBr(mergeBlock);
        llvm::BasicBlock *rightEndBlock = m_builder.GetInsertBlock();

        // Merge block
        m_builder.SetInsertPoint(mergeBlock);
        llvm::PHINode *phi = m_builder.CreatePHI(leftVal->getType(), 2, "log_phi");
        phi->addIncoming(leftVal, leftEndBlock);
        phi->addIncoming(rightVal, rightEndBlock);
        return phi;
    }

    // Generate code for left and right operands
    llvm::Value *leftVal = visit(node->left.get());
    llvm::Value *rightVal = visit(node->right.get());
    
    if (!leftVal || !rightVal) {
        throw std::runtime_error("Codegen Error: Failed to generate operands for binary expression");
    }
    
    // Bitwise operators are integer-only, and bind before the float promotion
    // below so that `&`/`|` never silently turn into floating-point work.
    if (node->op == BinaryExpressionNode::BIT_AND || node->op == BinaryExpressionNode::BIT_OR ||
        node->op == BinaryExpressionNode::BIT_XOR || node->op == BinaryExpressionNode::SHIFT_LEFT ||
        node->op == BinaryExpressionNode::SHIFT_RIGHT) {
        if (!leftVal->getType()->isIntegerTy() || !rightVal->getType()->isIntegerTy()) {
            throw std::runtime_error("Codegen Error: bitwise operators require integer operands");
        }
        // Widen to the larger of the two so i32 & i64 works
        llvm::Type *wide = leftVal->getType()->getIntegerBitWidth() >= rightVal->getType()->getIntegerBitWidth()
            ? leftVal->getType() : rightVal->getType();
        if (wide->getIntegerBitWidth() < 32) wide = llvm::Type::getInt32Ty(m_context);
        leftVal = coerceValue(leftVal, wide);
        rightVal = coerceValue(rightVal, wide);

        switch (node->op) {
            case BinaryExpressionNode::BIT_AND: return m_builder.CreateAnd(leftVal, rightVal, "andtmp");
            case BinaryExpressionNode::BIT_OR: return m_builder.CreateOr(leftVal, rightVal, "ortmp");
            case BinaryExpressionNode::BIT_XOR: return m_builder.CreateXor(leftVal, rightVal, "xortmp");
            case BinaryExpressionNode::SHIFT_LEFT: return m_builder.CreateShl(leftVal, rightVal, "shltmp");
            case BinaryExpressionNode::SHIFT_RIGHT: return m_builder.CreateAShr(leftVal, rightVal, "ashrtmp");
            default: break;
        }
    }

    // String concatenation: `+` where either side is a string.
    // Non-string operands are converted with cyps_i32_to_string / cyps_f64_to_string.
    if (node->op == BinaryExpressionNode::ADD &&
        (leftVal->getType()->isPointerTy() || rightVal->getType()->isPointerTy())) {
        llvm::Value *leftStr = toStringValue(leftVal);
        llvm::Value *rightStr = toStringValue(rightVal);
        llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
        llvm::FunctionCallee concatFunc = m_module->getOrInsertFunction("string_concat",
            charPtr, charPtr, charPtr);
        return m_builder.CreateCall(concatFunc, {leftStr, rightStr}, "concat");
    }

    // Floating-point arithmetic: promote to double when either side is f64
    if (leftVal->getType()->isDoubleTy() || rightVal->getType()->isDoubleTy()) {
        llvm::Type *doubleTy = llvm::Type::getDoubleTy(m_context);
        leftVal = coerceValue(leftVal, doubleTy);
        rightVal = coerceValue(rightVal, doubleTy);

        switch (node->op) {
            case BinaryExpressionNode::ADD:
                return m_builder.CreateFAdd(leftVal, rightVal, "faddtmp");
            case BinaryExpressionNode::SUBTRACT:
                return m_builder.CreateFSub(leftVal, rightVal, "fsubtmp");
            case BinaryExpressionNode::MULTIPLY:
                return m_builder.CreateFMul(leftVal, rightVal, "fmultmp");
            case BinaryExpressionNode::DIVIDE:
                return m_builder.CreateFDiv(leftVal, rightVal, "fdivtmp");
            case BinaryExpressionNode::MODULO:
                return m_builder.CreateFRem(leftVal, rightVal, "fmodtmp");
            case BinaryExpressionNode::EQUAL:
                return m_builder.CreateFCmpOEQ(leftVal, rightVal, "feqtmp");
            case BinaryExpressionNode::NOT_EQUAL:
                return m_builder.CreateFCmpONE(leftVal, rightVal, "fnetmp");
            case BinaryExpressionNode::LESS_THAN:
                return m_builder.CreateFCmpOLT(leftVal, rightVal, "flttmp");
            case BinaryExpressionNode::LESS_EQUAL:
                return m_builder.CreateFCmpOLE(leftVal, rightVal, "fletmp");
            case BinaryExpressionNode::GREATER_THAN:
                return m_builder.CreateFCmpOGT(leftVal, rightVal, "fgttmp");
            case BinaryExpressionNode::GREATER_EQUAL:
                return m_builder.CreateFCmpOGE(leftVal, rightVal, "fgetmp");
            default:
                throw std::runtime_error("Codegen Error: Unknown binary operator for f64");
        }
    }

    // Normalize mixed integer widths (i1 comparisons combined with i32 values)
    if (leftVal->getType()->isIntegerTy() && rightVal->getType()->isIntegerTy() &&
        leftVal->getType() != rightVal->getType()) {
        llvm::Type *i32Ty = llvm::Type::getInt32Ty(m_context);
        leftVal = coerceValue(leftVal, i32Ty);
        rightVal = coerceValue(rightVal, i32Ty);
    }

    // Check if we're dealing with strings. Object handles, `ptr` values and
    // `null` are pointers too, and must be compared by identity — handing them
    // to strcmp reads them as text and segfaults on anything non-null.
    bool isStringComparison = leftVal->getType()->isPointerTy() && rightVal->getType()->isPointerTy() &&
                              !isNonStringPointer(node->left.get()) &&
                              !isNonStringPointer(node->right.get());

    if (leftVal->getType()->isPointerTy() && rightVal->getType()->isPointerTy() && !isStringComparison &&
        (node->op == BinaryExpressionNode::EQUAL || node->op == BinaryExpressionNode::NOT_EQUAL)) {
        if (leftVal->getType() != rightVal->getType()) {
            rightVal = m_builder.CreateBitCast(rightVal, leftVal->getType(), "ptr_cmp_cast");
        }
        return node->op == BinaryExpressionNode::EQUAL
            ? m_builder.CreateICmpEQ(leftVal, rightVal, "ptr_eq")
            : m_builder.CreateICmpNE(leftVal, rightVal, "ptr_ne");
    }

    // Handle string comparisons
    if (isStringComparison && (node->op == BinaryExpressionNode::EQUAL || node->op == BinaryExpressionNode::NOT_EQUAL)) {
        // For string comparison, we need to call strcmp
        // First, get or create the strcmp function declaration
        llvm::Function *strcmpFunc = m_module->getFunction("strcmp");
        if (!strcmpFunc) {
            // Declare strcmp: int strcmp(const char* s1, const char* s2)
            llvm::FunctionType *strcmpType = llvm::FunctionType::get(
                llvm::Type::getInt32Ty(m_context),
                {llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                 llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0)},
                false
            );
            strcmpFunc = llvm::Function::Create(strcmpType, llvm::Function::ExternalLinkage, "strcmp", m_module.get());
        }
        
        // Call strcmp
        llvm::Value *cmpResult = m_builder.CreateCall(strcmpFunc, {leftVal, rightVal}, "strcmp_result");
        
        // strcmp returns 0 if strings are equal
        llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0);
        
        if (node->op == BinaryExpressionNode::EQUAL) {
            return m_builder.CreateICmpEQ(cmpResult, zero, "streq");
        } else { // NOT_EQUAL
            return m_builder.CreateICmpNE(cmpResult, zero, "strne");
        }
    }
    
    // For arithmetic and integer comparisons, both operands must be integers
    if (!leftVal->getType()->isIntegerTy() || !rightVal->getType()->isIntegerTy()) {
        if (isStringComparison) {
            throw std::runtime_error("Codegen Error: String operations other than == and != are not supported yet");
        } else {
            throw std::runtime_error("Codegen Error: Binary operations currently only support integers and string comparisons");
        }
    }
    
    // Generate the appropriate LLVM instruction based on the operator
    switch (node->op) {
        // Arithmetic operations
        case BinaryExpressionNode::ADD:
            return m_builder.CreateAdd(leftVal, rightVal, "addtmp");
        case BinaryExpressionNode::SUBTRACT:
            return m_builder.CreateSub(leftVal, rightVal, "subtmp");
        case BinaryExpressionNode::MULTIPLY:
            return m_builder.CreateMul(leftVal, rightVal, "multmp");
        case BinaryExpressionNode::DIVIDE:
            // Use signed division
            return m_builder.CreateSDiv(leftVal, rightVal, "divtmp");
        case BinaryExpressionNode::MODULO:
            // Use signed remainder
            return m_builder.CreateSRem(leftVal, rightVal, "modtmp");
            
        // Comparison operations (return i1 boolean values)
        case BinaryExpressionNode::EQUAL:
            return m_builder.CreateICmpEQ(leftVal, rightVal, "eqtmp");
        case BinaryExpressionNode::NOT_EQUAL:
            return m_builder.CreateICmpNE(leftVal, rightVal, "netmp");
        case BinaryExpressionNode::LESS_THAN:
            return m_builder.CreateICmpSLT(leftVal, rightVal, "lttmp");
        case BinaryExpressionNode::LESS_EQUAL:
            return m_builder.CreateICmpSLE(leftVal, rightVal, "letmp");
        case BinaryExpressionNode::GREATER_THAN:
            return m_builder.CreateICmpSGT(leftVal, rightVal, "gttmp");
        case BinaryExpressionNode::GREATER_EQUAL:
            return m_builder.CreateICmpSGE(leftVal, rightVal, "getmp");
            
        default:
            throw std::runtime_error("Codegen Error: Unknown binary operator");
    }
}

void CodeGen::visit(IfStatementNode *node)
{
    // Generate code for the condition
    llvm::Value *conditionVal = ensureI1(visit(node->condition.get()));
    if (!conditionVal) {
        throw std::runtime_error("Codegen Error: Failed to generate condition for if statement");
    }
    
    // Get the current function
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    
    // Create basic blocks for then, else (optional), and merge
    llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(m_context, "then", currentFunction);
    llvm::BasicBlock *elseBlock = nullptr;
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(m_context, "ifcont", currentFunction);
    
    if (!node->elseStatements.empty()) {
        elseBlock = llvm::BasicBlock::Create(m_context, "else", currentFunction);
    }
    
    // Create conditional branch
    if (elseBlock) {
        m_builder.CreateCondBr(conditionVal, thenBlock, elseBlock);
    } else {
        m_builder.CreateCondBr(conditionVal, thenBlock, mergeBlock);
    }
    
    // Generate then block
    m_builder.SetInsertPoint(thenBlock);
    for (const auto &stmt : node->thenStatements) {
        visit(stmt.get());
    }
    // Branch to merge block (if we haven't already branched elsewhere)
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(mergeBlock);
    }
    
    // Generate else block if it exists
    if (elseBlock) {
        m_builder.SetInsertPoint(elseBlock);
        for (const auto &stmt : node->elseStatements) {
            visit(stmt.get());
        }
        // Branch to merge block (if we haven't already branched elsewhere)
        if (!m_builder.GetInsertBlock()->getTerminator()) {
            m_builder.CreateBr(mergeBlock);
        }
    }
    
    // Continue with merge block
    m_builder.SetInsertPoint(mergeBlock);
}

void CodeGen::visit(WhileStatementNode *node)
{
    // Get the current function
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    
    // Create basic blocks for loop condition, body, and exit
    llvm::BasicBlock *condBlock = llvm::BasicBlock::Create(m_context, "loopcond", currentFunction);
    llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(m_context, "loopbody", currentFunction);
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(m_context, "loopexit", currentFunction);
    
    // Branch to condition block
    m_builder.CreateBr(condBlock);
    
    // Generate condition block
    m_builder.SetInsertPoint(condBlock);
    llvm::Value *conditionVal = ensureI1(visit(node->condition.get()));
    if (!conditionVal) {
        throw std::runtime_error("Codegen Error: Failed to generate condition for while statement");
    }
    
    // Create conditional branch: if true go to body, if false go to exit
    m_builder.CreateCondBr(conditionVal, bodyBlock, exitBlock);
    
    // Generate body block
    m_builder.SetInsertPoint(bodyBlock);
    loopTargets.push_back({condBlock, exitBlock});
    loopTargetTryDepths.push_back(tryDepth);
    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    loopTargets.pop_back();
    loopTargetTryDepths.pop_back();

    // Branch back to condition (creating the loop)
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(condBlock);
    }

    // Continue with exit block
    m_builder.SetInsertPoint(exitBlock);
}

void CodeGen::visit(BreakStatementNode *node)
{
    (void)node;
    if (loopTargets.empty()) {
        throw std::runtime_error("Codegen Error: 'break' used outside of a loop or switch");
    }
    // Pop recovery points of any try blocks this break jumps out of
    emitTryPops(tryDepth - loopTargetTryDepths.back());
    branchAndSealBlock(loopTargets.back().second, "after_break");
}

void CodeGen::visit(ContinueStatementNode *node)
{
    (void)node;
    // Find the nearest enclosing loop (switch entries have a null continue target)
    for (size_t i = loopTargets.size(); i-- > 0;) {
        if (loopTargets[i].first) {
            emitTryPops(tryDepth - loopTargetTryDepths[i]);
            branchAndSealBlock(loopTargets[i].first, "after_continue");
            return;
        }
    }
    throw std::runtime_error("Codegen Error: 'continue' used outside of a loop");
}

void CodeGen::visit(SwitchStatementNode *node)
{
    llvm::Value *condVal = visit(node->condition.get());
    if (!condVal) {
        throw std::runtime_error("Codegen Error: Failed to generate switch condition");
    }

    llvm::Function *fn = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(m_context, "switch_exit", fn);

    // One body block per clause, in source order (enables fallthrough)
    std::vector<llvm::BasicBlock*> bodyBlocks;
    for (size_t i = 0; i < node->cases.size(); ++i) {
        bodyBlocks.push_back(llvm::BasicBlock::Create(m_context, "case_body_" + std::to_string(i), fn));
    }

    // Default clause body (if present) is the fallback of the comparison chain
    llvm::BasicBlock *defaultBlock = exitBlock;
    for (size_t i = 0; i < node->cases.size(); ++i) {
        if (!node->cases[i].value) {
            defaultBlock = bodyBlocks[i];
            break;
        }
    }

    // Emit the comparison chain
    for (size_t i = 0; i < node->cases.size(); ++i) {
        if (!node->cases[i].value) continue; // default has no test

        llvm::Value *caseVal = visit(node->cases[i].value.get());

        // Next test target: the next non-default case's check block, or default
        llvm::BasicBlock *nextCheck = nullptr;
        for (size_t j = i + 1; j < node->cases.size(); ++j) {
            if (node->cases[j].value) {
                nextCheck = llvm::BasicBlock::Create(m_context, "case_check_" + std::to_string(j), fn);
                break;
            }
        }
        if (!nextCheck) nextCheck = defaultBlock;

        llvm::Value *matches;
        if (condVal->getType()->isPointerTy() && caseVal->getType()->isPointerTy()) {
            llvm::Function *strcmpFunc = m_module->getFunction("strcmp");
            if (!strcmpFunc) {
                llvm::FunctionType *strcmpType = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(m_context),
                    {llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                     llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0)},
                    false);
                strcmpFunc = llvm::Function::Create(strcmpType, llvm::Function::ExternalLinkage, "strcmp", m_module.get());
            }
            llvm::Value *cmp = m_builder.CreateCall(strcmpFunc, {condVal, caseVal}, "case_strcmp");
            matches = m_builder.CreateICmpEQ(cmp, llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0), "case_match");
        } else if (condVal->getType()->isDoubleTy() || caseVal->getType()->isDoubleTy()) {
            llvm::Type *doubleTy = llvm::Type::getDoubleTy(m_context);
            matches = m_builder.CreateFCmpOEQ(coerceValue(condVal, doubleTy), coerceValue(caseVal, doubleTy), "case_match");
        } else {
            matches = m_builder.CreateICmpEQ(condVal, coerceValue(caseVal, condVal->getType()), "case_match");
        }

        m_builder.CreateCondBr(matches, bodyBlocks[i], nextCheck);
        if (nextCheck != defaultBlock) {
            m_builder.SetInsertPoint(nextCheck);
        }
    }

    // If there were no testable cases at all, jump straight to default/exit
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(defaultBlock);
    }

    // Emit clause bodies; `break` exits, falling off a body falls through to the next one
    loopTargets.push_back({nullptr, exitBlock});
    loopTargetTryDepths.push_back(tryDepth);
    for (size_t i = 0; i < node->cases.size(); ++i) {
        m_builder.SetInsertPoint(bodyBlocks[i]);
        for (const auto &stmt : node->cases[i].statements) {
            visit(stmt.get());
        }
        if (!m_builder.GetInsertBlock()->getTerminator()) {
            llvm::BasicBlock *fallthrough = (i + 1 < node->cases.size()) ? bodyBlocks[i + 1] : exitBlock;
            m_builder.CreateBr(fallthrough);
        }
    }
    loopTargets.pop_back();
    loopTargetTryDepths.pop_back();

    m_builder.SetInsertPoint(exitBlock);
}

void CodeGen::visit(AssignmentStatementNode *node)
{
    // Look up the variable: a local first, then a module-level global
    llvm::Type *storedType = nullptr;
    llvm::Value *varAlloca = variableStorage(node->variableName, &storedType);
    if (!varAlloca) {
        throw std::runtime_error("Codegen Error: Undefined variable '" + node->variableName + "'");
    }

    // Check if the variable is const
    auto constIt = constVariables.find(node->variableName);
    if (constIt != constVariables.end() && constIt->second) {
        throw std::runtime_error("Codegen Error: Cannot reassign to const variable '" + node->variableName + "'");
    }
    // Generate code for the value expression
    llvm::Value *value = visit(node->value.get());
    if (!value) {
        throw std::runtime_error("Codegen Error: Failed to generate value for assignment");
    }

    // Store the value in the variable's memory location (with i1/i32/f64 coercion)
    value = coerceValue(value, storedType);
    m_builder.CreateStore(value, varAlloca);
}

void CodeGen::visit(ArrayAssignmentStatementNode *node)
{
    // Generate code for the array expression (should be a variable)
    llvm::Value *arrayValue = visit(node->array.get());
    if (!arrayValue) {
        throw std::runtime_error("Codegen Error: Failed to generate array for assignment");
    }
    
    // Generate code for the index expression
    llvm::Value *indexValue = visit(node->index.get());
    if (!indexValue) {
        throw std::runtime_error("Codegen Error: Failed to generate array index for assignment");
    }
    
    // Generate code for the value to assign
    llvm::Value *valueToAssign = visit(node->value.get());
    if (!valueToAssign) {
        throw std::runtime_error("Codegen Error: Failed to generate value for array assignment");
    }
    
    // Ensure index is an integer
    if (!indexValue->getType()->isIntegerTy()) {
        throw std::runtime_error("Codegen Error: Array index must be an integer");
    }
    
    // Determine element type by looking up the variable type
    std::string elemType = "i32"; // default
    
    // Try to get the variable name from the array expression
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->array.get())) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) {
            std::string varType = typeIt->second;
            // Extract element type from array type (e.g., "string[]" -> "string")
            if (varType.length() > 2 && varType.substr(varType.length() - 2) == "[]") {
                elemType = varType.substr(0, varType.length() - 2);
            }
        }
    }
    
    // Check if arrayValue is a pointer type
    if (!arrayValue->getType()->isPointerTy()) {
        throw std::runtime_error("Codegen Error: Array assignment requires a pointer type");
    }

    // Use dynamic array functions
    if (isObjectTypeName(elemType)) {
        llvm::FunctionCallee setFunc = m_module->getOrInsertFunction("array_set_object",
            llvm::Type::getVoidTy(m_context), llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0), llvm::Type::getInt32Ty(m_context), llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
        m_builder.CreateCall(setFunc, {arrayValue, indexValue, valueToAssign});
    } else if (elemType == "string") {
        llvm::FunctionCallee setFunc = m_module->getOrInsertFunction("array_set_string",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
        m_builder.CreateCall(setFunc, {arrayValue, indexValue, valueToAssign});
    } else if (elemType == "f64") {
        llvm::FunctionCallee setFunc = m_module->getOrInsertFunction("array_set_f64",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getDoubleTy(m_context));
        m_builder.CreateCall(setFunc, {arrayValue, indexValue,
            coerceValue(valueToAssign, llvm::Type::getDoubleTy(m_context))});
    } else {
        llvm::FunctionCallee setFunc = m_module->getOrInsertFunction("array_set_i32",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
        m_builder.CreateCall(setFunc, {arrayValue, indexValue, valueToAssign});
    }
}

void CodeGen::visit(ForStatementNode *node)
{
    // Get the current function
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    
    // Create basic blocks for initialization, condition, body, increment, and exit
    llvm::BasicBlock *initBlock = llvm::BasicBlock::Create(m_context, "forinit", currentFunction);
    llvm::BasicBlock *condBlock = llvm::BasicBlock::Create(m_context, "forcond", currentFunction);
    llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(m_context, "forbody", currentFunction);
    llvm::BasicBlock *incrBlock = llvm::BasicBlock::Create(m_context, "forincr", currentFunction);
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(m_context, "forexit", currentFunction);
    
    // Branch to initialization block
    m_builder.CreateBr(initBlock);
    
    // Generate initialization block
    m_builder.SetInsertPoint(initBlock);
    if (node->initialization) {
        visit(node->initialization.get());
    }
    m_builder.CreateBr(condBlock);
    
    // Generate condition block
    m_builder.SetInsertPoint(condBlock);
    if (node->condition) {
        llvm::Value *conditionVal = ensureI1(visit(node->condition.get()));
        if (!conditionVal) {
            throw std::runtime_error("Codegen Error: Failed to generate condition for for statement");
        }
        m_builder.CreateCondBr(conditionVal, bodyBlock, exitBlock);
    } else {
        // No condition means infinite loop (like for(;;))
        m_builder.CreateBr(bodyBlock);
    }
    
    // Generate body block
    m_builder.SetInsertPoint(bodyBlock);
    loopTargets.push_back({incrBlock, exitBlock});
    loopTargetTryDepths.push_back(tryDepth);
    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    loopTargets.pop_back();
    loopTargetTryDepths.pop_back();
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(incrBlock);
    }

    // Generate increment block
    m_builder.SetInsertPoint(incrBlock);
    if (node->increment) {
        visit(node->increment.get());
    }
    m_builder.CreateBr(condBlock); // Loop back to condition

    // Continue with exit block
    m_builder.SetInsertPoint(exitBlock);
}

void CodeGen::visit(ForOfStatementNode *node)
{
    // 1. Evaluate iterable expression to get the array pointer
    llvm::Value *arrPtr = visit(node->iterable.get());
    if (!arrPtr) {
        throw std::runtime_error("Codegen Error: Failed to evaluate iterable in for-of loop");
    }

    // 2. Determine element type of the iterable
    std::string elemType = "i32"; // default
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->iterable.get())) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) {
            std::string varType = typeIt->second;
            if (varType.length() > 2 && varType.substr(varType.length() - 2) == "[]") {
                elemType = varType.substr(0, varType.length() - 2);
            }
        }
    }

    // 3. Get current function and create basic blocks
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *condBlock = llvm::BasicBlock::Create(m_context, "forof_cond", currentFunction);
    llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(m_context, "forof_body", currentFunction);
    llvm::BasicBlock *incrBlock = llvm::BasicBlock::Create(m_context, "forof_incr", currentFunction);
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(m_context, "forof_exit", currentFunction);

    // 4. Initialize loop index: let i = 0
    llvm::Value *indexAlloca = m_builder.CreateAlloca(llvm::Type::getInt32Ty(m_context), nullptr, "forof_index");
    m_builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0), indexAlloca);
    
    // 5. Get array length
    llvm::FunctionCallee lenFunc = m_module->getOrInsertFunction("array_length",
        llvm::Type::getInt32Ty(m_context),
        llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    llvm::Value *len = m_builder.CreateCall(lenFunc, {arrPtr}, "arr_len");

    m_builder.CreateBr(condBlock);

    // 6. Condition block: i < len
    m_builder.SetInsertPoint(condBlock);
    llvm::Value *currentIndex = m_builder.CreateLoad(llvm::Type::getInt32Ty(m_context), indexAlloca, "current_index");
    llvm::Value *cond = m_builder.CreateICmpSLT(currentIndex, len, "forof_cond_val");
    m_builder.CreateCondBr(cond, bodyBlock, exitBlock);

    // 7. Body block
    m_builder.SetInsertPoint(bodyBlock);
    
    // Save current symbol table for loop scope
    auto oldNamedValues = namedValues;
    auto oldVariableTypes = variableTypes;
    auto oldConstVariables = constVariables;

    // Load current element from dynamic array
    llvm::Value *element;
    if (isObjectTypeName(elemType)) {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_object",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0), llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0), llvm::Type::getInt32Ty(m_context));
        element = m_builder.CreateCall(getFunc, {arrPtr, currentIndex}, "iter_element");
    } else if (elemType == "f64") {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_f64",
            llvm::Type::getDoubleTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
        element = m_builder.CreateCall(getFunc, {arrPtr, currentIndex}, "iter_element");
    } else if (elemType == "string" || elemType.length() == 1 || elemType.find('<') != std::string::npos) {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_string",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
        element = m_builder.CreateCall(getFunc, {arrPtr, currentIndex}, "iter_element");
    } else {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
        element = m_builder.CreateCall(getFunc, {arrPtr, currentIndex}, "iter_element");
    }

    // Allocate memory for the iterator variable and store the loaded element
    llvm::Type *varType = getLLVMType(elemType);
    llvm::AllocaInst *varAlloca = m_builder.CreateAlloca(varType, nullptr, node->iteratorVariable->variableName);
    m_builder.CreateStore(element, varAlloca);
    
    // Register iterator variable in local scope
    namedValues[node->iteratorVariable->variableName] = varAlloca;
    variableTypes[node->iteratorVariable->variableName] = elemType;
    constVariables[node->iteratorVariable->variableName] = node->iteratorVariable->isConst;

    // Visit body statements
    loopTargets.push_back({incrBlock, exitBlock});
    loopTargetTryDepths.push_back(tryDepth);
    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    loopTargets.pop_back();
    loopTargetTryDepths.pop_back();

    // Restore scope
    namedValues = oldNamedValues;
    variableTypes = oldVariableTypes;
    constVariables = oldConstVariables;

    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(incrBlock);
    }

    // 8. Increment block: i = i + 1
    m_builder.SetInsertPoint(incrBlock);
    llvm::Value *nextIndex = m_builder.CreateAdd(currentIndex, llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 1));
    m_builder.CreateStore(nextIndex, indexAlloca);
    m_builder.CreateBr(condBlock);

    // 9. Exit loop
    m_builder.SetInsertPoint(exitBlock);
}

void CodeGen::visit(DoWhileStatementNode *node)
{
    // Get the current function
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    
    // Create basic blocks for body, condition, and exit
    llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(m_context, "dobody", currentFunction);
    llvm::BasicBlock *condBlock = llvm::BasicBlock::Create(m_context, "docond", currentFunction);
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(m_context, "doexit", currentFunction);
    
    // Branch to body block (do-while always executes body at least once)
    m_builder.CreateBr(bodyBlock);
    
    // Generate body block
    m_builder.SetInsertPoint(bodyBlock);
    loopTargets.push_back({condBlock, exitBlock});
    loopTargetTryDepths.push_back(tryDepth);
    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    loopTargets.pop_back();
    loopTargetTryDepths.pop_back();
    // Generate condition check after body
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(condBlock);
    }
    m_builder.SetInsertPoint(condBlock);
    llvm::Value *conditionVal = ensureI1(visit(node->condition.get()));
    if (!conditionVal) {
        throw std::runtime_error("Codegen Error: Failed to generate condition for do-while statement");
    }
    
    // Create conditional branch: if true go back to body, if false go to exit
    m_builder.CreateCondBr(conditionVal, bodyBlock, exitBlock);
    
    // Continue with exit block
    m_builder.SetInsertPoint(exitBlock);
}

void CodeGen::visit(ExpressionStatementNode *node)
{
    if (node->expression) {
        visit(node->expression.get()); // Evaluate the expression (side effects like function calls)
    }
}

llvm::Value *CodeGen::visit(FunctionCallNode *node)
{
    // Check if it's a user-defined function first
    auto funcIt = declaredFunctions.find(node->functionName);
    if (funcIt != declaredFunctions.end()) {
        // User-defined function call
        llvm::Function* function = funcIt->second;
        
        // Check argument count
        if (node->arguments.size() != function->arg_size()) {
            throw std::runtime_error("Function '" + node->functionName + "' expects " + 
                                   std::to_string(function->arg_size()) + " arguments, got " + 
                                   std::to_string(node->arguments.size()));
        }
        
        // Generate arguments (coerced to the declared parameter types)
        std::vector<llvm::Value*> args;
        size_t argIndex = 0;
        for (const auto& arg : node->arguments) {
            llvm::Value* argValue = visit(arg.get());
            if (!argValue) {
                throw std::runtime_error("Failed to generate argument for function call");
            }
            if (argIndex < function->arg_size()) {
                argValue = coerceValue(argValue, function->getFunctionType()->getParamType(argIndex));
            }
            args.push_back(argValue);
            argIndex++;
        }
        
        // Create function call
        llvm::Value* callResult;
        if (function->getReturnType()->isVoidTy()) {
            // For void functions, don't assign a name to the call
            callResult = m_builder.CreateCall(function, args);
            return nullptr;
        } else {
            // For non-void functions, assign a name
            callResult = m_builder.CreateCall(function, args, "call");
            return callResult;
        }
        }

    // Closure call through a variable: let f = (x) => ...; f(5);
    auto arrowVarIt = variableToArrow.find(node->functionName);
    if (arrowVarIt != variableToArrow.end() && namedValues.count(node->functionName)) {
        ArrowFunctionNode *arrowNode = arrowVarIt->second;
        auto fnIt = arrowFunctions.find(arrowNode);
        if (fnIt == arrowFunctions.end()) {
            throw std::runtime_error("Codegen Error: Closure '" + node->functionName +
                                     "' called before it was created");
        }
        llvm::Function *fn = fnIt->second;

        llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
        llvm::Value *closureVal = m_builder.CreateLoad(charPtr, namedValues[node->functionName],
                                                       node->functionName + "_closure");
        llvm::StructType *closTy = getClosureType();
        llvm::Value *closPtr = m_builder.CreateBitCast(closureVal,
            llvm::PointerType::get(closTy, 0), "closure_ptr");
        llvm::Value *envPtr = m_builder.CreateLoad(charPtr,
            m_builder.CreateStructGEP(closTy, closPtr, 1), "closure_env");

        std::vector<llvm::Value*> args;
        args.push_back(envPtr);
        size_t argIndex = 1;
        for (const auto& arg : node->arguments) {
            llvm::Value *argValue = visit(arg.get());
            if (!argValue) {
                throw std::runtime_error("Codegen Error: Failed to generate closure argument");
            }
            if (argIndex < fn->arg_size()) {
                argValue = coerceValue(argValue, fn->getFunctionType()->getParamType(argIndex));
            }
            args.push_back(argValue);
            argIndex++;
        }

        if (fn->getReturnType()->isVoidTy()) {
            m_builder.CreateCall(fn, args);
            return nullptr;
        }
        return m_builder.CreateCall(fn, args, node->functionName + "_result");
    }

    // Closure call through a closure-typed variable/parameter, e.g.
    //   function apply(f: (i32) => i32, x: i32): i32 { return f(x); }
    // The call is indirect via the closure's stored function pointer.
    auto varTypeIt = variableTypes.find(node->functionName);
    if (varTypeIt != variableTypes.end() && varTypeIt->second.rfind("closure(", 0) == 0 &&
        namedValues.count(node->functionName)) {
        std::vector<std::string> argTypeNames;
        std::string retTypeName;
        if (!parseClosureSignature(varTypeIt->second, argTypeNames, retTypeName)) {
            throw std::runtime_error("Codegen Error: Malformed function type '" + varTypeIt->second + "'");
        }

        llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
        std::vector<llvm::Type*> paramTypes;
        paramTypes.push_back(charPtr); // env
        for (const auto &typeName : argTypeNames) paramTypes.push_back(getLLVMType(typeName));
        llvm::FunctionType *fnType = llvm::FunctionType::get(getLLVMType(retTypeName), paramTypes, false);

        llvm::Value *closureVal = m_builder.CreateLoad(charPtr, namedValues[node->functionName],
                                                       node->functionName + "_closure");
        llvm::StructType *closTy = getClosureType();
        llvm::Value *closPtr = m_builder.CreateBitCast(closureVal,
            llvm::PointerType::get(closTy, 0), "closure_ptr");
        llvm::Value *fnPtr = m_builder.CreateLoad(charPtr,
            m_builder.CreateStructGEP(closTy, closPtr, 0), "closure_fn");
        llvm::Value *envPtr = m_builder.CreateLoad(charPtr,
            m_builder.CreateStructGEP(closTy, closPtr, 1), "closure_env");

        std::vector<llvm::Value*> args;
        args.push_back(envPtr);
        size_t argIndex = 1;
        for (const auto &arg : node->arguments) {
            llvm::Value *argValue = visit(arg.get());
            if (!argValue) {
                throw std::runtime_error("Codegen Error: Failed to generate closure argument");
            }
            if (argIndex < paramTypes.size()) {
                argValue = coerceValue(argValue, paramTypes[argIndex]);
            }
            args.push_back(argValue);
            argIndex++;
        }

        if (fnType->getReturnType()->isVoidTy()) {
            m_builder.CreateCall(fnType, fnPtr, args);
            return nullptr;
        }
        return m_builder.CreateCall(fnType, fnPtr, args, node->functionName + "_result");
    }

    if (node->functionName == "JSON.parse") {
        if (node->arguments.size() != 1) {
            throw std::runtime_error("JSON.parse expects exactly one argument.");
        }
        llvm::Value* argValue = visit(node->arguments[0].get());
        if (!argValue) {
            throw std::runtime_error("Failed to generate code for JSON.parse argument.");
        }
        return argValue;
    }

    if (node->functionName == "JSON.stringify") {
        if (node->arguments.size() != 1) {
            throw std::runtime_error("JSON.stringify expects exactly one argument.");
        }
        
        bool isObject = false;
        std::string objectKey;
        if (auto* varExpr = dynamic_cast<VariableExpressionNode*>(node->arguments[0].get())) {
            auto objKeyIt = variableToObjectKey.find(varExpr->name);
            if (objKeyIt != variableToObjectKey.end()) {
                isObject = true;
                objectKey = objKeyIt->second;
            }
        }
        
        if (isObject) {
            auto layoutIt = objectLayouts.find(objectKey);
            if (layoutIt != objectLayouts.end()) {
                const ObjectOptimizer::ObjectLayout& layout = layoutIt->second;
                
                llvm::Value* storageSlot = variableStorage(dynamic_cast<VariableExpressionNode*>(node->arguments[0].get())->name);
                if (storageSlot != nullptr) {
                    llvm::Value* objectPtrAlloca = storageSlot;
                    llvm::Value* objectPtr = m_builder.CreateLoad(
                        llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                        objectPtrAlloca,
                        "obj_ptr_load"
                    );
                    llvm::Value* structPtr = m_builder.CreateBitCast(
                        objectPtr,
                        llvm::PointerType::get(layout.structType, 0),
                        "struct_cast"
                    );
                    
                    llvm::FunctionCallee createObjFunc = m_module->getOrInsertFunction("json_create_object",
                        llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                    llvm::Value* jsonObj = m_builder.CreateCall(createObjFunc, {}, "json_obj");
                    
                    for (const auto& prop : layout.properties) {
                        llvm::Value* keyStr = m_builder.CreateGlobalString(prop.first, ".json_key");
                        llvm::Value* propValue = objectOptimizer.generateDirectPropertyAccess(
                            m_builder, structPtr, prop.first, layout);
                        
                        if (prop.second.typeName == "string") {
                            llvm::FunctionCallee addStrFunc = m_module->getOrInsertFunction("json_add_string",
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                            jsonObj = m_builder.CreateCall(addStrFunc, {jsonObj, keyStr, propValue}, "json_add");
                        } else if (prop.second.typeName == "i32") {
                            llvm::FunctionCallee addIntFunc = m_module->getOrInsertFunction("json_add_int",
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::Type::getInt32Ty(m_context));
                            jsonObj = m_builder.CreateCall(addIntFunc, {jsonObj, keyStr, propValue}, "json_add");
                        } else if (prop.second.typeName == "boolean") {
                            llvm::FunctionCallee addBoolFunc = m_module->getOrInsertFunction("json_add_boolean",
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::Type::getInt32Ty(m_context));
                            jsonObj = m_builder.CreateCall(addBoolFunc, {jsonObj, keyStr, propValue}, "json_add");
                        }
                    }
                    
                    return jsonObj;
                }
            } else {
                // Legacy object handling
                auto propertiesIt = objectProperties.find(objectKey);
                if (propertiesIt != objectProperties.end()) {
                    auto& properties = propertiesIt->second;
                    auto& propertyTypes = objectPropertyTypes[objectKey];
                    
                    llvm::FunctionCallee createObjFunc = m_module->getOrInsertFunction("json_create_object",
                        llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                    llvm::Value* jsonObj = m_builder.CreateCall(createObjFunc, {}, "json_obj");
                    
                    for (const auto& pair : properties) {
                        llvm::Value* keyStr = m_builder.CreateGlobalString(pair.first, ".json_key");
                        llvm::Value* propValue = pair.second;
                        std::string propType = propertyTypes[pair.first];
                        
                        if (propType == "string") {
                            llvm::FunctionCallee addStrFunc = m_module->getOrInsertFunction("json_add_string",
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                            jsonObj = m_builder.CreateCall(addStrFunc, {jsonObj, keyStr, propValue}, "json_add");
                        } else if (propType == "i32") {
                            llvm::Value* loadedVal = m_builder.CreateLoad(llvm::Type::getInt32Ty(m_context), propValue);
                            llvm::FunctionCallee addIntFunc = m_module->getOrInsertFunction("json_add_int",
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::Type::getInt32Ty(m_context));
                            jsonObj = m_builder.CreateCall(addIntFunc, {jsonObj, keyStr, loadedVal}, "json_add");
                        } else if (propType == "boolean") {
                            llvm::Value* loadedVal = m_builder.CreateLoad(llvm::Type::getInt32Ty(m_context), propValue);
                            llvm::FunctionCallee addBoolFunc = m_module->getOrInsertFunction("json_add_boolean",
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                                llvm::Type::getInt32Ty(m_context));
                            jsonObj = m_builder.CreateCall(addBoolFunc, {jsonObj, keyStr, loadedVal}, "json_add");
                        }
                    }
                    
                    return jsonObj;
                }
            }
        }
        
        throw std::runtime_error("JSON.stringify only supports object variables currently.");
    }

    if (node->functionName == "print" || node->functionName == "println")
    {
        if (node->arguments.size() != 1)
        {
            std::cerr << "Codegen Error: '" << node->functionName << "' expects exactly one argument.\n";
            throw std::runtime_error("'" + node->functionName + "' expects one argument.");
        }

        bool addNewline = (node->functionName == "println");

        // CHECK FOR OBJECT
        bool isObject = false;
        std::string objectKey;
        if (auto* varExpr = dynamic_cast<VariableExpressionNode*>(node->arguments[0].get())) {
            auto objKeyIt = variableToObjectKey.find(varExpr->name);
            if (objKeyIt != variableToObjectKey.end()) {
                isObject = true;
                objectKey = objKeyIt->second;
            }
        }

        if (isObject) {
            auto layoutIt = objectLayouts.find(objectKey);
            if (layoutIt != objectLayouts.end()) {
                const ObjectOptimizer::ObjectLayout& layout = layoutIt->second;
                
                llvm::Value* storageSlot = variableStorage(dynamic_cast<VariableExpressionNode*>(node->arguments[0].get())->name);
                if (storageSlot != nullptr) {
                    llvm::Value* objectPtrAlloca = storageSlot;
                    llvm::Value* objectPtr = m_builder.CreateLoad(
                        llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                        objectPtrAlloca,
                        "obj_ptr_load"
                    );
                    llvm::Value* structPtr = m_builder.CreateBitCast(
                        objectPtr,
                        llvm::PointerType::get(layout.structType, 0),
                        "struct_cast"
                    );
                    
                    std::string formatString = "{";
                    std::vector<llvm::Value*> printfArgs;
                    printfArgs.push_back(nullptr); // Placeholder for format string
                    
                    for (size_t i = 0; i < layout.properties.size(); ++i) {
                        const auto& prop = layout.properties[i];
                        if (i > 0) formatString += ",";
                        formatString += "\"" + prop.first + "\":";
                        
                        llvm::Value* propValue = objectOptimizer.generateDirectPropertyAccess(
                            m_builder, structPtr, prop.first, layout);
                        
                        if (prop.second.typeName == "string") {
                            formatString += "\"%s\"";
                            printfArgs.push_back(propValue);
                        } else if (prop.second.typeName == "i32") {
                            formatString += "%d";
                            printfArgs.push_back(propValue);
                        } else if (prop.second.typeName == "boolean") {
                            formatString += "%s";
                            llvm::Value* isTrue = m_builder.CreateICmpNE(propValue, llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));
                            llvm::Value* trueStr = m_builder.CreateGlobalString("true", ".true_str");
                            llvm::Value* falseStr = m_builder.CreateGlobalString("false", ".false_str");
                            llvm::Value* boolStr = m_builder.CreateSelect(isTrue, trueStr, falseStr);
                            printfArgs.push_back(boolStr);
                        } else {
                            formatString += "%d";
                            printfArgs.push_back(propValue);
                        }
                    }
                    
                    formatString += "}";
                    if (addNewline) formatString += "\n";
                    
                    llvm::Value* formatStrVal = m_builder.CreateGlobalString(formatString, ".obj_format");
                    printfArgs[0] = formatStrVal;
                    
                    llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
                    return m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
                }
            } else {
                // Legacy object handling
                auto propertiesIt = objectProperties.find(objectKey);
                if (propertiesIt != objectProperties.end()) {
                    auto& properties = propertiesIt->second;
                    auto& propertyTypes = objectPropertyTypes[objectKey];
                    
                    std::string formatString = "{";
                    std::vector<llvm::Value*> printfArgs;
                    printfArgs.push_back(nullptr); // Placeholder
                    
                    bool first = true;
                    for (const auto& pair : properties) {
                        if (!first) formatString += ",";
                        first = false;
                        
                        formatString += "\"" + pair.first + "\":";
                        llvm::Value* propValue = pair.second;
                        std::string propType = propertyTypes[pair.first];
                        
                        if (propType == "string") {
                            formatString += "\"%s\"";
                            printfArgs.push_back(propValue);
                        } else if (propType == "i32") {
                            formatString += "%d";
                            llvm::Value* loadedVal = m_builder.CreateLoad(llvm::Type::getInt32Ty(m_context), propValue);
                            printfArgs.push_back(loadedVal);
                        } else if (propType == "boolean") {
                            formatString += "%s";
                            llvm::Value* loadedVal = m_builder.CreateLoad(llvm::Type::getInt32Ty(m_context), propValue);
                            llvm::Value* isTrue = m_builder.CreateICmpNE(loadedVal, llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));
                            llvm::Value* trueStr = m_builder.CreateGlobalString("true", ".true_str");
                            llvm::Value* falseStr = m_builder.CreateGlobalString("false", ".false_str");
                            llvm::Value* boolStr = m_builder.CreateSelect(isTrue, trueStr, falseStr);
                            printfArgs.push_back(boolStr);
                        }
                    }
                    
                    formatString += "}";
                    if (addNewline) formatString += "\n";
                    
                    llvm::Value* formatStrVal = m_builder.CreateGlobalString(formatString, ".obj_format");
                    printfArgs[0] = formatStrVal;
                    
                    llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
                    return m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
                }
            }
        }

        llvm::Value *argValue = visit(node->arguments[0].get());
        if (!argValue)
        {
            std::cerr << "Codegen Error: Failed to generate code for '" << node->functionName << "' argument.\n";
            throw std::runtime_error("Failed to generate code for '" + node->functionName + "' argument.");
        }

        llvm::Type *argType = argValue->getType();

        if (argType->isPointerTy())
        {
            // Assume it's a string pointer
            if (addNewline) {
                // Use puts for println (automatically adds newline)
                llvm::FunctionCallee putsFunc = getOrDeclarePuts();
                m_builder.CreateCall(putsFunc, argValue, "putsCall");
            } else {
                // Use printf with "%s" for print (no newline)
                llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
                llvm::Value *formatStr = m_builder.CreateGlobalString("%s", ".format_str");
                std::vector<llvm::Value *> printfArgs = {formatStr, argValue};
                m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
            }
        }
        else if (argType->isIntegerTy())
        {
            // Argument is an integer (i1 booleans widened to i32)
            if (!argType->isIntegerTy(32)) {
                argValue = coerceValue(argValue, llvm::Type::getInt32Ty(m_context));
            }
            llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
            // Create format string with or without newline
            std::string formatString = addNewline ? "%d\n" : "%d";
            llvm::Value *formatStr = m_builder.CreateGlobalString(formatString, ".format_int");
            std::vector<llvm::Value *> printfArgs = {formatStr, argValue};
            m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
        }
        else if (argType->isDoubleTy())
        {
            llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
            std::string formatString = addNewline ? "%g\n" : "%g";
            llvm::Value *formatStr = m_builder.CreateGlobalString(formatString, ".format_f64");
            std::vector<llvm::Value *> printfArgs = {formatStr, argValue};
            m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
        }
        else
        {
            std::cerr << "Codegen Error: '" << node->functionName << "' argument type not supported. Expected string, i32, f64, or boolean.\n";
            throw std::runtime_error("'" + node->functionName + "' argument type not supported.");
        }
        
        // print/println don't return values
        return nullptr;
    }
    else
    {
        // Handle external C++ functions
        // First check if the function exists
        llvm::FunctionCallee func = getOrDeclareExternalFunction(node->functionName);
        if (!func) {
            std::cerr << "Codegen Error: Unsupported function call '" << node->functionName << "'.\n";
            throw std::runtime_error("Unsupported function call: " + node->functionName);
        }
        
        // Function exists, generate the call
        llvm::Value *result = generateExternalFunctionCall(node);
        return result; // Can be nullptr for void functions, which is fine
    }
}

// --- Main Generation Method ---
llvm::Module *CodeGen::generate(ProgramNode *astRoot)
{
    if (!astRoot)
    {
        std::cerr << "Codegen Error: Cannot generate code from null AST.\n";
        return nullptr;
    }
    
    try
    {
        visit(astRoot);
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Caught codegen exception: " << e.what() << std::endl;
        return nullptr;
    }

    if (llvm::verifyModule(*m_module, &llvm::errs()))
    {
        std::cerr << "Error: LLVM module verification failed after generation!\n";
        return nullptr;
    }
    
    return m_module.get();
}

// Array literal implementation - proper version with dynamic allocation
llvm::Value *CodeGen::visit(ArrayLiteralNode *node)
{
    // Determine element type based on first element or explicit type
    std::string elemType = node->elementType;
    
    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    llvm::FunctionCallee createFunc;
    if (isObjectTypeName(elemType)) {
        createFunc = m_module->getOrInsertFunction("array_create_object", charPtr);
    } else if (elemType == "string") {
        createFunc = m_module->getOrInsertFunction("array_create_string", charPtr);
    } else if (elemType == "f64") {
        createFunc = m_module->getOrInsertFunction("array_create_f64", charPtr);
    } else {
        // Default to i32
        createFunc = m_module->getOrInsertFunction("array_create_i32", charPtr);
    }

    // Create the dynamic array
    llvm::Value* arrPtr = m_builder.CreateCall(createFunc, {}, "array_ptr");

    // Initialize array elements
    for (size_t i = 0; i < node->elements.size(); ++i) {
        llvm::Value *elementValue = visit(node->elements[i].get());
        if (!elementValue) {
            throw std::runtime_error("Codegen Error: Failed to generate array element " + std::to_string(i));
        }

        if (elemType == "f64") {
            llvm::FunctionCallee pushFunc = m_module->getOrInsertFunction("array_push_f64",
                llvm::Type::getVoidTy(m_context), charPtr, llvm::Type::getDoubleTy(m_context));
            m_builder.CreateCall(pushFunc, {arrPtr, coerceValue(elementValue, llvm::Type::getDoubleTy(m_context))});
        } else if (elemType == "string" || elementValue->getType()->isPointerTy()) {
            llvm::FunctionCallee pushFunc = m_module->getOrInsertFunction("array_push_string",
                llvm::Type::getVoidTy(m_context), charPtr, charPtr);
            m_builder.CreateCall(pushFunc, {arrPtr, elementValue});
        } else {
            llvm::FunctionCallee pushFunc = m_module->getOrInsertFunction("array_push_i32",
                llvm::Type::getVoidTy(m_context), charPtr, llvm::Type::getInt32Ty(m_context));
            m_builder.CreateCall(pushFunc, {arrPtr, coerceValue(elementValue, llvm::Type::getInt32Ty(m_context))});
        }
    }

    return arrPtr;
}

// Array access implementation - proper version with type tracking
llvm::Value *CodeGen::visit(ArrayAccessNode *node)
{
    // Get the array value (should be a pointer)
    llvm::Value *arrayValue = visit(node->array.get());
    if (!arrayValue) {
        throw std::runtime_error("Codegen Error: Failed to generate array for access");
    }
    
    // Get the index value
    llvm::Value *indexValue = visit(node->index.get());
    if (!indexValue) {
        throw std::runtime_error("Codegen Error: Failed to generate array index");
    }
    
    // Ensure index is an integer
    if (!indexValue->getType()->isIntegerTy()) {
        throw std::runtime_error("Codegen Error: Array index must be an integer");
    }
    
    // Determine element type by looking up the variable type
    std::string elemType = "i32"; // default
    
    // Try to get the variable name from the array expression
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->array.get())) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) {
            std::string varType = typeIt->second;
            // Extract element type from array type (e.g., "string[]" -> "string")
            if (varType.length() > 2 && varType.substr(varType.length() - 2) == "[]") {
                elemType = varType.substr(0, varType.length() - 2);
            }
        }
    }
    
    // Call the appropriate C++ dynamic array function
    if (isObjectTypeName(elemType)) {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_object",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0), llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0), llvm::Type::getInt32Ty(m_context));
        return m_builder.CreateCall(getFunc, {arrayValue, indexValue}, "array_element");
    } else if (elemType == "string") {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_string",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
        return m_builder.CreateCall(getFunc, {arrayValue, indexValue}, "array_element");
    } else if (elemType == "f64") {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_f64",
            llvm::Type::getDoubleTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
        return m_builder.CreateCall(getFunc, {arrayValue, indexValue}, "array_element");
    } else {
        llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("array_get_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
        return m_builder.CreateCall(getFunc, {arrayValue, indexValue}, "array_element");
    }
}

// Object literal implementation - basic version
llvm::Value *CodeGen::visit(ObjectLiteralNode *node)
{
    // PHASE 1 OPTIMIZATION: Use optimized object creation with direct struct access
    // This replaces hash map storage with native LLVM structs for 10-50x speedup
    
    if (node->properties.empty()) {
        // Empty object {} - create an empty object structure
        return createEmptyObject();
    }
    
    // Use OPTIMIZED object creation with direct struct access
    return createOptimizedObjectWithProperties(node);
}

// Helper function to create an empty object
llvm::Value *CodeGen::createEmptyObject()
{
    // Empty objects are represented as a null object pointer
    return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
}

// Helper function to create object with properties
llvm::Value *CodeGen::createObjectWithProperties(ObjectLiteralNode *node)
{
    // For the initial implementation, we'll create a simple object representation
    // that can be extended later for full TypeScript compatibility
    
    // Count properties to determine object size
    size_t propertyCount = node->properties.size();
    
    // Create a simple object ID based on property count
    // This is a simplified approach - a full implementation would use
    // proper object structures with property tables
    llvm::Value* objectId = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), propertyCount);
    
    // Store object properties for later access
    // We'll use the object node pointer as a unique identifier
    std::string objectKey = "obj_" + std::to_string(reinterpret_cast<uintptr_t>(node));
    
    std::map<std::string, llvm::Value*> properties;
    std::map<std::string, std::string> propertyTypes;
    
    // Store object properties in global variables and track them
    for (size_t i = 0; i < node->properties.size(); ++i) {
        const auto& prop = node->properties[i];
        
        // Generate a unique name for this property
        std::string propName = ".obj_" + std::to_string(reinterpret_cast<uintptr_t>(node)) + "_" + prop.key;
        
        // Generate code for the property value
        llvm::Value* propValue = visit(prop.value.get());
        
        if (propValue) {
            if (auto* strLit = dynamic_cast<StringLiteralNode*>(prop.value.get())) {
                // Handle string properties
                llvm::Constant* strConstant = llvm::ConstantDataArray::getString(m_context, strLit->value, true);
                llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                    *m_module,
                    strConstant->getType(),
                    true,
                    llvm::GlobalValue::PrivateLinkage,
                    strConstant,
                    propName
                );
                properties[prop.key] = globalStr;
                propertyTypes[prop.key] = "string";
            }
            else if (auto* intLit = dynamic_cast<IntegerLiteralNode*>(prop.value.get())) {
                // Handle integer properties
                llvm::Constant* intConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), intLit->value);
                llvm::GlobalVariable* globalInt = new llvm::GlobalVariable(
                    *m_module,
                    llvm::Type::getInt32Ty(m_context),
                    true,
                    llvm::GlobalValue::PrivateLinkage,
                    intConstant,
                    propName
                );
                properties[prop.key] = globalInt;
                propertyTypes[prop.key] = "i32";
            }
            else if (auto* boolLit = dynamic_cast<BooleanLiteralNode*>(prop.value.get())) {
                // Handle boolean properties
                llvm::Constant* boolConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), boolLit->value ? 1 : 0);
                llvm::GlobalVariable* globalBool = new llvm::GlobalVariable(
                    *m_module,
                    llvm::Type::getInt32Ty(m_context),
                    true,
                    llvm::GlobalValue::PrivateLinkage,
                    boolConstant,
                    propName
                );
                properties[prop.key] = globalBool;
                propertyTypes[prop.key] = "boolean";
            }
        }
    }
    
    // Store the properties for later access
    objectProperties[objectKey] = properties;
    objectPropertyTypes[objectKey] = propertyTypes;
    
    return objectId;
}

bool CodeGen::isObjectTypeName(const std::string &typeName) {
    return classes.count(typeName) > 0 || interfaces.count(typeName) > 0;
}

// True when an expression is a pointer that is NOT text: a class instance, an
// opaque `ptr` from C, or `null`. Equality on these must compare addresses;
// strcmp would read them as strings.
bool CodeGen::isNonStringPointer(ExpressionNode *expr) {
    if (!expr) return false;
    if (dynamic_cast<NullLiteralNode*>(expr)) return true;
    if (dynamic_cast<NewExpressionNode*>(expr)) return true;

    auto isHandleType = [&](const std::string &t) {
        return t == "ptr" || t == "object" || isObjectTypeName(t);
    };

    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(expr)) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end() && isHandleType(typeIt->second)) return true;
        return variableToObjectKey.count(varExpr->name) > 0;
    }
    if (auto *call = dynamic_cast<FunctionCallNode*>(expr)) {
        auto externIt = externFunctions.find(call->functionName);
        if (externIt != externFunctions.end()) return isHandleType(externIt->second->returnType);
        auto retIt = functionReturnTypes.find(call->functionName);
        if (retIt != functionReturnTypes.end()) return isHandleType(retIt->second);
    }
    if (auto *arrAccess = dynamic_cast<ArrayAccessNode*>(expr)) {
        if (auto *arrVar = dynamic_cast<VariableExpressionNode*>(arrAccess->array.get())) {
            auto typeIt = variableTypes.find(arrVar->name);
            if (typeIt != variableTypes.end()) {
                const std::string &arrType = typeIt->second;
                if (arrType.size() > 2 && arrType.substr(arrType.size() - 2) == "[]") {
                    return isHandleType(arrType.substr(0, arrType.size() - 2));
                }
            }
        }
    }
    return false;
}

std::string CodeGen::objectKeyForTypeName(const std::string &typeName) {
    auto classIt = classes.find(typeName);
    if (classIt != classes.end() && classIt->second->objectTemplate) {
        return "opt_obj_" +
               std::to_string(reinterpret_cast<uintptr_t>(classIt->second->objectTemplate.get()));
    }
    return "";
}

std::string CodeGen::getExpressionObjectKey(ExpressionNode* expr) {
    if (auto* varExpr = dynamic_cast<VariableExpressionNode*>(expr)) {
        if (varExpr->name == "this" && !currentThisObjectKey.empty()) {
            return currentThisObjectKey;
        }
        auto it = variableToObjectKey.find(varExpr->name);
        if (it != variableToObjectKey.end()) return it->second;
        // Fall back to the variable's recorded type: a class-typed value knows
        // its layout even when it was never bound to an object literal here
        // (a parameter, or a value returned from another function).
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) {
            std::string key = objectKeyForTypeName(typeIt->second);
            if (!key.empty()) return key;
        }
    } else if (auto* newExpr = dynamic_cast<NewExpressionNode*>(expr)) {
        return objectKeyForTypeName(newExpr->className);
    } else if (auto* call = dynamic_cast<FunctionCallNode*>(expr)) {
        // A function declared to return a class hands back that layout
        auto retIt = functionReturnTypes.find(call->functionName);
        if (retIt != functionReturnTypes.end()) return objectKeyForTypeName(retIt->second);
    } else if (auto* arrAccess = dynamic_cast<ArrayAccessNode*>(expr)) {
        // Element of an object array: rocks[i].x
        if (auto* arrVar = dynamic_cast<VariableExpressionNode*>(arrAccess->array.get())) {
            auto typeIt = variableTypes.find(arrVar->name);
            if (typeIt != variableTypes.end()) {
                const std::string &arrType = typeIt->second;
                if (arrType.size() > 2 && arrType.substr(arrType.size() - 2) == "[]") {
                    return objectKeyForTypeName(arrType.substr(0, arrType.size() - 2));
                }
            }
        }
    } else if (auto* objAccess = dynamic_cast<ObjectAccessNode*>(expr)) {
        std::string parentKey = getExpressionObjectKey(objAccess->object.get());
        if (!parentKey.empty()) {
            auto layoutIt = objectLayouts.find(parentKey);
            if (layoutIt != objectLayouts.end()) {
                const ObjectOptimizer::ObjectLayout& layout = layoutIt->second;
                auto idxIt = layout.propertyIndices.find(objAccess->property);
                if (idxIt != layout.propertyIndices.end()) {
                    const ObjectOptimizer::PropertyInfo& propInfo = layout.properties[idxIt->second].second;
                    if (propInfo.typeName.substr(0, 7) == "object:") {
                        return propInfo.typeName.substr(7);
                    }
                }
            }
        }
    }
    return "";
}

// Computes a class's struct layout from its declared field types alone, without
// generating any code. Runs in pass 0 so that property access on a class-typed
// value works even before the class has been instantiated anywhere — which is
// what makes `function make(): Vec` and `Vec[]` usable.
void CodeGen::registerClassLayout(ClassDeclarationNode *cls)
{
    if (!cls || !cls->objectTemplate) return;
    ObjectLiteralNode *tmpl = cls->objectTemplate.get();
    std::string objectKey = "opt_obj_" + std::to_string(reinterpret_cast<uintptr_t>(tmpl));
    if (objectLayouts.count(objectKey)) return;

    std::vector<std::pair<std::string, std::string>> propertyInfo;
    for (const auto &prop : tmpl->properties) {
        if (prop.method) {
            objectMethods[objectKey][prop.key] = prop.method.get();
            continue;
        }

        // Prefer the declared field type; fall back to the default's literal kind
        std::string propertyType = prop.declaredType;
        if (propertyType == "number") propertyType = "f64";
        if (propertyType.empty() || (propertyType != "string" && propertyType != "i32" &&
                                     propertyType != "f64" && propertyType != "boolean")) {
            if (dynamic_cast<StringLiteralNode*>(prop.value.get())) propertyType = "string";
            else if (dynamic_cast<FloatLiteralNode*>(prop.value.get())) propertyType = "f64";
            else if (dynamic_cast<BooleanLiteralNode*>(prop.value.get())) propertyType = "boolean";
            else if (dynamic_cast<IntegerLiteralNode*>(prop.value.get())) propertyType = "i32";
            else if (!prop.declaredType.empty()) {
                // Class-, interface- or array-typed field: an opaque pointer,
                // which the layout represents the same way it does a string
                propertyType = "string";
            } else {
                propertyType = "i32";
            }
        }
        propertyInfo.push_back({prop.key, propertyType});
    }

    if (propertyInfo.empty()) {
        propertyInfo.push_back({"__pad", "i32"});
    }

    objectLayouts[objectKey] = objectOptimizer.createObjectLayout(propertyInfo, m_context);
    variableToObjectKey[objectKey] = objectKey;
}

// OPTIMIZED: Phase 1 object creation with direct struct access
llvm::Value *CodeGen::createOptimizedObjectWithProperties(ObjectLiteralNode *node)
{
    // Phase 1 Optimization: Replace hash map storage with direct struct access
    
    // Generate unique object key that matches property access
    std::string objectKey = "opt_obj_" + std::to_string(reinterpret_cast<uintptr_t>(node));

    // Extract property information for layout creation
    std::vector<std::pair<std::string, std::string>> propertyInfo;
    std::vector<llvm::Value*> propertyValues;

    for (const auto& prop : node->properties) {
        std::string propertyType;
        llvm::Value* propValue = nullptr;

        // Methods are compiled as functions taking `this`; they occupy no struct slot
        if (prop.method) {
            objectMethods[objectKey][prop.key] = prop.method.get();
            continue;
        }

        // Determine property type and generate value
        if (auto* strLit = dynamic_cast<StringLiteralNode*>(prop.value.get())) {
            propertyType = "string";
            // Create string constant
            llvm::Constant* strConstant = llvm::ConstantDataArray::getString(m_context, strLit->value, true);
            llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                *m_module,
                strConstant->getType(),
                true,
                llvm::GlobalValue::PrivateLinkage,
                strConstant,
                "str_" + prop.key
            );
            propValue = m_builder.CreateBitCast(globalStr, llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
        }
        else if (auto* intLit = dynamic_cast<IntegerLiteralNode*>(prop.value.get())) {
            propertyType = "i32";
            propValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), intLit->value);
        }
        else if (auto* boolLit = dynamic_cast<BooleanLiteralNode*>(prop.value.get())) {
            propertyType = "boolean";
            propValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), boolLit->value ? 1 : 0);
        }
        else if (auto* floatLit = dynamic_cast<FloatLiteralNode*>(prop.value.get())) {
            propertyType = "f64";
            propValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(m_context), floatLit->value);
        }
        else if (auto* objLit = dynamic_cast<ObjectLiteralNode*>(prop.value.get())) {
            propValue = visit(objLit);
            std::string childKey = "opt_obj_" + std::to_string(reinterpret_cast<uintptr_t>(objLit));
            propertyType = "object:" + childKey;

            // Cast to generic pointer for storage
            propValue = m_builder.CreateBitCast(propValue, llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
        }
        else {
            // General expression: generate and classify by the produced value type
            propValue = visit(prop.value.get());
            if (!propValue) {
                throw std::runtime_error("Codegen Error: Failed to generate object property '" + prop.key + "'");
            }
            if (propValue->getType()->isPointerTy()) {
                propertyType = "string";
            } else if (propValue->getType()->isDoubleTy()) {
                propertyType = "f64";
            } else {
                if (propValue->getType()->isIntegerTy(1)) {
                    propValue = m_builder.CreateZExt(propValue, llvm::Type::getInt32Ty(m_context), "bool_ext");
                }
                propertyType = "i32";
            }
        }

        propertyInfo.push_back({prop.key, propertyType});
        propertyValues.push_back(propValue);
    }

    // Objects consisting only of methods still need a non-empty struct
    if (propertyInfo.empty()) {
        propertyInfo.push_back({"__pad", "i32"});
        propertyValues.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));
    }

    // Create optimized object layout (compile-time calculation)
    ObjectOptimizer::ObjectLayout layout = objectOptimizer.createObjectLayout(propertyInfo, m_context);

    // Store layout for property access optimization
    objectLayouts[objectKey] = layout;
    
    // Create optimized object with direct struct access
    llvm::Value* optimizedObject = objectCreator.createOptimizedObject(
        m_builder,
        m_context,
        m_module.get(),
        layout,
        propertyValues,
        allocateObjectsOnHeap
    );
    
    // Track this object for optimized property access
    variableToObjectKey[objectKey] = objectKey;
    
    return optimizedObject;
}

// Object access implementation - with array.length support and native object properties
llvm::Value *CodeGen::visit(ObjectAccessNode *node)
{
    // Check if this is array.length access
    if (node->property == "length") {
        // Check if the base is a variable that refers to an array
        if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->object.get())) {
            // Look up the variable type
            auto typeIt = variableTypes.find(varExpr->name);
            if (typeIt != variableTypes.end()) {
                std::string varType = typeIt->second;
                // Check if it's an array type
                if (varType.length() > 2 && varType.substr(varType.length() - 2) == "[]") {
                    llvm::Value* storageSlot = variableStorage(varExpr->name);
                    if (storageSlot != nullptr) {
                        llvm::Value* arrPtrAlloca = storageSlot;
                        llvm::Value* arrPtr = m_builder.CreateLoad(
                            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                            arrPtrAlloca,
                            "arr_ptr_load"
                        );
                        
                        llvm::FunctionCallee lenFunc = m_module->getOrInsertFunction("array_length",
                            llvm::Type::getInt32Ty(m_context),
                            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                        return m_builder.CreateCall(lenFunc, {arrPtr}, "array_len");
                    } else {
                        throw std::runtime_error("Codegen Error: Array size not found for variable '" + varExpr->name + "'");
                    }
                } else {
                    throw std::runtime_error("Codegen Error: .length property is only supported on arrays, not on '" + varType + "'");
                }
            } else {
                throw std::runtime_error("Codegen Error: Unknown variable '" + varExpr->name + "' in property access");
            }
        } else {
            // .length on a non-variable expression (e.g. arr.filter(f).length):
            // evaluate it and call array_length on the resulting pointer
            llvm::Value *value = visit(node->object.get());
            if (value && value->getType()->isPointerTy()) {
                llvm::FunctionCallee lenFunc = m_module->getOrInsertFunction("array_length",
                    llvm::Type::getInt32Ty(m_context),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                return m_builder.CreateCall(lenFunc, {value}, "array_len");
            }
            throw std::runtime_error("Codegen Error: .length is only supported on arrays");
        }
    }
    
    // PHASE 1 OPTIMIZATION: Handle optimized object property access
    std::string objectKey = getExpressionObjectKey(node->object.get());
    if (!objectKey.empty()) {
        auto layoutIt = objectLayouts.find(objectKey);
        
        if (layoutIt != objectLayouts.end()) {
            // OPTIMIZED PATH: Use direct struct access instead of hash map lookups
            const ObjectOptimizer::ObjectLayout& layout = layoutIt->second;
            
            llvm::Value* objectPtr = nullptr;
            
            if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->object.get())) {
                llvm::Value* storageSlot = variableStorage(varExpr->name);
                if (storageSlot != nullptr) {
                    llvm::Value* objectPtrAlloca = storageSlot;
                    objectPtr = m_builder.CreateLoad(
                        llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                        objectPtrAlloca,
                        "obj_ptr_load"
                    );
                }
            } else {
                // Any other expression that yields an object pointer: a call
                // result, an array element, a `new`, a nested property.
                objectPtr = visit(node->object.get());
            }
            
            if (objectPtr) {
                // Cast the generic pointer back to the struct type
                llvm::Value* structPtr = m_builder.CreateBitCast(
                    objectPtr,
                    llvm::PointerType::get(layout.structType, 0),
                    "struct_cast"
                );
                
                // Use optimized property access (10-50x faster than hash maps)
                llvm::Value* optimizedValue = objectOptimizer.generateDirectPropertyAccess(
                    m_builder,
                    structPtr,
                    node->property,
                    layout
                );
                
                if (optimizedValue) {
                    return optimizedValue;
                }
            }
        }
    }
    
    // FALLBACK: Legacy object property access (for compatibility)
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->object.get())) {
        auto objectKeyIt = variableToObjectKey.find(varExpr->name);
        
        if (objectKeyIt != variableToObjectKey.end()) {
            std::string objectKey = objectKeyIt->second;
            
            // Look up the object properties
            auto propertiesIt = objectProperties.find(objectKey);
            if (propertiesIt != objectProperties.end()) {
                auto& properties = propertiesIt->second;
                
                // Look up the specific property
                auto propertyIt = properties.find(node->property);
                if (propertyIt != properties.end()) {
                    llvm::Value* propertyValue = propertyIt->second;
                    
                    // Get the property type
                    auto propertyTypesIt = objectPropertyTypes.find(objectKey);
                    if (propertyTypesIt != objectPropertyTypes.end()) {
                        auto& propertyTypes = propertyTypesIt->second;
                        auto typeIt = propertyTypes.find(node->property);
                        if (typeIt != propertyTypes.end()) {
                            std::string propertyType = typeIt->second;
                            
                            // Load the value based on type
                            if (propertyType == "string") {
                                // For strings, return the global string pointer
                                return m_builder.CreateBitCast(propertyValue, llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                            } else if (propertyType == "i32" || propertyType == "boolean") {
                                // For integers and booleans, load the value
                                return m_builder.CreateLoad(llvm::Type::getInt32Ty(m_context), propertyValue);
                            }
                        }
                    }
                    
                    // Default: return the property value as-is
                    return propertyValue;
                } else {
                    throw std::runtime_error("Codegen Error: Property '" + node->property + "' not found on object '" + varExpr->name + "'");
                }
            } else {
                throw std::runtime_error("Codegen Error: Object properties not found for variable '" + varExpr->name + "'");
            }
        } else {
            // Check if it's a regular variable (not an object)
            auto typeIt = variableTypes.find(varExpr->name);
            if (typeIt != variableTypes.end()) {
                std::string varType = typeIt->second;
                
                if (varType == "json") {
                    llvm::Value* storageSlot = variableStorage(varExpr->name);
                    if (storageSlot != nullptr) {
                        llvm::Value* jsonStrPtrAlloca = storageSlot;
                        llvm::Value* jsonStrPtr = m_builder.CreateLoad(
                            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                            jsonStrPtrAlloca,
                            "json_str_load"
                        );
                        
                        llvm::Value* keyStr = m_builder.CreateGlobalString(node->property, ".json_key");
                        
                        llvm::FunctionCallee getAnyFunc = m_module->getOrInsertFunction("json_get_any",
                            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                            
                        return m_builder.CreateCall(getAnyFunc, {jsonStrPtr, keyStr}, "json_get_any_call");
                    }
                }
                
                throw std::runtime_error("Codegen Error: Cannot access property '" + node->property + "' on variable '" + varExpr->name + "' of type '" + varType + "'. Property access is only supported on objects.");
            } else {
                throw std::runtime_error("Codegen Error: Unknown variable '" + varExpr->name + "' in property access");
            }
        }
    }
    
    // For other cases, property access is not yet supported
    throw std::runtime_error("Codegen Error: Complex object property access is not yet supported. Only simple variable.property access is currently implemented.");
}

llvm::Value *CodeGen::visit(MethodCallNode *node)
{
    // Evaluate the object/base expression
    llvm::Value *objectValue = nullptr;
    std::string varType = "";

    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->object.get())) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) {
            varType = typeIt->second;
        }
        llvm::Value* storageSlot = variableStorage(varExpr->name);
        if (storageSlot != nullptr) {
            llvm::Value* ptrAlloca = storageSlot;
            objectValue = m_builder.CreateLoad(
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                ptrAlloca,
                "obj_ptr_load"
            );
        }
    } else {
        objectValue = visit(node->object.get());
    }

    if (!objectValue) {
        throw std::runtime_error("Codegen Error: Failed to evaluate object for method call");
    }

    // Is it a user-defined object method? (compiled with an implicit `this` parameter)
    std::string methodObjectKey = getExpressionObjectKey(node->object.get());
    if (!methodObjectKey.empty()) {
        auto objMethodsIt = objectMethods.find(methodObjectKey);
        if (objMethodsIt != objectMethods.end()) {
            auto methodIt = objMethodsIt->second.find(node->methodName);
            if (methodIt != objMethodsIt->second.end()) {
                llvm::Function *methodFn = getOrCreateMethodFunction(methodObjectKey, node->methodName);

                std::vector<llvm::Value*> args;
                args.push_back(objectValue); // `this`
                size_t argIndex = 1;
                for (const auto& arg : node->arguments) {
                    llvm::Value *argValue = visit(arg.get());
                    if (!argValue) {
                        throw std::runtime_error("Codegen Error: Failed to generate method argument");
                    }
                    if (argIndex < methodFn->arg_size()) {
                        argValue = coerceValue(argValue, methodFn->getFunctionType()->getParamType(argIndex));
                    }
                    args.push_back(argValue);
                    argIndex++;
                }

                if (methodFn->getReturnType()->isVoidTy()) {
                    m_builder.CreateCall(methodFn, args);
                    return nullptr;
                }
                return m_builder.CreateCall(methodFn, args, node->methodName + "_call");
            }
        }
    }

    // Is it an array method?
    if (varType.length() > 2 && varType.substr(varType.length() - 2) == "[]") {
        std::string elemType = varType.substr(0, varType.length() - 2);

        // Callback-based methods: map/filter/forEach/reduce/find
        if (node->methodName == "map" || node->methodName == "filter" ||
            node->methodName == "forEach" || node->methodName == "reduce" ||
            node->methodName == "find") {
            return generateArrayCallbackMethod(node, objectValue, elemType);
        }

        if (node->methodName == "push") {
            if (node->arguments.size() != 1) throw std::runtime_error("push() expects 1 argument");
            llvm::Value *argValue = visit(node->arguments[0].get());

            if (isObjectTypeName(elemType)) {
                llvm::FunctionCallee pushFunc = m_module->getOrInsertFunction("array_push_object",
                    llvm::Type::getVoidTy(m_context), llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0), llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                return m_builder.CreateCall(pushFunc, {objectValue, argValue});
            } else if (elemType == "f64") {
                llvm::FunctionCallee pushFunc = m_module->getOrInsertFunction("array_push_f64",
                    llvm::Type::getVoidTy(m_context),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                    llvm::Type::getDoubleTy(m_context));
                return m_builder.CreateCall(pushFunc,
                    {objectValue, coerceValue(argValue, llvm::Type::getDoubleTy(m_context))});
            } else if (elemType == "string" || argValue->getType()->isPointerTy()) {
                llvm::FunctionCallee pushFunc = m_module->getOrInsertFunction("array_push_string",
                    llvm::Type::getVoidTy(m_context),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                return m_builder.CreateCall(pushFunc, {objectValue, argValue});
            } else {
                llvm::FunctionCallee pushFunc = m_module->getOrInsertFunction("array_push_i32",
                    llvm::Type::getVoidTy(m_context),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                    llvm::Type::getInt32Ty(m_context));
                return m_builder.CreateCall(pushFunc, {objectValue, argValue});
            }
        } else if (node->methodName == "clear") {
            if (node->arguments.size() != 0) throw std::runtime_error("clear() expects 0 arguments");
            llvm::FunctionCallee clearFunc = m_module->getOrInsertFunction("array_clear",
                llvm::Type::getVoidTy(m_context),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
            return m_builder.CreateCall(clearFunc, {objectValue});
        } else if (node->methodName == "removeAt") {
            // Removing by index is what makes despawning possible in a game loop
            if (node->arguments.size() != 1) throw std::runtime_error("removeAt() expects 1 argument");
            llvm::Value *indexValue = coerceValue(visit(node->arguments[0].get()),
                                                  llvm::Type::getInt32Ty(m_context));
            llvm::FunctionCallee removeFunc = m_module->getOrInsertFunction("array_remove_at",
                llvm::Type::getVoidTy(m_context),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::Type::getInt32Ty(m_context));
            return m_builder.CreateCall(removeFunc, {objectValue, indexValue});
        } else if (node->methodName == "shift" || node->methodName == "pop") {
            if (node->arguments.size() != 0) throw std::runtime_error("shift/pop expects 0 arguments");

            std::string runtimeName = (node->methodName == "pop") ? "array_pop" : "array_shift";
            if (isObjectTypeName(elemType)) {
                llvm::FunctionCallee popFunc = m_module->getOrInsertFunction(runtimeName + "_object",
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                return m_builder.CreateCall(popFunc, {objectValue}, "removed_val");
            } else if (elemType == "f64") {
                llvm::FunctionCallee popFunc = m_module->getOrInsertFunction(runtimeName + "_f64",
                    llvm::Type::getDoubleTy(m_context),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                return m_builder.CreateCall(popFunc, {objectValue}, "removed_val");
            } else if (elemType == "string" || elemType.length() == 1 || elemType.find('<') != std::string::npos) {
                llvm::FunctionCallee popFunc = m_module->getOrInsertFunction(runtimeName + "_string",
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                return m_builder.CreateCall(popFunc, {objectValue}, "removed_val");
            } else {
                llvm::FunctionCallee popFunc = m_module->getOrInsertFunction(runtimeName + "_i32",
                    llvm::Type::getInt32Ty(m_context),
                    llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
                return m_builder.CreateCall(popFunc, {objectValue}, "removed_val");
            }
        }
    }
    
    // Is it a Set method?
    if (varType.substr(0, 4) == "Set<") {
        std::string elemType = varType.substr(4, varType.length() - 5);
        
        if (node->methodName == "add") {
            if (node->arguments.size() != 1) throw std::runtime_error("Set.add() expects 1 argument");
            llvm::Value *argValue = visit(node->arguments[0].get());
            
            // For now, default to string/pointer handling for Set
            llvm::FunctionCallee addFunc = m_module->getOrInsertFunction("set_add_string",
                llvm::Type::getVoidTy(m_context),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
            return m_builder.CreateCall(addFunc, {objectValue, argValue});
        } else if (node->methodName == "has") {
            if (node->arguments.size() != 1) throw std::runtime_error("Set.has() expects 1 argument");
            llvm::Value *argValue = visit(node->arguments[0].get());
            
            llvm::FunctionCallee hasFunc = m_module->getOrInsertFunction("set_has_string",
                llvm::Type::getInt32Ty(m_context),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
            return m_builder.CreateCall(hasFunc, {objectValue, argValue}, "set_has_val");
        }
    }
    
    // Is it a Map method?
    if (varType.find("Map<") != std::string::npos || varType.find("Graph<") != std::string::npos) {
        if (node->methodName == "set") {
            if (node->arguments.size() != 2) throw std::runtime_error("Map.set() expects 2 arguments");
            llvm::Value *keyVal = visit(node->arguments[0].get());
            llvm::Value *valVal = visit(node->arguments[1].get());
            
            // Map<string, object[]>
            llvm::FunctionCallee setFunc = m_module->getOrInsertFunction("map_set_s_o",
                llvm::Type::getVoidTy(m_context),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
            return m_builder.CreateCall(setFunc, {objectValue, keyVal, valVal});
        } else if (node->methodName == "get") {
            if (node->arguments.size() != 1) throw std::runtime_error("Map.get() expects 1 argument");
            llvm::Value *keyVal = visit(node->arguments[0].get());
            
            llvm::FunctionCallee getFunc = m_module->getOrInsertFunction("map_get_s_o",
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
            return m_builder.CreateCall(getFunc, {objectValue, keyVal}, "map_get_val");
        } else if (node->methodName == "has") {
            if (node->arguments.size() != 1) throw std::runtime_error("Map.has() expects 1 argument");
            llvm::Value *keyVal = visit(node->arguments[0].get());
            
            llvm::FunctionCallee hasFunc = m_module->getOrInsertFunction("map_has_s_o",
                llvm::Type::getInt32Ty(m_context),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
            return m_builder.CreateCall(hasFunc, {objectValue, keyVal}, "map_has_val");
        }
    }
    
    throw std::runtime_error("Codegen Error: Method '" + node->methodName + "' not supported on type '" + varType + "'");
}

llvm::Value *CodeGen::visit(NewExpressionNode *node)
{
    // User-defined class: instantiate the object template, then run the constructor
    auto classIt = classes.find(node->className);
    if (classIt != classes.end()) {
        ClassDeclarationNode *cls = classIt->second;
        // Class instances are heap-allocated so they can be returned from the
        // function that built them and stored in arrays. Object literals keep
        // their stack allocation, which is what the benchmarks measure.
        bool previousHeapMode = allocateObjectsOnHeap;
        allocateObjectsOnHeap = true;
        llvm::Value *objectPtr = visit(cls->objectTemplate.get());
        allocateObjectsOnHeap = previousHeapMode;
        llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
        llvm::Value *rawPtr = m_builder.CreateBitCast(objectPtr, charPtr, node->className + "_instance");

        if (cls->hasConstructor) {
            std::string objectKey = "opt_obj_" +
                std::to_string(reinterpret_cast<uintptr_t>(cls->objectTemplate.get()));
            llvm::Function *ctor = getOrCreateMethodFunction(objectKey, "constructor");

            std::vector<llvm::Value*> args;
            args.push_back(rawPtr); // `this`
            size_t argIndex = 1;
            for (const auto &arg : node->arguments) {
                llvm::Value *argValue = visit(arg.get());
                if (!argValue) {
                    throw std::runtime_error("Codegen Error: Failed to generate constructor argument for " + node->className);
                }
                if (argIndex < ctor->arg_size()) {
                    argValue = coerceValue(argValue, ctor->getFunctionType()->getParamType(argIndex));
                }
                args.push_back(argValue);
                argIndex++;
            }
            m_builder.CreateCall(ctor, args);
        }
        return rawPtr;
    }

    if (node->className == "Set") {
        std::string elemType = !node->genericTypes.empty() ? node->genericTypes[0] : "string";
        
        llvm::FunctionCallee createFunc;
        if (elemType == "string") {
            createFunc = m_module->getOrInsertFunction("set_create_string",
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
        } else {
            createFunc = m_module->getOrInsertFunction("set_create_i32",
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
        }
        return m_builder.CreateCall(createFunc, {}, "set_ptr");
    } else if (node->className == "Map") {
        // For BFS, we mostly need Map<string, string[]>
        llvm::FunctionCallee createFunc = m_module->getOrInsertFunction("map_create_s_o",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
        return m_builder.CreateCall(createFunc, {}, "map_ptr");
    }
    
    throw std::runtime_error("Codegen Error: 'new " + node->className + "' is not supported");
}

// External function call support
llvm::Value *CodeGen::generateExternalFunctionCall(FunctionCallNode *node)
{
    llvm::FunctionCallee func = getOrDeclareExternalFunction(node->functionName);
    if (!func) {
        return nullptr; // Function not found
    }
    
    // Generate arguments, coerced to the declared parameter types (e.g. i32 -> f64 for Math.*)
    std::vector<llvm::Value*> args;
    llvm::FunctionType* fnType = func.getFunctionType();
    size_t argIndex = 0;
    for (auto& arg : node->arguments) {
        llvm::Value* argValue = visit(arg.get());
        if (!argValue) {
            throw std::runtime_error("Codegen Error: Failed to generate argument for function " + node->functionName);
        }
        if (argIndex < fnType->getNumParams()) {
            argValue = coerceValue(argValue, fnType->getParamType(argIndex));
        }
        args.push_back(argValue);
        argIndex++;
    }
    
    // Check if the function returns void
    llvm::Type* returnType = func.getFunctionType()->getReturnType();
    if (returnType->isVoidTy()) {
        // For void functions, don't assign a name to the call
        m_builder.CreateCall(func, args);
        return nullptr;
    } else {
        // For non-void functions, create the call with a name
        return m_builder.CreateCall(func, args, node->functionName + "_call");
    }
}

llvm::FunctionCallee CodeGen::getOrDeclareExternalFunction(const std::string& name)
{
    // User-declared foreign functions win over the built-in stdlib table, so a
    // program can bind any C symbol (graphics, audio, ...) without compiler changes.
    auto externIt = externFunctions.find(name);
    if (externIt != externFunctions.end()) {
        const ExternDeclarationNode *decl = externIt->second;
        std::vector<llvm::Type*> paramTypes;
        paramTypes.reserve(decl->parameters.size());
        for (const auto &param : decl->parameters) {
            paramTypes.push_back(getLLVMType(param.type));
        }
        llvm::FunctionType *fnType =
            llvm::FunctionType::get(getLLVMType(decl->returnType), paramTypes, false);
        // Emit a call to the bound C symbol, which may differ from the name used
        // in Cypescript (`declare function drawRect(...) = "cyps_rect";`)
        return m_module->getOrInsertFunction(decl->symbolName, fnType);
    }

    // Math functions
    if (name == "math_sqrt") {
        return m_module->getOrInsertFunction("math_sqrt",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_pow") {
        return m_module->getOrInsertFunction("math_pow",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_abs_f64") {
        return m_module->getOrInsertFunction("math_abs_f64",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_abs_i32") {
        return m_module->getOrInsertFunction("math_abs_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "math_floor") {
        return m_module->getOrInsertFunction("math_floor",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_sin") {
        return m_module->getOrInsertFunction("math_sin",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_cos") {
        return m_module->getOrInsertFunction("math_cos",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_tan") {
        return m_module->getOrInsertFunction("math_tan",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_log") {
        return m_module->getOrInsertFunction("math_log",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_exp") {
        return m_module->getOrInsertFunction("math_exp",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_ceil") {
        return m_module->getOrInsertFunction("math_ceil",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_round") {
        return m_module->getOrInsertFunction("math_round",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_min") {
        return m_module->getOrInsertFunction("math_min",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_max") {
        return m_module->getOrInsertFunction("math_max",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_atan2") {
        return m_module->getOrInsertFunction("math_atan2",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "math_random") {
        return m_module->getOrInsertFunction("math_random",
            llvm::Type::getDoubleTy(m_context));
    }
    
    // String functions
    else if (name == "string_reverse") {
        return m_module->getOrInsertFunction("string_reverse",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "string_upper") {
        return m_module->getOrInsertFunction("string_upper",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "string_lower") {
        return m_module->getOrInsertFunction("string_lower",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "string_length") {
        return m_module->getOrInsertFunction("string_length",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "string_substring") {
        return m_module->getOrInsertFunction("string_substring",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "string_find") {
        return m_module->getOrInsertFunction("string_find",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "string_concat") {
        return m_module->getOrInsertFunction("string_concat",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    
    // Array functions
    else if (name == "array_sum_i32") {
        return m_module->getOrInsertFunction("array_sum_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "array_max_i32") {
        return m_module->getOrInsertFunction("array_max_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "array_min_i32") {
        return m_module->getOrInsertFunction("array_min_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    
    // File I/O functions
    else if (name == "file_read") {
        return m_module->getOrInsertFunction("file_read",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "file_write") {
        return m_module->getOrInsertFunction("file_write",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "file_exists") {
        return m_module->getOrInsertFunction("file_exists",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    
    // Utility functions
    else if (name == "random_int") {
        return m_module->getOrInsertFunction("random_int",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "random_double") {
        return m_module->getOrInsertFunction("random_double",
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "random_seed") {
        return m_module->getOrInsertFunction("random_seed",
            llvm::Type::getVoidTy(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    
    // Custom Math functions - Advanced Math
    else if (name == "math_gcd") {
        return m_module->getOrInsertFunction("math_gcd",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "math_lcm") {
        return m_module->getOrInsertFunction("math_lcm",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "math_is_prime") {
        return m_module->getOrInsertFunction("math_is_prime",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "math_fibonacci") {
        return m_module->getOrInsertFunction("math_fibonacci",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "math_factorial") {
        return m_module->getOrInsertFunction("math_factorial",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    
    // Custom Math functions - Statistics (return double, but we'll need f64 support)
    else if (name == "stats_mean") {
        return m_module->getOrInsertFunction("stats_mean",
            llvm::Type::getDoubleTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "stats_median") {
        return m_module->getOrInsertFunction("stats_median",
            llvm::Type::getDoubleTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "stats_stddev") {
        return m_module->getOrInsertFunction("stats_stddev",
            llvm::Type::getDoubleTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    
    // Custom Math functions - Geometry (return double, but we'll need f64 support)
    else if (name == "geom_distance") {
        return m_module->getOrInsertFunction("geom_distance",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "geom_circle_area") {
        return m_module->getOrInsertFunction("geom_circle_area",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "geom_rectangle_area") {
        return m_module->getOrInsertFunction("geom_rectangle_area",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "geom_triangle_area") {
        return m_module->getOrInsertFunction("geom_triangle_area",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    
    // SIMD-optimized functions (ARM NEON)
    else if (name == "neon_array_sum_i32") {
        return m_module->getOrInsertFunction("neon_array_sum_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "neon_array_max_i32") {
        return m_module->getOrInsertFunction("neon_array_max_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "neon_array_min_i32") {
        return m_module->getOrInsertFunction("neon_array_min_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "neon_array_multiply_i32") {
        return m_module->getOrInsertFunction("neon_array_multiply_i32",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "neon_array_add_i32") {
        return m_module->getOrInsertFunction("neon_array_add_i32",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "neon_dot_product_i32") {
        return m_module->getOrInsertFunction("neon_dot_product_i32",
            llvm::Type::getInt64Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "neon_array_count_equal_i32") {
        return m_module->getOrInsertFunction("neon_array_count_equal_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "neon_performance_ratio") {
        return m_module->getOrInsertFunction("neon_performance_ratio",
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "neon_available") {
        return m_module->getOrInsertFunction("neon_available",
            llvm::Type::getInt32Ty(m_context));
    }
    
    // Memory-optimized functions
    else if (name == "memory_pool_init") {
        return m_module->getOrInsertFunction("memory_pool_init",
            llvm::Type::getVoidTy(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "memory_pool_alloc") {
        return m_module->getOrInsertFunction("memory_pool_alloc",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "memory_pool_reset") {
        return m_module->getOrInsertFunction("memory_pool_reset",
            llvm::Type::getVoidTy(m_context));
    }
    else if (name == "cache_optimized_sum_i32") {
        return m_module->getOrInsertFunction("cache_optimized_sum_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "cache_optimized_max_i32") {
        return m_module->getOrInsertFunction("cache_optimized_max_i32",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "memory_efficient_copy_i32") {
        return m_module->getOrInsertFunction("memory_efficient_copy_i32",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "cache_aware_transpose_i32") {
        return m_module->getOrInsertFunction("cache_aware_transpose_i32",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "memory_bandwidth_test_i32") {
        return m_module->getOrInsertFunction("memory_bandwidth_test_i32",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "cache_miss_comparison_i32") {
        return m_module->getOrInsertFunction("cache_miss_comparison_i32",
            llvm::Type::getInt64Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "memory_optimized_string_compare") {
        return m_module->getOrInsertFunction("memory_optimized_string_compare",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "get_memory_stats") {
        return m_module->getOrInsertFunction("get_memory_stats",
            llvm::Type::getVoidTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0));
    }
    else if (name == "memory_pool_cleanup") {
        return m_module->getOrInsertFunction("memory_pool_cleanup",
            llvm::Type::getVoidTy(m_context));
    }
    
    // JSON functions
    else if (name == "json_create_object") {
        return m_module->getOrInsertFunction("json_create_object",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_create_array") {
        return m_module->getOrInsertFunction("json_create_array",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_add_string") {
        return m_module->getOrInsertFunction("json_add_string",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_add_number") {
        return m_module->getOrInsertFunction("json_add_number",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getDoubleTy(m_context));
    }
    else if (name == "json_add_int") {
        return m_module->getOrInsertFunction("json_add_int",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "json_add_boolean") {
        return m_module->getOrInsertFunction("json_add_boolean",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "json_get_string") {
        return m_module->getOrInsertFunction("json_get_string",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_get_number") {
        return m_module->getOrInsertFunction("json_get_number",
            llvm::Type::getDoubleTy(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_get_int") {
        return m_module->getOrInsertFunction("json_get_int",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_get_boolean") {
        return m_module->getOrInsertFunction("json_get_boolean",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_is_valid") {
        return m_module->getOrInsertFunction("json_is_valid",
            llvm::Type::getInt32Ty(m_context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_prettify") {
        return m_module->getOrInsertFunction("json_prettify",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    else if (name == "json_minify") {
        return m_module->getOrInsertFunction("json_minify",
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
            llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0));
    }
    
    return nullptr; // Function not found
}

// Function Declaration Visitor
// Creates the LLVM function and registers it, without generating a body.
// Signatures are declared for every function up front so that main (generated
// first, because it is what creates the module-level globals) can call them.
llvm::Function *CodeGen::declareFunctionSignature(FunctionDeclarationNode *node)
{
    auto existing = declaredFunctions.find(node->functionName);
    if (existing != declaredFunctions.end()) return existing->second;

    // Convert parameter types to LLVM types
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node->parameters) {
        llvm::Type* paramType = getLLVMType(param.type);
        if (!paramType) {
            throw std::runtime_error("Unknown parameter type: " + param.type);
        }
        paramTypes.push_back(paramType);
    }

    // Convert return type to LLVM type
    llvm::Type* returnType = getLLVMType(node->returnType);
    if (!returnType) {
        throw std::runtime_error("Unknown return type: " + node->returnType);
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* function = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node->functionName,
        m_module.get()
    );

    declaredFunctions[node->functionName] = function;
    // Remembered so property access on a call result can find the layout
    functionReturnTypes[node->functionName] = node->returnType;

    auto argIt = function->arg_begin();
    for (size_t i = 0; i < node->parameters.size(); ++i, ++argIt) {
        argIt->setName(node->parameters[i].name);
    }
    return function;
}

void CodeGen::visit(FunctionDeclarationNode *node)
{
    llvm::Function* function = declareFunctionSignature(node);

    // Create basic block for function body
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(m_context, "entry", function);
    m_builder.SetInsertPoint(entryBlock);
    
    // Save current function context
    llvm::Function* prevFunction = currentFunction;
    currentFunction = function;
    
    // Save current symbol table (for nested scopes later)
    auto prevNamedValues = namedValues;
    auto prevVariableTypes = variableTypes;

    // A function body sees its parameters and module-level globals, never the
    // caller's locals — those allocas belong to a different LLVM function.
    namedValues.clear();

    // Create allocas for parameters
    auto argIt = function->arg_begin();
    for (size_t i = 0; i < node->parameters.size(); ++i, ++argIt) {
        const auto& param = node->parameters[i];
        
        // Create alloca for parameter
        llvm::AllocaInst* alloca = m_builder.CreateAlloca(
            getLLVMType(param.type), 
            nullptr, 
            param.name
        );
        
        // Store parameter value in alloca
        m_builder.CreateStore(&*argIt, alloca);
        
        // Add to symbol table
        namedValues[param.name] = alloca;
        variableTypes[param.name] = param.type;
    }
    
    // Generate function body
    for (const auto& stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    
    // Terminate the final block: void functions get an implicit return; for
    // non-void functions an unterminated block here is unreachable (e.g. the
    // dead block created after a trailing `return`).
    llvm::BasicBlock* currentBlock = m_builder.GetInsertBlock();
    if (!currentBlock->getTerminator()) {
        if (node->returnType == "void") {
            m_builder.CreateRetVoid();
        } else {
            m_builder.CreateUnreachable();
        }
    }
    
    // Restore previous context
    currentFunction = prevFunction;
    namedValues = prevNamedValues;
    variableTypes = prevVariableTypes;
}

// Return Statement Visitor
void CodeGen::visit(ReturnStatementNode *node)
{
    if (!currentFunction) {
        throw std::runtime_error("Return statement outside of function");
    }
    
    if (node->expression) {
        // Return with value (coerced to the function's declared return type)
        llvm::Value* returnValue = visit(node->expression.get());
        returnValue = coerceValue(returnValue, currentFunction->getReturnType());
        // Pop recovery points of any try blocks this return exits
        emitTryPops(tryDepth);
        m_builder.CreateRet(returnValue);
    } else {
        // Return void
        emitTryPops(tryDepth);
        m_builder.CreateRetVoid();
    }
    // Statements after a return land in an unreachable block
    llvm::Function *fn = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *deadBlock = llvm::BasicBlock::Create(m_context, "after_return", fn);
    m_builder.SetInsertPoint(deadBlock);
}

void CodeGen::visit(TypeAliasNode *node)
{
    // Type aliases are a compile-time construct only and don't generate code
}

void CodeGen::visit(InterfaceDeclarationNode *node)
{
    // Interfaces are compile-time only; register for structural checks
    interfaces[node->interfaceName] = node;
}

void CodeGen::visit(ClassDeclarationNode *node)
{
    // Classes are registered in pass 0; instantiation happens at `new` sites
    classes[node->className] = node;
}

bool CodeGen::parseClosureSignature(const std::string &signature,
                                    std::vector<std::string> &argTypes, std::string &returnType)
{
    if (signature.rfind("closure(", 0) != 0) return false;
    size_t i = 8; // past "closure("
    int depth = 0;
    std::string current;
    for (; i < signature.size(); ++i) {
        char c = signature[i];
        if (c == '(' ) { depth++; current += c; }
        else if (c == '<') { depth++; current += c; }
        else if (c == '>' && i > 0 && signature[i - 1] == '=') { current += c; } // "=>" in nested types
        else if (c == '>') { depth--; current += c; }
        else if (c == ')' && depth == 0) { i++; break; }
        else if (c == ')') { depth--; current += c; }
        else if (c == ',' && depth == 0) { argTypes.push_back(current); current.clear(); }
        else current += c;
    }
    if (!current.empty()) argTypes.push_back(current);
    if (signature.compare(i, 2, "=>") != 0) return false;
    returnType = signature.substr(i + 2);
    return true;
}

void CodeGen::collectInterfaceMembers(const std::string& interfaceName,
                                      std::vector<InterfaceDeclarationNode::Member>& out)
{
    auto it = interfaces.find(interfaceName);
    if (it == interfaces.end()) return;
    if (!it->second->parentInterface.empty()) {
        collectInterfaceMembers(it->second->parentInterface, out);
    }
    for (const auto& member : it->second->members) {
        out.push_back(member);
    }
}

void CodeGen::checkInterfaceConformance(const std::string& interfaceName, ObjectLiteralNode* literal,
                                        const std::string& variableName)
{
    std::vector<InterfaceDeclarationNode::Member> members;
    collectInterfaceMembers(interfaceName, members);

    for (const auto& member : members) {
        const ObjectLiteralNode::Property* found = nullptr;
        for (const auto& prop : literal->properties) {
            if (prop.key == member.name) { found = &prop; break; }
        }
        if (!found) {
            throw std::runtime_error("Type Error: Object assigned to '" + variableName +
                                     "' does not satisfy interface '" + interfaceName +
                                     "': missing property '" + member.name + "'");
        }

        if (member.type.rfind("method:", 0) == 0) {
            if (!found->method) {
                throw std::runtime_error("Type Error: Interface '" + interfaceName + "' requires '" +
                                         member.name + "' to be a method on '" + variableName + "'");
            }
            continue;
        }

        // Check literal value kinds where they are statically known
        ExpressionNode* value = found->value.get();
        if (!value) {
            throw std::runtime_error("Type Error: Interface '" + interfaceName + "' property '" +
                                     member.name + "' must not be a method on '" + variableName + "'");
        }
        bool ok = true;
        if (member.type == "string") {
            if (dynamic_cast<IntegerLiteralNode*>(value) || dynamic_cast<BooleanLiteralNode*>(value) ||
                dynamic_cast<FloatLiteralNode*>(value) || dynamic_cast<ObjectLiteralNode*>(value)) ok = false;
        } else if (member.type == "i32" || member.type == "number") {
            if (dynamic_cast<StringLiteralNode*>(value) || dynamic_cast<BooleanLiteralNode*>(value) ||
                dynamic_cast<ObjectLiteralNode*>(value)) ok = false;
        } else if (member.type == "boolean") {
            if (dynamic_cast<StringLiteralNode*>(value) || dynamic_cast<ObjectLiteralNode*>(value) ||
                dynamic_cast<FloatLiteralNode*>(value)) ok = false;
        } else if (member.type == "f64") {
            if (dynamic_cast<StringLiteralNode*>(value) || dynamic_cast<BooleanLiteralNode*>(value) ||
                dynamic_cast<ObjectLiteralNode*>(value)) ok = false;
        } else if (interfaces.count(member.type)) {
            if (auto* nested = dynamic_cast<ObjectLiteralNode*>(value)) {
                checkInterfaceConformance(member.type, nested, variableName + "." + member.name);
            } else {
                ok = false;
            }
        }
        if (!ok) {
            throw std::runtime_error("Type Error: Property '" + member.name + "' of '" + variableName +
                                     "' does not match interface '" + interfaceName +
                                     "' (expected " + member.type + ")");
        }
    }
}

llvm::Function *CodeGen::getOrCreateMethodFunction(const std::string& objectKey,
                                                   const std::string& methodName)
{
    std::string cacheKey = objectKey + "::" + methodName;
    auto cached = methodFunctions.find(cacheKey);
    if (cached != methodFunctions.end()) return cached->second;

    FunctionDeclarationNode *decl = objectMethods[objectKey][methodName];
    if (!decl) {
        throw std::runtime_error("Codegen Error: Unknown method '" + methodName + "'");
    }

    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    std::vector<llvm::Type*> paramTypes;
    paramTypes.push_back(charPtr); // implicit `this`
    for (const auto& param : decl->parameters) {
        paramTypes.push_back(getLLVMType(param.type));
    }
    llvm::FunctionType *fnType = llvm::FunctionType::get(getLLVMType(decl->returnType), paramTypes, false);
    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::InternalLinkage,
                                                objectKey + "_" + methodName, m_module.get());
    methodFunctions[cacheKey] = fn;

    // Save the entire generation context: methods are generated lazily at first call site
    llvm::IRBuilderBase::InsertPoint savedIP = m_builder.saveIP();
    llvm::Function *prevFunction = currentFunction;
    auto prevNamedValues = namedValues;
    auto prevVariableTypes = variableTypes;
    auto prevConstVariables = constVariables;
    auto prevVariableToObjectKey = variableToObjectKey;
    std::string prevThisKey = currentThisObjectKey;

    int prevTryDepth = tryDepth;
    tryDepth = 0; // method bodies start outside any try protection

    currentFunction = fn;
    currentThisObjectKey = objectKey;

    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(m_context, "entry", fn);
    m_builder.SetInsertPoint(entryBlock);

    auto argIt = fn->arg_begin();
    argIt->setName("this");
    llvm::AllocaInst *thisAlloca = m_builder.CreateAlloca(charPtr, nullptr, "this");
    m_builder.CreateStore(&*argIt, thisAlloca);
    namedValues["this"] = thisAlloca;
    variableTypes["this"] = "object";
    variableToObjectKey["this"] = objectKey;
    ++argIt;

    for (size_t i = 0; i < decl->parameters.size(); ++i, ++argIt) {
        const auto& param = decl->parameters[i];
        argIt->setName(param.name);
        llvm::AllocaInst *alloca = m_builder.CreateAlloca(getLLVMType(param.type), nullptr, param.name);
        m_builder.CreateStore(&*argIt, alloca);
        namedValues[param.name] = alloca;
        variableTypes[param.name] = param.type;
    }

    for (const auto& stmt : decl->bodyStatements) {
        visit(stmt.get());
    }

    if (!m_builder.GetInsertBlock()->getTerminator()) {
        if (fn->getReturnType()->isVoidTy()) {
            m_builder.CreateRetVoid();
        } else if (fn->getReturnType()->isPointerTy()) {
            m_builder.CreateRet(llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(fn->getReturnType())));
        } else if (fn->getReturnType()->isDoubleTy()) {
            m_builder.CreateRet(llvm::ConstantFP::get(fn->getReturnType(), 0.0));
        } else {
            m_builder.CreateRet(llvm::ConstantInt::get(fn->getReturnType(), 0));
        }
    }

    // Restore the caller's context
    currentFunction = prevFunction;
    namedValues = prevNamedValues;
    variableTypes = prevVariableTypes;
    constVariables = prevConstVariables;
    variableToObjectKey = prevVariableToObjectKey;
    currentThisObjectKey = prevThisKey;
    tryDepth = prevTryDepth;
    m_builder.restoreIP(savedIP);

    return fn;
}

// ============================================================
// Arrow functions & closures
// ============================================================

llvm::StructType *CodeGen::getClosureType()
{
    if (!closureType) {
        llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
        closureType = llvm::StructType::create(m_context, {charPtr, charPtr}, "CypsClosure");
    }
    return closureType;
}

void CodeGen::collectFreeVarsExpr(ExpressionNode *expr, std::set<std::string> &bound,
                                  std::set<std::string> &free)
{
    if (!expr) return;

    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(expr)) {
        if (!bound.count(varExpr->name)) free.insert(varExpr->name);
    } else if (auto *binOp = dynamic_cast<BinaryExpressionNode*>(expr)) {
        collectFreeVarsExpr(binOp->left.get(), bound, free);
        collectFreeVarsExpr(binOp->right.get(), bound, free);
    } else if (auto *unaryOp = dynamic_cast<UnaryExpressionNode*>(expr)) {
        collectFreeVarsExpr(unaryOp->operand.get(), bound, free);
    } else if (auto *call = dynamic_cast<FunctionCallNode*>(expr)) {
        // A call target that is not a declared function may be a captured closure
        if (!declaredFunctions.count(call->functionName) && !bound.count(call->functionName)) {
            free.insert(call->functionName);
        }
        for (auto &arg : call->arguments) collectFreeVarsExpr(arg.get(), bound, free);
    } else if (auto *method = dynamic_cast<MethodCallNode*>(expr)) {
        collectFreeVarsExpr(method->object.get(), bound, free);
        for (auto &arg : method->arguments) collectFreeVarsExpr(arg.get(), bound, free);
    } else if (auto *arrAccess = dynamic_cast<ArrayAccessNode*>(expr)) {
        collectFreeVarsExpr(arrAccess->array.get(), bound, free);
        collectFreeVarsExpr(arrAccess->index.get(), bound, free);
    } else if (auto *objAccess = dynamic_cast<ObjectAccessNode*>(expr)) {
        collectFreeVarsExpr(objAccess->object.get(), bound, free);
    } else if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(expr)) {
        for (auto &element : arrLit->elements) collectFreeVarsExpr(element.get(), bound, free);
    } else if (auto *objLit = dynamic_cast<ObjectLiteralNode*>(expr)) {
        for (auto &prop : objLit->properties) {
            if (prop.value) collectFreeVarsExpr(prop.value.get(), bound, free);
        }
    } else if (auto *newExpr = dynamic_cast<NewExpressionNode*>(expr)) {
        for (auto &arg : newExpr->arguments) collectFreeVarsExpr(arg.get(), bound, free);
    } else if (auto *nested = dynamic_cast<ArrowFunctionNode*>(expr)) {
        // Names free in a nested arrow (minus its params) are free here too
        std::set<std::string> nestedBound = bound;
        for (const auto &param : nested->parameters) nestedBound.insert(param.name);
        for (auto &stmt : nested->bodyStatements) collectFreeVars(stmt.get(), nestedBound, free);
    }
}

void CodeGen::collectFreeVars(StatementNode *stmt, std::set<std::string> &bound,
                              std::set<std::string> &free)
{
    if (!stmt) return;

    if (auto *varDecl = dynamic_cast<VariableDeclarationNode*>(stmt)) {
        collectFreeVarsExpr(varDecl->initializer.get(), bound, free);
        bound.insert(varDecl->variableName);
    } else if (auto *destruct = dynamic_cast<DestructuringDeclarationNode*>(stmt)) {
        collectFreeVarsExpr(destruct->initializer.get(), bound, free);
        for (const auto &name : destruct->bindings) bound.insert(name);
    } else if (auto *assign = dynamic_cast<AssignmentStatementNode*>(stmt)) {
        // Assigning to an outer variable still requires capturing it
        if (!bound.count(assign->variableName)) free.insert(assign->variableName);
        collectFreeVarsExpr(assign->value.get(), bound, free);
    } else if (auto *arrAssign = dynamic_cast<ArrayAssignmentStatementNode*>(stmt)) {
        collectFreeVarsExpr(arrAssign->array.get(), bound, free);
        collectFreeVarsExpr(arrAssign->index.get(), bound, free);
        collectFreeVarsExpr(arrAssign->value.get(), bound, free);
    } else if (auto *propAssign = dynamic_cast<ObjectPropertyAssignmentNode*>(stmt)) {
        collectFreeVarsExpr(propAssign->object.get(), bound, free);
        collectFreeVarsExpr(propAssign->value.get(), bound, free);
    } else if (auto *exprStmt = dynamic_cast<ExpressionStatementNode*>(stmt)) {
        collectFreeVarsExpr(exprStmt->expression.get(), bound, free);
    } else if (auto *ifStmt = dynamic_cast<IfStatementNode*>(stmt)) {
        collectFreeVarsExpr(ifStmt->condition.get(), bound, free);
        for (auto &s : ifStmt->thenStatements) collectFreeVars(s.get(), bound, free);
        for (auto &s : ifStmt->elseStatements) collectFreeVars(s.get(), bound, free);
    } else if (auto *whileStmt = dynamic_cast<WhileStatementNode*>(stmt)) {
        collectFreeVarsExpr(whileStmt->condition.get(), bound, free);
        for (auto &s : whileStmt->bodyStatements) collectFreeVars(s.get(), bound, free);
    } else if (auto *doWhile = dynamic_cast<DoWhileStatementNode*>(stmt)) {
        for (auto &s : doWhile->bodyStatements) collectFreeVars(s.get(), bound, free);
        collectFreeVarsExpr(doWhile->condition.get(), bound, free);
    } else if (auto *forStmt = dynamic_cast<ForStatementNode*>(stmt)) {
        if (forStmt->initialization) collectFreeVars(forStmt->initialization.get(), bound, free);
        collectFreeVarsExpr(forStmt->condition.get(), bound, free);
        if (forStmt->increment) collectFreeVars(forStmt->increment.get(), bound, free);
        for (auto &s : forStmt->bodyStatements) collectFreeVars(s.get(), bound, free);
    } else if (auto *forOf = dynamic_cast<ForOfStatementNode*>(stmt)) {
        collectFreeVarsExpr(forOf->iterable.get(), bound, free);
        bound.insert(forOf->iteratorVariable->variableName);
        for (auto &s : forOf->bodyStatements) collectFreeVars(s.get(), bound, free);
    } else if (auto *switchStmt = dynamic_cast<SwitchStatementNode*>(stmt)) {
        collectFreeVarsExpr(switchStmt->condition.get(), bound, free);
        for (auto &clause : switchStmt->cases) {
            collectFreeVarsExpr(clause.value.get(), bound, free);
            for (auto &s : clause.statements) collectFreeVars(s.get(), bound, free);
        }
    } else if (auto *tryStmt = dynamic_cast<TryCatchStatementNode*>(stmt)) {
        for (auto &s : tryStmt->tryStatements) collectFreeVars(s.get(), bound, free);
        if (!tryStmt->errorVariable.empty()) bound.insert(tryStmt->errorVariable);
        for (auto &s : tryStmt->catchStatements) collectFreeVars(s.get(), bound, free);
        for (auto &s : tryStmt->finallyStatements) collectFreeVars(s.get(), bound, free);
    } else if (auto *retStmt = dynamic_cast<ReturnStatementNode*>(stmt)) {
        collectFreeVarsExpr(retStmt->expression.get(), bound, free);
    } else if (auto *throwStmt = dynamic_cast<ThrowStatementNode*>(stmt)) {
        collectFreeVarsExpr(throwStmt->expression.get(), bound, free);
    }
}

llvm::Type *CodeGen::inferExpressionLLVMType(ExpressionNode *expr,
                                             const std::map<std::string, std::string> &paramTypes)
{
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(m_context);
    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    if (!expr) return i32Ty;

    if (dynamic_cast<StringLiteralNode*>(expr)) return charPtr;
    if (dynamic_cast<FloatLiteralNode*>(expr)) return llvm::Type::getDoubleTy(m_context);
    if (dynamic_cast<IntegerLiteralNode*>(expr) || dynamic_cast<BooleanLiteralNode*>(expr)) return i32Ty;
    if (dynamic_cast<ArrowFunctionNode*>(expr)) return charPtr;

    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(expr)) {
        auto paramIt = paramTypes.find(varExpr->name);
        if (paramIt != paramTypes.end()) return getLLVMType(paramIt->second);
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) return getLLVMType(typeIt->second);
        return i32Ty;
    }
    if (auto *unaryOp = dynamic_cast<UnaryExpressionNode*>(expr)) {
        if (unaryOp->op == UnaryExpressionNode::NOT) return i32Ty;
        return inferExpressionLLVMType(unaryOp->operand.get(), paramTypes);
    }
    if (auto *binOp = dynamic_cast<BinaryExpressionNode*>(expr)) {
        switch (binOp->op) {
            case BinaryExpressionNode::EQUAL:
            case BinaryExpressionNode::NOT_EQUAL:
            case BinaryExpressionNode::LESS_THAN:
            case BinaryExpressionNode::LESS_EQUAL:
            case BinaryExpressionNode::GREATER_THAN:
            case BinaryExpressionNode::GREATER_EQUAL:
            case BinaryExpressionNode::LOGICAL_AND:
            case BinaryExpressionNode::LOGICAL_OR:
                return i32Ty;
            default: break;
        }
        llvm::Type *left = inferExpressionLLVMType(binOp->left.get(), paramTypes);
        llvm::Type *right = inferExpressionLLVMType(binOp->right.get(), paramTypes);
        if (binOp->op == BinaryExpressionNode::ADD &&
            (left->isPointerTy() || right->isPointerTy())) return charPtr;
        if (left->isDoubleTy() || right->isDoubleTy()) return llvm::Type::getDoubleTy(m_context);
        return i32Ty;
    }
    if (auto *call = dynamic_cast<FunctionCallNode*>(expr)) {
        auto fnIt = declaredFunctions.find(call->functionName);
        if (fnIt != declaredFunctions.end()) return fnIt->second->getReturnType();
        if (call->functionName == "JSON.stringify" || call->functionName == "JSON.parse") return charPtr;
        return i32Ty;
    }
    return i32Ty;
}

llvm::Type *CodeGen::inferArrowReturnType(ArrowFunctionNode *node,
                                          const std::map<std::string, std::string> &paramTypes)
{
    // Use the first top-level return statement's expression as the signal
    for (const auto &stmt : node->bodyStatements) {
        if (auto *retStmt = dynamic_cast<ReturnStatementNode*>(stmt.get())) {
            if (!retStmt->expression) return llvm::Type::getVoidTy(m_context);
            return inferExpressionLLVMType(retStmt->expression.get(), paramTypes);
        }
        // returns nested inside control flow: fall through to default
    }
    return llvm::Type::getVoidTy(m_context);
}

llvm::Function *CodeGen::getOrCreateArrowFunction(ArrowFunctionNode *node)
{
    auto cached = arrowFunctions.find(node);
    if (cached != arrowFunctions.end()) return cached->second;

    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    const auto &captures = arrowCaptures[node];
    llvm::StructType *envType = arrowEnvTypes[node];

    std::map<std::string, std::string> paramTypeNames;
    for (const auto &param : node->parameters) paramTypeNames[param.name] = param.type;

    llvm::Type *returnType = (node->returnType != "auto")
        ? getLLVMType(node->returnType)
        : inferArrowReturnType(node, paramTypeNames);

    std::vector<llvm::Type*> paramTypes;
    paramTypes.push_back(charPtr); // env
    for (const auto &param : node->parameters) paramTypes.push_back(getLLVMType(param.type));

    llvm::FunctionType *fnType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::InternalLinkage,
                                                "arrow_" + std::to_string(arrowCounter++), m_module.get());
    arrowFunctions[node] = fn;

    // Save generation context (arrows can be created mid-statement anywhere)
    llvm::IRBuilderBase::InsertPoint savedIP = m_builder.saveIP();
    llvm::Function *prevFunction = currentFunction;
    auto prevNamedValues = namedValues;
    auto prevVariableTypes = variableTypes;
    auto prevConstVariables = constVariables;
    auto prevLoopTargets = loopTargets;
    auto prevLoopTryDepths = loopTargetTryDepths;
    int prevTryDepth = tryDepth;

    currentFunction = fn;
    loopTargets.clear();
    loopTargetTryDepths.clear();
    tryDepth = 0;

    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(m_context, "entry", fn);
    m_builder.SetInsertPoint(entryBlock);

    auto argIt = fn->arg_begin();
    argIt->setName("env");
    llvm::Value *envRaw = &*argIt;
    ++argIt;

    // Unpack captured variables into locals (by-value snapshot)
    if (!captures.empty()) {
        llvm::Value *envPtr = m_builder.CreateBitCast(envRaw, llvm::PointerType::get(envType, 0), "env_cast");
        for (size_t i = 0; i < captures.size(); ++i) {
            const auto &capture = captures[i];
            llvm::Type *fieldType = envType->getElementType(i);
            llvm::Value *fieldPtr = m_builder.CreateStructGEP(envType, envPtr, i, capture.first + "_env_ptr");
            llvm::Value *value = m_builder.CreateLoad(fieldType, fieldPtr, capture.first + "_captured");
            llvm::AllocaInst *alloca = m_builder.CreateAlloca(fieldType, nullptr, capture.first);
            m_builder.CreateStore(value, alloca);
            namedValues[capture.first] = alloca;
            variableTypes[capture.first] = capture.second;
        }
    }

    // Parameters
    for (size_t i = 0; i < node->parameters.size(); ++i, ++argIt) {
        const auto &param = node->parameters[i];
        argIt->setName(param.name);
        llvm::AllocaInst *alloca = m_builder.CreateAlloca(getLLVMType(param.type), nullptr, param.name);
        m_builder.CreateStore(&*argIt, alloca);
        namedValues[param.name] = alloca;
        variableTypes[param.name] = param.type;
    }

    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }

    if (!m_builder.GetInsertBlock()->getTerminator()) {
        if (fn->getReturnType()->isVoidTy()) {
            m_builder.CreateRetVoid();
        } else if (fn->getReturnType()->isPointerTy()) {
            m_builder.CreateRet(llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(fn->getReturnType())));
        } else if (fn->getReturnType()->isDoubleTy()) {
            m_builder.CreateRet(llvm::ConstantFP::get(fn->getReturnType(), 0.0));
        } else {
            m_builder.CreateRet(llvm::ConstantInt::get(fn->getReturnType(), 0));
        }
    }

    // Restore context
    currentFunction = prevFunction;
    namedValues = prevNamedValues;
    variableTypes = prevVariableTypes;
    constVariables = prevConstVariables;
    loopTargets = prevLoopTargets;
    loopTargetTryDepths = prevLoopTryDepths;
    tryDepth = prevTryDepth;
    m_builder.restoreIP(savedIP);

    return fn;
}

llvm::Value *CodeGen::visit(ArrowFunctionNode *node)
{
    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);

    // Determine captures once per arrow node (cached for repeated visits, e.g. in loops)
    if (!arrowEnvTypes.count(node)) {
        std::set<std::string> bound, freeNames;
        for (const auto &param : node->parameters) bound.insert(param.name);
        for (const auto &stmt : node->bodyStatements) {
            collectFreeVars(stmt.get(), bound, freeNames);
        }

        std::vector<std::pair<std::string, std::string>> captures;
        for (const auto &name : freeNames) {
            auto nv = namedValues.find(name);
            if (nv == namedValues.end()) continue; // e.g. builtin function names
            std::string typeName = "i32";
            auto typeIt = variableTypes.find(name);
            if (typeIt != variableTypes.end()) typeName = typeIt->second;
            captures.push_back({name, typeName});
        }

        std::vector<llvm::Type*> envFields;
        for (const auto &capture : captures) {
            auto nv = namedValues.find(capture.first);
            envFields.push_back(nv->second->getAllocatedType());
        }
        arrowCaptures[node] = std::move(captures);
        arrowEnvTypes[node] = llvm::StructType::create(m_context, envFields, "ArrowEnv");
    }

    const auto &captures = arrowCaptures[node];
    llvm::StructType *envType = arrowEnvTypes[node];

    llvm::FunctionCallee mallocFn = m_module->getOrInsertFunction("malloc",
        charPtr, llvm::Type::getInt64Ty(m_context));

    // Snapshot the captured variables into a heap environment
    llvm::Value *envRaw;
    if (captures.empty()) {
        envRaw = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(charPtr));
    } else {
        uint64_t envSize = m_module->getDataLayout().getTypeAllocSize(envType);
        envRaw = m_builder.CreateCall(mallocFn,
            {llvm::ConstantInt::get(llvm::Type::getInt64Ty(m_context), envSize)}, "arrow_env");
        llvm::Value *envPtr = m_builder.CreateBitCast(envRaw, llvm::PointerType::get(envType, 0), "env_cast");
        for (size_t i = 0; i < captures.size(); ++i) {
            llvm::AllocaInst *sourceAlloca = namedValues[captures[i].first];
            llvm::Value *value = m_builder.CreateLoad(sourceAlloca->getAllocatedType(), sourceAlloca,
                                                      captures[i].first + "_snap");
            llvm::Value *fieldPtr = m_builder.CreateStructGEP(envType, envPtr, i);
            m_builder.CreateStore(value, fieldPtr);
        }
    }

    llvm::Function *fn = getOrCreateArrowFunction(node);

    // Build the closure value: {fn, env}
    llvm::StructType *closTy = getClosureType();
    uint64_t closSize = m_module->getDataLayout().getTypeAllocSize(closTy);
    llvm::Value *closRaw = m_builder.CreateCall(mallocFn,
        {llvm::ConstantInt::get(llvm::Type::getInt64Ty(m_context), closSize)}, "closure");
    llvm::Value *closPtr = m_builder.CreateBitCast(closRaw, llvm::PointerType::get(closTy, 0), "closure_cast");
    m_builder.CreateStore(m_builder.CreateBitCast(fn, charPtr),
                          m_builder.CreateStructGEP(closTy, closPtr, 0));
    m_builder.CreateStore(envRaw, m_builder.CreateStructGEP(closTy, closPtr, 1));
    return closRaw;
}

ArrowFunctionNode *CodeGen::resolveArrowArgument(ExpressionNode *expr)
{
    if (auto *arrowLit = dynamic_cast<ArrowFunctionNode*>(expr)) return arrowLit;
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(expr)) {
        auto it = variableToArrow.find(varExpr->name);
        if (it != variableToArrow.end()) return it->second;
    }
    return nullptr;
}

std::pair<llvm::Function*, llvm::Value*> CodeGen::materializeCallback(ExpressionNode *expr)
{
    ArrowFunctionNode *arrowNode = resolveArrowArgument(expr);
    if (!arrowNode) {
        throw std::runtime_error("Codegen Error: Callback must be an arrow function "
                                 "or a variable bound to one");
    }

    llvm::Value *closureVal = visit(expr); // creates or loads the closure
    llvm::StructType *closTy = getClosureType();
    llvm::Value *closPtr = m_builder.CreateBitCast(closureVal,
        llvm::PointerType::get(closTy, 0), "cb_closure");
    llvm::Value *envPtr = m_builder.CreateLoad(
        llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
        m_builder.CreateStructGEP(closTy, closPtr, 1), "cb_env");

    return {arrowFunctions[arrowNode], envPtr};
}

std::string CodeGen::inferMethodCallTypeName(MethodCallNode *node)
{
    std::string varType;
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->object.get())) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) varType = typeIt->second;
    }
    if (varType.length() < 3 || varType.substr(varType.length() - 2) != "[]") return "";
    std::string elemType = varType.substr(0, varType.length() - 2);

    // Type name produced by the callback's return type: "string" / "f64" / "i32"
    auto arrowReturnTypeName = [&](ExpressionNode *cb) -> std::string {
        ArrowFunctionNode *arrowNode = resolveArrowArgument(cb);
        if (!arrowNode) return "i32";
        llvm::Type *retType = nullptr;
        auto fnIt = arrowFunctions.find(arrowNode);
        if (fnIt != arrowFunctions.end()) {
            retType = fnIt->second->getReturnType();
        } else if (arrowNode->returnType != "auto") {
            retType = getLLVMType(arrowNode->returnType);
        } else {
            std::map<std::string, std::string> paramTypeNames;
            for (const auto &p : arrowNode->parameters) paramTypeNames[p.name] = p.type;
            retType = inferArrowReturnType(arrowNode, paramTypeNames);
        }
        if (retType->isPointerTy()) return "string";
        if (retType->isDoubleTy()) return "f64";
        return "i32";
    };

    if (node->methodName == "map" && !node->arguments.empty()) {
        return arrowReturnTypeName(node->arguments[0].get()) + "[]";
    }
    if (node->methodName == "filter") return varType;
    if (node->methodName == "find" || node->methodName == "shift" || node->methodName == "pop") {
        if (elemType == "string" || elemType == "f64") return elemType;
        return "i32";
    }
    if (node->methodName == "reduce" && !node->arguments.empty()) {
        return arrowReturnTypeName(node->arguments[0].get());
    }
    return "";
}

llvm::Value *CodeGen::generateArrayCallbackMethod(MethodCallNode *node, llvm::Value *arrayPtr,
                                                  const std::string &elemType)
{
    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(m_context);
    const std::string &method = node->methodName;

    if (node->arguments.empty()) {
        throw std::runtime_error("Codegen Error: ." + method + "() requires a callback argument");
    }

    auto [callback, envPtr] = materializeCallback(node->arguments[0].get());

    llvm::Type *f64Ty = llvm::Type::getDoubleTy(m_context);
    bool stringElems = (elemType == "string");
    bool f64Elems = (elemType == "f64");
    llvm::Type *elemLLVMType = stringElems ? charPtr : (f64Elems ? f64Ty : i32Ty);

    llvm::FunctionCallee getFn = stringElems
        ? m_module->getOrInsertFunction("array_get_string", charPtr, charPtr, i32Ty)
        : (f64Elems
            ? m_module->getOrInsertFunction("array_get_f64", f64Ty, charPtr, i32Ty)
            : m_module->getOrInsertFunction("array_get_i32", i32Ty, charPtr, i32Ty));
    llvm::FunctionCallee lenFn = m_module->getOrInsertFunction("array_length", i32Ty, charPtr);

    // Selects create/push functions for a given element LLVM type
    auto createFnFor = [&](llvm::Type *t) {
        if (t->isPointerTy()) return m_module->getOrInsertFunction("array_create_string", charPtr);
        if (t->isDoubleTy()) return m_module->getOrInsertFunction("array_create_f64", charPtr);
        return m_module->getOrInsertFunction("array_create_i32", charPtr);
    };
    auto pushFnFor = [&](llvm::Type *t) {
        if (t->isPointerTy()) return m_module->getOrInsertFunction("array_push_string",
            llvm::Type::getVoidTy(m_context), charPtr, charPtr);
        if (t->isDoubleTy()) return m_module->getOrInsertFunction("array_push_f64",
            llvm::Type::getVoidTy(m_context), charPtr, f64Ty);
        return m_module->getOrInsertFunction("array_push_i32",
            llvm::Type::getVoidTy(m_context), charPtr, i32Ty);
    };

    llvm::Value *length = m_builder.CreateCall(lenFn, {arrayPtr}, "cb_len");
    llvm::Function *fn = m_builder.GetInsertBlock()->getParent();

    // Shared loop skeleton
    llvm::BasicBlock *condBlock = llvm::BasicBlock::Create(m_context, method + "_cond", fn);
    llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(m_context, method + "_body", fn);
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(m_context, method + "_exit", fn);

    llvm::AllocaInst *indexAlloca = m_builder.CreateAlloca(i32Ty, nullptr, method + "_i");
    m_builder.CreateStore(llvm::ConstantInt::get(i32Ty, 0), indexAlloca);

    // Method-specific setup
    llvm::Value *resultArray = nullptr;
    llvm::AllocaInst *accAlloca = nullptr;
    llvm::AllocaInst *foundAlloca = nullptr;
    llvm::Type *cbRetType = callback->getReturnType();

    if (method == "map") {
        resultArray = m_builder.CreateCall(createFnFor(cbRetType), {}, "map_result");
    } else if (method == "filter") {
        resultArray = m_builder.CreateCall(createFnFor(elemLLVMType), {}, "filter_result");
    } else if (method == "reduce") {
        if (node->arguments.size() < 2) {
            throw std::runtime_error("Codegen Error: .reduce() requires (callback, initialValue)");
        }
        llvm::Value *initial = visit(node->arguments[1].get());
        initial = coerceValue(initial, cbRetType);
        accAlloca = m_builder.CreateAlloca(cbRetType, nullptr, "reduce_acc");
        m_builder.CreateStore(initial, accAlloca);
    } else if (method == "find") {
        foundAlloca = m_builder.CreateAlloca(elemLLVMType, nullptr, "find_result");
        llvm::Value *zero;
        if (stringElems) zero = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(charPtr));
        else if (f64Elems) zero = llvm::ConstantFP::get(f64Ty, 0.0);
        else zero = llvm::ConstantInt::get(i32Ty, 0);
        m_builder.CreateStore(zero, foundAlloca);
    } else if (method != "forEach") {
        throw std::runtime_error("Codegen Error: Unsupported callback method '" + method + "'");
    }

    m_builder.CreateBr(condBlock);
    m_builder.SetInsertPoint(condBlock);
    llvm::Value *index = m_builder.CreateLoad(i32Ty, indexAlloca, "cb_idx");
    llvm::Value *inRange = m_builder.CreateICmpSLT(index, length, "cb_inrange");
    m_builder.CreateCondBr(inRange, bodyBlock, exitBlock);

    m_builder.SetInsertPoint(bodyBlock);
    llvm::Value *element = m_builder.CreateCall(getFn, {arrayPtr, index}, "cb_elem");

    auto nextIteration = [&]() {
        llvm::Value *next = m_builder.CreateAdd(
            m_builder.CreateLoad(i32Ty, indexAlloca), llvm::ConstantInt::get(i32Ty, 1));
        m_builder.CreateStore(next, indexAlloca);
        m_builder.CreateBr(condBlock);
    };

    llvm::FunctionType *cbType = callback->getFunctionType();
    auto coerceElem = [&](unsigned paramIdx) {
        return coerceValue(element, cbType->getParamType(paramIdx));
    };

    if (method == "map") {
        llvm::Value *mapped = m_builder.CreateCall(callback, {envPtr, coerceElem(1)}, "mapped");
        m_builder.CreateCall(pushFnFor(cbRetType), {resultArray, mapped});
        nextIteration();
    } else if (method == "filter") {
        llvm::Value *keep = ensureI1(m_builder.CreateCall(callback, {envPtr, coerceElem(1)}, "keep"));
        llvm::BasicBlock *pushBlock = llvm::BasicBlock::Create(m_context, "filter_push", fn);
        llvm::BasicBlock *skipBlock = llvm::BasicBlock::Create(m_context, "filter_skip", fn);
        m_builder.CreateCondBr(keep, pushBlock, skipBlock);
        m_builder.SetInsertPoint(pushBlock);
        m_builder.CreateCall(pushFnFor(elemLLVMType), {resultArray, element});
        m_builder.CreateBr(skipBlock);
        m_builder.SetInsertPoint(skipBlock);
        nextIteration();
    } else if (method == "forEach") {
        m_builder.CreateCall(callback, {envPtr, coerceElem(1)});
        nextIteration();
    } else if (method == "reduce") {
        llvm::Value *acc = m_builder.CreateLoad(cbRetType, accAlloca, "acc");
        llvm::Value *newAcc = m_builder.CreateCall(callback,
            {envPtr, coerceValue(acc, cbType->getParamType(1)), coerceElem(2)}, "new_acc");
        m_builder.CreateStore(coerceValue(newAcc, cbRetType), accAlloca);
        nextIteration();
    } else if (method == "find") {
        llvm::Value *matches = ensureI1(m_builder.CreateCall(callback, {envPtr, coerceElem(1)}, "matches"));
        llvm::BasicBlock *hitBlock = llvm::BasicBlock::Create(m_context, "find_hit", fn);
        llvm::BasicBlock *missBlock = llvm::BasicBlock::Create(m_context, "find_miss", fn);
        m_builder.CreateCondBr(matches, hitBlock, missBlock);
        m_builder.SetInsertPoint(hitBlock);
        m_builder.CreateStore(element, foundAlloca);
        m_builder.CreateBr(exitBlock); // early exit on first match
        m_builder.SetInsertPoint(missBlock);
        nextIteration();
    }

    m_builder.SetInsertPoint(exitBlock);
    if (method == "map" || method == "filter") return resultArray;
    if (method == "reduce") return m_builder.CreateLoad(cbRetType, accAlloca, "reduce_final");
    if (method == "find") return m_builder.CreateLoad(foundAlloca->getAllocatedType(), foundAlloca, "find_final");
    return nullptr; // forEach
}

void CodeGen::visit(ObjectPropertyAssignmentNode *node)
{
    std::string objectKey = getExpressionObjectKey(node->object.get());
    if (objectKey.empty()) {
        throw std::runtime_error("Codegen Error: Property assignment is only supported on native objects");
    }
    auto layoutIt = objectLayouts.find(objectKey);
    if (layoutIt == objectLayouts.end()) {
        throw std::runtime_error("Codegen Error: Unknown object layout for property assignment");
    }
    const ObjectOptimizer::ObjectLayout& layout = layoutIt->second;

    auto idxIt = layout.propertyIndices.find(node->property);
    if (idxIt == layout.propertyIndices.end()) {
        throw std::runtime_error("Codegen Error: Property '" + node->property + "' not found for assignment");
    }

    // Resolve the object pointer
    llvm::Value *objectPtr = nullptr;
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->object.get())) {
        llvm::Value* storageSlot = variableStorage(varExpr->name);
        if (storageSlot != nullptr) {
            objectPtr = m_builder.CreateLoad(
                llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0),
                storageSlot,
                "obj_ptr_load");
        }
    } else {
        objectPtr = visit(node->object.get());
    }
    if (!objectPtr) {
        throw std::runtime_error("Codegen Error: Failed to resolve object for property assignment");
    }

    llvm::Value *structPtr = m_builder.CreateBitCast(
        objectPtr, llvm::PointerType::get(layout.structType, 0), "struct_cast");

    llvm::Value *value = visit(node->value.get());
    if (!value) {
        throw std::runtime_error("Codegen Error: Failed to generate value for property assignment");
    }
    value = coerceValue(value, layout.properties[idxIt->second].second.type);

    objectOptimizer.generateDirectPropertyStore(m_builder, structPtr, node->property, layout, value);
}

void CodeGen::visit(DestructuringDeclarationNode *node)
{
    auto *varRef = dynamic_cast<VariableExpressionNode*>(node->initializer.get());
    if (!varRef) {
        throw std::runtime_error("Codegen Error: Destructuring currently requires an object variable "
                                 "on the right-hand side (e.g. let { a, b } = user;)");
    }

    std::string objectKey = getExpressionObjectKey(varRef);
    const ObjectOptimizer::ObjectLayout* layout = nullptr;
    if (!objectKey.empty()) {
        auto layoutIt = objectLayouts.find(objectKey);
        if (layoutIt != objectLayouts.end()) layout = &layoutIt->second;
    }

    for (const auto& binding : node->bindings) {
        // Reuse the normal property-access codegen via a temporary access node
        ObjectAccessNode access(std::make_unique<VariableExpressionNode>(varRef->name), binding);
        llvm::Value *value = visit(&access);
        if (!value) {
            throw std::runtime_error("Codegen Error: Failed to destructure property '" + binding + "'");
        }

        // Work out the recorded type for the new binding
        std::string bindingType = "string";
        if (layout) {
            auto idxIt = layout->propertyIndices.find(binding);
            if (idxIt == layout->propertyIndices.end()) {
                throw std::runtime_error("Codegen Error: Property '" + binding + "' not found while destructuring '" +
                                         varRef->name + "'");
            }
            const std::string& propType = layout->properties[idxIt->second].second.typeName;
            if (propType.rfind("object:", 0) == 0) {
                bindingType = "object";
                variableToObjectKey[binding] = propType.substr(7);
            } else {
                bindingType = propType;
            }
        } else if (value->getType()->isDoubleTy()) {
            bindingType = "f64";
        } else if (value->getType()->isIntegerTy()) {
            bindingType = "i32";
        }

        llvm::Type *bindingLLVMType = value->getType();
        llvm::Function *fn = m_builder.GetInsertBlock()->getParent();
        llvm::IRBuilder<> TmpB(&fn->getEntryBlock(), fn->getEntryBlock().begin());
        llvm::AllocaInst *alloca = TmpB.CreateAlloca(bindingLLVMType, nullptr, binding);
        m_builder.CreateStore(value, alloca);

        namedValues[binding] = alloca;
        variableTypes[binding] = bindingType;
        constVariables[binding] = node->isConst;
    }
}

void CodeGen::visit(ThrowStatementNode *node)
{
    llvm::Value *value = visit(node->expression.get());
    if (!value) {
        throw std::runtime_error("Codegen Error: Failed to generate throw expression");
    }
    llvm::Value *message = toStringValue(value);

    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    llvm::FunctionCallee throwFn = m_module->getOrInsertFunction("cyps_throw",
        llvm::Type::getVoidTy(m_context), charPtr);
    if (auto *fn = llvm::dyn_cast<llvm::Function>(throwFn.getCallee())) {
        fn->addFnAttr(llvm::Attribute::NoReturn);
    }
    m_builder.CreateCall(throwFn, {message});
    m_builder.CreateUnreachable();

    // Statements after a throw land in an unreachable block
    llvm::Function *parentFn = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *deadBlock = llvm::BasicBlock::Create(m_context, "after_throw", parentFn);
    m_builder.SetInsertPoint(deadBlock);
}

void CodeGen::visit(TryCatchStatementNode *node)
{
    llvm::Type *charPtr = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    llvm::FunctionCallee pushFn = m_module->getOrInsertFunction("cyps_try_push", charPtr);
    llvm::FunctionCallee popFn = m_module->getOrInsertFunction("cyps_try_pop",
        llvm::Type::getVoidTy(m_context));
    llvm::FunctionCallee errFn = m_module->getOrInsertFunction("cyps_last_error", charPtr);

    llvm::Function *fn = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *tryBlock = llvm::BasicBlock::Create(m_context, "try_body", fn);
    llvm::BasicBlock *catchBlock = llvm::BasicBlock::Create(m_context, "catch_body", fn);
    llvm::BasicBlock *contBlock = llvm::BasicBlock::Create(m_context, "try_cont", fn);

    // Register the recovery point: cyps_throw longjmps back here with rc != 0
    llvm::Value *buf = m_builder.CreateCall(pushFn, {}, "try_buf");
    llvm::Value *rc = m_builder.CreateCall(getOrDeclareSetjmp(), {buf}, "setjmp_rc");
    llvm::Value *isNormal = m_builder.CreateICmpEQ(rc,
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0), "try_normal");
    m_builder.CreateCondBr(isNormal, tryBlock, catchBlock);

    // Try body: pop the recovery point on normal completion.
    // tryDepth lets return/break/continue inside the body pop it too.
    m_builder.SetInsertPoint(tryBlock);
    tryDepth++;
    for (const auto& stmt : node->tryStatements) {
        visit(stmt.get());
    }
    tryDepth--;
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateCall(popFn, {});
        m_builder.CreateBr(contBlock);
    }

    // Catch body (cyps_throw already popped the recovery point)
    m_builder.SetInsertPoint(catchBlock);
    auto savedNamedValues = namedValues;
    auto savedVariableTypes = variableTypes;
    auto savedConstVariables = constVariables;
    if (!node->errorVariable.empty()) {
        llvm::Value *errMsg = m_builder.CreateCall(errFn, {}, "err_msg");
        llvm::AllocaInst *errAlloca = m_builder.CreateAlloca(charPtr, nullptr, node->errorVariable);
        m_builder.CreateStore(errMsg, errAlloca);
        namedValues[node->errorVariable] = errAlloca;
        variableTypes[node->errorVariable] = "string";
        constVariables[node->errorVariable] = true;
    }
    for (const auto& stmt : node->catchStatements) {
        visit(stmt.get());
    }
    namedValues = savedNamedValues;
    variableTypes = savedVariableTypes;
    constVariables = savedConstVariables;
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(contBlock);
    }

    // Continuation: finally statements run on both paths that reach here
    m_builder.SetInsertPoint(contBlock);
    for (const auto& stmt : node->finallyStatements) {
        visit(stmt.get());
    }
}
