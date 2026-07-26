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
| 7.3 | A real type checker | 🔶 **Core landed** — see below |
| 7.4 | `class extends` | 🔶 **Inheritance landed; dispatch is static** |
| 7.5 | Property access on call results (`f().prop`) | ✅ **DONE** |
| 7.6 | Enums, 2D arrays, typed buffers | ⬜ **NEXT** |
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

## 7.3 A real type checker 🔶 CORE LANDED

The headline bug is fixed:

```ts
let s: string = "hi";
let bad: i32 = s;
// ✗ Semantic Error: Type mismatch in declaration of 'bad':
//   expected 'i32', got 'string' at line 2, column 16
```

It used to compile clean and print `-174948096` — a pointer truncated to `i32`.

### What it checks

Types are inferred for literals, annotated variables, class instances, calls with declared
return types, arithmetic and comparison results, array literals and indexing, and class
fields. Mismatches are reported, with positions, at four sites:

| Site | Example |
|---|---|
| Declaration | `let bad: i32 = someString;` |
| Assignment | `count = "nope";` |
| Return | `function n(): i32 { return "three"; }` |
| Call argument | `double("not a number")` |

### The design decision that made it safe

**The checker reports an error only when both sides are confidently known and definitely
incompatible.** An unknown type — the empty string — silences the check rather than
failing it.

That is not timidity; it is what makes the checker adoptable. Cypescript has many places
where a type genuinely is not known at this stage: generic parameters, `json` values,
`Map`/`Set` contents, closures, destructured bindings, method call results. A stricter
checker would reject programs that compile and run correctly today.

Types are grouped into families — numeric, text, handle, void — and only *crossing*
families is an error. Within a family codegen already coerces: `i32`↔`f64`, `boolean` as
`i32`, and `string`/`ptr` both being `i8*`. So `let f: f64 = 5;` stays legal while
`let n: i32 = "five";` does not.

Validated in both directions: 17 legitimate patterns (widening, concatenation, template
literals, `null` into handles, `json`, array-method results, closure parameters,
destructuring, `for...of`, catch variables, FFI handles) are all still accepted, and the
whole suite — 32 programs, 21 examples, both games — passes unchanged.

### Negative tests

The suite had **no negative tests at all**: nothing verified that errors were reported, so
the entire semantic pass could have stopped working with every test still green.
`tests/negative/` now holds programs that must be rejected, each with the message it must
be rejected with, and `run_tests.sh` checks both. Confirmed to bite by making a negative
test valid and watching it fail.

### Still to do

- **Property access on non-class objects.** `obj.missing` on an object literal or interface
  still reaches codegen (*"Object properties not found for variable 'obj'"*, no position).
  Class fields are checked; object literals and interfaces need their shapes tracked here.
- **Class-to-class assignability.** All handles are currently interchangeable, so
  `let r: Rock = someBullet;` is accepted. Needs the class hierarchy — which is 7.4's work
  anyway.
- **Narrowing.** `let n: i32 = someF64;` is allowed because codegen coerces it. TypeScript
  would reject it; tightening this needs a survey of existing code first.
- **Method call results** are unknown types, so nothing downstream of them is checked.

## 7.4 `class extends` 🔶 INHERITANCE LANDED

```ts
class Animal {
    name: string = "animal";
    legs: i32 = 4;
    describe(): void { println(`${this.name} has ${this.legs} legs`); }
    speak(): void { println("..."); }
}

class Dog extends Animal {
    speak(): void { println("woof"); }   // overrides
}

let d: Dog = new Dog();
d.describe();     // animal has 4 legs   — inherited method, inherited fields
d.speak();        // woof                — overridden
```

**How it works.** A subclass lays its ancestors' members out *first*, then its own, so a
subclass and its parent share a struct prefix and identical offsets for the inherited
fields. That means `new`, property access and method dispatch needed no changes at all —
after flattening, a subclass is just a class with more members.

The inherited members are non-owning pointers into the ancestors' own templates, resolved
once in codegen's pass 0 (`resolveClassInheritance`) before layouts are built. Chains are
arbitrarily deep, redeclaring a member overrides it, and a subclass with no constructor
inherits its parent's. Unknown parents and inheritance cycles are rejected.

The semantic pass folds each ancestor's fields into its subclasses, so a field's declared
type is checked through the whole chain.

### The limitation: dispatch is static, not virtual

**There are no vtables.** The method chosen depends on the type the compiler knows at the
call site, not on the object's runtime class:

```ts
class Base    { tag(): void { println("base"); } }
class Derived extends Base { tag(): void { println("derived"); } }

let d: Derived = new Derived();
d.tag();                                  // derived  ✓

let viaSlot: Base = new Derived();
viaSlot.tag();                            // derived  — the declaration tracks the
                                          //   initializer's concrete class

let arr: Base[] = [];  arr.push(new Derived());
let fromArray: Base = arr[0];
fromArray.tag();                          // base     ✗ not virtual

function callIt(b: Base): void { b.tag(); }
callIt(new Derived());                    // base     ✗ not virtual
```

So `extends` buys **code reuse and overriding**, not polymorphism. It is genuinely useful
when the concrete type is visible, and it is exactly wrong if you expect a `Shape[]` to
dispatch to each element's own `area()`. `tests/test_inheritance.csc` pins this behaviour
down deliberately so it cannot drift unnoticed.

Real virtual dispatch needs a vtable pointer in the struct layout, built per class and
stored at construction, with method calls going indirect when the static type has
subclasses. That is a substantial change and it would put an indirect call in a path the
benchmarks measure, so it is its own piece of work rather than a follow-on tweak.

### Also still missing

`super` — a subclass constructor cannot call its parent's. It can assign the inherited
fields directly (`this.name = ...`), since they are in its layout, which covers most of
what `super()` would do.

## 7.5 Property access on call results ✅ DONE

```ts
println(make(41).value);      // was: Parse Error: Expected ';'. Found DOT
println(new Box(7).value);
pick(c).hits += 5;
```

The cause was narrower than "call results are special": **only the plain-variable path
chained into `parseArrayOrObjectAccess`**. Every other producer — calls, `new`, array and
object literals, parenthesised expressions — returned straight out and stopped dead at the
following dot.

So the fix is one chaining loop at the end of `parsePrimaryExpression` rather than a patch
at each of the five call sites. It interleaves with the non-null assertion, so
`map.get(k)!.length` parses as well as `queue.shift()!`.

Nothing was needed in codegen: `visit(ObjectAccessNode)` already falls back to visiting an
arbitrary base expression, and `getExpressionObjectKey` already resolves a call's declared
return type — both from Phase 3.

`tests/test_postfix_chaining.csc` covers calls, `new`, literals, parenthesised expressions,
`!` interleaving, and — closing a loop from 7.1 — `pick(c).hits += 5`, which needs *both*
this and the evaluate-the-target-once fix to be right. It reports one call, not two.

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

**7.4 next.** The type checker's core is in, and the two things it cannot yet do —
class-to-class assignability, and knowing an object literal's shape — both want the type
relationships that `extends` forces you to build anyway. 7.5 is small and self-contained
enough to slot in whenever it gets annoying; 7.6 is optional polish; 7.7 needs hardware
nobody in this loop has.
