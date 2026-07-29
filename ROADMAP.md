# Cypescript Roadmap

One place to see what is done, what is next, and what is deliberately not being done.

> Consolidated 2026-07-25 from `GAME_ROADMAP.md`, `LANGUAGE_ROADMAP.md`,
> `SHIPPING_BLOCKERS.md`, `progress.md`, `NATIVE_OBJECTS_ROADMAP.md` and
> `OPTIMIZATION_ROADMAP.md`. Their full text is in git history.

**Current state:** 68/68 language tests, 14/14 game tests, 23 examples, 39 of 46
README snippets compiled in CI (the other 7 are illustrative), benchmarks at Rust
parity (0.051s primes, 0.025s fib(35)). CI green on macOS and Linux.

Two companion documents: [STEERING.md](STEERING.md) for the rules that shouldn't
change and where a new feature lands, and [HANDOVER.md](HANDOVER.md) for what is
verified on which platform and how to test Linux and Windows.

---

## Next

| # | Item | Why it matters | Size |
|---|---|---|---|
| 1 | **Windows validation** | The only platform never verified. CI job exists, `workflow_dispatch`-only. Needs a Windows machine. | Unknown |
| 2 | **By-reference closure captures** | Captures are by-value snapshots; mutating a captured primitive doesn't propagate | Medium |
| 3 | **Mixed-representation unions** | `string \| i32` needs a tagged value and `typeof` narrowing | Large |

Two items that briefly sat here came out of sizing `benchmark_bfs` up to where its
numbers meant something, and both are now **fixed**: indexing a `T[]` inside a
generic function read the wrong storage vector, and `shift()` erased from the front
of a vector. Neither was visible while that benchmark ran in under a millisecond on
a six-node graph. `tests/test_generic_arrays.csc` and `tests/test_array_shift.csc`
guard them.

### Windows — what is known

The portability work is in: `GetModuleFileNameW` for executable-relative paths,
`cmd.exe`-safe quoting, platform-qualified `link` directives, and a `setjmp`/`longjmp`
pairing centralised per platform. It has never gone green, so the CI job runs on demand
only rather than staying permanently red. In likely order of breakage:

1. Chocolatey's `llvm` package may not ship `LLVMConfig.cmake`, so CMake cannot find LLVM.
2. `clang++` on Windows targets the MSVC ABI, where `-l`/`-L` behave differently from the
   GNU driver the link line assumes.
3. Windows has no `_longjmp`, and SEH unwinding differs from the POSIX model the exception
   runtime is built on.

---

## Done

### The language

| Area | State |
|---|---|
| Types | `i32`, `i64`, `i8`/`u8`, `f32`, `f64`, `boolean`, `string`, `ptr`, `void`, enums, arrays, nested arrays |
| Control flow | `if`/`else if`, `switch` with fallthrough, `while`, `for`, `do-while`, `for...of`, `break`/`continue` |
| Functions | Parameters, returns, generics, arrow functions, closures, function-type parameters |
| Objects | Literals, interfaces with structural checking, methods with `this`, destructuring, JSON |
| Classes | Fields, defaults, constructors, methods, **inheritance, virtual dispatch, `super`** |
| Operators | Arithmetic, comparison, logical with short-circuit, **bitwise**, compound assignment, `++`/`--` as expressions |
| Errors | `try`/`catch`/`finally`, `throw` |
| Modules | `import`/`export` by path, plus bundled modules by bare name |
| FFI | `declare function`, `link`, `link source`, `link include`, platform qualifiers |
| Buffers | `Buffer<T>` — fixed-size, indexed inline with no runtime call |
| Unions | `T \| null` and other same-representation unions; `implements` on a class |
| Checking | Scoping, arity, `const`, **types** at declarations/assignments/returns/arguments, and **property names** against classes, interfaces and object literals |

### Games (Phases 1–6, all complete)

A native arcade game is writable in idiomatic Cypescript:
`example/game/02_asteroids.csc` runs at 60 fps with flat memory, and
`cscript --bundle` turns it into a double-clickable `.app`.

- **The compiler knows nothing about graphics.** Everything game-specific is a C shim
  (`runtime/game/cyps_game.c`) plus a module of `declare`s (`lib/game.csc`). Adding an API
  means one line in each — never a compiler change. This is a hard rule.
- **raylib is vendored**: CMake builds it from source and links it statically, so an
  install is self-contained. `-DCYPESCRIPT_VENDOR_RAYLIB=OFF` uses a system one.
- **Headless mode** (`CYPS_HEADLESS=1`, `CYPS_FRAMES=n`) runs the whole loop with no
  window, which is how games are asserted in CI.
- **Memory is flat**: both games hold steady RSS from 2,000 to 300,000 frames — 83 minutes
  of play at 60 fps. Two mechanisms, and a game needs both: frame-scoped strings
  (`enableFrameStrings()`, opt-in) and entity pooling.

### Phase 7 — safety and ergonomics (complete except Windows)

