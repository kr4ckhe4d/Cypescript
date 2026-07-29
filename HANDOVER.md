# Handover

State of the tree, what is actually verified, and what to do on a Linux or Windows
machine. Written for picking this up cold.

- [ROADMAP.md](ROADMAP.md) — what is done and what is next
- [STEERING.md](STEERING.md) — the rules that shouldn't change

**Head:** `7522e8d` on `main`, pushed. **CI: green on macOS and Linux.**

---

## What is verified, and where

| | macOS (arm64) | Linux (x86-64) | Windows |
|---|---|---|---|
| Compiler builds | ✅ local + CI | ✅ CI | ❌ **never** |
| 66 language tests | ✅ local + CI | ✅ CI | ❌ never |
| 14 game tests (headless) | ✅ local + CI | ✅ CI | ❌ never |
| 23 examples compile and run | ✅ local + CI | ✅ CI | ❌ never |
| C and C++ interop | ✅ local + CI | ✅ CI | ❌ never |
| Benchmarks at parity | ✅ local | ✅ CI (single run) | ❌ never |
| `.deb` builds and installs | — | ✅ CI | — |
| A game in a **real window** | ✅ local | ❌ **never** | ❌ never |
| `--bundle` output | ✅ local (`.app`) | ❌ never | ❌ never |

Two gaps worth naming: **no Linux run has ever opened a window** — CI is headless,
which exercises the loop, physics and scoring but never X11/Wayland, GL context
creation or audio. And the non-Apple `--bundle` branch has never been executed by
anyone.

### What CI does not cover on Linux

`ubuntu-latest`, x86-64, GitHub's toolchain. Not your distro, not glibc's
alternatives, not ARM, not a display server. Your machine is the first that isn't
those.

---

## Recent work (last three commits)

| Commit | What |
|---|---|
| `7522e8d` | Ignore the executable `example/14_modules/main.csc` drops at the repo root |
| `4b0fd6d` | `++` and `--` as expressions, prefix and postfix |
| `2218efa` | **The Linux CI fix** — don't hand a null string to `puts()` |

### The Linux fix, and why it matters to you

`println` of a string emitted a bare `puts`. A missing JSON key comes back as a
**null pointer**, and glibc's `puts` calls `strlen` and segfaults on NULL where
Darwin's prints `(null)`. So `tests/test_property_checking.csc` passed on macOS and
crashed on Linux, and its expected-output fixture had been written against the
behaviour of the only platform it had been run on.

This was diagnosed from the diff rather than from logs (job logs need admin auth),
so it was an inference when it was written — **CI has since gone green on Linux,
including every step that had never run before**, which confirms it. Printing a null
string now yields `null` on every platform, substituted in the IR rather than left
to libc.

The lesson generalises and is rule 3 in [STEERING.md](STEERING.md).

---

