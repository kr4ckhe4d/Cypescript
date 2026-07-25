# 🕹️ Game Roadmap — from v1.0 to a fully native arcade game

Goal: **write a complete arcade game in Cypescript** — window, sprites, keyboard/gamepad
input, sound, HUD, 60 fps, shipping as a single native binary — with the game written in
idiomatic Cypescript, not hacked in via compiler special-cases.

Started 2026-07-25 against `main` @ `06e8d87` (v1.0.0).

---

## Status

| Phase | Scope | Status |
|---|---|---|
| **1** | FFI (`declare function`) + linker control + raylib shim | ✅ **DONE** |
| **2** | First playable game (Breakout, struct-of-arrays) | ✅ **DONE** |
| **3** | Heap objects — escape, object arrays, despawning | ✅ **DONE** |
| 4 | Frame arena / memory hygiene | ⬜ **NEXT** |
| **5** | Numeric & ergonomic gaps | ✅ **DONE** |
| 6 | Packaging & shipping | 🔶 Vendoring + CI done; bundling left |

**Milestones reached:** M1 (a window opened from Cypescript), M2 (a playable
60 fps Breakout), and M3 (Asteroids with real entity objects — spawned by
functions, stored in `Rock[]`/`Bullet[]`, split on impact, removed when they die).

Test suites: `bash tests/run_tests.sh` → **27/27**, `bash tests/run_game_tests.sh` → **7/7**.
Benchmarks unchanged at Rust parity.

**raylib is vendored** — CMake builds it from source and links it statically, so a
Cypescript install is self-contained with no system package. **`lib/game.csc` ships with
the compiler** and is importable by bare name: `import { } from "game";`. The API reads
like TypeScript (`drawRect`, `isKeyDown`, `deltaTime`), not like C.

---

## 1. Where we started

### What already worked in our favour

Native codegen at ~Rust-parity scalar speed, `while`/`for`/`break`/`continue`, `f64` math
including `%`, dynamic `f64[]`/`i32[]`/`string[]` arrays, classes and methods, arrow
functions and closures, modules, interfaces, exceptions, and a relocatable compiler with a
precompiled runtime. The performance story was already good enough for 60 fps. **The gaps
were structural, not performance.**

### The five hard blockers (all verified by probing the real compiler)

**B1 — No FFI.** External functions were a hand-written `if/else` chain in
`CodeGen::getOrDeclareExternalFunction`. `sdl_create_window(...)` → *"Unsupported function
call"*. Every graphics call would have had to be hardcoded into the compiler.
→ ✅ **FIXED in Phase 1.**

**B2 — No control over linking.** `src/main.cpp` hardcoded the entire clang++ command. No
`-L`, no `-l`, no `-framework`. Nothing external could ever be linked.
→ ✅ **FIXED in Phase 1.**

**B3 — Objects cannot escape, and cannot live in arrays.** The deepest one. Object identity
is a *compile-time string key bound to a variable name* (`variableToObjectKey`,
`getExpressionObjectKey`), and instances are `alloca`'d:

```ts
function make(v: f64): V { return new V(v); }
let q = make(3.5);   ✗ Cannot access property 'x' on variable 'q' of type 'string'

let a: E[] = []; a.push(new E(7.0));
let first = a[0];    ✗ Cannot access property 'x' on variable 'first' of type 'i32'
```

→ ⬜ **Still open. This is Phase 3 and it is the main remaining work.**

**B4 — Type set too narrow for a C ABI.** Only `i32`, `f64`, `boolean`, `i8*`, `void`.
→ ✅ **FIXED in Phase 1** (`f32`, `i64`, `i8`/`u8`, and an opaque `ptr`).

**B5 — Memory is leak-until-exit.** The stdlib `new`s strings and never frees them.
→ ⬜ **Still open (Phase 4). Now measured** — see the numbers below.

---

## 2. Strategy

The organising principle: **`cscript` must never know that graphics exist.** Everything
game-specific lives in a C shim plus a Cypescript module of `declare`s. Adding
`DrawTexturePro` requires zero compiler changes.

**Backend: raylib, behind our own flat C shim** (`runtime/game/cyps_game.c`). raylib gives
window + sprites + input + audio + text in one dependency. Its catch is that it passes
structs by value (`Vector2`, `Color`, `Rectangle`) and Cypescript has no struct ABI — so
raylib is never exposed directly. The shim flattens every call to scalars and opaque
pointers, and packs colors into a single `i32`. That boundary also means SDL3 could be
swapped in without touching a line of game code.

