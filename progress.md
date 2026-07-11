# Progress — Language Completion Session (2026-07-07)

> **v1.0.0 SHIPPED 2026-07-08**: GitHub release published, Homebrew tap
> (kr4ckhe4d/homebrew-cypescript) live, `brew install cypescript` verified
> building from the tag tarball in 9s on the user's machine. CI green.

## Goal
Implement everything outstanding in `NATIVE_OBJECTS_ROADMAP.md`, `OPTIMIZATION_ROADMAP.md`,
and the README "Planned Features" list. **Status: DONE.** All 18 tests pass
(`bash tests/run_tests.sh`), all examples compile/run. Changes are **uncommitted**.

## What was implemented (all verified working)

| Feature | Where |
|---|---|
| `break` / `continue` (loops + switch) | `CodeGen` loop-target stack (`loopTargets`), `branchAndSealBlock` |
| `else if` chains | `Parser::parseIfStatement` |
| `switch/case/default` w/ fallthrough, int/float/string conditions | `parseSwitchStatement`, `CodeGen::visit(SwitchStatementNode*)` |
| `f64` end-to-end (literals, arith, compare, promotion, print `%g`) | `FloatLiteralNode`, FP branch in `visit(BinaryExpressionNode*)`, `coerceValue` |
| String concat `+` (was documented but broken), string+number | `toStringValue` + stdlib `cyps_i32_to_string` / `cyps_f64_to_string` / `string_concat` |
| Template literals `` `Hi ${x}` `` | Lexer desugars to `("Hi " + (x))` via pending-token queue (`Lexer::makeTemplateLiteral`) |
| Compound assign `+= -= *= /= %=`, `++`, `--` (vars, obj props, array elems, for-increments) | `Parser::parseExpressionOrAssignmentStatement` (re-parses lhs tokens to clone) |
| Interfaces + `extends` + structural checks | `InterfaceDeclarationNode`, `CodeGen::checkInterfaceConformance` (pass 0 registry in `visit(ProgramNode*)`) |
| Object methods + `this` (both `f: function(){}` and shorthand `f(){}`) | `ObjectLiteralNode::Property::method`, `objectMethods` map, `getOrCreateMethodFunction` (lazy, implicit i8* `this` param, save/restoreIP) |
| `obj.prop = value` | `ObjectPropertyAssignmentNode`, `ObjectOptimizer::generateDirectPropertyStore` |
| Destructuring `let { a, b } = obj` | `DestructuringDeclarationNode`, codegen reuses ObjectAccess |
| `try/catch/finally/throw` | setjmp/longjmp: IR calls `_setjmp` (ReturnsTwice attr); stdlib `cyps_try_push/pop`, `cyps_throw` (`_longjmp`), `cyps_last_error`; fixed-depth 64 jmp_buf stack |
| Modules `import {x} from "./f"` / `export` | Compile-time inlining in `main.cpp` (`resolveImports`, cycle-safe, auto `.csc` ext); parser skips `export`, errors on stray `import` |
| Constant folding + dead-branch elim (`if(false)`, `while(false)`) | NEW `src/Optimizer.{h,cpp}` (`ASTOptimizer`), runs post-parse, `--no-fold` flag |

## Bug fixes made along the way
- `boolean` produced invalid IR (i1 alloca vs i32 literals) → boolean is i32 everywhere; `coerceValue` handles i1↔i32↔f64 at decl/assign/return/call/print sites.
- Lexer: `0.5` lexed as `0` + DOT (octal branch) → fixed in `makeIntegerLiteral`.
- Generic calls with builtin type args (`bfs<string>(...)`) didn't parse → `isTypeToken` lambda in `parseVariableExpression`. (This is why `example/bfs.csc` compiles now.)
- `.pop()` was aliased to shift → real `array_pop_i32/_string` added to stdlib.
- Unterminated blocks after `return`/`break`/`throw` → dead-block sealing; func end emits `RetVoid`/`Unreachable`.
- `createEmptyObject` returned i32 0 → now null i8*.
- `visit(VariableDeclarationNode)` rewritten: initializer generated first, value-driven inference (incl. NewExpression → `Map<...>` type string, var aliasing copies object key), object tracking works for explicitly-typed (interface) decls.

