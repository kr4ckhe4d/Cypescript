# Handover

State of the tree, what is actually verified, and what to do on a Linux or Windows
machine. Written for picking this up cold.

- [ROADMAP.md](ROADMAP.md) — what is done and what is next
- [STEERING.md](STEERING.md) — the rules that shouldn't change

**Head:** `b5b0f4b` on `main`. **CI: green on macOS and Linux** as of `d1b9a5c`.
**v1.1.0 is released** — the tag contains the LLVM linkage fix (`97dcb55`), the
`.deb` dependency fix (`cd4fe07`) and the CI guard (`d1b9a5c`), so the published
tarball is not the one that failed on Arch.

Reading CI logs needs admin on the repo — the REST API returns `403 Must have admin
rights` — so use `gh`, not `curl`:

```bash
gh run list --limit 5
gh run view <run-id> --log | grep "LLVM Linkage"
```

---

## What is verified, and where

"Arch" is a local x86-64 CachyOS machine — the first Linux that isn't
`ubuntu-latest`, and the first to disagree with it.

| | macOS (arm64) | Ubuntu CI (x86-64) | Arch (x86-64) | Windows |
|---|---|---|---|---|
| Compiler builds | ✅ local + CI | ✅ CI | ✅ local, **after the LLVM fix** | ❌ **never** |
| 66 language tests | ✅ local + CI | ✅ CI | ✅ local | ❌ never |
| 14 game tests (headless) | ✅ local + CI | ✅ CI | ✅ local, **14/14** | ❌ never |
| 23 examples compile and run | ✅ local + CI | ✅ CI | ✅ local | ❌ never |
| C and C++ interop | ✅ local + CI | ✅ CI | ✅ local (suites) | ❌ never |
| README snippets compile | ✅ CI | ✅ CI | ✅ local | ❌ never |
| Benchmarks at parity | ✅ local | ✅ CI (single run) | ❌ **not run** | ❌ never |
| `.deb` builds and installs | — | ✅ CI | — | — |
| Game runtime vs **system** raylib | ❌ never | ❌ never | ✅ local (raylib 6.0) | ❌ never |
| A game in a **real window** | ✅ local | ❌ **never** | ❌ never | ❌ never |
| `--bundle` output | ✅ local (`.app`) | ✅ CI (directory) | ✅ local (directory) | ❌ never |

Counts as of `c714dfe`: **66** language tests (42 positive with output fixtures, 24
negative), **14** game tests, **46** README snippets (39 compiled, 7 illustrative),
**23** examples. The README numbers moved in `f825d68`/`c714dfe`; if a suite reports
fewer than these, something regressed rather than the docs being stale.

Arch previously reported 12 pass and 2 skip; the 2 were the RSS checks, skipping
because **GNU `time` was not installed** — `/usr/bin/time`, not the shell builtin.
`pacman -S time` (or `apt-get install time`) was enough, and they now pass: breakout
5.48 MB at 2k frames against 5.46 MB at 60k, asteroids 5.84 against 5.80. The header
line now reads `/usr/bin/time: present (RSS checks enabled)`, which is the thing to
check. This was the trap STEERING.md names — the script skips cleanly rather than
failing, so 12/12 green was not 14/14 green. Read the "⏭ SKIP" lines before calling
it a pass.

Two gaps still open: **no Linux run has ever opened a window** — every Linux run so
far is headless, which exercises the loop, physics and scoring but never
X11/Wayland, GL context creation or audio. And **no benchmark has run on Arch**, so
rule 2's parity claim rests on macOS local plus a single Ubuntu CI run.

The `--bundle` row is corrected from an earlier version of this file, which said the
non-Apple branch had never executed. It had: `tests/run_game_tests.sh:243` has an
explicit non-Darwin arm asserting a `Probe-bundle/` **directory**, it runs in Ubuntu
CI, and it passes on Arch too.

### What CI does not cover on Linux

`ubuntu-latest`, x86-64, GitHub's toolchain. Not your distro, not glibc's
alternatives, not ARM, not a display server. Arch has now disagreed with it once,
over how LLVM is packaged — details below.

---

## Recent work

| Commit | What |
|---|---|
| `b5b0f4b` | Warn, naming raylib, when a program imports `"game"` without the runtime built |
| `bfd95db` | Correct which packaging path runs `cmake --install` |
| `1e7db29` | Link the system raylib in the Arch package — **now verified, see below** |
| `cd788fa` | The Homebrew formula could not build 1.1.0 — depend on raylib, install the game runtime |
| `7a658cb` | **Release v1.1.0** |
| `d1b9a5c` | CI asserts the `.deb` declares what the binary links against |
| `cd4fe07` | Declare the LLVM runtime dep the `.deb` actually has; keep the install rpath |
| `97dcb55` | **The Arch build fix** — link the LLVM dylib where components aren't shipped |

