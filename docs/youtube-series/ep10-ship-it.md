# EP10 — Ship It: Benchmarks, Homebrew, and v1.0

**Length target:** 26–30 min · **Repo anchor:** `src/Optimizer.cpp`,
`benchmarks/cross/`, `findRuntimeLibrary` in `src/main.cpp`, `RELEASING.md`,
`packaging/`, `.github/workflows/ci.yml`
**Goal:** the series finale — compile-time optimization, the on-camera
benchmark against Rust/Node/Python, making the compiler installable, and
cutting a real v1.0 release.

---

## [COLD OPEN] (0:00–0:45)

[SCREEN: a *clean machine* (or fresh user account). Nothing installed.]

[TYPE]
```bash
brew tap kr4ckhe4d/cypescript
brew install cypescript
echo 'println("hello, world");' > hi.csc
cscript -r hi.csc
```
```
hello, world
```

NARRATION:
> That machine has never seen my source code. It downloaded a tarball,
> verified a checksum, built a compiler, and ran a program in a language that
> didn't exist a few months ago. Today, in the finale: we optimize, we
> benchmark against Rust with witnesses, and we ship version 1.0 — for real,
> with a license, a package manager, and CI. Because a language you can't
> install is a diary entry.

---

## Segment 1 — Compile-time optimization: constant folding (0:45–8:00)

[SHOW: run with `-v`]
```
✓ Constant folding complete (0.049ms, 2 expressions folded, 1 dead branch removed)
```

NARRATION:
> LLVM's -O2 already optimizes our output brutally. So why write our own
> pass? Because some knowledge lives in *your* AST, before LLVM ever sees it
> — and because you deserve to write one real optimizer in your life.

[SHOW: `src/Optimizer.cpp` — the fold, condensed]
```cpp
// post-order: fold children first, then try to fold this node
if (bothChildrenAreLiterals(binOp)) {
    switch (binOp->op) {
        case ADD: expr = makeLiteral(l + r); break;   // 5 + 3*2 → 11
        case DIVIDE:
            if (r == 0) return;   // ← leave it! runtime's problem, not ours
            ...
    }
}
// dead branches: if (false) {...} else {B} → just B; while(false) → gone
```

NARRATION:
> Constant folding: if both operands are literals, do the math *now* —
> post-order, so `5 + 3 * 2` folds the multiply first and then the whole
> thing collapses to 11. String concat of literals folds too. Then
> dead-branch elimination: `if (false)` splices in the else branch,
> `while (false)` vanishes.
>
> Two lessons hiding in ten lines. See the divide-by-zero bail? An optimizer
> must **never change what a program does** — including how it fails. And we
> made it optional — `--no-fold` — because when your optimizer has a bug,
> you'll want to know in one flag-flip whether it's the culprit.

---

## Segment 2 — The benchmark, with witnesses (8:00–16:00)

[B-ROLL: the four files side by side — primes in .csc/.ts/.py/.rs — visibly
the same algorithm]

NARRATION:
> Benchmark rules, because language benchmarks are where honesty goes to die:
> same algorithm, idiomatic in each language, checked to produce the *same
> answer* before timing, best-of-N wall clock, and the script is public so
> anyone can re-run it. Our two workloads: counting primes below a million —
> tight branchy loops — and naive recursive fib of 35 — thirty million calls.

[SCREEN: **run it live**, full output]
```bash
bash benchmarks/cross/run_cross_benchmarks.sh
```
```
Benchmark                     Cypescript   Node (TS)     Python       Rust
Primes <1M (trial div)           0.051s      0.146s      1.777s     0.050s
Fibonacci fib(35) recursive      0.025s      0.125s      0.621s     0.026s
```

NARRATION (over the table — this is the emotional peak of the series):
> Twenty-five times faster than Python. Three to five times faster than Node
> running the *identical* TypeScript source. And Rust — the gold standard —
> beats us by about *one millisecond*. Why? No mystery, and give credit
> honestly: both languages hand LLVM clean, statically-typed IR, and LLVM
> does the rest. Everything this series did — struct objects instead of hash
> maps, direct method calls, inline map/filter loops, i32s staying i32s — was
> about *not destroying information* before LLVM could use it. The lesson of
> the whole series in one table: **speed is mostly about what you refuse to
> give up at compile time.**

---

## Segment 3 — Making it installable (16:00–22:00)

NARRATION:
> Here's the unglamorous wall between a project and a product: my compiler
> only worked *from my repo directory*, because it linked the runtime by the
> relative path `src/cypescript_stdlib.cpp`. Type `cscript` anywhere else:
> broken. Every young compiler has this disease.

