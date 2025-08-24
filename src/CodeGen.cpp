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
    else if (typeName.length() > 2 && typeName.substr(typeName.length() - 2) == "[]")
    {
        // Array type like "i32[]"
        std::string elementTypeName = typeName.substr(0, typeName.length() - 2);
        llvm::Type *elementType = getLLVMType(elementTypeName);
        
        // For now, return a pointer to the element type (representing a dynamic array)
        // In a full implementation, we might use a struct with size and data pointer
        return llvm::PointerType::get(elementType, 0);
    }
    else if (typeName == "auto")
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
    llvm::FunctionType *mainFuncType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), false);
    llvm::Function *mainFunc = llvm::Function::Create(
        mainFuncType, llvm::Function::ExternalLinkage, "main", m_module.get());

    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(m_context, "entry", mainFunc);
    m_builder.SetInsertPoint(entryBlock);

    // Clear symbol table for each new program generation
    namedValues.clear();
    variableTypes.clear();
    arraySizes.clear();

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
    else
    {
        std::cerr << "Codegen Error: Unsupported statement type.\n";
        throw std::runtime_error("Unsupported statement type in codegen.");
    }
}

void CodeGen::visit(VariableDeclarationNode *node)
{
    llvm::Type *varLLVMType;
    
    // Handle type inference
    if (node->typeName == "auto") {
        // Infer type from initializer
        if (node->initializer) {
            // Try to determine type from the initializer
            if (auto *strLit = dynamic_cast<StringLiteralNode*>(node->initializer.get())) {
                varLLVMType = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
            } else if (auto *intLit = dynamic_cast<IntegerLiteralNode*>(node->initializer.get())) {
                varLLVMType = llvm::Type::getInt32Ty(m_context);
            } else if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(node->initializer.get())) {
                // For arrays, use pointer to element type
                if (arrLit->elementType == "i32") {
                    varLLVMType = llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0);
                } else if (arrLit->elementType == "string") {
                    varLLVMType = llvm::PointerType::get(llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0), 0);
                } else {
                    varLLVMType = llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0);
                }
            } else if (auto *arrAccess = dynamic_cast<ArrayAccessNode*>(node->initializer.get())) {
                // Array access - infer from context, default to i32
                varLLVMType = llvm::Type::getInt32Ty(m_context);
            } else {
                // Default to i32 for unknown types
                varLLVMType = llvm::Type::getInt32Ty(m_context);
                std::cerr << "Codegen Warning: Could not infer type for variable '" << node->variableName << "', defaulting to i32\n";
            }
        } else {
            varLLVMType = llvm::Type::getInt32Ty(m_context);
            std::cerr << "Codegen Warning: No initializer for auto variable '" << node->variableName << "', defaulting to i32\n";
        }
    } else {
        varLLVMType = getLLVMType(node->typeName);
    }
    
    // Create alloca at the beginning of the function
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> TmpB(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    llvm::AllocaInst *allocaInst = TmpB.CreateAlloca(varLLVMType, nullptr, node->variableName);

    namedValues[node->variableName] = allocaInst;
    
    // Store type information for later use (especially for array element types)
    std::string typeToStore = node->typeName;
    if (node->typeName == "auto" && node->initializer) {
        // For auto variables, store the inferred type
        if (auto *strLit = dynamic_cast<StringLiteralNode*>(node->initializer.get())) {
            typeToStore = "string";
        } else if (auto *intLit = dynamic_cast<IntegerLiteralNode*>(node->initializer.get())) {
            typeToStore = "i32";
        } else if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(node->initializer.get())) {
            typeToStore = arrLit->elementType + "[]";
            // Store array size for .length property access
            arraySizes[node->variableName] = arrLit->elements.size();
        } else {
            typeToStore = "i32"; // default
        }
    } else if (node->typeName.length() > 2 && node->typeName.substr(node->typeName.length() - 2) == "[]") {
        // Explicit array type declaration
        if (auto *arrLit = dynamic_cast<ArrayLiteralNode*>(node->initializer.get())) {
            // Store array size for .length property access
            arraySizes[node->variableName] = arrLit->elements.size();
        }
    }
    variableTypes[node->variableName] = typeToStore;

    if (node->initializer)
    {
        llvm::Value *initVal = visit(node->initializer.get());
        if (!initVal)
        {
            throw std::runtime_error("Codegen Error: Failed to generate initializer for variable " + node->variableName);
        }
        
        // Special handling for array assignments
        if (node->typeName.length() > 2 && node->typeName.substr(node->typeName.length() - 2) == "[]") {
            // This is an array type assignment
            // The initVal should be a pointer to the array data
            if (initVal->getType()->isPointerTy() && varLLVMType->isPointerTy()) {
                // Both are pointers, store the pointer value
                m_builder.CreateStore(initVal, allocaInst);
            } else {
                std::cerr << "Codegen Warning: Array type mismatch for variable '" << node->variableName << "'\n";
                m_builder.CreateStore(initVal, allocaInst);
            }
        } else {
            // Regular type compatibility check
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
                    // For array access results, be more lenient
                    if (auto *arrAccess = dynamic_cast<ArrayAccessNode*>(node->initializer.get())) {
                        // Array access - allow type mismatches for now
                    } else {
                        std::cerr << "Codegen Warning: Type mismatch for variable '" << node->variableName << "'\n";
                    }
                }
            }
            
            m_builder.CreateStore(initVal, allocaInst);
        }
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
    
    // Check if we're dealing with strings
    bool isStringComparison = leftVal->getType()->isPointerTy() && rightVal->getType()->isPointerTy();
    
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
    
    // Determine element type by looking up the variable type (similar to array access)
    llvm::Type *elementType = llvm::Type::getInt32Ty(m_context); // default
    
    // Try to get the variable name from the array expression
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->array.get())) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) {
            std::string varType = typeIt->second;
            // Extract element type from array type (e.g., "string[]" -> "string")
            if (varType.length() > 2 && varType.substr(varType.length() - 2) == "[]") {
                std::string elemType = varType.substr(0, varType.length() - 2);
                if (elemType == "string") {
                    elementType = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
                } else if (elemType == "i32") {
                    elementType = llvm::Type::getInt32Ty(m_context);
                } else if (elemType == "f64") {
                    elementType = llvm::Type::getDoubleTy(m_context);
                }
                // Add more types as needed
            }
        }
    }
    
    // Check if arrayValue is a pointer type
    if (!arrayValue->getType()->isPointerTy()) {
        throw std::runtime_error("Codegen Error: Array assignment requires a pointer type");
    }
    
    // Create GEP to get pointer to array element
    llvm::Value *elementPtr;
    
    // If this is a direct array access (not through a variable), handle differently
    if (auto *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(arrayValue)) {
        // Direct access to allocated array
        llvm::Value *indices[] = {
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0),  // Array base
            indexValue  // Element index
        };
        elementPtr = m_builder.CreateGEP(allocaInst->getAllocatedType(), arrayValue, indices, "array_assign_ptr");
    } else {
        // Access through pointer (e.g., from variable)
        elementPtr = m_builder.CreateGEP(elementType, arrayValue, indexValue, "array_assign_ptr");
    }
    
    // Store the value at the array element location
    m_builder.CreateStore(valueToAssign, elementPtr);
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