Older but still load-bearing: `2218efa` was the Linux CI fix — `println` of a null
string emitted a bare `puts`, which segfaults on glibc and prints `(null)` on
Darwin. That lesson is rule 3 in [STEERING.md](STEERING.md) and needs no repeating
here.

### The Arch build failure, and what it means for other distros

**Symptom:** `cannot find -lLLVMCore`, plus `libLLVMSupport.a: file format not
recognized`, at the final link of `cscript`. The compiler's own sources build fine;
only the link fails.

**Cause:** one bug wearing two faces. Arch ships LLVM 22.1.6 built with
`LLVM_LINK_LLVM_DYLIB=ON`, so every component lives in `/usr/lib/libLLVM.so.22.1`
and the per-component archives are simply not installed. The handful that survive
are LTO artifacts — `ar x libLLVMSupport.a` yields members that `file` reports as
`LLVM IR bitcode`, which `ld.bfd` cannot read without the LLVM plugin. Meanwhile
`llvm_map_components_to_libnames` hands back `LLVMCore`, `LLVMSupport` and friends
unconditionally, so the link could never have succeeded.

**Fix:** `CMakeLists.txt` asks `LLVM_LINK_LLVM_DYLIB` — which `LLVMConfig.cmake`
always defines — and links the single `LLVM` dylib when it is `ON`, falling back to
the mapped component archives otherwise. An undefined variable falls through to the
archives, so the fallback is the old behaviour exactly.

**Which arm each platform takes — measured, not assumed.** The configure step
prints it, because rule 8 says a branch nobody compiles is not portable code:

```
-- LLVM Linkage: shared (libLLVM)      # or: static components
```

Run `ce76bc9` answered it for all three, and the answer was *not* the expected one:

| | LLVM | arm |
|---|---|---|
| Arch (local) | 22.1.6 | shared |
| macOS (Homebrew) | 22.1.8 | shared |
| Ubuntu CI | 18.1.3 | shared |

**Every platform we build on is `DYLIB=ON`.** So this was not an Arch quirk — it
moved macOS and Ubuntu off the archives too, and both stayed green. The visible
effect is size: the macOS artifact went 3,253,749 → 304,881 bytes and the `.deb`
2,797,706 → 1,266,525, which is LLVM leaving the binary. Artifact sizes are
readable without admin (`gh run view`, or the public artifacts API) and are a decent
proxy for this if you ever need to check it again without log access.

Two consequences, both now fixed in `cd4fe07`:

1. **The `.deb` had an undeclared dependency.** `readelf -d` on the shipped
   artifact showed `NEEDED libLLVM.so.18.1` and `RUNPATH /usr/lib/llvm-18/lib`
   against a control file saying only `Depends: clang`. `clang` is unversioned, so
   on a release where it resolves to a different LLVM major the package installs
   and then fails to start. `build-deb.sh` now computes the dependency from the
   binary with `dpkg-shlibdeps`. The smoke test cannot catch this class of bug —
   the machine that builds the package always has `llvm-dev` on it, so `cscript`
   starts either way — so a **"Check the .deb declares what the binary needs"**
   step now asserts the control file against the binary's own `NEEDED` entries,
   via `readelf` and `dpkg -S`. It deliberately does *not* rerun the
   `dpkg-shlibdeps` call that produced the list, because a check that reruns the
   producer passes vacuously whenever the producer silently does nothing.
2. **`cmake --install` could drop the rpath.** Fixed with
   `INSTALL_RPATH_USE_LINK_PATH`. Which packaging path this matters to was stated
   wrongly at first, so to be exact: `packaging/cypescript.rb` and
   `packaging/build-deb.sh` both *copy* `build/cscript` with its rpath intact and
   never needed it, while **`packaging/arch/PKGBUILD` does run `cmake --install`**.
   It is a no-op on Arch regardless — `libLLVM.so` is in `/usr/lib`, and CMake
   omits rpath entries for default search paths — but the `install(TARGETS ...)`
   rules are a real shipping path, not just a convenience for hand-installers.

**What is now unexercised.** With every platform on the dylib arm, the
`static components` fallback is compiled by nobody — the situation rule 8 exists to
flag. It is not speculative code (it was CI-green until `97dcb55`), but it will rot
silently. It stays because a stock source-built LLVM defaults to `DYLIB=OFF` and
would need it. If you ever build against one, that is the configuration to report
back on.

