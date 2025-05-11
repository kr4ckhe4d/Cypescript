// src/CodeGen.cpp
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
#include "llvm/IR/Instructions.h" // For AllocaInst, StoreInst, LoadInst
#include <iostream>
#include <vector>
#include <stdexcept> // For runtime_error

// --- Constructor ---
CodeGen::CodeGen(llvm::LLVMContext &context) : m_context(context),
                                               m_builder(context)
{
    m_module = std::make_unique<llvm::Module>("MyLangModule", m_context);
}

// --- Helper Methods ---

llvm::Type *CodeGen::getLLVMType(const std::string &typeName)
{
    if (typeName == "string")
    {
        // Strings will be represented as char* (i8*)
        return llvm::Type::getInt8PtrTy(m_context);
    }
    else if (typeName == "i32")
    {
        return llvm::Type::getInt32Ty(m_context);
    }
    // Add other types like "f64", "bool", custom types, etc.
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
    llvm::Type *argType = llvm::Type::getInt8PtrTy(m_context);
    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argType, false);
    llvm::Function *func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "puts", m_module.get());
    return llvm::FunctionCallee(funcType, func);
}

// New helper function to declare printf
llvm::FunctionCallee CodeGen::getOrDeclarePrintf()
{
    if (auto *func = m_module->getFunction("printf"))
    {
        return llvm::FunctionCallee(func->getFunctionType(), func);
    }
    // printf signature: int printf(const char* format, ...);
    // LLVM type: i32 (i8*, ...) - note the 'true' for vararg
    llvm::Type *returnType = llvm::Type::getInt32Ty(m_context);
    llvm::Type *formatArgType = llvm::Type::getInt8PtrTy(m_context);
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

    // Clear symbol table for each new program generation (or function later)
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
    // CreateAlloca is usually placed at the beginning of the function for clarity and correctness
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
        // Basic type check for assignment (can be more sophisticated)
        if (initVal->getType() != varLLVMType)
        {
            // Allow storing i8* (from string literal) to an i8* alloca.
            // Allow storing i32 (from int literal) to an i32 alloca.
            // Other cases might need explicit casts or are errors.
            // For now, we rely on LLVM's verifier or later semantic checks.
            // A common case is string literal (global i8*) to local string variable (alloca i8*).
            if (!(varLLVMType->isPointerTy() && initVal->getType()->isPointerTy() &&
                  varLLVMType->getContainedType(0)->isIntegerTy(8) &&
                  initVal->getType()->getContainedType(0)->isIntegerTy(8)) &&          /* for string to string */
                !(varLLVMType->isIntegerTy(32) && initVal->getType()->isIntegerTy(32)) /* for i32 to i32 */
            )
            {
                std::cerr << "Codegen Warning: Type mismatch for variable '" << node->variableName
                          << "'. Expected ";
                varLLVMType->print(llvm::errs());
                std::cerr << " but got ";
                initVal->getType()->print(llvm::errs());
                std::cerr << std::endl;
                // Potentially throw an error here in a stricter compiler
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
    // Returns an i8* pointer to the global string
    return m_builder.CreateGlobalStringPtr(node->value, ".str_literal");
}

llvm::Value *CodeGen::visit(IntegerLiteralNode *node)
{
    // For now, assume i32 type for all integer literals.
    // Type inference or explicit type suffixes (e.g., 10L for i64) could be added later.
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), node->value, true); // true for signed
}

llvm::Value *CodeGen::visit(VariableExpressionNode *node)
{
    llvm::AllocaInst *allocaInst = namedValues[node->name];
    if (!allocaInst)
    {
        throw std::runtime_error("Codegen Error: Unknown variable name: " + node->name);
    }
    // Load the value from the memory location
    // The type of the load is the type the alloca was created with
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
        llvm::Type *expectedStringLLVMType = llvm::Type::getInt8PtrTy(m_context);

        if (argType == expectedStringLLVMType)
        { // Direct comparison for i8*
            // Argument is i8* (string)
            llvm::FunctionCallee putsFunc = getOrDeclarePuts();
            m_builder.CreateCall(putsFunc, argValue, "putsCall");
        }
        else if (argType->isIntegerTy(32))
        {
            // Argument is i32
            llvm::FunctionCallee printfFunc = getOrDeclarePrintf();
            // Create format string "%d\n" for printf
            llvm::Value *formatStr = m_builder.CreateGlobalStringPtr("%d\n", ".format_int");
            std::vector<llvm::Value *> printfArgs = {formatStr, argValue};
            m_builder.CreateCall(printfFunc, printfArgs, "printfCall");
        }
        else
        {
            std::cerr << "Codegen Error: 'print' argument type not supported. Expected string (i8*) or i32. Got: ";
            argType->print(llvm::errs());
            llvm::errs() << "\n";
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
