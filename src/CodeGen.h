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
class ArrayLiteralNode;        // For array literals
class ObjectLiteralNode;       // For object literals
class ArrayAccessNode;         // For array access
class ObjectAccessNode;        // For object access
class IfStatementNode;         // For if/else statements
class WhileStatementNode;      // For while loops
class ForStatementNode;        // For traditional for loops
class DoWhileStatementNode;    // For do-while loops
class AssignmentStatementNode; // For variable assignments
class ArrayAssignmentStatementNode; // For array element assignments

class CodeGen
{
private:
    llvm::LLVMContext &m_context;
    std::unique_ptr<llvm::Module> m_module;
    llvm::IRBuilder<> m_builder;

    // Symbol Table: Maps variable names to their allocated memory location (AllocaInst)
    // This will need to be enhanced for scopes later.
    std::map<std::string, llvm::AllocaInst *> namedValues;
    
    // Type tracking: Maps variable names to their type information
    // For arrays, stores the element type (e.g., "i32" for i32[], "string" for string[])
    std::map<std::string, std::string> variableTypes;
    
    // Array size tracking: Maps array variable names to their sizes
    std::map<std::string, size_t> arraySizes;

    // Helper to get LLVM type from our string type names
    llvm::Type *getLLVMType(const std::string &typeName);

    // Visitor methods for different AST node types
    void visit(ProgramNode *node);
    void visit(StatementNode *node);           // Dispatcher
    void visit(VariableDeclarationNode *node); // New
    void visit(ExpressionStatementNode *node); // For expressions used as statements
    llvm::Value *visit(FunctionCallNode *node); // Now returns a value since it's an expression
    void visit(IfStatementNode *node);         // For if/else statements
    void visit(WhileStatementNode *node);      // For while loops
    void visit(ForStatementNode *node);        // For traditional for loops
    void visit(DoWhileStatementNode *node);    // For do-while loops
    void visit(AssignmentStatementNode *node); // For variable assignments
    void visit(ArrayAssignmentStatementNode *node); // For array element assignments

    llvm::Value *visit(ExpressionNode *node); // Dispatcher
    llvm::Value *visit(StringLiteralNode *node);
    llvm::Value *visit(IntegerLiteralNode *node);     // New
    llvm::Value *visit(VariableExpressionNode *node); // New
    llvm::Value *visit(BinaryExpressionNode *node);   // For arithmetic operations
    llvm::Value *visit(ArrayLiteralNode *node);       // For array literals
    llvm::Value *visit(ObjectLiteralNode *node);      // For object literals
    llvm::Value *visit(ArrayAccessNode *node);        // For array access
    llvm::Value *visit(ObjectAccessNode *node);       // For object access

    llvm::FunctionCallee getOrDeclarePuts();
    llvm::FunctionCallee getOrDeclarePrintf(); // New: For printf for integers
    
    // External function support
    llvm::Value *generateExternalFunctionCall(FunctionCallNode *node);
    llvm::FunctionCallee getOrDeclareExternalFunction(const std::string& name);

public:
    CodeGen(llvm::LLVMContext &context);
    llvm::Module *generate(ProgramNode *astRoot);
};

#endif // CODEGEN_H