Do not assume Fedora, Homebrew or Debian match each other here. That assumption was
made once while writing this fix, about Debian, and was wrong in the other
direction — the printed line is there so nobody has to guess again.

### The system-raylib path, now verified

`1e7db29` made the Arch package pass `-DCYPESCRIPT_VENDOR_RAYLIB=OFF` and depend on
`raylib`, but shipped with only the CMake branch confirmed — the machine had no
raylib. That mattered more than it sounds: **when raylib is missing CMake skips the
game runtime with a status message instead of failing**, so "it configured" proves
nothing about whether games work. It is now confirmed end to end against Arch's
**raylib 6.0**: `libcypescript_game.a` builds, `-lraylib` resolves to
`/usr/lib/libraylib.so`, and all 14 game tests pass. So the package builds a
compiler that can actually compile games, which is the claim the dependency exists
to make.

**One trap when you re-run this.** A `build/` that previously vendored raylib still
has `build/libraylib.a` sitting in it, and `cscript` adds a `-L` pointing at exactly
that directory — so the link silently picks up the **vendored** archive and the test
proves nothing. Delete it before believing a system-raylib result:

```bash
rm -f build/libraylib.a
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCYPESCRIPT_VENDOR_RAYLIB=OFF
cmake --build build --parallel
```

A correct system-raylib build stages **no** `libraylib.a` at all; if one is there,
it is left over. Note this cuts the other way for the vendored build, where staging
that archive next to the runtime is the whole mechanism (`CMakeLists.txt:147`).

`b5b0f4b` covers the failure this leaves behind: a program that imports `"game"`
built by a `cscript` with no game runtime used to fail with `cannot find
-lcypescript_game`, naming our artifact rather than the missing package. It now
warns first, names raylib, and gives both ways out. The warning searches any `-L`
the caller passed as well as the runtime dir, so a runtime built elsewhere is not
warned at.

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
bash tests/run_readme_tests.sh   # 46 snippets: 39 compiled, 7 marked illustrative
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

The toolchain path is proven by CI and by Arch. What is still untested is a display
and any distro that is neither of those.

### Dependencies

```bash
sudo apt-get install -y cmake llvm-dev clang zlib1g-dev libzstd-dev
sudo apt-get install -y libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev libxkbcommon-dev libasound2-dev pkg-config time
```

Arch equivalent:

```bash
sudo pacman -S --needed cmake llvm clang zlib zstd \
  mesa libx11 libxrandr libxinerama libxcursor libxi libxkbcommon alsa-lib \
  pkgconf time
```

On any other distro the names differ; the shapes are LLVM development headers,
clang, and X11 + GL + ALSA development packages. raylib is **built from source by
CMake**, so it needs those headers rather than a raylib package. Do not skip `time`
— it is GNU `/usr/bin/time`, without which two game tests skip rather than run.

### Run, in this order

1. The three suites and the examples above. All pass on Arch, so anything that
   doesn't is new information about your distro. Check the configure log's
   `LLVM Linkage:` line while you are here, and record which arm it took.
2. **The thing CI has never done — open a window:**
   ```bash
   ./build/cscript -r example/game/01_breakout.csc
   ./build/cscript -r example/game/02_asteroids.csc
   ```
   Watch for: a window that actually appears, 60 fps in the HUD, keyboard input
   (arrows/A-D, SPACE, R, ESC), and audio — every sound is **synthesised**, so
   silence means the audio path is broken, not that an asset is missing.
3. **The non-Apple bundle branch on a real game.** The branch itself is covered by
   `run_game_tests.sh`, but only against a synthetic probe:
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

Most likely, in order: **LLVM packaging**, if the failure is at the link of
`cscript` itself and names `-lLLVMCore` or "file format not recognized" — see the
Arch section above, and check what `LLVM Linkage:` printed; a missing dev package
(link error naming `-lX11`, `-lGL`, `-lasound`); **link order** if the error is an
undefined `cyps_*` symbol — GNU ld scans archives left-to-right, and
`libcypescript.a` must come last on the line (`src/main.cpp`, the comment above
`compileCmd += " \"" + stdlibPath + "\""`); or no display, if you're over SSH
without `$DISPLAY` — that's not a bug, use `CYPS_HEADLESS=1`.

Note which failed and which didn't. The Arch entry above exists because the
prediction list did not have LLVM packaging on it.

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