---

## 3. Phase 1 — FFI + linker control ✅ DONE

### What shipped

**`declare function` — general FFI.** Any C symbol can now be bound from Cypescript:

```ts
declare function atan2(y: f64, x: f64): f64;
declare function malloc(size: i64): ptr;
declare function cyps_rect(x: f64, y: f64, w: f64, h: f64, color: i32): void;
```

- `ExternDeclarationNode` in `src/AST.h`; parsed by `Parser::parseExternDeclaration`.
- Registered in CodeGen's **pass 0** (alongside interfaces and classes), so declarations are
  visible above their own position in the file.
- `getOrDeclareExternalFunction` consults the extern table **first**, building the
  `FunctionType` from the declared types; the old hardcoded stdlib chain remains as a
  fallback, so nothing regressed.
- The semantic pass registers externs too, so they get arity checking like any function.
- Variable-declaration inference reads the declared return type — without this an opaque
  `ptr` handle would be silently recorded as a `string`.

**`declare` and `link` are contextual keywords.** They are only keywords in the exact
shapes `declare function …` and `link "…";`, so any existing program using either as an
identifier keeps working. No new reserved words, no lexer changes at all.

**C-ABI types.** `getLLVMType` gained `f32`, `i64`, `i8`/`u8`, and an opaque `ptr`.
`coerceValue` was generalised from four hardcoded cases to full int↔int (any width),
int↔float, and f32↔f64 conversion.

**Link control.** `-l`, `-L`, `--framework`, `--link-flag` on the command line, plus
source-level directives so a program is self-contained:

```ts
link path "/opt/homebrew/lib";
link "raylib";
link framework "Cocoa";
```

Every flag is shell-quoted before it reaches `std::system`. `cscript` also adds its own
`lib/` directory automatically, so `link "cypescript_game";` resolves wherever the compiler
is installed.

**The game runtime shim** (`runtime/game/cyps_game.c` → `libcypescript_game.a`): window and
frame lifecycle, rectangles/circles/lines/text, textures with sprite-sheet blitting, keyboard/
mouse/gamepad input, and audio. Two touches worth calling out:

- **`cyps_tone(freq, ms, volume)`** synthesises a square-wave blip at runtime, so a game can
  have sound effects with **no asset files at all** — an arcade game fits in one `.csc`.
- **Headless mode** (`CYPS_HEADLESS=1`, `CYPS_FRAMES=n`) runs the entire game loop with no
  window and no drawing, and `cyps_headless()` lets a game detect it. This is what makes a
  game testable in CI, and it landed in Phase 1 rather than Phase 6 because it was nearly free.

**The build is optional-by-design.** CMake probes for raylib; if it is missing, the game
runtime is skipped, the compiler still builds, and all 24 language tests still pass. The
game tests skip themselves with a clear message.

### Verification

```
$ cscript -r tests/test_ffi.csc          # binds atan2/strlen/abs/fmax/malloc/free from libc+libm
0.785398 / 10 / 42 / 7.25 / 3.5 / 5 / ptr handle round-tripped through C
```

**M1:** `example/game/00_window.csc` opens a real window. Proven programmatically rather
than by screenshot — after `InitWindow` the GL context reports its true size, and the
frame pacing is real:

```
window size reported by the GL context: 800x450
90 frames in ~1.5s wall clock  → SetTargetFPS(60) is genuinely limiting
```

---

## 4. Phase 2 — Playable Breakout ✅ DONE

`example/game/01_breakout.csc` — a complete game in ~260 lines of Cypescript: paddle,
ball with angle-of-deflection off the paddle, a 10×6 brick field with per-row colors and
scoring, 3 lives, win/lose states, restart, synthesised sound effects, and a live HUD.

Entities are held in **parallel arrays (struct-of-arrays)** rather than an array of
objects, because of B3. That is a deliberate, documented choice — the file is written
against what the language can do *today*, and it is exactly the code that Phase 3 will let
us rewrite idiomatically.

