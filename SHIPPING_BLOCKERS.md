# Shipping Blockers & v1.0 Checklist

Gap analysis for distributing Cypescript to real users (written 2026-07-07).
Ordered roughly by recommended attack order.

## 🔴 Blockers — must fix before anyone outside this repo can use it

### 1. No LICENSE file — ⏳ OWNER: user
README claims MIT but there is no `LICENSE` file in the repo. Nobody can legally
use or redistribute the compiler without it. **Fix:** add the MIT license text.

### 2. Compiler is not relocatable — ✅ FIXED (2026-07-08)
`build/cscript` only works when invoked from the repository root:
- `src/main.cpp` hardcodes the stdlib as a relative path: `src/cypescript_stdlib.cpp`
- Every compile shells out to `clang++ <ir> src/cypescript_stdlib.cpp ...`

**Fix:**
- Precompile the stdlib once at build time into `libcypescript.a` (CMake target)
- Install it next to the binary (e.g. `<prefix>/lib/libcypescript.a`)
- `cscript` resolves it via its own executable path, overridable with `CYPESCRIPT_HOME`
- Link with `clang++ <ir> -lcypescript -L<prefix>/lib`

### 3. Repo contains committed build artifacts — ✅ FIXED (2026-07-08)
Compiled binaries and intermediates are tracked at the repo root: `bfs`, `hello`,
`my_program`, `cypescript_program`, `functions_demo`, `all_loops`, `simple_test`,
`nested_test`, `array_length_test`, `phase1_optimization_test`, `*.o`, `*.ll`,
`.aider.*`, `.DS_Store`. **Fix:** `git rm` them, extend `.gitignore` (partially done),
and note that `NATIVE_OBJECTS_ROADMAP.md` / `OPTIMIZATION_ROADMAP.md` are currently
gitignored — decide whether that is intentional before shipping docs.

### 4. Toolchain dependency undocumented — ✅ FIXED (documented in README Requirements)
Users need `clang++` (Xcode CLT on macOS) and, to build the compiler, Homebrew LLVM.
**Fix:** document as prerequisites (like Rust requiring a linker), or bundle `lld`.
Decide minimum supported LLVM version (currently built against LLVM 22).

### 5. No packaging / release pipeline — ✅ MOSTLY FIXED (--version, CI workflow, formula template; actual GitHub release/tap still to publish)
**Fix:**
- `--version` flag + semantic versioning (start v0.x honestly, or v1.0 with the
  documented TS-subset scope)
- GitHub Actions CI: `./build.sh && ./test.sh && bash benchmarks/cross/run_cross_benchmarks.sh 1`
  on push (macOS arm64 at minimum; add x86_64 + Linux runners later)
- GitHub Releases with prebuilt binaries; Homebrew formula (tap) as install path

## 🟠 Robustness — needed for a credible v1.0

### 6. No line/column numbers in error messages — ✅ FIXED for lexer/parser errors (codegen-stage errors still positionless)
`Token.h` has the TODO. All parse/codegen errors say *what* went wrong but not
*where*. Single biggest developer-experience gap.
**Fix:** track line/col in `Lexer`, carry through `Token` into parser/codegen errors.

### 7. Memory model is "leak until exit"
- stdlib allocates strings/arrays with `new` and never frees (by design for scripts)
- objects are stack-allocated (`alloca`) → returning an object from the function
  that created it dangles
**Fix (v1):** document as script-lifetime memory + "don't return object literals
from functions". **Fix (later):** arena allocator or refcounted heap objects
(see OPTIMIZATION_ROADMAP Phase 6).

### 8. Known semantic sharp edges — 🔶 PARTIALLY FIXED
- ✅ FIXED: return/break/continue exiting try blocks now pop their recovery points (tryDepth tracking in CodeGen)
- Compound assignment re-parses its lhs → side-effecting targets evaluate twice
- Exceptions runtime has a fixed 64-deep try-nesting limit
- No type checker pass — type errors surface as codegen exceptions

### 9. Tests only assert exit codes — ✅ FIXED (tests/expected/*.out diffed by run_tests.sh)
`tests/run_tests.sh` passes if the binary exits 0. **Fix:** add expected-output
files and diff stdout, so wrong-output regressions are caught.

## 🟡 Polish — shortly after v1.0

### 10. VSCode extension is stale — 🔶 GRAMMAR UPDATED (new keywords + template literals; snippets/completions + .vsix repackage still pending)
Grammar/snippets predate: `switch/case`, `interface`, `try/catch/throw`,
`import/export`, template literals, `break/continue`, compound assignment,
`console.log`. Update `vscode-extension/` and repackage the `.vsix`.

### 11. Web docs playground interpreter is stale
`docs/cypescript-interpreter.js` is a hand-written JS interpreter implementing the
*old* language subset; new-feature examples cannot run in the browser. Options:
extend it (double maintenance) or drop the in-browser execution in favor of
`cscript -r` instructions. Static doc content updated 2026-07-07.

### 12. Define the supported TypeScript subset
"Runs TS with minute changes" should be a precise claim. Not yet supported:
arrow functions/closures, `class`, union types, npm imports, `x++` as an
expression, f64 arrays / array methods (`.map/.filter/.reduce`).
Publish the compatibility table (now in README) as part of the docs.

## Suggested attack order
1. LICENSE + repo cleanup (30 min)
2. Relocatable stdlib (`libcypescript.a`) (half day)
3. Line/column error messages (1–2 days)
4. CI + output-assertion tests (half day)
5. Homebrew formula + GitHub release (half day)
6. VSCode extension refresh (half day)
7. Memory-model documentation, then arena/refcounting (ongoing)