## Building and testing, any platform

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/cscript --version
```

```bash
bash tests/run_tests.sh          # 66: 42 positive with output fixtures, 24 negative
bash tests/run_game_tests.sh     # 14, headless — skips cleanly if raylib isn't built
bash benchmarks/cross/run_cross_benchmarks.sh 3
```

Examples, the way CI runs them:

```bash
for f in example/*.csc example/14_modules/main.csc; do
  case "$f" in *math_utils*) continue;; esac
  echo "=== $f"; ./build/cscript -r "$f" || echo "FAILED: $f"
done
```

`cscript -r foo.csc` drops the compiled binary in the current directory. Use
`-o /tmp/name` for scratch work so it doesn't end up in `git status`.

---

## Linux

Expect this to work — CI proves the toolchain path. What you're testing is your
distro and, for the first time, an actual display.

### Dependencies

```bash
sudo apt-get install -y cmake llvm-dev clang zlib1g-dev libzstd-dev
sudo apt-get install -y libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev libxkbcommon-dev libasound2-dev pkg-config time
```

On a non-Debian distro the names differ; the shapes are LLVM development headers,
clang, and X11 + GL + ALSA development packages. raylib is **built from source by
CMake**, so it needs those headers rather than a raylib package.

### Run, in this order

1. The three suites and the examples above. All should pass; anything that doesn't
   is new information.
2. **The thing CI has never done — open a window:**
   ```bash
   ./build/cscript -r example/game/01_breakout.csc
   ./build/cscript -r example/game/02_asteroids.csc
   ```
   Watch for: a window that actually appears, 60 fps in the HUD, keyboard input
   (arrows/A-D, SPACE, R, ESC), and audio — every sound is **synthesised**, so
   silence means the audio path is broken, not that an asset is missing.
3. **The non-Apple bundle branch**, which no one has ever executed:
   ```bash
   ./build/cscript --bundle example/game/02_asteroids.csc
   ```
   On Linux this produces a `02_asteroids-bundle/` **directory** holding the binary
   and its `assets/` — not a macOS `.app`. Run it from a *different* working
   directory: the point of bundling is that assets resolve relative to the binary
   rather than the cwd, and that resolution is what you're testing.
4. Memory, if you want to confirm the flat-RSS claim yourself:
   ```bash
   CYPS_HEADLESS=1 CYPS_FRAMES=2000   /usr/bin/time -v ./02_asteroids 2>&1 | grep Maximum
   CYPS_HEADLESS=1 CYPS_FRAMES=300000 /usr/bin/time -v ./02_asteroids 2>&1 | grep Maximum
   ```
   These two numbers should be near-identical. 300,000 frames is 83 minutes of play
   at 60 fps.

### If something fails

Most likely, in order: a missing dev package (link error naming `-lX11`, `-lGL`,
`-lasound`); **link order** if the error is an undefined `cyps_*` symbol — GNU ld
scans archives left-to-right, and `libcypescript.a` must come last on the line
(`src/main.cpp`, the comment above `compileCmd += " \"" + stdlibPath + "\""`); or no
display, if you're over SSH without `$DISPLAY` — that's not a bug, use
`CYPS_HEADLESS=1`.

---

## Windows

**Nothing here has ever built, let alone passed.** The portability work is written
but unexercised. Treat the first green run as the goal, not the baseline. The CI job
exists and is `workflow_dispatch`-only precisely so it isn't permanently red.

```bash
choco install llvm --no-progress -y
export PATH="/c/Program Files/LLVM/bin:$PATH"
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --config Release
```

Use Git Bash — the test scripts are bash, and CI runs them with `shell: bash`.
The compiler lands at `build/cscript.exe` or `build/Release/cscript.exe`; both test
scripts already look in both places.

### Expected failure points, in the order they will probably bite

**1. CMake cannot find LLVM.** Chocolatey's `llvm` package may not ship
`LLVMConfig.cmake` at all. If `find_package(LLVM)` fails, that's this. Fallbacks: a
different LLVM distribution, or `-DLLVM_DIR=` pointed at wherever the config
actually lives.

**2. The link line assumes a GNU driver.** `clang++` on Windows targets the MSVC
ABI, where `-l`/`-L` do not behave the way the link line in `src/main.cpp` assumes.
Symptom: the compiler builds fine, then *your compiled `.csc` programs* fail to
link. Relevant code is `collectLinkFlags` and `shellQuote` in `src/main.cpp` —
`shellQuote` already has a `cmd.exe` branch that rejects embedded quotes rather than
mangling them.

**3. Exceptions — `setjmp`/`longjmp`.** This is the one I'd bet on being subtly
wrong rather than loudly broken. Two things to check:

- `src/CodeGen.cpp` declares the symbol via `CYPS_SETJMP_SYMBOL`, whose `#if
  defined(_WIN32)` and `#else` branches are **currently identical** (`"_setjmp"`) —
  a conditional that was written for a difference that then wasn't made. It pairs
  with `CYPS_LONGJMP` in `src/cypescript_stdlib.cpp`, which *is* different
  (`longjmp` on Windows, `_longjmp` elsewhere). Keep them paired.
- The generated IR declares `i32 _setjmp(ptr)` — **one argument**. On x64 Windows
  the UCRT's `_setjmp` takes a second frame-pointer argument for SEH. If that's
  right, a one-argument call may link and still corrupt unwinding. **Unverified —
  check it against your UCRT headers before changing anything.**

  The test that exercises this is `tests/test_exceptions.csc`. If everything else
  passes and that one crashes or hangs, this is why.

**4. Game runtime.** `lib/game.csc` already carries the Windows link set
(`opengl32`, `gdi32`, `winmm`, `shell32`). raylib builds from source via CMake. If
the game runtime doesn't build, `run_game_tests.sh` **skips cleanly rather than
failing** — check its output says "skipping" and don't read that as a pass.

### Worth recording as you go

The failure list above is a prediction, not a finding. What's useful back:

- Which of the four bit, in what order, and what the actual error was
- Anything that failed for a reason **not** on the list — that's the valuable part
- Whether `run_game_tests.sh` genuinely ran or skipped
- The exact LLVM version and how you installed it

Getting through step 1 alone is real progress; the roadmap has Windows as
unknown-size for exactly this reason.

---

## Picking up mid-stream

If you want to leave a note for the next session, the useful shape is: what you ran,
what the machine was, what broke, and what you had already ruled out. The three
things most likely to be re-derived otherwise are in
[STEERING.md](STEERING.md#traps-already-paid-for).
