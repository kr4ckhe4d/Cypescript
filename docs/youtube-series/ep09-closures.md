# EP09 — Closures: The Hardest Thing You'll Ever Compile

**Length target:** 30–34 min (the long one — earn it) · **Repo anchor:**
`visit(ArrowFunctionNode*)`, `collectFreeVars`, `getOrCreateArrowFunction`,
`generateArrayCallbackMethod`, class support in `visit(NewExpressionNode*)`
**Goal:** arrow functions, capture analysis, heap environments, callback
array methods compiled as inline loops — and classes, which turn out to be
the easy dessert.

---

## [COLD OPEN] (0:00–0:45)

[SCREEN: run this]
```typescript
let base: i32 = 10;
let addBase = (x: i32): i32 => x + base;
println(addBase(5));        // 15

let numbers: i32[] = [1, 2, 3, 4, 5, 6];
let sum = numbers.reduce((acc, x) => acc + x, 0);
println(sum);               // 21
```

NARRATION:
> Look at `addBase`. It's a function... that uses a variable from *outside
> itself*. When we call it later — maybe from a totally different function,
> maybe after `base`'s stack frame is long gone — it still knows base was 10.
> A function that remembers its birthplace. That's a closure, it's the reason
> functional programming took decades to get fast, and today we compile it
> to native code. This is the hardest episode of the series. It's also the
> best one.

---

## Segment 1 — Why closures are hard (0:45–6:00)

