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

// --- Helper Methods ---

llvm::Type *CodeGen::getLLVMType(const std::string &typeName)
{
    if (typeName == "string")
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
    else if (typeName == "boolean")
    {
        return llvm::Type::getInt1Ty(m_context);
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
    llvm::FunctionType *mainFuncType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), false);
    llvm::Function *mainFunc = llvm::Function::Create(
        mainFuncType, llvm::Function::ExternalLinkage, "main", m_module.get());

    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(m_context, "entry", mainFunc);
    m_builder.SetInsertPoint(entryBlock);

    // Clear symbol table for each new program generation
    namedValues.clear();

    for (const auto &stmt : node->statements)
    {
        visit(stmt.get());
    }

    m_builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));

    if (llvm::verifyFunction(*mainFunc, &llvm::errs()))
    {
        std::cerr << "Error: main function verification failed!\n";
    }
}

void CodeGen::visit(StatementNode *node)
{
    if (auto *declNode = dynamic_cast<VariableDeclarationNode *>(node))
    {
        visit(declNode);
    }
    else if (auto *callNode = dynamic_cast<FunctionCallNode *>(node))
    {
        visit(callNode);
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
    else if (auto *doWhileNode = dynamic_cast<DoWhileStatementNode *>(node))
    {
        visit(doWhileNode);
    }
    else if (auto *assignNode = dynamic_cast<AssignmentStatementNode *>(node))
    {
        visit(assignNode);
    }
    else
    {
        std::cerr << "Codegen Error: Unsupported statement type.\n";
        throw std::runtime_error("Unsupported statement type in codegen.");
    }
}

void CodeGen::visit(VariableDeclarationNode *node)
{
    llvm::Type *varLLVMType = getLLVMType(node->typeName);
    
    // Create alloca at the beginning of the function
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> TmpB(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    llvm::AllocaInst *allocaInst = TmpB.CreateAlloca(varLLVMType, nullptr, node->variableName);

    namedValues[node->variableName] = allocaInst;

    if (node->initializer)
    {
        llvm::Value *initVal = visit(node->initializer.get());
        if (!initVal)
        {
            throw std::runtime_error("Codegen Error: Failed to generate initializer for variable " + node->variableName);
        }
        
        // Basic type compatibility check
        if (initVal->getType() != varLLVMType)
        {
            // Allow compatible pointer types (string literals to string variables)
            if (varLLVMType->isPointerTy() && initVal->getType()->isPointerTy())
            {
                // Both are pointers, allow the assignment (LLVM will handle compatibility)
            }
            else if (varLLVMType->isIntegerTy() && initVal->getType()->isIntegerTy())
            {
                // Both are integers, allow the assignment
            }
            else
            {
                std::cerr << "Codegen Warning: Type mismatch for variable '" << node->variableName << "'\n";
            }
        }
        
        m_builder.CreateStore(initVal, allocaInst);
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
    else if (auto *varNode = dynamic_cast<VariableExpressionNode *>(node))
    {
        return visit(varNode);
    }
    else if (auto *binNode = dynamic_cast<BinaryExpressionNode *>(node))
    {
        return visit(binNode);
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

llvm::Value *CodeGen::visit(VariableExpressionNode *node)
{
    llvm::AllocaInst *allocaInst = namedValues[node->name];
    if (!allocaInst)
    {
        throw std::runtime_error("Codegen Error: Unknown variable name: " + node->name);
    }
    
    // Load the value from the memory location
    return m_builder.CreateLoad(allocaInst->getAllocatedType(), allocaInst, node->name + "_val");
}

llvm::Value *CodeGen::visit(BinaryExpressionNode *node)
{
    // Generate code for left and right operands
    llvm::Value *leftVal = visit(node->left.get());
    llvm::Value *rightVal = visit(node->right.get());
    
    if (!leftVal || !rightVal) {
        throw std::runtime_error("Codegen Error: Failed to generate operands for binary expression");
    }
    
    // For now, we only support integer arithmetic
    // TODO: Add type checking and support for other types
    if (!leftVal->getType()->isIntegerTy() || !rightVal->getType()->isIntegerTy()) {
        throw std::runtime_error("Codegen Error: Binary operations currently only support integers");
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
    llvm::Value *conditionVal = visit(node->condition.get());
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
    llvm::Value *conditionVal = visit(node->condition.get());
    if (!conditionVal) {
        throw std::runtime_error("Codegen Error: Failed to generate condition for while statement");
    }
    
    // Create conditional branch: if true go to body, if false go to exit
    m_builder.CreateCondBr(conditionVal, bodyBlock, exitBlock);
    
    // Generate body block
    m_builder.SetInsertPoint(bodyBlock);
    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    
    // Branch back to condition (creating the loop)
    if (!m_builder.GetInsertBlock()->getTerminator()) {
        m_builder.CreateBr(condBlock);
    }
    
    // Continue with exit block
    m_builder.SetInsertPoint(exitBlock);
}

void CodeGen::visit(AssignmentStatementNode *node)
{
    // Look up the variable in our symbol table
    auto it = namedValues.find(node->variableName);
    if (it == namedValues.end()) {
        throw std::runtime_error("Codegen Error: Undefined variable '" + node->variableName + "'");
    }
    
    llvm::AllocaInst *varAlloca = it->second;
    
    // Generate code for the value expression
    llvm::Value *value = visit(node->value.get());
    if (!value) {
        throw std::runtime_error("Codegen Error: Failed to generate value for assignment");
    }
    
    // Store the value in the variable's memory location
    m_builder.CreateStore(value, varAlloca);
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
        llvm::Value *conditionVal = visit(node->condition.get());
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
    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    m_builder.CreateBr(incrBlock);
    
    // Generate increment block
    m_builder.SetInsertPoint(incrBlock);
    if (node->increment) {
        visit(node->increment.get());
    }
    m_builder.CreateBr(condBlock); // Loop back to condition
    
    // Continue with exit block
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
    for (const auto &stmt : node->bodyStatements) {
        visit(stmt.get());
    }
    m_builder.CreateBr(condBlock);
    
    // Generate condition block
    m_builder.SetInsertPoint(condBlock);
    llvm::Value *conditionVal = visit(node->condition.get());
    if (!conditionVal) {
        throw std::runtime_error("Codegen Error: Failed to generate condition for do-while statement");
    }
    
    // Create conditional branch: if true go back to body, if false go to exit
    m_builder.CreateCondBr(conditionVal, bodyBlock, exitBlock);
    
    // Continue with exit block
    m_builder.SetInsertPoint(exitBlock);
}

void CodeGen::visit(FunctionCallNode *node)
{
    if (node->functionName == "print" || node->functionName == "println")
    {
        if (node->arguments.size() != 1)
        {
            std::cerr << "Codegen Error: '" << node->functionName << "' expects exactly one argument.\n";
            throw std::runtime_error("'" + node->functionName + "' expects one argument.");
        }

        llvm::Value *argValue = visit(node->arguments[0].get());
        if (!argValue)
        {
            std::cerr << "Codegen Error: Failed to generate code for '" << node->functionName << "' argument.\n";
            throw std::runtime_error("Failed to generate code for '" + node->functionName + "' argument.");
        }

        llvm::Type *argType = argValue->getType();
        bool addNewline = (node->functionName == "println");

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
        else if (argType->isIntegerTy(32))
        {
            // Argument is i32
            llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
            // Create format string with or without newline
            std::string formatString = addNewline ? "%d\n" : "%d";
            llvm::Value *formatStr = m_builder.CreateGlobalString(formatString, ".format_int");
            std::vector<llvm::Value *> printfArgs = {formatStr, argValue};
            m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
        }
        else
        {
            std::cerr << "Codegen Error: '" << node->functionName << "' argument type not supported. Expected string or i32.\n";
            throw std::runtime_error("'" + node->functionName + "' argument type not supported.");
        }
    }
    else
    {
        std::cerr << "Codegen Error: Unsupported function call '" << node->functionName << "'.\n";
        throw std::runtime_error("Unsupported function call: " + node->functionName);
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
