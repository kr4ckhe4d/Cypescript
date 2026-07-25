// src/CodeGen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"
#include "ObjectOptimizer.h"  // NEW: Include optimizer
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
#include <set>

// Forward declare AST node types we'll visit
class ProgramNode;
class StatementNode;
class ExpressionNode; // For the general expression visitor
class FunctionCallNode;
class FunctionDeclarationNode;
class ReturnStatementNode;
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
class ForOfStatementNode;      // For for-of loops
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

    // Module-level variables promoted to LLVM globals so that functions can see
    // them. Only variables actually referenced by some function body are
    // promoted — everything else stays an alloca in main, which keeps top-level
    // hot loops (the benchmarks) exactly as fast as before.
    std::map<std::string, llvm::GlobalVariable *> globalValues;
    // Names referenced inside any function/method/arrow body (collected in pass 0)
    std::set<std::string> namesUsedByFunctions;
    // True while generating main's top-level statements
    bool atModuleLevel = false;

    // Resolves a variable to its storage slot: a local alloca if one is in
    // scope, otherwise a module-level global. Returns null if neither exists.
    llvm::Value *variableStorage(const std::string &name, llvm::Type **outType = nullptr);
    // Collects every identifier referenced by function bodies, so module-level
    // declarations know whether they need to be globals
    void collectFunctionReferencedNames(ProgramNode *node);
    
    // Type tracking: Maps variable names to their type information
    // For arrays, stores the element type (e.g., "i32" for i32[], "string" for string[])
    std::map<std::string, std::string> variableTypes;

    // Const tracking: Maps variable names to boolean (true if const)
    std::map<std::string, bool> constVariables;

    // Array size tracking: Maps array variable names to their sizes
    std::map<std::string, size_t> arraySizes;
    
    // OPTIMIZED: Object system with direct access (Phase 1 optimization)
    ObjectOptimizer objectOptimizer;
    OptimizedObjectCreator objectCreator;
    std::map<std::string, ObjectOptimizer::ObjectLayout> objectLayouts;
    
    // Legacy object property tracking (to be phased out after optimization)
    std::map<std::string, std::map<std::string, llvm::Value*>> objectProperties;
    std::map<std::string, std::map<std::string, std::string>> objectPropertyTypes;
    
    // Object variable tracking: Maps variable names to their object keys
    std::map<std::string, std::string> variableToObjectKey;

    // Helper to get the object key for an expression
    std::string getExpressionObjectKey(ExpressionNode* expr);

    // Function context tracking
    llvm::Function *currentFunction = nullptr;  // Track current function being generated
    std::map<std::string, llvm::Function*> declaredFunctions; // Track declared functions

    // Loop/switch target stack for break/continue.
    // Each entry is {continueTarget (may be null inside switch), breakTarget}.
    std::vector<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>> loopTargets;
    // Try-nesting depth recorded when each loop/switch was entered (parallel to loopTargets)
    std::vector<int> loopTargetTryDepths;

    // Current try-nesting depth in the function being generated. return/break/continue
    // that exit try blocks must pop the exception recovery points they skip.
    int tryDepth = 0;
    // Emits `count` calls to cyps_try_pop()
    void emitTryPops(int count);

    // Interface registry for structural type checking (nodes owned by the AST)
    std::map<std::string, const InterfaceDeclarationNode*> interfaces;

    // Foreign functions declared with `declare function` (nodes owned by the AST).
    // Consulted before the built-in stdlib table, so a program can bind any C
    // symbol without the compiler knowing anything about it.
    std::map<std::string, const ExternDeclarationNode*> externFunctions;

    // Class registry: `new ClassName(...)` instantiates the class's object
    // template and calls its constructor (nodes owned by the AST)
    std::map<std::string, ClassDeclarationNode*> classes;

    // Parses "closure(a,b)=>r" type strings used for closure-typed parameters
    bool parseClosureSignature(const std::string &signature,
                               std::vector<std::string> &argTypes, std::string &returnType);

    // Object methods: objectKey -> methodName -> declaration (owned by the AST)
    std::map<std::string, std::map<std::string, FunctionDeclarationNode*>> objectMethods;
    // Cache of generated method functions, keyed by "objectKey::method"
    std::map<std::string, llvm::Function*> methodFunctions;
    // Object key bound to `this` while generating a method body
    std::string currentThisObjectKey;

    // Lazily generates the LLVM function for an object method
    llvm::Function *getOrCreateMethodFunction(const std::string& objectKey,
                                              const std::string& methodName);

    // --- Arrow functions / closures ---
    // Closures are {i8* fn, i8* env} pairs; captures are snapshotted by value.
    std::map<std::string, ArrowFunctionNode*> variableToArrow; // var name -> arrow node
    std::map<ArrowFunctionNode*, llvm::Function*> arrowFunctions;
    std::map<ArrowFunctionNode*, llvm::StructType*> arrowEnvTypes;
    std::map<ArrowFunctionNode*, std::vector<std::pair<std::string, std::string>>> arrowCaptures;
    llvm::StructType *closureType = nullptr; // {i8* fn, i8* env}
    int arrowCounter = 0;

    llvm::StructType *getClosureType();
    // Free-variable analysis for capture lists
    void collectFreeVars(StatementNode *stmt, std::set<std::string> &bound,
                         std::set<std::string> &free);
    void collectFreeVarsExpr(ExpressionNode *expr, std::set<std::string> &bound,
                             std::set<std::string> &free);
    // Heuristic return-type inference for un-annotated arrows
    llvm::Type *inferArrowReturnType(ArrowFunctionNode *node,
                                     const std::map<std::string, std::string> &paramTypes);
    llvm::Type *inferExpressionLLVMType(ExpressionNode *expr,
                                        const std::map<std::string, std::string> &paramTypes);
    // Generates (and caches) the LLVM function for an arrow node
    llvm::Function *getOrCreateArrowFunction(ArrowFunctionNode *node);
    // Resolves a callback argument to its arrow node (literal or bound variable)
    ArrowFunctionNode *resolveArrowArgument(ExpressionNode *expr);
    // Evaluates a callback argument into {function, env} for direct calls
    std::pair<llvm::Function*, llvm::Value*> materializeCallback(ExpressionNode *expr);
    // Array methods taking callbacks: map/filter/forEach/reduce/find
    llvm::Value *generateArrayCallbackMethod(MethodCallNode *node, llvm::Value *arrayPtr,
                                             const std::string &elemType);
    // Recorded type name for variables initialized from method calls (map/filter/...)
    std::string inferMethodCallTypeName(MethodCallNode *node);

    // Structural check of an object literal against an interface
    void checkInterfaceConformance(const std::string& interfaceName, ObjectLiteralNode* literal,
                                   const std::string& variableName);
    // Collects all members including inherited ones
    void collectInterfaceMembers(const std::string& interfaceName,
                                 std::vector<InterfaceDeclarationNode::Member>& out);

    // Converts any value to a C string (for concatenation / throw)
    llvm::Value *toStringValue(llvm::Value *val);
    // Coerces a value to the target type (i1<->i32, i32<->f64)
    llvm::Value *coerceValue(llvm::Value *val, llvm::Type *targetType);
    // Declares _setjmp with the returns_twice attribute
    llvm::FunctionCallee getOrDeclareSetjmp();
    // Emits a branch to `target` and re-anchors the builder in a dead block
    void branchAndSealBlock(llvm::BasicBlock *target, const std::string& deadName);

    // Helper to get LLVM type from our string type names
    llvm::Type *getLLVMType(const std::string &typeName);

    // Visitor methods for different AST node types
    void visit(ProgramNode *node);
    void visit(StatementNode *node);           // Dispatcher
    void visit(VariableDeclarationNode *node); // New
    void visit(FunctionDeclarationNode *node); // For function definitions
    // Declares a function's signature without generating its body
    llvm::Function *declareFunctionSignature(FunctionDeclarationNode *node);
    void visit(TypeAliasNode *node);           // For type aliases
    void visit(ReturnStatementNode *node);     // For return statements
    void visit(ExpressionStatementNode *node); // For expressions used as statements
    llvm::Value *visit(FunctionCallNode *node); // Now returns a value since it's an expression
    void visit(IfStatementNode *node);         // For if/else statements
    void visit(WhileStatementNode *node);      // For while loops
    void visit(ForStatementNode *node);        // For traditional for loops
    void visit(ForOfStatementNode *node);      // For for-of loops
    void visit(DoWhileStatementNode *node);    // For do-while loops
    void visit(AssignmentStatementNode *node); // For variable assignments
    void visit(ArrayAssignmentStatementNode *node); // For array element assignments
    void visit(BreakStatementNode *node);      // For break
    void visit(ContinueStatementNode *node);   // For continue
    void visit(SwitchStatementNode *node);     // For switch/case
    void visit(InterfaceDeclarationNode *node); // For interface declarations
    void visit(ClassDeclarationNode *node);     // For class declarations
    void visit(ObjectPropertyAssignmentNode *node); // For obj.prop = value
    void visit(DestructuringDeclarationNode *node); // For let { a, b } = obj
    void visit(TryCatchStatementNode *node);   // For try/catch/finally
    void visit(ThrowStatementNode *node);      // For throw

    llvm::Value *visit(ExpressionNode *node); // Dispatcher
    llvm::Value *visit(StringLiteralNode *node);
    llvm::Value *visit(FloatLiteralNode *node);       // For f64 literals
    llvm::Value *visit(IntegerLiteralNode *node);     // New
    llvm::Value *visit(BooleanLiteralNode *node);     // New
    llvm::Value *visit(VariableExpressionNode *node); // New
    llvm::Value *visit(BinaryExpressionNode *node);   // For arithmetic operations
    llvm::Value *visit(UnaryExpressionNode *node);    // For unary operations
    llvm::Value *visit(ArrayLiteralNode *node);       // For array literals
    llvm::Value *visit(ObjectLiteralNode *node);      // For object literals
    llvm::Value *visit(ArrayAccessNode *node);        // For array access
    llvm::Value *visit(ObjectAccessNode *node);       // For object access
    llvm::Value *visit(MethodCallNode *node);         // For method calls
    llvm::Value *visit(NewExpressionNode *node);      // For new expressions
    llvm::Value *visit(ArrowFunctionNode *node);      // For arrow functions / closures

    llvm::FunctionCallee getOrDeclarePuts();
    llvm::FunctionCallee getOrDeclarePrintf(); // New: For printf for integers
    
    // External function support
    llvm::Value *generateExternalFunctionCall(FunctionCallNode *node);
    llvm::FunctionCallee getOrDeclareExternalFunction(const std::string& name);
    
    // Object support helper functions
    llvm::Value *createEmptyObject();
    llvm::Value *createObjectWithProperties(ObjectLiteralNode *node);
    
    // OPTIMIZED: Phase 1 object creation with direct struct access
    llvm::Value *createOptimizedObjectWithProperties(ObjectLiteralNode *node);

    // Truthy check helper
    llvm::Value *ensureI1(llvm::Value *val);

public:
    CodeGen(llvm::LLVMContext &context);
    llvm::Module *generate(ProgramNode *astRoot);
};

#endif // CODEGEN_H
