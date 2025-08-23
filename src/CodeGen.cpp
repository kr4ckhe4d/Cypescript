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

void CodeGen::visit(FunctionCallNode *node)
{
    if (node->functionName == "print")
    {
        if (node->arguments.size() != 1)
        {
            std::cerr << "Codegen Error: 'print' expects exactly one argument.\n";
            throw std::runtime_error("'print' expects one argument.");
        }

        llvm::Value *argValue = visit(node->arguments[0].get());
        if (!argValue)
        {
            std::cerr << "Codegen Error: Failed to generate code for 'print' argument.\n";
            throw std::runtime_error("Failed to generate code for 'print' argument.");
        }

        llvm::Type *argType = argValue->getType();
        llvm::Type *expectedStringLLVMType = llvm::PointerType::get(llvm::Type::getInt8Ty(m_context), 0);

        if (argType->isPointerTy())
        {
            // Assume it's a string pointer, use puts
            llvm::FunctionCallee putsFunc = getOrDeclarePuts();
            m_builder.CreateCall(putsFunc, argValue, "putsCall");
        }
        else if (argType->isIntegerTy(32))
        {
            // Argument is i32
            llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
            // Create format string "%d\n" for printf
            llvm::Value *formatStr = m_builder.CreateGlobalString("%d\n", ".format_int");
            std::vector<llvm::Value *> printfArgs = {formatStr, argValue};
            m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
        }
        else
        {
            std::cerr << "Codegen Error: 'print' argument type not supported. Expected string or i32.\n";
            throw std::runtime_error("'print' argument type not supported.");
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