**Self-play mode.** With no window there is no keyboard, so under `CYPS_HEADLESS=1` the
game plays itself: the paddle chases the ball and auto-launches. This turns a headless run
into a genuine test of physics, collision and scoring rather than a smoke test:

```
$ CYPS_HEADLESS=1 CYPS_FRAMES=1200 cscript -r example/game/01_breakout.csc
Breakout ended — score 290, 1200 frames, 51 bricks left
```

`tests/run_game_tests.sh` asserts that bricks are actually destroyed and points are
actually scored — a regression in collision or scoring fails the build.

### What building a real game taught us

**The leak (B5) is real and now measured.** RSS of the self-playing binary:

| Frames | RSS |
|---|---|
| 2,000 | 8.5 MB |
| 20,000 | 10.2 MB |
| 100,000 | 17.5 MB |

≈ **92 bytes per frame**, from the HUD's template literals (`` `SCORE ${score}` ``). That is
~5.5 MB/hour of play — survivable for an arcade session, unacceptable long-term, and exactly
what the Phase 4 frame arena fixes. Without writing a game this would have stayed theoretical.

**New gaps found while writing the game** (none blocked M2, all now on the list):

- **Hex literals are broken** — `0xFF` evaluates to `0`. Sidestepped by composing colors
  through `cyps_rgb(r, g, b)`, which was the right API anyway, but the lexer bug is real.
- **`null` is not an expression** — `let p: ptr = null;` is a parse error. Matters for
  optional handles.
- **Functions cannot see module-level constants** — `const SPEED = 4.0;` used inside a
  `function` fails semantic analysis with *"Use of undefined variable"*. This is why
  Breakout is written as one main flow with helpers taking explicit parameters. It is the
  single biggest obstacle to organising a larger game.
- **No bitwise operators** — `5 & 3` and `1 << 4` are parse errors. Interestingly the
  tokens (`TOK_AMPERSAND`, `TOK_PIPE`, `TOK_CARET`, `TOK_TILDE`) already exist and the
  lexer emits them; only parser precedence levels and codegen are missing, so this is
  cheaper than it looks.

**Math gaps closed along the way** (Phase 5 item 1): `Math.min`, `max`, `round`, `ceil`,
`atan2`, `random`, plus `Math.PI` / `Math.E` as constants. `Math.max(0.0, Math.min(x, hi))`
is the clamp idiom the game uses everywhere.

---

## 5. Phase 3 — Heap objects ✅ DONE

**The bulk of the work, and the real prize.** This is now the target code, and it
runs — see `example/game/02_asteroids.csc`:

```ts
class Asteroid { x: f64; y: f64; vx: f64; vy: f64; size: i32; alive: boolean; }

let rocks: Asteroid[] = [];
rocks.push(spawnAsteroid(3));

for (const r of rocks) {
    r.x += r.vx * dt;
    if (hit(ship, r)) { r.alive = false; score += 100; }
}
```

**3a. Object identity now travels with the type, not with a variable name.** ✅
`getExpressionObjectKey` resolves from an expression's *static type*, so it works for a
function's declared return type (`: Rock`), an array's element type (`Rock[]` ⇒
`rocks[i].x`), a `new` expression, and a variable whose recorded type is a class. As
predicted, one change fixed both failing probes.

A prerequisite fell out of it: **class layouts are now computed in pass 0** from declared
field types (`registerClassLayout`), before any code is generated. Previously a layout only
existed once the class had been instantiated somewhere — which, after the Phase 5 codegen
reordering, was *after* main had already been emitted. Field declarations carry their
declared type on the AST node for this.

**3b. Escaping objects are heap-allocated.** ✅ `new ClassName(...)` mallocs the struct and
runs the constructor. Class instances were already opaque `i8*` at the type level, so
nothing about the ABI changed. **Object literals keep their `alloca`**, which is what the
benchmarks measure — still 0.051s/0.026s, at Rust parity.

**3c. Object arrays.** ✅ The runtime's `DynamicArray` already had an unused `object_data`
vector; it now has `array_push_object / get_object / set_object / pop_object /
shift_object`, plus **`array_remove_at`** for despawning (`entities.removeAt(i)`).
Object pointers are stored and returned verbatim — routing them through the string vector
would have corrupted them, because `array_get_string` copies its element through a
`std::string`. Codegen picks the object path from the *declared* element type, since
object pointers and strings are both `i8*`. Array literals, indexing, `push`, `pop`,
`shift`, element assignment and `for...of` all handle object elements.