## Files touched
- `src/Token.h`, `Lexer.{h,cpp}` — 18 new keywords, 5 compound-op tokens, template literals, pending queue
- `src/AST.h` — FloatLiteral, Break, Continue, Switch, InterfaceDecl, ObjectPropertyAssignment, DestructuringDecl, Throw, TryCatch nodes; Property::method
- `src/Parser.{h,cpp}` — new statements, expression-based statement dispatch, `parseFunctionRest` shared by functions/methods
- `src/CodeGen.{h,cpp}` — bulk of work (see table)
- `src/ObjectOptimizer.{h,cpp}` — `generateDirectPropertyStore`
- `src/Optimizer.{h,cpp}` — NEW (added to CMakeLists)
- `src/cypescript_stdlib.cpp` — to-string helpers, array_pop, exception runtime
- `src/main.cpp` — module resolution stage, optimizer stage, `--no-fold`
- Tests NEW: `test_control_flow, test_floats, test_strings, test_compound_assign, test_interfaces, test_methods, test_destructuring, test_exceptions, test_modules` + `tests/modules/math_utils.csc`
- `example/16_typescript_compat.csc` — NEW end-to-end showcase
- Docs: README (features/syntax/status/structure), both roadmap MDs (note: roadmap MDs are **gitignored**, changes are on disk only)

## Known limitations (documented in README/roadmaps)
- Objects are stack-allocated → returning an object from its creating function dangles (pre-existing design).
- `return` inside `try` skips `cyps_try_pop` (stale recovery point).
- Compound-assign clones lhs by re-parsing → side-effecting targets evaluate twice.
- No `f64[]` arrays; Set/Map mostly string-keyed; compound ops only where lhs is re-parseable.
- Not implemented (now listed as Planned): arrow functions/closures, classes, union types, JIT/GC/custom SIMD passes (Phases 4/6/7).

## Session 2 additions (same day)
- **TS compatibility layer**: `console.log/error/warn/info` (multi-arg, space-joined),
  `===`/`!==` (lexed as `==`/`!=`), `Math.sqrt/pow/abs/floor/sin/cos/log/exp` → native
  math stdlib (added `math_floor`); external-call args now coerced to param types.
  All in `Parser::parseVariableExpression` + `Lexer` + `CodeGen::generateExternalFunctionCall`.
- **README usage rewrite**: documented that `cscript` compiles directly to an executable
  (must run from repo root for the stdlib path), `-r`, `-o name` / `-o file.ll`, `--no-fold`;
  replaced outdated llc/clang pipeline; added "Running TypeScript Scripts (Compatibility)"
  section with the porting table.
- **Cross-language benchmarks** in `benchmarks/cross/` — identical primes (<1M trial
  division) + recursive fib(35) in .csc/.ts/.py/.rs with `run_cross_benchmarks.sh`
  (verifies matching outputs, best-of-N wall time, skips missing toolchains).
  Results (Apple Silicon, Node 26.3, Python 3.14.4, rustc 1.89):
  primes 0.051s / 0.146s / 1.777s / 0.050s; fib(35) 0.025s / 0.125s / 0.621s / 0.026s
  → **~Rust parity, 2.9-5x vs Node, 25-35x vs Python**. Documented in
  `benchmarks/cross/README.md` + main README performance section.

## Session 3 additions
- **`SHIPPING_BLOCKERS.md`** — full v1.0 gap analysis: LICENSE missing, compiler not
  relocatable (stdlib linked by relative path), committed build artifacts, no CI/packaging,
  no line/col errors, memory model caveats, stale VSCode extension, suggested attack order.
