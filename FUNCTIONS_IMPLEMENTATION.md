# Function Implementation Guide

This document outlines the technical implementation plan for user-defined functions in Cypescript.

## Overview

Functions are one of the most requested features for Cypescript. This implementation will add:
- Function declarations with parameters and return types
- Function calls with argument passing
- Local variable scoping
- Return statements
- Function overloading (future)
- Recursive function support

## Implementation Phases

### Phase 1: Core Function Support

#### 1.1 Lexer Changes (`src/Lexer.cpp`)
Add new tokens:
```cpp
enum class TokenType {
    // ... existing tokens ...
    FUNCTION,    // "function"
    RETURN,      // "return"
    // ... rest of tokens ...
};
```

Update keyword recognition:
```cpp
if (identifier == "function") return Token(TokenType::FUNCTION, "function");
if (identifier == "return") return Token(TokenType::RETURN, "return");
```

#### 1.2 AST Nodes (`src/AST.h`)
Add new AST node types:
```cpp
// Function declaration node
class FunctionDeclarationNode : public ASTNode {
public:
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters; // (name, type)
    std::string returnType;
    std::unique_ptr<BlockStatementNode> body;
    
    FunctionDeclarationNode(const std::string& name, 
                           std::vector<std::pair<std::string, std::string>> params,
                           const std::string& returnType,
                           std::unique_ptr<BlockStatementNode> body);
};

// Function call node
class FunctionCallNode : public ExpressionNode {
public:
    std::string functionName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    
    FunctionCallNode(const std::string& name, 
                    std::vector<std::unique_ptr<ExpressionNode>> args);
};

// Return statement node
class ReturnStatementNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expression; // nullptr for void returns
    
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expr);
};

// Block statement node (for function bodies)
class BlockStatementNode : public StatementNode {
public:
    std::vector<std::unique_ptr<StatementNode>> statements;
    
    BlockStatementNode(std::vector<std::unique_ptr<StatementNode>> stmts);
};
```

#### 1.3 Parser Changes (`src/Parser.cpp`)
Add parsing methods:
```cpp
class Parser {
private:
    // ... existing methods ...
    std::unique_ptr<FunctionDeclarationNode> parseFunctionDeclaration();
    std::unique_ptr<FunctionCallNode> parseFunctionCall(const std::string& name);
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();
    std::unique_ptr<BlockStatementNode> parseBlockStatement();
    
public:
    // Update existing methods to handle functions
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<ExpressionNode> parseExpression();
};
```

Implementation:
```cpp
std::unique_ptr<FunctionDeclarationNode> Parser::parseFunctionDeclaration() {
    consume(TokenType::FUNCTION);
    std::string name = consume(TokenType::IDENTIFIER).value;
    consume(TokenType::LPAREN);
    
    std::vector<std::pair<std::string, std::string>> parameters;
    while (currentToken.type != TokenType::RPAREN) {
        std::string paramName = consume(TokenType::IDENTIFIER).value;
        consume(TokenType::COLON);
        std::string paramType = consume(TokenType::TYPE_I32 | TokenType::TYPE_STRING).value;
        parameters.push_back({paramName, paramType});
        
        if (currentToken.type == TokenType::COMMA) {
            consume(TokenType::COMMA);
        }
    }
    consume(TokenType::RPAREN);
    consume(TokenType::COLON);
    std::string returnType = consume(TokenType::TYPE_I32 | TokenType::TYPE_STRING | TokenType::VOID).value;
    
    auto body = parseBlockStatement();
    return std::make_unique<FunctionDeclarationNode>(name, parameters, returnType, std::move(body));
}
```

#### 1.4 Symbol Table (`src/SymbolTable.h` - new file)
Create symbol table for function and variable tracking:
```cpp
class SymbolTable {
private:
    struct FunctionInfo {
        std::string name;
        std::vector<std::string> parameterTypes;
        std::string returnType;
        llvm::Function* llvmFunction;
    };
    
    struct VariableInfo {
        std::string name;
        std::string type;
        llvm::Value* llvmValue;
        int scope; // 0 = global, 1+ = function local
    };
    
    std::unordered_map<std::string, FunctionInfo> functions;
    std::unordered_map<std::string, VariableInfo> variables;
    int currentScope = 0;
    
public:
    void enterScope();
    void exitScope();
    void addFunction(const FunctionInfo& func);
    void addVariable(const VariableInfo& var);
    FunctionInfo* getFunction(const std::string& name);
    VariableInfo* getVariable(const std::string& name);
};
```