**3d. Memory: pooling, not GC.** Still the plan. Removing an entity from an array drops the
pointer without freeing it — see the measurement below.

**M3 reached** — `example/game/02_asteroids.csc`: a ship, `Rock[]` and `Bullet[]`, rocks
that split into two smaller rocks on impact, bullets that expire, waves that respawn. A
40,000-frame headless run reaches wave 12 with a score of 24,220.

### What Phase 3 costs

Heap objects introduce a leak of their own: `removeAt` drops the pointer, and nothing frees
it. Measured on Asteroids, RSS grows ~135 bytes/frame (8.9 MB → 14.2 MB over 40k frames).
Most of that is still the four HUD template literals per frame — Breakout measured ~92
bytes/frame with three — so entity churn is the smaller share, but it is real and it is new.

Both halves are Phase 4's problem: the frame arena for strings, and either pooling (the
documented idiom) or freeing on removal for objects.

---

## 6. Phase 4 — Memory & frame hygiene ⬜

- **Frame arena for strings.** `cyps_frame_begin` resets an arena; `string_concat` and the
  `cyps_*_to_string` helpers allocate from it. Strings become frame-lifetime, with
  `string_persist()` to promote one. ~50 lines in the stdlib, and it is the larger share
  of both games' leak.
  - Risk: programs that hold strings across frames. **Make it opt-in** (`--frame-strings`,
    or active only when the game runtime is linked).
- **Objects removed from an array are never freed** (new in Phase 3). Either free on
  removal, or ship pooling as the documented idiom and add a `Pool<T>`-shaped example.
- Free on `pop`/`shift`; add `array_free` / `.clear()`.
- `--leak-report` debug flag.
- **Acceptance test: 100,000 headless frames with flat RSS.** Current baselines:
  Breakout 8.5 → 17.5 MB over 100k frames (~92 B/frame), Asteroids 8.9 → 14.2 MB over
  40k frames (~135 B/frame).

---

## 7. Phase 5 — Numeric & ergonomic gaps ✅ DONE

Every gap that writing Breakout exposed is now closed, before starting Phase 3.

1. ✅ **`Math.min/max/round/ceil/atan2/random`, `Math.PI`/`Math.E`.**
2. ✅ **Module-level globals visible inside functions.** Was the top obstacle to
   organising a larger game — `const SPEED = 4.0;` used inside a `function` failed with
   *"Use of undefined variable"*.
   - Only variables a function actually references are promoted to LLVM globals; a pass
     over every function/method body collects the referenced names first. Everything else
     keeps its alloca in main, **so top-level hot loops stay register-allocated and the
     benchmarks are untouched** (verified: 0.051s/0.024s, still at Rust parity).
   - Required reordering codegen to *signatures → main → function bodies*, since main is
     what creates the globals. Function bodies now also correctly clear `namedValues`,
     so they can no longer see the caller's locals (which would have been invalid IR).
   - The Semantic pass keeps module-level names visible inside functions via `m_globals`,
     including their const-ness.
3. ✅ **Bitwise operators** `& | ^ ~ << >>` with TypeScript/C precedence
   (`|` < `^` < `&` < comparison < shift < additive).
   - `<<`/`>>` are detected as **two adjacent tokens** rather than lexed as one, so
     nested generics like `Map<string, Set<i32>>` still close correctly. Verified.
4. ✅ **Hex/binary/octal literals.** `0xFF` parsed as `0` because `std::stoll` defaults to
   base 10 and stopped at the `x`. Now `0xFF`, `0b1010` and `0755` all work.
5. ✅ **`null` as an expression**, for optional `ptr` handles.
6. ⬜ **Enums** — deferred; key codes work fine as `const`s now that globals are visible.
7. ⬜ **Typed fixed-size buffers** with inlined GEP load/store instead of a call per element
   — the tilemap/pixel performance path.
8. ⬜ **2D arrays** (`i32[][]`) for tilemaps.

A bug found while testing the globals work: seven codegen sites resolved variables through
`namedValues` directly (array `.length`, object access, JSON paths), so they could not see
globals. All now go through a single `variableStorage()` helper.

---

## 8. Phase 6 — Ship it 🔶

