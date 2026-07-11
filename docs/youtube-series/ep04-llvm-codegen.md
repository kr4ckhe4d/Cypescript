# EP04 — Hello, LLVM: Generating Real Machine Code

**Length target:** 26–30 min · **Repo anchor:** `src/CodeGen.cpp` (first 500 lines)
**Goal:** the series' payoff moment — walk the AST, emit LLVM IR, link, and
run a native executable. Variables, arithmetic, and print.

---

## [COLD OPEN] (0:00–0:40)

[SCREEN: terminal]

[TYPE]
```bash
echo 'let x: i32 = 6 * 7; println(x);' > answer.csc
./build/cscript -r answer.csc
```
```
42
```

[TYPE]
```bash
file answer
# answer: Mach-O 64-bit executable arm64
```

NARRATION:
> That is not an interpreter. That is not a VM. That's a real native
> executable, indistinguishable from one produced by Clang — because in a
> sense, it *was* produced by Clang's engine. Today your language starts
> producing machine code.

---

## Segment 1 — LLVM IR in ten minutes (0:40–8:00)

[SHOW: the IR for the cold-open program, annotated on screen]
```llvm
@.format_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %x = alloca i32                ; reserve stack space for x
  store i32 42, ptr %x           ; x = 42  (6*7 folded by LLVM)
  %x_val = load i32, ptr %x      ; read x
  call i32 (ptr, ...) @printf(ptr @.format_int, i32 %x_val)
  ret i32 0
}
```

NARRATION (annotate each line with an arrow overlay):
> LLVM IR is assembly for a computer that doesn't exist — infinite registers,
> explicit types everywhere. Four instructions matter today:
>
> **alloca** — carve out stack space for a variable.
> **store** — write a value into that space.
> **load** — read it back.
> **call** — call a function, here C's own printf.
>
> Notice what we *don't* do: pick real registers, think about calling
> conventions, schedule instructions. LLVM does all of it, better than we
> ever could. Our job is just to translate the tree faithfully.

[B-ROLL / DIAGRAM: "the deal" — you emit honest IR, LLVM gives you -O2, every
CPU architecture, and 20 years of optimization research for free]

---

## Segment 2 — The CodeGen class and the visitor pattern (8:00–14:00)

[SHOW: `src/CodeGen.h`, trimmed]
```cpp
class CodeGen {
    llvm::LLVMContext& m_context;             // owns types & constants
    std::unique_ptr<llvm::Module> m_module;   // one compilation unit
    llvm::IRBuilder<> m_builder;              // the instruction "pen"

    std::map<std::string, llvm::AllocaInst*> namedValues;  // symbol table
    std::map<std::string, std::string> variableTypes;

    llvm::Value* visit(ExpressionNode* node); // dispatcher
    void visit(StatementNode* node);          // dispatcher
    llvm::Value* visit(IntegerLiteralNode*);
    llvm::Value* visit(BinaryExpressionNode*);
    void visit(VariableDeclarationNode*);
    // ... one visit per node type ...
};
```

NARRATION:
> Three LLVM objects to know. The **Context** is a bag holding types and
> constants. The **Module** is your output file — functions and globals live
> in it. The **IRBuilder** is a pen: it remembers *where* you're writing and
> gives you one method per instruction — CreateAdd, CreateLoad, CreateStore.
>
> And our own crucial piece: the **symbol table**. When codegen meets
> `let x`, it allocas stack space and remembers "x lives at this address."
> When an expression mentions `x`, we look it up and load. That map IS
> variable scoping, materialized.

[SHOW: the expression dispatcher — dynamic_cast chain]
```cpp
llvm::Value* CodeGen::visit(ExpressionNode* node) {
    if (auto* n = dynamic_cast<IntegerLiteralNode*>(node)) return visit(n);
    if (auto* n = dynamic_cast<BinaryExpressionNode*>(node)) return visit(n);
    if (auto* n = dynamic_cast<VariableExpressionNode*>(node)) return visit(n);
    // ...
}
```

> Purists will tell you to use virtual methods or std::variant. They're not
> wrong. But dynamic_cast dispatch is *readable*, and this exact chain ships
> in the finished compiler. Clarity first; cleverness when it hurts.

---

