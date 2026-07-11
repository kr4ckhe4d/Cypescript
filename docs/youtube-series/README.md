# "Build Your Own Language" — YouTube Series Scripts

A 10-episode series teaching viewers to build a real compiled language from
scratch, based on **Cypescript**: a TypeScript-style language that compiles to
native code via LLVM and benchmarks within ~2% of Rust.

## Why this series works

Most "build a language" content stops at a tree-walking interpreter for a toy
Lisp. This series ends with something nobody else delivers:

- a language whose programs **look like TypeScript** and **run like Rust**
- installable with `brew install` / `apt install`
- with closures, classes, exceptions, generics, and a real module system
- backed by receipts: a public repo, CI, benchmarks, and a v1.0 release

Every episode ends with a working compiler the viewer can run. Every line of
code shown exists in the repo — when a viewer pauses the video and reads the
source, it matches.

## Episode list

| # | Title | Core skill | Repo anchor |
|---|-------|------------|-------------|
| 01 | I Built a TypeScript That Runs at Rust Speed | architecture, setup | whole repo, `benchmarks/cross/` |
| 02 | Lexing: Turning Text into Tokens | lexer | `src/Lexer.cpp`, `src/Token.h` |
| 03 | Parsing: Building the Tree | recursive descent, AST | `src/Parser.cpp`, `src/AST.h` |
| 04 | Hello, LLVM: Generating Real Machine Code | LLVM IR, codegen | `src/CodeGen.cpp` |
| 05 | Control Flow: The Basic Block Puzzle | branches, loops, break | `CodeGen::visit(If/While/Switch...)` |
| 06 | Functions and a Real Runtime | calling conventions, C++ interop | `src/cypescript_stdlib.cpp` |
| 07 | Objects at Native Speed | struct layout, GEP, `this` | `src/ObjectOptimizer.cpp` |
| 08 | Exceptions Without a Runtime: setjmp/longjmp | unwinding, error messages | `cyps_try_*`, `src/Semantic.cpp` |
| 09 | Closures: The Hardest Thing You'll Ever Compile | capture analysis, environments | `CodeGen::visit(ArrowFunctionNode*)` |
| 10 | Ship It: Benchmarks, Homebrew, and v1.0 | optimization, packaging, release | `RELEASING.md`, `packaging/` |

## Format conventions used in the scripts

- **[COLD OPEN]** — pre-intro hook, 20–45 seconds, always shows a result first
- **[SCREEN: ...]** — what's on screen (editor file, terminal, diagram)
- **[TYPE]** — code the presenter writes live (keep it short; pre-write the rest)
- **[SHOW]** — pre-written code being walked through
- **[B-ROLL / DIAGRAM]** — cutaway visual
- **NARRATION** — spoken track; written to be read aloud, contractions and all
- **⚠ PITFALL** — a mistake viewers will make; call it out on screen
- Timestamps are targets for a 18–30 min episode; adjust to your pace

## Production notes

- Record the terminal at 120+ columns, font ≥ 16pt, dark theme
- Every episode's final code should be a git tag (`ep02-lexer`, `ep03-parser`, ...)
  so viewers can `git checkout` any checkpoint
- The series builds a simplified spine of Cypescript ("start from the 20% that
  teaches 80%"); episodes link to the full implementation for the rest
- Re-run `benchmarks/cross/run_cross_benchmarks.sh` on camera in EP01 and EP10 —
  live numbers beat screenshots
