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
| 7.3 | A real type checker | ✅ **DONE** |
| 7.4 | `class extends` — inheritance, virtual dispatch, `super` | ✅ **DONE** |
| 7.5 | Property access on call results (`f().prop`) | ✅ **DONE** |
| 7.6 | Enums and nested arrays | ✅ **DONE** (typed buffers deferred) |
| 7.7 | Windows validation | ⬜ **Needs a Windows machine** |

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

## 7.3 A real type checker ✅ DONE

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

### Class assignability and method results

Once `extends` gave the checker a class hierarchy, two of its blind spots closed:

**Classes are no longer interchangeable.** A subclass fits its ancestor's slot; nothing
else does:

```ts
let ok:  Shape  = new Square();   // accepted — Square extends Shape
let bad: Square = new Shape();    // rejected — a parent is not a subclass
let no:  Rock   = new Bullet();   // rejected — unrelated classes
```

Interfaces stay permissive on purpose: conformance is checked structurally in codegen, and
a class may satisfy one without naming it.

**A class method's declared return type is known**, so the value flowing out of a call is
checked — including through inheritance:

```ts
class User { name(): string { return "x"; } }
class Admin extends User { }
let n: i32 = new Admin().name();   // rejected: expected 'i32', got 'string'
```

### Still to do

- **Object literal and interface shapes.** `obj.missing` on an object literal still reaches
  codegen (*"Object properties not found"*, no position). Class members are checked; object
  literals would need their shapes tracked here too.
- **Narrowing.** `let n: i32 = someF64;` is allowed because codegen coerces it. TypeScript
  would reject it; tightening this needs a survey of existing code first.

## 7.4 `class extends` ✅ DONE

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

### Virtual dispatch

Dispatch follows the object's **runtime** class, not the type at the call site:

```ts
class Shape  { area(): f64 { return 0.0; } }
class Circle extends Shape { area(): f64 { return 3.14159 * this.r * this.r; } }
class Square extends Shape { area(): f64 { return this.side * this.side; } }

let shapes: Shape[] = [];
shapes.push(new Circle(2.0));
shapes.push(new Square(3.0));

for (const s of shapes) { println(s.area()); }   // 12.5664, 9
```

It works through parent-typed array elements, `for...of` bindings, parameters and locals,
and through a method inherited from the parent that calls an overridden one
(`describe()` calling `this.area()` resolves per subclass).

**Only hierarchies that actually override something get vtables.** `computeVirtualDispatch`
walks each chain and marks it polymorphic only if some method name is declared in more than
one class in it. Everything else — including every class in the benchmarks — keeps a direct
call and generates exactly the code it did before:

```llvm
; class with no subclasses
%get_call = call i32 @opt_obj_4386780832_get(ptr %obj_ptr_load)

; polymorphic hierarchy
@B_vtable = internal constant [1 x ptr] [ptr @opt_obj_4326782944_get]
%vptr = load ptr, ptr %vptr_addr
%vfn  = load ptr, ptr %vslot
```

Mechanically: a polymorphic class carries a hidden `__vptr` as field 0, so parent and child
still share a struct prefix; every class in a hierarchy shares one slot assignment, so the
same method sits at the same index everywhere; the vtable is installed by `new` *before*
the constructor runs, so a constructor calling a virtual method dispatches correctly. The
constructor itself is never virtual. Vtables are emitted lazily and registered before their
method bodies are generated, so a method that constructs its own class does not recurse
forever. Benchmarks unchanged at 0.051s / 0.025s.

A bug this surfaced: an **empty class body produced a null instance**, because an object
template with no properties took the `{}` fast path. It was harmless only while nothing
dereferenced it — storing a vtable into it crashed immediately. A class template now always
gets a real allocation.

### `super`

```ts
class Dog extends Animal {
    constructor(name: string, tricks: i32) {
        super(name);            // the parent constructor
        this.tricks = tricks;
    }
    speak(): void {
        super.speak();          // the implementation this override replaced
        println(`...and knows ${this.tricks} tricks`);
    }
}
```

Both forms are **always direct calls** — dispatching them virtually would re-enter the
override that asked for them. `super` is contextual, so it remains usable as an ordinary
identifier anywhere it is not followed by `(` or `.`.

The parent's body is generated against the parent's layout, and `this` is passed through
unchanged. That is safe precisely because of the prefix property: a subclass's inherited
fields sit at the same offsets they do in the parent, so the parent's code reading them
through its own layout lands in the right place.

Chains work at every level — `A ← B ← C` with each constructor calling `super()` and each
`who()` calling `super.who()` produces `A>B>C` for both — and the outermost call is still
virtual, so a `C` reached through an `A`-typed slot runs `C`'s override first.

Misuse is caught by the semantic pass with a position: `super` outside a class method, and
`super` in a class that extends nothing.

### Still missing

Nothing structural. Interfaces are still checked structurally rather than nominally, and
a class cannot declare that it implements one.

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

## 7.6 Enums and nested arrays ✅ DONE

### Enums

```ts
enum Color  { Red, Green, Blue }                 // 0, 1, 2
enum Key    { Left = 263, Right = 262 }
enum Status { Ok = 0, Warn = 10, Error, Fatal }  // 10, 11, 12 — continues
enum Offset { Behind = -1, Here, Ahead }
```

**Members are folded to integers by the parser**, so `Color.Red` is literally `0` by the
time codegen sees it — an enum costs nothing at runtime and needed no codegen support
beyond treating the enum's name as a type meaning `i32`. They work in comparisons,
`switch`, arithmetic, arrays and parameters. `Color.Purple` is a parse error naming the
enum and the member.

Because folding happens during parsing, an enum must be declared before it is used —
the same rule a C enum has.

### Nested arrays

`i32[][]` is an array whose elements are array handles, so it shares the object-array
runtime that stores elements verbatim rather than copying them.

The subtler half was that an element's type was only resolved when the array was a plain
*variable*, so `grid[0][1]` fell back to `i32` and read a pointer array as integers.
`arrayTypeOfExpression` now resolves an array's static type through the whole access chain,
which fixed both the element type and — via the same generalisation in
`getExpressionObjectKey` — `cells[0][0].v` on a nested array of objects.

Works with `i32`, `f64`, `string` and class element types, through indexing, assignment,
compound assignment and `for...of`.

### Typed fixed-size buffers — deferred

Inlined GEP load/store instead of a call per element remains the tilemap and pixel-work
performance path. It is a genuine optimisation rather than a capability, nothing depends
on it, and it deserves to be measured rather than assumed — so it is not being rushed in
alongside the correctness work.

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