- **Web docs updated** (`docs/`): new "Modern Features" nav section covering all new
  language features (with `native only` badges since the browser playground interpreter
  only supports the old subset — see blocker #11), getting-started now shows the direct
  `cscript -r` workflow, performance section gained the cross-language (TS/Python/Rust)
  results table, `.native-badge` style added to `styles.css`. HTML validated balanced.

## Session 4 additions
- **`example/` rebuilt from scratch** as a graded tour: `01_hello` → `18_bfs_graph`
  (18 numbered examples + `14_modules/` dir + `18_bfs_graph.ts` TS twin), all verified
  to compile and run; BFS capstone output is byte-identical to Node running the `.ts`.
  Old examples (basic/, cpp-integration/, browser-only/, *_test.csc, language_tour)
  removed — recoverable from git history. `example/README.md` documents the
  progression table and run commands. Stale `example/*` references fixed in main
  README, docs/index.html, build.sh, compile-run.sh, and the VSCode extension docs.

## Session 5 additions — shipping blockers addressed (2026-07-08)
- ✅ **Relocatable compiler**: stdlib builds as `build/libcypescript.a` (new CMake target);
  `cscript` resolves the runtime via `CYPESCRIPT_HOME` → next-to-binary → `../lib` →
  `../src` fallback (`findRuntimeLibrary` + `getExecutablePath` in main.cpp, macOS/Linux).
  Verified working from an arbitrary cwd. `cmake --install` layout: bin/ + lib/.
- ✅ **Repo cleanup**: `git rm --cached all_loops bfs hello`, deleted all stale root
  artifacts, hardened .gitignore. (Note: `*_ROADMAP.md` gitignore rule still hides the
  roadmap docs from git — user decision pending.)
- ✅ **`--version` / `-V`** (from CMake PROJECT_VERSION), **`.github/workflows/ci.yml`**
  (build, tests, all examples, 1-run benchmark, artifact upload),
  **`packaging/cypescript.rb`** Homebrew formula template (needs url+sha at release time).
- ✅ **Line/column error messages**: Token has line/column; Lexer tracks via advance();
  template-literal expansion tokens stamped with template start; parser errors
  (`consume`, statement dispatch, primary expression) print "at line X, column Y".
  Codegen-stage errors still positionless.
- ✅ **try/return fix**: `tryDepth` counter + per-loop-entry depth stack in CodeGen;
  return/break/continue emit the right number of `cyps_try_pop()` calls before exiting.
  Verified: uncaught throw after returns/breaks out of trys exits cleanly (code 1).
- ✅ **Output-assertion tests**: `tests/expected/*.out` (18 files) diffed by rewritten
  `tests/run_tests.sh`; mismatch shows a diff excerpt. Negative-tested.
- 🔶 **VSCode grammar** updated (all new keywords, `number` type, console/Math/JSON
  builtins, template-literal scope with `${}` embedding) — snippets/completions and
  `.vsix` repackage still pending (needs `vsce`).
- ⏳ LICENSE — user is handling.
- SHIPPING_BLOCKERS.md annotated with per-item status.

## Session 6 additions — arrow functions & closures (2026-07-08)
- **ArrowFunctionNode** (AST), parser lookahead disambiguation (`(...) =>` vs parens,
  `x =>` single param; untyped params default i32; optional `: ret` annotation;
  expression bodies desugar to return).
- **Closures**: `{i8* fn, i8* env}` malloc'd pairs; free-variable analysis
  (`collectFreeVars*` in CodeGen) determines captures; env snapshots captured values
  **by value** at creation (captured object/array pointers share underlying data — the
  documented idiom for mutable state). Arrow bodies generated lazily with full context
  save/restore incl. loopTargets/tryDepth. Calls through variables bind statically via
  `variableToArrow` (aliasing copies the binding).
- **Callback array methods** on `T[]`: `.map` (elem type can change, e.g. i32->string),
  `.filter`, `.forEach`, `.reduce(cb, init)`, `.find` (early exit) — all generated as
  inline loops calling the arrow function directly (no dispatch overhead).
  Var-decl inference knows map/filter/reduce/find/shift/pop result types.
- `.length` now works on arbitrary array expressions (`arr.filter(f).length`).
- Tests: `tests/test_arrows.csc` (+ expected out, suite now 19); example
  `19_arrow_functions.csc`; docs/README/blockers updated.
- **Limitations**: captures are by-value (mutating a captured primitive doesn't affect
  the outer var — capture an object instead); closures can't yet be passed as params
  to user-defined `function`s (only stored in vars / passed to array methods);
  un-annotated return types use a heuristic — annotate for exotic cases.

## Session 7 additions — v1.0 ship batch (2026-07-08)
- **LICENSE** — MIT, copyright Nipuna H Herath.
- **Classes** (`class` keyword): fields with zero-value or explicit defaults,
  `constructor`, methods with `this`, `new ClassName(args)`, class names as type
  annotations. Implementation: parser synthesizes an ObjectLiteralNode template per
  class; `new` instantiates it and calls the constructor method. No inheritance yet.
- **Function-type parameters**: `function apply(f: (i32) => i32, x: i32)` — parseType
  canonicalizes to "closure(i32)=>i32"; calls through such params are indirect via the
  closure's fn pointer (parseClosureSignature builds the LLVM FunctionType).
  Inline arrows and captured closures both work as arguments.
- **f64[] arrays**: stdlib f64 vector + create/push/get/set/shift/pop; codegen paths
  for literals (annotation drives element type, int elements coerce), indexing,
  assignment, for...of, push/pop/shift, and all callback methods (map can produce
  f64[]/i32[]/string[] based on callback return type).
- **Semantic analysis pass** (`src/Semantic.{h,cpp}`, runs post-parse): undefined
  variables, assignment to undefined/const, break/continue placement, user-function
  arity — all with line/column (ASTNode now carries positions; parser stamps
  statements + variable refs + calls). Function bodies correctly isolated from
  enclosing locals; arrows analyzed inside enclosing scope (captures).
- **VSCode extension 1.1.0**: grammar now includes `class`; 13 new snippets (switch,
  interface, class, try, arrow, import/export, for-of, template literal,
  map/filter/reduce); packaged cypescript-1.1.0.vsix (old 1.0.0 removed).
- **Playground** labeled core-subset-only with pointer to `cscript -r`.
- **Release prep**: Homebrew formula URLs point at kr4ckhe4d/Cypescript; v1.0.0 tag
  (see below); suite now 22 tests, examples 01-20.

## Session 8 additions — release docs + Linux packaging (2026-07-08)
- **RELEASING.md**: full release runbook — version bump (CMake is source of truth),
  test gauntlet, tag discipline (never re-tag), tarball sha256 generation, GitHub
  release, Homebrew tap update, .deb/PKGBUILD updates, vsce packaging, post-release
  checklist, v1.0.0 reference values.
- **Linux packaging**: `packaging/arch/PKGBUILD` (builds from tag tarball, runs test
  suite in check(), depends clang/llvm-libs) and `packaging/build-deb.sh` (binary
  .deb via dpkg-deb: /usr/bin/cscript + /usr/lib/libcypescript.a, Depends: clang).
- **Linux CI job** (ubuntu-latest, continue-on-error while it builds a track
  record): apt LLVM build, full test suite, all examples, builds AND installs the
  .deb, smoke-tests the installed compiler, uploads the .deb as an artifact.
- README Install section now covers macOS/Debian/Arch.
- NOTE: Linux is CI-validated only — never run on a local Linux box. Watch the
  first ubuntu-latest run; likely risks: apt LLVM version differences (18 vs 22),
  _setjmp/_longjmp symbol availability (glibc has them), static LLVM lib linking.

## How to resume
```bash
./build.sh          # or: cmake --build build
bash tests/run_tests.sh   # 18/18 should pass
./build/cscript -r example/16_typescript_compat.csc
```
Next logical steps: commit this work; then arrow functions, union types, `f64[]`,
line/column error reporting, or heap objects w/ refcounting.
