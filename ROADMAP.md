# Cypescript Roadmap

One place to see what is done, what is next, and what is deliberately not being done.

> Consolidated 2026-07-25 from `GAME_ROADMAP.md`, `LANGUAGE_ROADMAP.md`,
> `SHIPPING_BLOCKERS.md`, `progress.md`, `NATIVE_OBJECTS_ROADMAP.md` and
> `OPTIMIZATION_ROADMAP.md`. Their full text is in git history.

**Current state:** 53/53 language tests, 14/14 game tests, 23 examples, benchmarks at
Rust parity (0.051s primes, 0.025s fib(35)). CI green on macOS and Linux.

---

## Next

| # | Item | Why it matters | Size |
|---|---|---|---|
| 1 | **Windows validation** | The only platform never verified. CI job exists, `workflow_dispatch`-only. Needs a Windows machine. | Unknown |
| 2 | **Union types** (`string \| number`) | The largest remaining TypeScript gap | Large |
| 3 | **`implements` on a class** | Interfaces are structural only; a class cannot declare conformance | Medium |
| 4 | **`x++` as an expression** (`arr[i++]`) | Statement-level only today | Small |
| 5 | **By-reference closure captures** | Captures are by-value snapshots; mutating a captured primitive doesn't propagate | Medium |

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
| Operators | Arithmetic, comparison, logical with short-circuit, **bitwise**, compound assignment |
| Errors | `try`/`catch`/`finally`, `throw` |
| Modules | `import`/`export` by path, plus bundled modules by bare name |
| FFI | `declare function`, `link`, `link source`, `link include`, platform qualifiers |
| Checking | Scoping, arity, `const`, and **types** at declarations, assignments, returns, arguments |

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
| 7.6 | Enums, nested arrays | Enum members fold to integers at parse time, so they cost nothing at runtime |
| 7.7 | Windows | **Not done** — see Next |

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
- **Interfaces stay structural.** Conformance is checked by shape, not declared. A class may
  satisfy an interface without naming it.
- **The compiler never learns about graphics.** See the games section.

---

## Known limitations

| Limitation | Detail |
|---|---|
| Object literal shapes unchecked | `obj.missing` on a literal reaches codegen without a position. Class members *are* checked |
| No narrowing check | `let n: i32 = someF64;` is allowed because codegen coerces it |
| Typed fixed-size buffers | Array element access is an out-of-line call. Inlined GEP load/store is the tilemap and pixel-work path — an optimisation, deliberately not rushed in beside correctness work |
| Fixed try-nesting depth | The exception runtime allows 64 nested `try` blocks |
| Generic functions over scalars | `id<i32>(5)` fails LLVM verification; generics work with pointer-shaped types |
| `Map`/`Set` values | Only pointer-shaped values; `Map<string, i32>` fails |
| Enums declare-before-use | Members fold at parse time, like a C enum |

## TypeScript compatibility

Not supported: union types, npm imports, `x++` as an expression, by-reference closure
captures, `implements` on a class.

Supported with a caveat: `number` is `f64` (use `i32` for integer math);
objects are structs, so property access is one instruction rather than a hash lookup.

---

## Performance

Best-of-3 on Apple Silicon, all at `-O2`:

| Benchmark | Cypescript | Rust | Node (TS) | Python |
|---|---|---|---|---|
| Primes < 1M | **0.051s** | 0.051s | 0.150s | 1.78s |
| `fib(35)` | **0.025s** | 0.025s | 0.126s | 0.634s |

`benchmarks/cross/run_cross_benchmarks.sh 3` — always best-of-3, a single run is too noisy
to conclude from. **This is a regression gate**: two changes that could have cost it were
shaped to avoid it. Module-level globals are promoted only when a function actually
references them, so top-level hot loops keep their allocas; and only hierarchies that
actually override a method get vtables, so a class that is never subclassed keeps its
direct call.