[B-ROLL / DIAGRAM: a function box trying to reach "up and out" into a stack
frame that's crossed out / freed]

NARRATION:
> Every function we've compiled so far is self-contained: parameters in,
> locals inside, value out. A closure breaks the seal — its body references
> variables that belong to *someone else's* stack frame. Two hard problems:
>
> One: **which** outside variables does it use? Nobody annotates that; we
> must *discover* it by analyzing the body.
>
> Two: **where do those variables live?** The enclosing function's stack
> frame dies when it returns. The closure might be called after that. Stack
> won't do.
>
> Our answers: a compile-time *free-variable analysis*, and a heap-allocated
> *environment*. And one honest design decision up front: Cypescript captures
> **by value** — a snapshot at creation time. JavaScript captures by
> reference. I'll show you exactly what that trade-off buys and costs.

---

## Segment 2 — The closure object (6:00–10:00)

[B-ROLL / DIAGRAM: the closure value: a 2-pointer struct `{fn, env}`; fn
points at generated code, env points at a heap box `{base: 10}`]

[SHOW: from the repo]
```cpp
// A closure VALUE is two pointers on the heap:
llvm::StructType* getClosureType() {   // { i8* fn, i8* env }
    return llvm::StructType::create(m_context, {charPtr, charPtr}, "CypsClosure");
}
```

NARRATION:
> Rule one of compiling closures: a closure is a *value*, and the value is
> just two pointers. Pointer one: the machine code — because the body still
> compiles to a perfectly ordinary function. Pointer two: the environment — a
> heap-allocated struct holding a copy of every captured variable. Calling a
> closure means: call pointer one, passing pointer two as a secret first
> argument. If that "secret first argument" trick sounds familiar — it's
> exactly how we did `this` for methods two episodes ago. Compilers reuse
> their good ideas.

---

## Segment 3 — Free-variable analysis (10:00–17:00)

[SHOW: `collectFreeVars` / `collectFreeVarsExpr` — show the shape, not every case]
```cpp
void collectFreeVarsExpr(ExpressionNode* e, std::set<std::string>& bound,
                                            std::set<std::string>& free) {
    if (auto* v = dynamic_cast<VariableExpressionNode*>(e)) {
        if (!bound.count(v->name)) free.insert(v->name);   // ← the whole idea
    }
    else if (auto* b = dynamic_cast<BinaryExpressionNode*>(e)) {
        collectFreeVarsExpr(b->left.get(), bound, free);
        collectFreeVarsExpr(b->right.get(), bound, free);
    }
    // ... every node type recurses; declarations ADD to `bound` ...
}
```

NARRATION:
> "Free variable" is a scary term for a simple question: which names does
> this body *use* but not *define*? We walk the arrow's body with two sets.
> `bound` starts as the parameters, and grows as we pass local declarations.
> Every variable reference not in `bound` is free — it must come from
> outside. Those are the captures.
>
> Edge cases make it feel real: an assignment target counts as a use — if
> the closure writes `counter`, it needs counter captured. A *nested* arrow's
> free variables — minus its own parameters — bubble up as ours. And a name
> that turns out to be a global function isn't a capture at all; we filter
> against the symbol table at the end.

[SCREEN: instrument the compiler for one run — print the captures found for
`(x) => x + base` → `captures: [base : i32]`. Seeing the analysis "think"
lands the concept better than any diagram.]

---

## Segment 4 — Building environments + generating the function (17:00–24:00)

[SHOW: `visit(ArrowFunctionNode*)` — creation site, condensed to the beats]
```cpp
// 1. captures = free-variable analysis (cached per arrow node)
// 2. envType  = struct of the captured variables' types
// 3. SNAPSHOT: malloc env, copy current values in
llvm::Value* envRaw = m_builder.CreateCall(mallocFn, {envSize});
for (i, capture : captures) {
    llvm::Value* now = m_builder.CreateLoad(..., namedValues[capture.name]);
    m_builder.CreateStore(now, gep(envType, envRaw, i));
}
// 4. fn = getOrCreateArrowFunction(node)   — compile the body
// 5. closure = malloc {fn, env}; return it
```

[SHOW: inside `getOrCreateArrowFunction` — the unpack prologue]
```cpp
// signature: (i8* env, params...) — env is the secret first argument
for (i, capture : captures) {
    llvm::Value* v = m_builder.CreateLoad(fieldType, gep(env, i));
    auto* slot = m_builder.CreateAlloca(fieldType, nullptr, capture.name);
    m_builder.CreateStore(v, slot);
    namedValues[capture.name] = slot;    // body sees it as a NORMAL local
}
```

NARRATION:
> Creation site: build the env struct type from the captures, malloc it, and
> copy the variables' *current values* in. That copy is the snapshot — the
> "by value" in capture-by-value.
>
> Function side: the generated function takes env as its hidden first
> parameter and starts with an unpack prologue — load each captured value out
> of env into a local alloca *registered under the original name*. After the
> prologue, the body compiles with zero special cases: `base` is in the
> symbol table like any local. All the machinery from episodes 4 through 8
> just works inside a closure body.

[SCREEN: demo the semantics honestly — both behaviors]
```typescript
let base: i32 = 10;
let f = (x: i32): i32 => x + base;
base = 999;
println(f(5));      // 15 — snapshot! not 1004

let state = { count: 0 };                 // capture an OBJECT...
let bump = (): i32 => { state.count += 1; return state.count; };
println(bump()); println(bump());          // 1, 2 — shared mutation works
```

> By-value means mutating a captured *primitive* doesn't leak out — different
> from JavaScript, and we document it loudly. But capture an *object* and the
> pointer is what's snapshotted — so mutations through it are shared. That
> idiom covers nearly every real use, and we skipped the hard part of
> by-reference captures: heap-promoting variables and sharing cells. Know
> what you're deferring, say it out loud, ship.

---

## Segment 5 — map/filter/reduce as inline loops (24:00–28:00)

[SHOW: `generateArrayCallbackMethod` — the map skeleton]
```cpp
// numbers.map(x => x * 2) compiles to, roughly:
// result = array_create_i32()
// for (i = 0; i < len; i++)
//     array_push(result, ARROW_FN(env, array_get(numbers, i)))
```

NARRATION:
> Here's where owning the compiler pays off. In JavaScript, `.map` is a
> library function that invokes your callback dynamically. We know the
> callback *at compile time* — so codegen emits the loop **inline**, calling
> the arrow's generated function directly. No dispatch, no iterator protocol.
> LLVM can even inline the callback *into* the loop. Your `.map` compiles to
> the code a C programmer would have written by hand — and `.filter`,
> `.reduce`, `.find`, `.forEach` are the same loop skeleton with different
> middles.

---

## Segment 6 — Dessert: classes in fifteen minutes (28:00–32:00)

[SHOW: the parser's trick — `ClassDeclarationNode` holds a synthesized
ObjectLiteralNode of fields + methods]

NARRATION:
> After closures, classes are dessert, because we built everything already.
> Watch the trick: at *parse time*, a class becomes an object-literal
> template — fields with their defaults, methods attached. `new Point(3, 4)`
> compiles to: instantiate the template — episode 7's struct machinery — then
> call the method named `constructor` with `this` pointing at the fresh
> object. Fields, methods, `this`, property assignment: all reused. The whole
> feature was an afternoon. That's the compounding return on clean layers.

[SCREEN: run the BankAccount demo from `example/20_classes.csc` — including
`throw "insufficient funds"` from inside a method, caught by the caller.
Three episodes shaking hands in one demo.]

---

## [OUTRO] (32:00–33:00)

> Closures, capture analysis, heap environments, zero-overhead array methods,
> and classes. There is no feature left that should scare you. One episode
> remains: we make it *fast* on purpose, benchmark it against Rust, Node, and
> Python on camera — and ship version 1.0 to Homebrew.

**Checkpoint tag:** `ep09-closures` · **Homework:** make `.find` return the
first match with an early `break` out of the generated loop — then check the
repo's version, which jumps straight to the exit block.
