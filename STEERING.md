# Steering

How to make decisions in this codebase. [ROADMAP.md](ROADMAP.md) says what is left;
this says what should stay true regardless of what gets built next.

Every rule here exists because breaking it cost something. The cost is written down
so the rule can be argued with rather than merely obeyed.

---

## The hard rules

### 1. The compiler knows nothing about graphics

Adding a game API means **one line in `runtime/game/cyps_game.c` and one `declare`
in `lib/game.csc`** — never a change to the compiler.

*Why:* the day `drawCircle` becomes a builtin, the language has a graphics
dependency it can never remove, and every other domain (audio, networking,
whatever comes next) gets a precedent for demanding the same. The FFI already
carries this weight; a builtin would be admitting it doesn't.

*Test of compliance:* `grep -i raylib src/` returns nothing.

### 2. Benchmarks are the regression gate

`benchmarks/cross/run_cross_benchmarks.sh 3` — **always best-of-3**, a single run is
too noisy to conclude anything from. Rust parity (0.051s primes, 0.025s fib(35)) is
the number to protect.

*Why:* it is the one claim the language makes that is falsifiable, and it is the
first thing anyone checks. Two changes that could have cost it were shaped
specifically to avoid it, and both shapes must survive:

- Module-level globals are promoted **only when a function actually references
  them**, so a top-level hot loop keeps its allocas.
- **Only hierarchies that actually override a method get vtables**, so a class
  that is never subclassed keeps its direct call.

*When it regresses:* read the emitted IR before theorising. Both of the above were
found by reading IR, not by reasoning about the source.

### 3. Define behaviour in the IR; don't inherit libc's accidents

If generated code can reach a value libc handles differently on different
platforms, **substitute in the IR**.

*Why:* `println` of a null string emitted a bare `puts`. glibc's `puts` calls
`strlen` and segfaults on NULL; Darwin's prints `(null)`. A missing JSON key is a
null string, so ordinary code reached it — and the test fixture was written against
the behaviour of the one platform it had been run on. macOS was green for weeks
while Linux was broken.

*The tell:* "it works on my machine" for anything that bottoms out in libc.

### 4. Evaluate a target once

`a[f()] += v`, `a[f()]++`, `obj.p++` — the target is read and written through **one**
evaluation of the container and index.

*Why:* the original compound assignment desugared to `a[f()] = a[f()] + v`, which
called `f()` twice and could read one slot while writing another. Any new
read-modify-write form inherits this requirement; `emitArrayStore` and
`resolveObjectProperty` exist so it is shared rather than re-derived.

### 5. An unknown type silences the check

The type checker reports a mismatch only when it knows **both** sides. Unknown,
`auto`, `json` and unresolved generics pass.

*Why:* it lets checking be tightened incrementally without a flag day. The cost is
real and accepted — `let n: i32 = someF64;` is still allowed because codegen coerces
it, where TypeScript would reject. Tightening that needs a survey of existing code,
not a one-line change.

### 6. Interfaces stay structural

A class satisfies an interface by **shape**. `implements` declares an intent that is
*verified*; it does not become the only way to conform.

*Why:* object literals satisfy interfaces too, and they have nothing to write
`implements` on. Making conformance nominal would split the rule in two.

### 7. No garbage collector

Heap objects live until exit unless pooled. Removing an entity from an array does
**not** free it — other references may still point at it.

*Why:* a GC forfeits the no-pause story the benchmarks exist to protect. The
alternative is real and demonstrated: frame-scoped strings
(`enableFrameStrings()`) plus entity pooling hold RSS flat from 2,000 to 300,000
frames. A game needs both mechanisms, not either one.

### 8. Every platform branch must be compiled by someone

A `#else` branch that no one on the team compiles is not portable code, it is
untested code that looks reassuring.

*Why:* `fs::path bundle = executable.parent_path() / appName + "-bundle";` sat in a
non-Apple `#else` and does not compile at all — `/` binds tighter than `+`, and
`fs::path` has no `operator+`. It was the entire Linux CI build failure, and macOS
could never have caught it. It was found by locally rewriting the platform macros to
force every branch through the compiler. Do that when touching platform code.