void CodeGen::visit(ExpressionStatementNode *node)
{
    if (node->expression) {
        visit(node->expression.get()); // Evaluate the expression (side effects like function calls)
    }
}

llvm::Value *CodeGen::visit(FunctionCallNode *node)
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
    if (node->elements.empty()) {
        // Empty array - return null pointer
        return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt32Ty(m_context), 0));
    }
    
    // Determine element type based on first element or explicit type
    llvm::Type *elementType;
    if (node->elementType == "i32") {
        elementType = llvm::Type::getInt32Ty(m_context);
    } else if (node->elementType == "string") {
        elementType = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
    } else {
        // Default to i32
        elementType = llvm::Type::getInt32Ty(m_context);
    }
    
    size_t arraySize = node->elements.size();
    
    // Create array type
    llvm::ArrayType *arrayType = llvm::ArrayType::get(elementType, arraySize);
    
    // Allocate array on stack
    llvm::AllocaInst *arrayAlloca = m_builder.CreateAlloca(arrayType, nullptr, "array_literal");
    
    // Initialize array elements
    for (size_t i = 0; i < arraySize; ++i) {
        llvm::Value *elementValue = visit(node->elements[i].get());
        if (!elementValue) {
            throw std::runtime_error("Codegen Error: Failed to generate array element " + std::to_string(i));
        }
        
        // Get pointer to array element using GEP
        llvm::Value *indices[] = {
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0),  // Array base
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), i)   // Element index
        };
        llvm::Value *elementPtr = m_builder.CreateGEP(arrayType, arrayAlloca, indices, "element_ptr_" + std::to_string(i));
        
        // Store the value
        m_builder.CreateStore(elementValue, elementPtr);
    }
    
    // Return pointer to the array (cast to generic pointer for compatibility)
    return m_builder.CreateBitCast(arrayAlloca, llvm::PointerType::get(elementType, 0), "array_ptr");
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
    llvm::Type *elementType = llvm::Type::getInt32Ty(m_context); // default
    
    // Try to get the variable name from the array expression
    if (auto *varExpr = dynamic_cast<VariableExpressionNode*>(node->array.get())) {
        auto typeIt = variableTypes.find(varExpr->name);
        if (typeIt != variableTypes.end()) {
            std::string varType = typeIt->second;
            // Extract element type from array type (e.g., "string[]" -> "string")
            if (varType.length() > 2 && varType.substr(varType.length() - 2) == "[]") {
                std::string elemType = varType.substr(0, varType.length() - 2);
                if (elemType == "string") {
                    elementType = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);
                } else if (elemType == "i32") {
                    elementType = llvm::Type::getInt32Ty(m_context);
                } else if (elemType == "f64") {
                    elementType = llvm::Type::getDoubleTy(m_context);
                }
                // Add more types as needed
            }
        }
    }
    
    // Check if arrayValue is a pointer type
    if (!arrayValue->getType()->isPointerTy()) {
        throw std::runtime_error("Codegen Error: Array access requires a pointer type");
    }
    
    // Create GEP to access array element
    llvm::Value *elementPtr;
    
    // If this is a direct array access (not through a variable), handle differently
    if (auto *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(arrayValue)) {
        // Direct access to allocated array
        llvm::Value *indices[] = {
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0),  // Array base
            indexValue  // Element index
        };
        elementPtr = m_builder.CreateGEP(allocaInst->getAllocatedType(), arrayValue, indices, "array_element_ptr");
    } else {
        // Access through pointer (e.g., from variable)
        elementPtr = m_builder.CreateGEP(elementType, arrayValue, indexValue, "array_element_ptr");
    }
    
    // Load and return the value using the correct element type
    return m_builder.CreateLoad(elementType, elementPtr, "array_element");
}