- ✅ **Headless self-test mode** — `CYPS_HEADLESS=1 CYPS_FRAMES=n` plus `isHeadless()`,
  so games run and assert in CI without a display.
- ✅ **`tests/run_game_tests.sh`**, wired into CI on macOS and Linux; skips cleanly when
  the game runtime wasn't built.
- ✅ **raylib is vendored.** CMake fetches raylib 5.5 and links it statically; the built
  `libraylib.a` is staged next to our own runtime, where `cscript`'s automatic `-L`
  already points. No system package, no machine-specific paths.
  `-DCYPESCRIPT_VENDOR_RAYLIB=OFF` uses a system raylib instead;
  `-DCYPESCRIPT_BUILD_GAME_RUNTIME=OFF` leaves it out entirely.
  Attribution for its zlib license is in [THIRD_PARTY.md](THIRD_PARTY.md), installed to
  `<prefix>/share/cypescript/`.
- ✅ **`lib/game.csc` ships with the compiler**, installed to `<prefix>/lib/cypescript/`
  and importable by bare name (`import { } from "game";`). Resolution mirrors
  `findRuntimeLibrary`: `$CYPESCRIPT_HOME` → next to the binary → installed layout.
- ✅ **Cross-platform link requirements** are generated into `lib/game.csc` at configure
  time from `lib/game.csc.in`, because raylib's system dependencies differ per OS and
  Cypescript has no conditional compilation.
- ⬜ Asset paths resolved relative to the binary, not the cwd.
- ⬜ `cscript build` producing a distributable bundle; macOS `.app`, Linux binary + assets.
- ⬜ Docs: a "Writing a Game" chapter; VSCode extension gains the game API.

### Platform support

| Platform | Compiler | Game runtime | Status |
|---|---|---|---|
| macOS (arm64) | ✅ | ✅ | Verified locally and in CI |
| Linux (x86_64) | ✅ | ✅ | CI: full suite + headless game tests |
| Windows (x86_64) | 🔶 | 🔶 | **CI job added, not yet green — see below** |

Windows work that has landed: `GetModuleFileNameW` for executable-relative path
resolution, `cmd.exe`-compatible quoting of link flags (the POSIX single-quote form would
have been passed through literally), platform link directives (`opengl32`/`gdi32`/
`winmm`), a `setjmp`/`longjmp` symbol pairing centralised in one place per platform, and
test scripts that find `cscript.exe` under both single- and multi-config layouts.

**This is unverified — there is no Windows machine in this loop.** The CI job is
`continue-on-error` until it has a track record, exactly as the Linux job was. The most
likely failures are LLVM discovery under Chocolatey and the `setjmp`/`longjmp` pairing,
since Windows has no `_longjmp` and its SEH-based unwinding differs from the POSIX model
the exception runtime assumes.

---

## 9. Critical path and risk

```
Phase 1 (FFI + linking) ✅ ──► Phase 2 (Breakout) ✅
        │                            │
        └──────────► Phase 3 (heap objects) ──► Phase 4 (memory) ──► Phase 6 (ship)
                             ▲
                     Phase 5 pulled in on demand
```

Phase 1 was the unlock and is done. **Phase 3 is now the critical path** and carries the
real risk.

| Risk | Mitigation |
|---|---|
| Object-model refactor destabilises a 4,100-line `CodeGen.cpp` where identity threads through several maps | 24 language tests + 4 game tests + 20 examples are the safety net; land 3a as a standalone, fully-green step before 3b/3c |
| Losing ~Rust-parity benchmarks to heap allocation | Keep the `alloca` fast path for non-escaping objects; `benchmarks/cross` is the regression gate |
| C struct-by-value ABI (raylib `Vector2`/`Color`) | ✅ Handled — the shim flattens everything to scalars |
| macOS window/main-thread requirements from an LLVM-generated `main` | ✅ Handled — raylib initialises correctly from our `main`, verified by the GL context reporting its true size |
| Scope creep into a game engine | Hard rule, held so far: `cscript` never learns that graphics exist |

---

## 10. The game API

`lib/game.csc` is an ordinary Cypescript file of `declare`s and `link`s. The
`= "cyps_..."` suffix on a declaration binds a natural Cypescript name to the C symbol,
so game code reads like TypeScript while the shim stays C:

```ts
declare function drawRect(x: f64, y: f64, width: f64, height: f64, color: i32): void = "cyps_rect";
declare function isKeyDown(key: i32): boolean = "cyps_key_down";
declare function deltaTime(): f64 = "cyps_dt";
```

```ts
import { } from "game";

openWindow(640, 360, "my game");
setTargetFps(60);

while (!windowShouldClose()) {
    let dt: f64 = deltaTime();
    if (isKeyDown(KEY_LEFT)) { x -= 200.0 * dt; }

    beginFrame();
    clearScreen(rgb(20, 20, 30));
    drawRect(x, 100.0, 80.0, 40.0, rgb(255, 140, 60));
    endFrame();
}

closeWindow();
```

Declared return types of `boolean` mean `while (!windowShouldClose())` and
`if (isKeyDown(...))` read naturally instead of comparing against `0`.

## 11. Progress log

**2026-07-25 — Phases 1 and 2 complete.**

New files:
- `runtime/game/cyps_game.c` — the raylib shim (flat C ABI, packed i32 colors, headless mode)
- `lib/game.csc` — the Cypescript game API: `declare`s, `link` directives, key constants
- `example/game/00_window.csc` — M1
- `example/game/01_breakout.csc` — M2
- `tests/test_ffi.csc`, `tests/test_math.csc` (+ expected output)
- `tests/run_game_tests.sh`

Compiler changes:
- `src/AST.h` — `ExternDeclarationNode`, `LinkDirectiveNode`
- `src/Parser.{h,cpp}` — `declare`/`link` as contextual keywords; `Math.*` additions
- `src/CodeGen.{h,cpp}` — extern registry + pass-0 registration, extern-first resolution,
  `f32`/`i64`/`i8`/`u8`/`ptr` types, generalised `coerceValue`, extern return-type inference
- `src/Semantic.cpp` — extern arity checking
- `src/main.cpp` — link flags (CLI + source directives), shell quoting, runtime lib dir
- `src/cypescript_stdlib.cpp` — `math_min/max/round/ceil/atan2/random`
- `CMakeLists.txt` — optional `cypescript_game` target gated on finding raylib

Tests: 24/24 language, 4/4 game. No regressions.

**2026-07-25 (later) — Phase 5 closed, raylib vendored, cross-platform groundwork.**

- **Language gaps fixed** (all with tests): bitwise operators, hex/binary/octal literals,
  `null`, and module-level globals visible inside functions. See Phase 5 above.
- **raylib vendored** via CMake FetchContent, statically linked, staged next to the
  runtime. Proven self-contained by unlinking the system raylib and rebuilding a game.
  [THIRD_PARTY.md](THIRD_PARTY.md) carries the zlib attribution.
- **`lib/game.csc` ships with the compiler**; bare-name imports resolve from the install.
  `lib/game.csc.in` + `configure_file` supplies per-OS link directives.
- **API renamed to TypeScript style** via a new optional `= "symbol"` clause on
  `declare function`, so `cyps_win_open` is now `openWindow` with no wrapper layer.
- **Windows/Linux portability**: `GetModuleFileNameW`, cmd.exe-safe quoting,
  per-platform setjmp/longjmp pairing, platform-tolerant test scripts, Linux game tests
  and a (non-blocking) Windows CI job.
- Fixed seven codegen sites that looked up variables via `namedValues` directly and so
  could not see globals; all now share one `variableStorage()` helper.

New tests: `test_bitwise`, `test_globals`. Suite is now 26/26 language + 4/4 game.

**2026-07-25 (later still) — Phase 3 complete: heap objects.**

- Object identity resolves from static types, so objects can be returned from functions
  and stored in arrays. Class layouts are registered in pass 0 from declared field types.
- `new` heap-allocates; object literals keep their alloca, so benchmarks are unchanged.
- Object arrays end to end: create, push, index, assign, `for...of`, `pop`, `shift`, and
  `removeAt` for despawning. Object pointers never go through the string vector, which
  copies its elements.
- `example/game/02_asteroids.csc` (M3) and `tests/test_object_arrays.csc`.
- `drawCircleOutline` added to the shim.
- Also fixed: a stale `cscript` on PATH produced a misleading "module not found"; the
  message now names the search directory and the running version, and the project is
  1.1.0 so the two builds are distinguishable.

Suite is now 27/27 language + 7/7 game.
