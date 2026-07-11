# EP06 — Functions and a Real Runtime

**Length target:** 24–28 min · **Repo anchor:** `CodeGen::visit(FunctionDeclarationNode*)`,
`src/cypescript_stdlib.cpp`, `getOrDeclareExternalFunction`
**Goal:** user-defined functions with params/returns/recursion, then the
C++ runtime library trick: dynamic arrays, string helpers, file I/O.

---

## [COLD OPEN] (0:00–0:35)

[SCREEN: run recursive fibonacci]
```bash
./build/cscript -r fib.csc     # function fib(n: i32): i32 { ... fib(n-1)+fib(n-2) }
# 9227465  (fib 35, in ~25ms)
```

NARRATION:
> Thirty million recursive calls in twenty-five milliseconds. Today we add
> functions to the language — and then I'll show you the single highest-
> leverage trick in language building: how to give your language strings,
> dynamic arrays, and file I/O in one afternoon, by cheating gracefully.

---

## Segment 1 — Compiling a function (0:35–9:00)

[SHOW: the function visitor, walked in four beats]
```cpp
// 1. Signature: our types → LLVM types
std::vector<llvm::Type*> paramTypes;
for (auto& p : node->parameters) paramTypes.push_back(getLLVMType(p.type));
auto* fnType = llvm::FunctionType::get(getLLVMType(node->returnType), paramTypes, false);
auto* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage,
                                  node->functionName, m_module.get());
declaredFunctions[node->functionName] = fn;

// 2. Fresh scope: save the caller's symbol table
auto savedNames = namedValues;

// 3. Params become local variables (alloca + store)
auto* entry = llvm::BasicBlock::Create(m_context, "entry", fn);
m_builder.SetInsertPoint(entry);
auto argIt = fn->arg_begin();
for (auto& p : node->parameters) {
    auto* slot = m_builder.CreateAlloca(getLLVMType(p.type), nullptr, p.name);
    m_builder.CreateStore(&*argIt++, slot);
    namedValues[p.name] = slot;
}

// 4. Body, implicit return for void, restore scope
for (auto& s : node->bodyStatements) visit(s.get());
if (!m_builder.GetInsertBlock()->getTerminator()) m_builder.CreateRetVoid();
namedValues = savedNames;
```

NARRATION:
> Four beats. **Signature** — translate your type names into LLVM types and
> create the function. **Scope** — save the symbol table, because `x` inside
> a function is not `x` outside it; restoring the map after IS lexical
> scoping. **Parameters** — here's the non-obvious one: LLVM arguments are
> read-only values, but your language lets users assign to parameters. So we
> copy each argument into an alloca. Yes, a copy — LLVM's optimizer deletes
> every one of these that doesn't need to exist. Emit honest, simple IR and
> let -O2 be clever. **Body** — visit statements, exactly like main.

> Recursion? Look at step 1: we register the function in `declaredFunctions`
> *before* generating the body. So when the body calls itself, the lookup
> succeeds. That ordering is the entire implementation of recursion.

[SHOW: two-pass trick in `visit(ProgramNode*)` — functions generated first so
main can call them regardless of declaration order.]

⚠ PITFALL:
> If a non-void function's body can fall off the end, the final block has no
> terminator — THE RULE from last episode. The repo seals it with
> `CreateUnreachable()`. Do something; doing nothing fails verification.

---

## Segment 2 — Calls, and the arity check (9:00–13:00)

[SHOW: user-function call path in `visit(FunctionCallNode*)`]
```cpp
auto it = declaredFunctions.find(node->functionName);
if (it != declaredFunctions.end()) {
    llvm::Function* fn = it->second;
    if (node->arguments.size() != fn->arg_size())
        throw std::runtime_error("Function '" + node->functionName + "' expects " +
            std::to_string(fn->arg_size()) + " arguments");
    std::vector<llvm::Value*> args;
    size_t i = 0;
    for (auto& a : node->arguments)
        args.push_back(coerceValue(visit(a.get()),
                       fn->getFunctionType()->getParamType(i++)));
    return m_builder.CreateCall(fn, args, "call");
}
```