| # | Item | Outcome |
|---|---|---|
| 7.1 | Compound assignment | Evaluates its target once. `a[f()] += v` used to call `f()` twice and read one slot while writing another |
| 7.2 | C/C++ interop | `link source "x.c";` — no library, header, makefile or wrapper script. Each file gets its own language's driver |
| 7.3 | Type checker | Declarations, assignments, returns, call arguments, class members, class assignability |
| 7.4 | `class extends` | Inheritance, overriding, virtual dispatch, `super` |
| 7.5 | Postfix chaining | `.prop`, `[i]`, `.method()` after any expression |
| 7.6 | Enums, nested arrays, typed buffers | Enums fold to integers at parse time; `Buffer<T>` indexes inline, 37x faster than an array |
| 7.7 | Windows | **Not done** — see Next |

**Phase 7 is complete except Windows.**

### Shipping

LICENSE, relocatable compiler, Homebrew tap, `.deb` and Arch packaging, CI on macOS and
Linux, `--version`, line/column errors, output-assertion tests, negative tests, VSCode
extension 1.2.0.

---

## Deliberate non-goals

These are decisions, not omissions.

- **No garbage collector.** Heap objects live until exit unless pooled. A GC would forfeit
  the no-pause performance story that the benchmarks exist to protect.
- **No automatic free on removal.** Taking an entity out of an array does not free it —
  other references may still point at it. Pool and reuse instead.
- **Interfaces stay structural.** Conformance is checked by shape. `implements` declares an
  intent that is *verified*, but a class may still satisfy an interface without naming one.
- **The compiler never learns about graphics.** See the games section.

---

## Known limitations

| Limitation | Detail |
|---|---|
| No narrowing check | `let n: i32 = someF64;` is allowed because codegen coerces it. TypeScript would reject it; tightening needs a survey of existing code |
| Unions need a shared representation | `Shape \| null` and `i32 \| f64` work; `string \| i32` is rejected with an explanation. A mixed union needs a tagged value and `typeof` narrowing |
| `null` fits any handle | There is no strict-null mode, so `let c: C = null;` is legal and `T \| null` is about intent rather than enforcement |
| Buffers are not bounds-checked | `T[]` returns 0 out of range; a `Buffer<T>` reads or writes past its allocation. Part of why it is faster |
| Fixed try-nesting depth | The exception runtime allows 64 nested `try` blocks |
| Generic functions over scalars | `id<i32>(5)` fails LLVM verification; generics work with pointer-shaped types |
| `Map`/`Set` values | Only pointer-shaped values; `Map<string, i32>` fails |
| Enums declare-before-use | Members fold at parse time, like a C enum |

## TypeScript compatibility

Not supported: npm imports, by-reference closure captures,
mixed-representation unions (`string | number`), strict null checks.

Supported with a caveat: `number` is `f64` (use `i32` for integer math);
objects are structs, so property access is one instruction rather than a hash lookup.

---

## Performance

Best-of-3 on an Apple M3, all at `-O2`:

| Benchmark | Cypescript | Rust | Node (TS) | Python |
|---|---|---|---|---|
| Primes < 1M | **0.051s** | 0.051s | 0.155s | 1.796s |
| `fib(35)` | **0.025s** | 0.026s | 0.133s | 0.636s |

Against Node on the same machine, across the wider suite: Simple Loop 9.4x, Matrix
Mult 16.7x, Prime Sieve 7.3x, Fibonacci 3.0x, BFS 2.7x. Those ratios are
machine-dependent — see the README, where the same suite on x86-64 puts Matrix Mult
at 5.5x — so treat the ordering as the claim rather than the multiples.

Confirmed on a second platform: on Arch x86-64, primes is 0.088s against Rust's
0.090s and `fib(35)` is 0.023s against 0.023s. Different machine and architecture,
so the absolute seconds are not comparable with the table above — the parity with
Rust is what reproduces, and it does.

### `Buffer<T>` vs `T[]`

1.6 billion element updates, identical output from both, best of 3:

| | Time |
|---|---|
| `Buffer<i32>` | **0.07s** |
| `i32[]` | 2.60s |

**37x.** An array element costs a call into the runtime; a buffer element is an address
and a load. The bigger half of the win is second-order — LLVM can vectorise a loop over
plain memory, and cannot see through an opaque call at all. Use `T[]` when you want a
growable list, `Buffer<T>` for a fixed-size block you index in a hot loop.

### Data structures, not just arithmetic

The benchmarks above are compute. `benchmarks/benchmark_bfs.csc` is the counterweight —
a BFS over a 5,000-node graph exercising `Map`, `Set` and an array queue, traversed 40
times, with both languages printing a checksum so they can be shown to do identical
work. It is the benchmark that found `shift()` erasing from the front of a vector:

| | Cypescript | Node |
|---|---|---|
| Before the head offset | 0.79s | 0.08s |
| After | **0.059s** | 0.073s |

Keep something like it in the suite. Every other benchmark here is a loop over numbers,
and a language can look excellent at those while a data-structure path is quadratic.

`benchmarks/cross/run_cross_benchmarks.sh 3` — always best-of-3, a single run is too noisy
to conclude from. **This is a regression gate**: two changes that could have cost it were
shaped to avoid it. Module-level globals are promoted only when a function actually
references them, so top-level hot loops keep their allocas; and only hierarchies that
actually override a method get vtables, so a class that is never subclassed keeps its
direct call.
