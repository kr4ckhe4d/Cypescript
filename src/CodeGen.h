// src/CodeGen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h" // Need AST node definitions
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include <memory> // For unique_ptr from AST

// Forward declare AST node types we'll visit
class ProgramNode;
class StatementNode;
class FunctionCallNode;
class StringLiteralNode;
// Add others as the language grows

class CodeGen {
private:
    llvm::LLVMContext& m_context;
    std::unique_ptr<llvm::Module> m_module; // Generator owns the Module
    llvm::IRBuilder<> m_builder;

    // Visitor methods for different AST node types
    // Return llvm::Value* for expressions, void for statements (or refine later)
    void visit(ProgramNode* node);
    void visit(StatementNode* node);
    void visit(FunctionCallNode* node);
    llvm::Value* visit(ExpressionNode* node); // Dispatches to specific expressions
    llvm::Value* visit(StringLiteralNode* node);

    // Helper to get or declare the 'puts' function
    llvm::FunctionCallee getOrDeclarePuts();


public:
    // Constructor takes context and ownership of the AST root (or just a ref?)
    // Let's pass AST by reference/pointer, generator doesn't own it.
    CodeGen(llvm::LLVMContext& context);

    // Main function to generate code for the given AST
    // Returns the generated Module (caller can take ownership or just use it)
    llvm::Module* generate(ProgramNode* astRoot);

};

#endif // CODEGEN_H