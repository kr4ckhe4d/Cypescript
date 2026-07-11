# EP01 — I Built a TypeScript That Runs at Rust Speed

**Length target:** 18–22 min · **Difficulty:** none (motivation + setup)
**Goal:** hook viewers with the end result, explain the compiler pipeline in
plain language, and get their environment building by the end.

---

## [COLD OPEN] (0:00–0:45)

[SCREEN: split terminal. Left: `primes.ts`. Right: `primes.csc` — visibly
almost identical code.]

NARRATION:
> These two files are almost character-for-character identical. The left one
> is TypeScript. The right one is a language I built myself. Watch what
> happens when I run the same algorithm — counting every prime under a
> million — in TypeScript, in Python, in Rust... and in mine.

[SCREEN: run `bash benchmarks/cross/run_cross_benchmarks.sh` live. Let the
table print.]

> Node: about 150 milliseconds. Python: almost two seconds. Rust: 50
> milliseconds. And my language? **51 milliseconds. One millisecond behind
> Rust.** In this series, I'm going to show you exactly how to build this —
> from an empty folder to `brew install`.

[TITLE CARD: "Build Your Own Language — Episode 1"]

---

## Segment 1 — What you'll build (0:45–4:00)

[SCREEN: VS Code with `example/20_classes.csc` open, syntax highlighted.]

NARRATION:
> By the end of this series you'll have a compiler for a language that looks
> like this. Classes with constructors and methods. Interfaces that are
> checked at compile time. Arrow functions with closures. try/catch. Template
> strings. Modules. And it doesn't interpret anything — it compiles straight
> to native machine code through LLVM, the same backend Rust, Swift, and
> Clang use.

[SHOW: quick montage — run 4–5 examples from `example/` with `cscript -r`,
one second each: hello, bfs graph, exceptions, arrows.]

> And because I believe in receipts: everything you'll see is in a public
> repo, with a test suite, CI, and a real v1.0 release you can install with
> Homebrew right now.

[SCREEN: `brew install` output + `cscript --version`]

⚠ PITFALL (say this now, honestly):
> One warning before we start: this is a *real* project but a *teaching*
> compiler. It leaks memory on purpose, its type checker is thin, and there
> are sharp edges. That's fine. V8 started somewhere too.

---

## Segment 2 — How a compiler actually works (4:00–9:00)

[B-ROLL / DIAGRAM: the pipeline, boxes left to right]

```
source code → LEXER → tokens → PARSER → AST → SEMANTIC CHECK
           → CODEGEN → LLVM IR → LLVM -O2 → native executable
```

NARRATION (walk the diagram box by box):
> A compiler is a pipeline of translations, and each stage only has one job.
>
> The **lexer** reads raw characters and groups them into words — "let",
> an identifier, an equals sign, the number 42. We call those tokens.
>
> The **parser** takes that flat list of words and discovers the grammar —
> this is a variable declaration, whose initializer is an addition, whose
> left side is... It builds a tree. The Abstract Syntax Tree.
>
> The **semantic checker** walks that tree looking for nonsense that is
> grammatically fine but meaningless — using a variable that doesn't exist,
> reassigning a constant.
>
> And **codegen** walks the tree one more time and emits LLVM IR — a kind of
> portable assembly language. Then LLVM does the hard part: register
> allocation, optimization, machine code. That's the trick of this whole
> series: **we don't beat Rust by being geniuses. We ride the same
> optimizer Rust rides.**

[SHOW: `./build/cscript -o out.ll example/01_hello.csc` then `cat out.ll` —
scroll slowly over the IR]

> This is LLVM IR. It looks scary. By episode 4 you'll read it like a menu.

---

## Segment 3 — Project setup (9:00–16:00)

[SCREEN: empty terminal]

NARRATION:
> Let's set up. You need three things: a C++ compiler, CMake, and LLVM.

[TYPE]
```bash
# macOS
brew install cmake llvm
# Ubuntu/Debian
sudo apt-get install -y cmake llvm-dev clang
```

[TYPE — create the skeleton]
```bash
mkdir mylang && cd mylang && git init
mkdir src example tests
```

[SHOW: `CMakeLists.txt` — walk through the real one, trimmed]
```cmake
cmake_minimum_required(VERSION 3.15)
project(Cypescript VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})

add_executable(cscript src/main.cpp)
llvm_map_components_to_libnames(LLVM_LIBS Core Support)
target_link_libraries(cscript PRIVATE ${LLVM_LIBS})
```

NARRATION:
> Three interesting lines. `find_package(LLVM)` locates the LLVM you just
> installed. `llvm_map_components_to_libnames` picks which pieces of LLVM we
> link — Core and Support are enough to start. Everything else is boilerplate.

⚠ PITFALL:
> On macOS, Homebrew's LLVM is *keg-only* — CMake won't find it by default.
> Either export `LLVM_DIR="$(brew --prefix llvm)/lib/cmake/llvm"` or add the
> hint paths like the repo's CMakeLists does. This is the #1 setup failure —
> put it on screen in big text.

[TYPE: `src/main.cpp` — minimal]
```cpp
#include "llvm/Support/raw_ostream.h"
int main() {
    llvm::outs() << "compiler online\n";
    return 0;
}
```

[TYPE]
```bash
cmake -B build && cmake --build build && ./build/cscript
# -> compiler online
```

> If you see "compiler online", LLVM is linked and you're ready.

---

## Segment 4 — The plan, and homework (16:00–19:00)

[SCREEN: episode list graphic (from series README table)]

NARRATION:
> Here's the roadmap. Episode 2: the lexer — honestly the easiest 300 lines
> you'll ever write, and suddenly you're "writing a compiler". Episode 3: the
> parser, where the tree appears. Episode 4 is the magic one — first native
> executable. Then control flow, functions, objects, exceptions... Episode 9
> is closures, the single hardest thing in this series. And episode 10, we
> benchmark it against Rust and ship v1.0 to Homebrew.
>
> Homework before episode 2: get "compiler online" printing on your machine,
> and skim the tokens section of the repo — `src/Token.h`. It's just an enum.
> You already understand it.

## [OUTRO]

> Everything's linked below — repo, the exact commands, and the benchmark
> script so you can verify that Rust number yourself. I'll see you in the
> lexer.

---

## Description box

```
Build a real compiled programming language from scratch — one that looks like
TypeScript and benchmarks within ~2% of Rust. Episode 1: how compilers work,
LLVM setup, and the plan.

Repo: https://github.com/kr4ckhe4d/Cypescript
Benchmarks: benchmarks/cross/ (verify the numbers yourself!)
Install the finished language: brew tap kr4ckhe4d/cypescript && brew install cypescript

00:00 The demo (TS syntax, Rust speed)
00:45 What you'll build in this series
04:00 How a compiler actually works
09:00 Environment setup (CMake + LLVM)
16:00 Roadmap + homework
```