[SHOW: the fix — `findRuntimeLibrary` in main.cpp]
```cpp
// 1. $CYPESCRIPT_HOME/lib/libcypescript.a         (explicit override)
// 2. libcypescript.a next to the cscript binary    (build tree)
// 3. ../lib/libcypescript.a relative to the binary (installed: /usr/bin → /usr/lib)
// 4. ../src/cypescript_stdlib.cpp                  (running from the repo)
```

NARRATION:
> Two changes. First, CMake precompiles the runtime into a static library,
> `libcypescript.a`, so users never need the C++ source. Second, the compiler
> finds that library **relative to its own executable path** — asking the OS
> where it lives via `_NSGetExecutablePath` on macOS, `/proc/self/exe` on
> Linux — with a search chain covering every layout. Now `/usr/bin/cscript`
> finds `/usr/lib/libcypescript.a` with zero configuration. This is what
> "relocatable" means, and it's the prerequisite for every package below.

[SHOW: quick tour, 60–90 seconds each]
- **Homebrew formula** (`packaging/cypescript.rb`): url + sha256 + build
  commands. Emphasize the checksum: brew downloads the *tag tarball* and
  refuses a byte of drift. Show generating it: `curl -sL <tag.tar.gz> | shasum -a 256`.
- **Debian** (`packaging/build-deb.sh`): stage `/usr/bin` + `/usr/lib`, write
  a control file, `dpkg-deb --build`. An installer is a tarball with manners.
- **Arch** (`packaging/arch/PKGBUILD`): same tarball, same sha256 — and it
  runs our whole test suite during install. Arch users expect no less.
- **CI** (`.github/workflows/ci.yml`): every push builds macOS *and* Linux,
  runs all 22 tests and all 20 examples, then **builds the .deb, installs
  it, and compiles a program with the installed compiler**. The install path
  is tested code.

---

## Segment 4 — Cutting v1.0 (22:00–26:00)

[SCREEN: terminal, the actual ceremony]
```bash
# the gauntlet
bash tests/run_tests.sh          # 22/22, output-asserted
# the paperwork
git add LICENSE                  # MIT — a language nobody may legally use isn't shipped
git tag -a v1.0.0 -m "Cypescript v1.0.0 — first release"
git push origin main v1.0.0
# the checksum, into the formula & PKGBUILD
curl -sL .../v1.0.0.tar.gz | shasum -a 256
```

NARRATION:
> The release ritual, in order: green tests, a LICENSE file — do not skip
> this, unlicensed code is legally untouchable — an annotated tag, and the
> checksum baked into every package file. One rule above all: **never move a
> published tag.** The sha256 is derived from it; if something's wrong, that's
> what version 1.0.1 is for. The repo has the whole ceremony written down in
> RELEASING.md, because six months from now, past-you is a stranger.

[SCREEN: the GitHub release page, the green CI badge, the README install
one-liner.]

---

## Segment 5 — What we built, and what's next (26:00–29:00)

[B-ROLL: fast montage — the token stream from ep2, the AST from ep3, the
first `42` from ep4, the 2.1x chart from ep7, the closure demo, the
benchmark table, the brew install.]

NARRATION:
> Ten episodes ago this was an empty folder. Now it's a language with
> classes, interfaces, generics, closures, exceptions, and modules — that
> reads like TypeScript, benchmarks against Rust, and installs with one
> command on three platforms.
>
> And the honest ledger, because real projects have one: memory is
> script-lifetime — we leak on purpose and documented it. The type checker
> is thin. No class inheritance yet, no union types. Every one of those is
> written down in the repo as future work, and every one is a video I might
> still make.
>
> If you build your own — and you should, yours will be different and that's
> the point — tag me. The best thing this series can produce isn't my
> language. It's yours.

## [OUTRO]

> Repo, all ten checkpoint tags, the benchmark script, and the release
> runbook are linked below. Thank you for building a language with me.

---

## Description box

```
The finale: we write a constant-folding optimizer, benchmark the language
against Rust / Node / Python on camera (spoiler: ~2% behind Rust), make the
compiler relocatable, and ship v1.0 with Homebrew, a .deb, an Arch PKGBUILD,
and CI on macOS + Linux.

Install it: brew tap kr4ckhe4d/cypescript && brew install cypescript
Repo + release runbook: https://github.com/kr4ckhe4d/Cypescript (RELEASING.md)

00:00 Installing my own language on a clean machine
00:45 Writing a constant-folding optimizer
08:00 The benchmark: Cypescript vs Rust vs Node vs Python (live)
16:00 Making a compiler installable (relocatable runtime, brew/deb/PKGBUILD, CI)
22:00 Cutting v1.0: tags, checksums, LICENSE
26:00 What we built + the honest ledger
```
