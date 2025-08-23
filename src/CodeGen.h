// src/CodeGen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"        // For llvm::Value
#include "llvm/IR/Type.h"         // For llvm::Type
#include "llvm/IR/DerivedTypes.h" // For llvm::FunctionType etc.
#include "llvm/IR/Instructions.h" // For llvm::AllocaInst

#include <memory>
#include <string>
#include <map> // For symbol table

// Forward declare AST node types we'll visit
class ProgramNode;
class StatementNode;
class ExpressionNode; // For the general expression visitor
class FunctionCallNode;
class StringLiteralNode;
class IntegerLiteralNode;      // New
class VariableExpressionNode;  // New
class VariableDeclarationNode; // New
class BinaryExpressionNode;    // For arithmetic operations
class IfStatementNode;         // For if/else statements

class CodeGen
{
private:
    llvm::LLVMContext &m_context;
    std::unique_ptr<llvm::Module> m_module;
    llvm::IRBuilder<> m_builder;

    // Symbol Table: Maps variable names to their allocated memory location (AllocaInst)
    // This will need to be enhanced for scopes later.
    std::map<std::string, llvm::AllocaInst *> namedValues;

    // Helper to get LLVM type from our string type names
    llvm::Type *getLLVMType(const std::string &typeName);

    // Visitor methods for different AST node types
    void visit(ProgramNode *node);
    void visit(StatementNode *node);           // Dispatcher
    void visit(VariableDeclarationNode *node); // New
    void visit(FunctionCallNode *node);
    void visit(IfStatementNode *node);         // For if/else statements

    llvm::Value *visit(ExpressionNode *node); // Dispatcher
    llvm::Value *visit(StringLiteralNode *node);
    llvm::Value *visit(IntegerLiteralNode *node);     // New
    llvm::Value *visit(VariableExpressionNode *node); // New
    llvm::Value *visit(BinaryExpressionNode *node);   // For arithmetic operations

    llvm::FunctionCallee getOrDeclarePuts();
    llvm::FunctionCallee getOrDeclarePrintf(); // New: For printf for integers

public:
    CodeGen(llvm::LLVMContext &context);
    llvm::Module *generate(ProgramNode *astRoot);
};

#endif // CODEGEN_H
