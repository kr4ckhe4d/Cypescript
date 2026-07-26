# Language Roadmap — Phase 7 onward

[GAME_ROADMAP.md](GAME_ROADMAP.md) is complete: all six phases done, a native arcade game
written in idiomatic Cypescript, flat memory, packaged as a double-clickable app.

This picks up where that left off. The theme is no longer *capability* — it is **making
the language safe and pleasant to use**, because the biggest remaining gaps are places
where Cypescript accepts a program it should reject.

---

## Status

| Item | Scope | Status |
|---|---|---|
| 7.1 | Compound assignment evaluates its target once | ✅ **DONE** |
| 7.2 | Frictionless C/C++ interop (`link source`) | ✅ **DONE** |
| 7.3 | A real type checker | ⬜ **NEXT** |
| 7.4 | `class extends` | ⬜ |
| 7.5 | Property access on call results (`f().prop`) | ⬜ |
| 7.6 | Enums, 2D arrays, typed buffers | ⬜ |
| 7.7 | Windows validation | ⬜ |

---

## 7.1 Compound assignment ✅ DONE

`a[f()] += v` was desugared by re-parsing the left-hand side into
`a[f()] = a[f()] + v`. That called `f()` **twice**, and because the two calls returned
different values it **read one slot and wrote another**:

```ts
let a: i32[] = [10, 20, 30];
let i: i32 = 0;
function next(): i32 { i += 1; return i; }

a[next()] += 100;
// TypeScript:  a = [10, 120, 30],  i = 1
// Cypescript:  a = [10, 130, 30],  i = 2      ← read a[2], wrote a[1]
```

A silent wrong answer to an ordinary line of code — the worst failure mode there is.

**Fixed** by carrying the operator on the assignment node instead of desugaring.
`ArrayAssignmentStatementNode` and `ObjectPropertyAssignmentNode` gained `isCompound` and
`compoundOp`; codegen evaluates the array and index (or the object expression) once, then
does a load-modify-store through the values it already has. A plain variable target has
nothing to evaluate twice, so it keeps the simpler desugared path.

Covered by `tests/test_compound_side_effects.csc`: side-effecting indices, every operator,
`++`/`--`, `f64` and string elements, and a side-effecting object expression.

## 7.2 C/C++ interop without a build step ✅ DONE

Using your own native code used to mean a separate shell script
(`compile-with-custom-cpp.sh file.csc out src/lib.cpp`) — a whole build path outside
`cscript`. Now it is one line in the source:

```ts
link source "native/stats.c";
declare function stats_sum(values: ptr, count: i32): i32;
```

```bash
cscript -r myprogram.csc
```

The path resolves relative to the `.csc` file, so a project keeps its native code beside
its Cypescript. **Each source is compiled with the driver its language needs** — `.c` gets
`clang -std=c11`, `.cpp`/`.cc`/`.cxx` get `clang++ -std=c++17`, and `.m`/`.mm` are handled
too. That detail matters: a single mixed invocation compiled `.c` files as C++, where
ordinary C like `char *p = malloc(n);` is a hard error, and `-std=c++17` then conflicted
with the C input. Objects are built to a temp directory and cleaned up after linking.

See `example/21_c_interop.csc` and `tests/test_c_interop.csc` (which links a `.c` and a
`.cpp` into the same program).

---

## 7.3 A real type checker ⬜ NEXT

**The single highest-value remaining piece.** Cypescript has TypeScript's syntax and
almost none of its checking. The semantic pass covers scoping, arity and `const`; nothing
verifies that a value matches its annotation:

```ts
let s: string = "hi";
let bad: i32 = s;      // compiles clean, prints -174948096
```

That is a pointer truncated to `i32`. No error, no warning — garbage at runtime. For a
language whose README says "statically-typed", this is the gap that matters most.

Two related symptoms:

- **Type errors surface at codegen, without positions.** `println(obj.missing)` gives
  *"Codegen Error: Object properties not found for variable 'obj'"* — right diagnosis,
  wrong message, and no line or column. Parser errors carry positions; codegen errors
  do not.
- **Codegen throws where a checker should have spoken.** Most `Codegen Error` messages
  are really type errors that escaped.

### Shape of the work

1. **A type representation.** Types are currently canonical strings (`"i32"`, `"Rock[]"`,
   `"closure(i32)=>i32"`). A small `Type` struct with a kind, name and element type would
   be clearer, but strings are workable and far less invasive — decide this first, because
   everything else builds on it.
2. **A checking pass between Semantic and CodeGen**, reusing the position plumbing
   `SemanticAnalyzer` already has. It annotates each expression with its type and reports
   mismatches at assignment, `return`, call arguments and property access.
3. **Delete the guesswork in CodeGen.** Much of `visit(VariableDeclarationNode)` is
   inference-by-inspection (`initVal->getType()->isPointerTy() ? "string" : ...`). With a
   checked AST, codegen reads types instead of re-deriving them.

### Why it is worth the disruption

It closes blocker #6 (positions on all errors), removes most `Codegen Error` crashes, and
is the prerequisite for both `extends` and union types — neither can be checked without
knowing type relationships.

## 7.4 `class extends` ⬜

```ts
class Dog extends Animal { speak(): void { println("woof"); } }
// Parse Error: Expected '{' after class name. Found EXTENDS
```

Not parsed at all. Needs a layout that prefixes the parent's fields, method lookup that
walks the chain, and assignability of a subclass to its parent — which is why it should
follow the type checker rather than precede it.

## 7.5 Property access on call results ⬜

```ts
findBox(box).hits += 5;
// Parse Error: Expected ';' after statement. Found DOT
```

A call result cannot be indexed or have a property read. `parseVariableExpression` handles
calls but does not chain into `parseArrayOrObjectAccess`. Small, self-contained, and it
removes a surprising papercut — it was hit while writing the compound-assignment tests.

## 7.6 Enums, 2D arrays, typed buffers ⬜

Carried over from the game roadmap; none are blocking. Enums would replace magic integers
for key codes and entity kinds. Typed fixed-size buffers with inlined GEP load/store
(rather than a call per element) are the tilemap and pixel-work performance path.

## 7.7 Windows validation ⬜

The portability work is in — executable-path lookup, `cmd.exe`-safe quoting,
platform-qualified link directives, per-platform `setjmp`/`longjmp` — but it has never
gone green, so the CI job is `workflow_dispatch`-only. The three things most likely to
break it are written on the job in `.github/workflows/ci.yml`.

---

## Suggested order

**7.3 first.** 7.4 depends on it, 7.5 is small enough to slot in anywhere, and 7.6 is
optional polish. Nothing else changes what the language will *accept* — and accepting
`let bad: i32 = someString` is the most surprising thing Cypescript currently does.