#### 1.5 CodeGen Changes (`src/CodeGen.cpp`)
Add LLVM function generation:
```cpp
class CodeGen {
private:
    SymbolTable symbolTable;
    llvm::Function* currentFunction = nullptr;
    
public:
    void visit(FunctionDeclarationNode* node) override;
    void visit(FunctionCallNode* node) override;
    void visit(ReturnStatementNode* node) override;
    void visit(BlockStatementNode* node) override;
};

void CodeGen::visit(FunctionDeclarationNode* node) {
    // Create LLVM function type
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node->parameters) {
        paramTypes.push_back(getLLVMType(param.second));
    }
    
    llvm::Type* returnType = getLLVMType(node->returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // Create LLVM function
    llvm::Function* func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, node->name, module.get());
    
    // Add to symbol table
    SymbolTable::FunctionInfo funcInfo;
    funcInfo.name = node->name;
    funcInfo.llvmFunction = func;
    funcInfo.returnType = node->returnType;
    for (const auto& param : node->parameters) {
        funcInfo.parameterTypes.push_back(param.second);
    }
    symbolTable.addFunction(funcInfo);
    
    // Create function body
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(entryBlock);
    
    currentFunction = func;
    symbolTable.enterScope();
    
    // Add parameters to symbol table
    auto argIt = func->arg_begin();
    for (const auto& param : node->parameters) {
        llvm::Value* arg = &(*argIt++);
        arg->setName(param.first);
        
        SymbolTable::VariableInfo varInfo;
        varInfo.name = param.first;
        varInfo.type = param.second;
        varInfo.llvmValue = arg;
        varInfo.scope = symbolTable.getCurrentScope();
        symbolTable.addVariable(varInfo);
    }
    
    // Generate function body
    node->body->accept(this);
    
    // Add default return if needed
    if (node->returnType == "void" && !builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRetVoid();
    }
    
    symbolTable.exitScope();
    currentFunction = nullptr;
}
```

### Phase 2: Advanced Features

#### 2.1 Local Variable Scoping
- Implement proper scope management in symbol table
- Handle variable shadowing
- Automatic cleanup of local variables

#### 2.2 Function Overloading
- Extend symbol table to handle multiple functions with same name
- Implement overload resolution based on parameter types
- Generate mangled names for LLVM functions

#### 2.3 Recursive Functions
- Ensure proper stack frame management
- Add stack overflow protection
- Optimize tail recursion

## Testing Strategy

### Unit Tests
```cpp
// Test function declaration parsing
TEST(ParserTest, FunctionDeclaration) {
    std::string code = "function add(a: i32, b: i32): i32 { return a + b; }";
    Parser parser(code);
    auto ast = parser.parse();
    // Verify AST structure
}

// Test function call parsing
TEST(ParserTest, FunctionCall) {
    std::string code = "let result: i32 = add(5, 3);";
    Parser parser(code);
    auto ast = parser.parse();
    // Verify AST structure
}
```

### Integration Tests
```cypescript
// Basic function test
function double(x: i32): i32 {
    return x * 2;
}

let result: i32 = double(21);
println(result); // Should print 42

// Recursive function test
function factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

let fact: i32 = factorial(5);
println(fact); // Should print 120
```

## Performance Considerations

1. **LLVM Optimization**: Functions will benefit from LLVM's optimization passes
2. **Inlining**: Small functions can be inlined automatically
3. **Tail Call Optimization**: Recursive functions can be optimized
4. **Stack Management**: Efficient stack frame allocation

## Compatibility

- Functions will work with existing C++ integration
- Existing code will continue to work unchanged
- Functions can call C++ integration functions
- C++ functions can potentially call Cypescript functions (future)

## Timeline Estimate

- **Phase 1 (Basic Functions)**: 2-3 weeks
- **Phase 2 (Advanced Features)**: 1-2 weeks
- **Testing and Polish**: 1 week
- **Total**: 4-6 weeks for complete implementation

## Getting Started

To contribute to function implementation:

1. Start with lexer changes (add FUNCTION and RETURN tokens)
2. Add AST nodes for function declarations and calls
3. Implement parser methods for function syntax
4. Create symbol table for function tracking
5. Add LLVM code generation for functions
6. Write comprehensive tests

This feature will significantly enhance Cypescript's capabilities and bring it closer to being a full-featured programming language!
