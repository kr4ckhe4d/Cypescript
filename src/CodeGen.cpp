// src/CodeGen.cpp
#include "CodeGen.h"
#include "AST.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h" // To check generated code for errors
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Constants.h" // For ConstantInt etc.
#include <iostream> // For errors
#include <vector>

// --- Constructor ---
CodeGen::CodeGen(llvm::LLVMContext& context) :
    m_context(context),
    m_builder(context) // Initialize IRBuilder with the context
{
    // Create the module immediately. The 'generate' function will populate it.
    m_module = std::make_unique<llvm::Module>("MyLangModule", m_context);
}

// --- Helper Methods ---

llvm::FunctionCallee CodeGen::getOrDeclarePuts() {
    // Check if 'puts' is already declared in the module
    if (auto* func = m_module->getFunction("puts")) {
        // llvm::Function* isn't directly callable, need FunctionCallee
        return llvm::FunctionCallee(func->getFunctionType(), func);
    }

    // 'puts' function signature: int puts(char* str);
    // In LLVM types: i32 puts(i8* str);
    llvm::Type* returnType = llvm::Type::getInt32Ty(m_context);
    llvm::Type* argType = llvm::Type::getInt8PtrTy(m_context); // Explicitly i8*
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, argType, false);

    // Add the declaration to the module
    llvm::Function* func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "puts", m_module.get()
    );
    return llvm::FunctionCallee(funcType, func);
}


// --- Visitor Methods ---

void CodeGen::visit(ProgramNode* node) {
    // Set up the main function for the program
    // Function type: int main() -> i32 ()
    llvm::FunctionType* mainFuncType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), false);
    llvm::Function* mainFunc = llvm::Function::Create(
        mainFuncType, llvm::Function::ExternalLinkage, "main", m_module.get()
    );

    // Create the entry basic block for main
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(m_context, "entry", mainFunc);
    m_builder.SetInsertPoint(entryBlock); // Tell builder where to insert instructions

    // Visit all statements in the program
    for (const auto& stmt : node->statements) {
        visit(stmt.get()); // Visit each statement
    }

    // Add return 0 from main
    m_builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));

    // Verify the main function for correctness (optional but good)
    if (llvm::verifyFunction(*mainFunc, &llvm::errs())) {
         std::cerr << "Error: main function verification failed!\n";
         // Consider cleaning up or halting
    }
}

void CodeGen::visit(StatementNode* node) {
    // Use dynamic_cast or a better visitor pattern later. For MVP:
    if (auto* callNode = dynamic_cast<FunctionCallNode*>(node)) {
        visit(callNode);
    } else {
        std::cerr << "Codegen Error: Unsupported statement type.\n";
        // Handle other statement types later
    }
}

llvm::Value* CodeGen::visit(ExpressionNode* node) {
     // Use dynamic_cast or a better visitor pattern later. For MVP:
    if (auto* strNode = dynamic_cast<StringLiteralNode*>(node)) {
        return visit(strNode);
    } else {
        std::cerr << "Codegen Error: Unsupported expression type.\n";
         // Handle other expression types later
        return nullptr;
    }
}


llvm::Value* CodeGen::visit(StringLiteralNode* node) {
    // Create a global string constant and return a pointer (i8*) to it.
    // IRBuilder::CreateGlobalStringPtr automatically handles creating the
    // global variable and the pointer computation (GEP).
    return m_builder.CreateGlobalStringPtr(node->value, ".str"); // Name hint ".str"
}


void CodeGen::visit(FunctionCallNode* node) {
    // Remove or comment out any previous DEBUG print lines we added
    if (node->functionName == "print") {
        if (node->arguments.size() != 1) {
            std::cerr << "Codegen Error: 'print' expects exactly one argument.\n";
            return;
        }

        auto* argExpr = node->arguments[0].get();
        auto* strNode = dynamic_cast<StringLiteralNode*>(argExpr);
        if (!strNode) {
            std::cerr << "Codegen Error: 'print' argument must be a string literal for MVP.\n";
            return;
        }

        // --- Explicit Global String and BitCast ---
        // 1. Create the LLVM constant string data
        //    (Ensure null terminator is included if puts needs it, getString does this by default)
        llvm::Constant* strConstant = llvm::ConstantDataArray::getString(m_context, strNode->value, true);

        // 2. Create the global variable to hold the string constant
        auto* globalVar = new llvm::GlobalVariable(
            *m_module,                    // Module
            strConstant->getType(),       // Type: [N x i8]
            true,                         // isConstant
            llvm::GlobalValue::PrivateLinkage, // Linkage
            strConstant,                  // Initializer
            ".str"                        // Name hint
        );
        globalVar->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        globalVar->setAlignment(llvm::MaybeAlign(1));

        // 3. Explicitly BitCast the pointer to the global array ([N x i8]*) to i8*
        llvm::Type* i8PtrTy = llvm::Type::getInt8PtrTy(m_context); // Target type: i8*
        // globalVar itself is the pointer -> type [N x i8]* (or ptr in opaque)
        llvm::Value* castedStrPtr = m_builder.CreateBitCast(globalVar, i8PtrTy, "str_casted"); // Name hint

        // 4. Get the 'puts' function (declared as i32(i8*))
        llvm::FunctionCallee putsFunc = getOrDeclarePuts(); // Ensure this uses i8*

        // 5. Create the call instruction using the EXPLICITLY CASTED pointer
        m_builder.CreateCall(putsFunc, castedStrPtr, "putsCall");
        // --- End Explicit Global String and BitCast ---

    } else {
        std::cerr << "Codegen Error: Unsupported function call '" << node->functionName << "'.\n";
    }
}


// --- Main Generation Method ---

llvm::Module* CodeGen::generate(ProgramNode* astRoot) {
    if (!astRoot) {
        std::cerr << "Codegen Error: Cannot generate code from null AST.\n";
        return nullptr;
    }

    // Start visiting from the root node
    visit(astRoot);

    // Verify the entire module (optional but highly recommended)
    if (llvm::verifyModule(*m_module, &llvm::errs())) {
        std::cerr << "Error: LLVM module verification failed!\n";
        // Optionally dump module even if verification fails for debugging:
        // m_module->print(llvm::errs(), nullptr);
        return nullptr; // Indicate failure
    }

    // Return the raw pointer to the generated module.
    // NOTE: The CodeGen object still OWNS the unique_ptr to the module.
    // The caller should NOT delete this pointer.
    // A better design might be to *transfer* ownership if needed.
    return m_module.get();
}