NARRATION:
> Evaluate each argument, coerce it to the declared parameter type — that
> little `coerceValue` quietly handles int-to-float promotion — and
> CreateCall. Note we check the argument count *ourselves* with a decent
> message. LLVM would also object, but its objection is an assertion failure
> at IR-build time. Guard at your level; keep errors human.

---

## Segment 3 — The runtime library trick (13:00–21:00)

[B-ROLL / DIAGRAM: two boxes side by side — "your compiled program (.ll)" and
"cypescript_stdlib.cpp", both flowing into clang, out comes one executable]

NARRATION:
> Now the trick that changes everything. Your language needs dynamic arrays.
> Growable, heap-managed arrays. You could implement realloc logic in raw
> LLVM IR... and you would still be doing it next month.
>
> Or: write it in C++, in twenty minutes, and *link it in*.

[SHOW: `src/cypescript_stdlib.cpp` — the dynamic array]
```cpp
class DynamicArray {
public:
    std::vector<int32_t> i32_data;
    std::vector<std::string> string_data;
    // ...
};

extern "C" {
    void* array_create_i32() { return new DynamicArray(...); }
    void array_push_i32(void* arr, int32_t v) {
        static_cast<DynamicArray*>(arr)->i32_data.push_back(v);
    }
    int32_t array_get_i32(void* arr, int32_t i) { /* bounds-checked read */ }
    int32_t array_length(void* arr) { /* ... */ }
}
```

NARRATION:
> Two crucial details. **extern "C"** — C++ normally mangles function names
> into things like `_Z14array_push_i32Pvi`; extern "C" keeps them plain so
> our generated IR can call them by name. And **void-pointer handles** — the
> compiled program never knows what a DynamicArray is. It holds an opaque
> pointer and passes it back to the runtime. Your language's array is,
> secretly, a battle-tested `std::vector`. That's not cheating. That's what
> "runtime library" means — Go's garbage collector and Rust's allocator are
> the same move at bigger scale.

[SHOW: the codegen side — declaring and calling]
```cpp
llvm::FunctionCallee pushFn = m_module->getOrInsertFunction("array_push_i32",
    llvm::Type::getVoidTy(ctx),      // returns void
    charPtr,                          // void* arr
    llvm::Type::getInt32Ty(ctx));     // i32 value
m_builder.CreateCall(pushFn, {arrPtr, value});
```

> `getOrInsertFunction` declares the signature; the linker connects it to the
> C++ definition later. So `arr.push(5)` in your language compiles to one
> call instruction into a std::vector. Suddenly, in one episode: strings
> (`string_concat`, `string_upper`), file I/O (`file_read`, `file_write`),
> random numbers — each one is ten lines of C++ plus one declaration.

[SCREEN: run a demo — array push/length/for-of + file_write/file_read.]

---

## Segment 4 — String concat done honestly (21:00–25:00)

[SHOW: the `+` operator's string branch in `visit(BinaryExpressionNode*)`]
```cpp
if (node->op == ADD && (L->getType()->isPointerTy() || R->getType()->isPointerTy())) {
    llvm::Value* ls = toStringValue(L);   // i32/f64 → cyps_i32_to_string(...)
    llvm::Value* rs = toStringValue(R);
    return m_builder.CreateCall(concatFn, {ls, rs}, "concat");
}
```

NARRATION:
> With a runtime, `"age: " + 28` stops being scary: if either side of a plus
> is a string, convert the other side with a runtime helper and call
> string_concat. Four lines in codegen; the C++ does the work.
>
> Honesty checkpoint: we `new` these strings and never free them. That's a
> real decision, and it's the right one *for now* — compilers are allowed to
> have script-lifetime memory in v1. It's written down in the repo's
> SHIPPING_BLOCKERS doc, not swept under the rug. Ship, document, iterate.

---

## [OUTRO] (25:00–26:00)

> Functions, recursion, arrays, strings, files — your language is now
> *useful*. Next episode is my favorite systems lesson in the series: objects.
> We'll store them as raw structs, access fields with a single instruction,
> and I'll tell you the story of the 2.1x speedup that decision bought — with
> the benchmark to prove it.

**Checkpoint tag:** `ep06-functions-runtime` · **Homework:** add a
`string_repeat(s, n)` runtime function and call it from your language.