## Segment 3 — Literals, variables, arithmetic (14:00–21:00)

[TYPE: the three easiest visitors]
```cpp
llvm::Value* CodeGen::visit(IntegerLiteralNode* node) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), node->value);
}

void CodeGen::visit(VariableDeclarationNode* node) {
    llvm::Value* initVal = visit(node->initializer.get());
    llvm::AllocaInst* slot = m_builder.CreateAlloca(
        llvm::Type::getInt32Ty(m_context), nullptr, node->variableName);
    m_builder.CreateStore(initVal, slot);
    namedValues[node->variableName] = slot;          // remember the address
}

llvm::Value* CodeGen::visit(VariableExpressionNode* node) {
    llvm::AllocaInst* slot = namedValues[node->name];
    return m_builder.CreateLoad(slot->getAllocatedType(), slot);
}
```

NARRATION:
> Declaration: evaluate the initializer, alloca a slot, store, remember.
> Use: look up the slot, load. That's variables. Genuinely. That's all of it.

[TYPE: binary expressions]
```cpp
llvm::Value* CodeGen::visit(BinaryExpressionNode* node) {
    llvm::Value* L = visit(node->left.get());
    llvm::Value* R = visit(node->right.get());
    switch (node->op) {
        case ADD:      return m_builder.CreateAdd(L, R, "addtmp");
        case SUBTRACT: return m_builder.CreateSub(L, R, "subtmp");
        case MULTIPLY: return m_builder.CreateMul(L, R, "multmp");
        case DIVIDE:   return m_builder.CreateSDiv(L, R, "divtmp");
        case LESS_THAN: return m_builder.CreateICmpSLT(L, R, "lttmp");
        // ...
    }
}
```

> Recursion does the heavy lifting again: evaluate left subtree, evaluate
> right subtree, combine. The tree shape from episode 3 — where `*` sat
> deeper than `+` — is *exactly* evaluation order now. The parser already
> solved precedence; codegen just walks.

⚠ PITFALL:
> `CreateSDiv`, not `CreateUDiv` — signed vs unsigned division are different
> instructions. Same with comparisons: `ICmpSLT` (signed less-than). Pick the
> unsigned ones by accident and negative numbers go quietly insane.

---

## Segment 4 — main(), printf, and the run pipeline (21:00–27:00)

[SHOW: wrapping statements in main]
```cpp
llvm::FunctionType* mainType =
    llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), false);
llvm::Function* mainFn = llvm::Function::Create(
    mainType, llvm::Function::ExternalLinkage, "main", m_module.get());
llvm::BasicBlock* entry = llvm::BasicBlock::Create(m_context, "entry", mainFn);
m_builder.SetInsertPoint(entry);
// ... visit every top-level statement ...
m_builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));
```

NARRATION:
> Your language's top-level statements become the body of a C main function.
> That's the secret handshake with the operating system — the OS knows how
> to run main, so we hand it one.

[SHOW: println → printf declaration + call; then verifyModule]
```cpp
if (llvm::verifyModule(*m_module, &llvm::errs())) {
    // broken IR — fail loudly NOW, not mysteriously later
}
```

⚠ PITFALL:
> Call `verifyModule` after every generation, forever. It catches malformed
> IR — a block without a terminator, a type mismatch — at compile time with
> a real message, instead of a segfault inside LLVM three stages later. This
> one call saved me dozens of hours across this project.

[SCREEN: the full pipeline, live]
```bash
./build/cscript -o out.ll answer.csc     # your compiler → IR
clang out.ll -o answer -O2               # LLVM → native (linker included)
./answer                                 # 42
```

> And notice `-O2` — we just inherited the entire LLVM optimizer. Later, the
> finished compiler wraps these steps so `cscript -r` does it all, but it's
> exactly this underneath.

---

## [OUTRO] (27:00–28:00)

> Take a second. You wrote a language, and a real executable came out.
> Everything from here is *more of the same walk* — fancier nodes, same
> pattern. Next episode: if statements and loops, where we meet basic blocks
> and the one rule about them that will bite you if nobody warns you. I'm
> going to warn you.

**Checkpoint tag:** `ep04-codegen` · **Homework:** add `-` and `%` codegen,
then compile `println(100 % 7);` to a native binary.
