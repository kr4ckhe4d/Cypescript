# EP08 — Exceptions Without a Runtime: setjmp/longjmp (+ Real Error Messages)

**Length target:** 24–28 min · **Repo anchor:** `visit(TryCatchStatementNode*)`,
`cyps_try_push/pop/throw` in `cypescript_stdlib.cpp`, `src/Semantic.cpp`
**Goal:** working try/catch/finally/throw via setjmp/longjmp, and a semantic
analysis pass that gives users positioned errors before codegen.

---

## [COLD OPEN] (0:00–0:40)

[SCREEN: run this live]
```typescript
function risky(n: i32): i32 {
    if (n < 0) { throw "negative input: " + n; }
    return n * 2;
}
try {
    println(risky(-1));
} catch (e) {
    println("caught: " + e);
}
```
```
caught: negative input: -1
```

NARRATION:
> A throw, deep inside a function call, unwinding back to a catch block in
> the caller — in a language that compiles to raw machine code with no
> exception runtime. Industrial compilers do this with DWARF unwind tables
> and landing pads, and it takes them years. We're going to do it today,
> with two functions from 1979.

---

## Segment 1 — setjmp/longjmp: the time machine (0:40–7:00)

[B-ROLL / DIAGRAM: a call stack tower. setjmp plants a flag at frame 2; the
stack grows to frame 5; longjmp cuts straight back to the flag.]

NARRATION:
> C's `setjmp` bookmarks the current execution state — registers, stack
> pointer — into a buffer, and returns 0. `longjmp` teleports back to that
> bookmark... at which point setjmp returns *again*, this time with a nonzero
> value. One function, two returns. It's `goto` across stack frames.
>
> Mapping to exceptions: **try = plant a bookmark. throw = teleport to the
> nearest bookmark. catch = the code that runs when setjmp returns the second
> time.** Nested tries? A *stack* of bookmarks; throw always uses the top.

[SHOW: the runtime, `cypescript_stdlib.cpp`]
```cpp
static jmp_buf g_try_stack[64];
static int g_try_top = 0;
static std::string g_last_error;

void* cyps_try_push() { return g_try_stack[g_try_top++]; }
void  cyps_try_pop()  { if (g_try_top > 0) g_try_top--; }

void cyps_throw(const char* msg) {
    g_last_error = msg;
    if (g_try_top == 0) {                       // no bookmark: crash politely
        fprintf(stderr, "Uncaught exception: %s\n", msg);
        exit(1);
    }
    _longjmp(g_try_stack[--g_try_top], 1);      // teleport
}
```

> Thirty lines. That's the entire exception runtime.

---

## Segment 2 — Codegen for try/catch (7:00–14:00)

[SHOW: `visit(TryCatchStatementNode*)` — the skeleton]
```cpp
llvm::Value* buf = m_builder.CreateCall(pushFn, {}, "try_buf");
llvm::Value* rc  = m_builder.CreateCall(getOrDeclareSetjmp(), {buf});
llvm::Value* ok  = m_builder.CreateICmpEQ(rc, zero);
m_builder.CreateCondBr(ok, tryBB, catchBB);

// tryBB: body...; cyps_try_pop(); br cont     (normal completion)
// catchBB: e = cyps_last_error(); catch body; br cont
// cont: finally statements
```

NARRATION:
> Follow both timelines. Normal run: setjmp returns 0, we take the try
> branch, run the body, *pop our bookmark*, continue. Exceptional run:
> somewhere — any function deep — calls cyps_throw, longjmp fires, and
> execution *reappears at our setjmp*, now returning 1. We take the catch
> branch, fetch the message, bind it to `e` as an ordinary local string
> variable, run the handler. `finally` is simply the block both paths fall
> into.

⚠ PITFALL #1 (this one is exotic):
> setjmp must be declared with LLVM's **returns_twice** attribute. Without
> it, the optimizer assumes — like for every normal function — that setjmp
> returns once, and it will happily cache values in registers across the
> call. Then longjmp rewinds the registers and your variables hold stale
> garbage, but only at -O2, and only sometimes. One attribute:
> `fn->addFnAttr(llvm::Attribute::ReturnsTwice)`. Write it in blood.

⚠ PITFALL #2 (a real shipped bug — tell it as a story):
> Version one of this feature had a bug that survived weeks: `return` inside
> a try block. The return jumps out of the function... but the bookmark never
> got popped. Stack now holds a bookmark pointing into a *dead stack frame*.
> The next throw teleports into a corpse. The fix, in the repo: codegen
> tracks `tryDepth`, and every return/break/continue that exits try blocks
> emits the right number of `cyps_try_pop()` calls first. When you mix
> nonlocal exits, always ask: "what cleanup am I jumping over?"

[SCREEN: run the nested try + rethrow demo, and the uncaught-exception path
showing the clean "Uncaught exception:" exit.]

---

## Segment 3 — Semantic analysis: errors before codegen (14:00–22:00)

[SCREEN: demo the errors first]
```
Semantic Error: Use of undefined variable 'undefinedVar' at line 2, column 9
Semantic Error: Cannot reassign const variable 'C' at line 2, column 1
Semantic Error: 'break' used outside of a loop or switch at line 1, column 1
Semantic Error: Function 'f' expects 1 argument(s), got 2 at line 2, column 14
```

NARRATION:
> Second half of the episode: being kind to your users. Until now, a typo'd
> variable name exploded somewhere inside codegen with no location. The fix
> is a dedicated pass between parsing and codegen — it changes nothing,
> generates nothing; it only *walks and judges*.

[SHOW: `src/Semantic.h` — the core state]
```cpp
class SemanticAnalyzer {
    std::vector<std::map<std::string, Binding>> m_scopes;  // scope stack
    std::map<std::string, size_t> m_functions;             // name → arity
    int m_loopDepth = 0;
    bool m_inMethod = false;   // is `this` legal here?
};
```

NARRATION (over a walk of `analyzeStatement`):
> The whole pass is one idea: a **stack of scopes**. Enter a block, push a
> map. Declare a variable, write it into the top map. Meet a name, search
> the maps top-down — not found anywhere? That's the undefined-variable
> error, with the line and column we've been carrying in every AST node
> since episode 2. Finally paying off.
>
> Everything else rides the same walk. Const-ness is a bool in the binding —
> assignment to a const binding is an error. `break` legality is an integer:
> loops increment `m_loopDepth` going in, decrement coming out; break with
> depth zero is an error. Function arity is a map lookup at every call site.
>
> One subtlety worth pausing on: **function bodies must NOT see the caller's
> locals** — so entering a function body, we swap the whole scope stack out
> and start fresh. But an *arrow function* — episode 9 — captures its
> environment, so arrows analyze *inside* the current scopes. That
> distinction is lexical scoping, stated in two lines of code.

[SHOW: main.cpp — the pass wired in between parse and codegen; compile fails
before any IR exists.]

---

## [OUTRO] (22:00–23:00)

> Your language now fails the way professionals expect: early, located, and
> in plain English — and when a user's *program* fails, it throws and catches
> like TypeScript. Next episode is the summit. Closures. The feature that
> makes language implementers sweat: functions that remember where they came
> from. Bring coffee.

**Checkpoint tag:** `ep08-exceptions` · **Homework:** add a semantic check
that `continue` is rejected inside a `switch` unless a loop encloses it —
the repo does this with a null continue-target in the same stack.