// Object literal implementation - basic version
llvm::Value *CodeGen::visit(ObjectLiteralNode *node)
{
    // For now, objects are not fully supported in native compilation
    // This is a placeholder that will throw an error
    throw std::runtime_error("Codegen Error: Object literals are not yet supported in native compilation. Use the browser interpreter for full object support.");
}

// Object access implementation - with array.length support
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
                    // Look up the array size
                    auto sizeIt = arraySizes.find(varExpr->name);
                    if (sizeIt != arraySizes.end()) {
                        // Return the array size as an i32 constant
                        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), sizeIt->second, false);
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
            throw std::runtime_error("Codegen Error: .length property access is only supported on array variables");
        }
    }
    
    // For other property access, object access is not yet supported
    throw std::runtime_error("Codegen Error: Object property access (other than array.length) is not yet supported in native compilation. Use the browser interpreter for full object support.");
}

// External function call support
llvm::Value *CodeGen::generateExternalFunctionCall(FunctionCallNode *node)
{
    llvm::FunctionCallee func = getOrDeclareExternalFunction(node->functionName);
    if (!func) {
        return nullptr; // Function not found
    }
    
    // Generate arguments
    std::vector<llvm::Value*> args;
    for (auto& arg : node->arguments) {
        llvm::Value* argValue = visit(arg.get());
        if (!argValue) {
            throw std::runtime_error("Codegen Error: Failed to generate argument for function " + node->functionName);
        }
        args.push_back(argValue);
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
    
    return nullptr; // Function not found
}
