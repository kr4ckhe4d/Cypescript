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
    return nullptr; // Or throw
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
    }
}

void CodeGen::visit(VariableDeclarationNode *node)
{
    llvm::Type *varLLVMType = getLLVMType(node->typeName);
    if (!varLLVMType)
    {
        throw std::runtime_error("Codegen Error: Invalid type for variable declaration: " + node->typeName);
    }

    // Create an alloca instruction in the entry block of the current function.
    // For simplicity, assuming all variables are declared at the top of 'main'.
    // A more robust approach would find the current function's entry block.
    llvm::Function *currentFunction = m_builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> TmpB(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    llvm::AllocaInst *allocaInst = TmpB.CreateAlloca(varLLVMType, nullptr, node->variableName);

    // Store the alloca in the symbol table
    namedValues[node->variableName] = allocaInst;

    // Generate code for the initializer and store its value if it exists
    if (node->initializer)
    {
        llvm::Value *initVal = visit(node->initializer.get());
        if (!initVal)
        {
            throw std::runtime_error("Codegen Error: Failed to generate initializer for variable " + node->variableName);
        }
        // Check if types are compatible before storing (simplified for now)
        // e.g. if varLLVMType is i8* and initVal is a global string (which is i8*), it's fine.
        // if varLLVMType is i32 and initVal is an i32 ConstantInt, it's fine.
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
    return nullptr;
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
            return;
        }

        llvm::Value *argValue = visit(node->arguments[0].get()); // Generate code for the argument expression
        if (!argValue)
        {
            std::cerr << "Codegen Error: Failed to generate code for 'print' argument.\n";
            return;
        }

        // For MVP, 'print' uses 'puts' and thus expects an i8* (char*)
        // If argValue is not i8*, we might need a cast or a different print function.
        // Our StringLiteralNode returns i8*.
        // Our VariableExpressionNode for a string variable (stored as i8*) will also load an i8*.
        // If we try to print an i32, this will currently fail type checking in LLVM or at runtime.
        if (argValue->getType() != llvm::Type::getInt8PtrTy(m_context))
        {
            std::cerr << "Codegen Error: 'print' currently only supports string arguments. Argument type was: ";
            argValue->getType()->print(llvm::errs());
            llvm::errs() << "\n";
            // For now, we'll let LLVM's verifier catch this if it's a problem, or it might misbehave.
            // A proper solution involves type checking or different print functions.
        }

        llvm::FunctionCallee putsFunc = getOrDeclarePuts();
        m_builder.CreateCall(putsFunc, argValue, "putsCall");
    }
    else
    {
        std::cerr << "Codegen Error: Unsupported function call '" << node->functionName << "'.\n";
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
        visit(astRoot); // This will populate m_module
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Caught codegen exception: " << e.what() << std::endl;
        // Optionally print module state for debugging even on error
        // m_module->print(llvm::errs(), nullptr);
        return nullptr; // Indicate failure
    }

    if (llvm::verifyModule(*m_module, &llvm::errs()))
    {
        std::cerr << "Error: LLVM module verification failed after generation!\n";
        // m_module->print(llvm::errs(), nullptr); // Dump for debugging
        return nullptr;
    }
    return m_module.get();
}