### 9. FFI has no scaffolding

`declare function name(...): T = "c_symbol";` plus `link source "x.c";`. No header,
no makefile, no wrapper script, no library to build first. A `.c` gets
`clang -std=c11`, a `.cpp` gets `clang++ -std=c++17`, in the same program.

*Why:* the previous flow needed four files and a build step to call one C function,
which meant nobody reached for C until they were desperate.

---

## Where a language feature lands

In order. Skipping a row is how a feature ends up half-implemented.

| Step | File | What goes there |
|---|---|---|
| 1 | `src/Lexer.cpp`, `src/Token.h` | New tokens. Prefer a **contextual keyword** over a reserved one — `declare`, `link`, `super`, `implements` are all contextual, so existing code using them as identifiers still compiles |
| 2 | `src/AST.h` | The node. Give it `line`/`column` from the token, or its errors come out positionless |
| 3 | `src/Parser.cpp` | Construction. Expression forms go in `parseUnaryExpression` (prefix) or the postfix chain at the end of `parsePrimaryExpression` |
| 4 | `src/Semantic.cpp` | `typeOf` for inference, `analyzeExpression`/`analyzeStatement` for checking. **Check whether an existing statement-level rule now gets bypassed** — moving `i++` into the expression parser silently dropped its `const` check until it was re-added |
| 5 | `src/CodeGen.cpp` | The `visit` overload **and** the dispatch chain in `visit(ExpressionNode*)`, plus `inferExpressionLLVMType` and `collectFreeVarsExpr` if the node can appear in a declaration initializer or a closure |
| 6 | `tests/` | A positive test **with an expected-output fixture**, and a negative test per distinct error message |
| 7 | `README.md`, `docs/index.html` | Both. They drift independently |
| 8 | `ROADMAP.md` | Move the row from Next to Done, update the test count in the header |

### Definition of done

- `bash tests/run_tests.sh` — all green, and the count went **up**
- `bash tests/run_game_tests.sh` — all green
- `bash tests/run_readme_tests.sh` — every README snippet compiles
- Every example compiles and runs
- `bash benchmarks/cross/run_cross_benchmarks.sh 3` — unchanged
- Docs updated in both places; roadmap reflects reality

**A README snippet is code, and it is compiled.** If a snippet cannot stand alone
— an FFI fragment naming a symbol that does not exist, an `import` needing a
sibling module — it opts out with an HTML comment above its fence that names the
example backing it:

```
<!-- snippet: illustrative — runnable version in example/21_c_interop.csc -->
```

The named path is checked for existence, so opting out is not a way to avoid
having an example; it is how a snippet declares which one it has.

A feature that is documented but not tested, or tested but not documented, is not
done. Both have happened here.

---

## Traps already paid for

Short list, so they are not re-learned.

| Trap | What happened |
|---|---|
| `.gitignore` swallowing fixtures | `*.out` excluded every `tests/expected/*.out`. A fresh clone had no fixtures, so CI had *only ever* asserted exit codes. Verify a new fixture kind actually reaches a clean clone |
| `set -e` in test scripts | `raw=$(… \| grep …)` returning non-zero killed the script before its own "could not measure" branch could run. Twice |
| GNU ld archive order | Archives are scanned strictly left-to-right and discarded once passed. `libcypescript.a` goes **last** because `libcypescript_game.a` calls into it. macOS's linker is order-insensitive, so this only ever appears on Linux |
| `strcmp` on non-strings | Pointer equality dispatched to `strcmp`, so `rock == null` segfaulted on a non-null pointer. Check `isNonStringPointer` before reaching for a string comparison |
| `&&`/`\|\|` phi mismatch | Mixing an i32 and an i1 arm emits invalid IR. Reconcile both arms |
| Stray executables in commits | `cscript -r foo.csc` drops a binary in the cwd. Three got committed. Compile scratch work with `-o` into a scratch dir, and check `git status` before `git add -A` |

---

## Deliberate non-goals

Decisions, not omissions — do not "fix" these without a reason that outweighs the
rationale above: no GC, no automatic free on removal, no nominal interfaces, no
graphics knowledge in the compiler.
