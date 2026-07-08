# Cypescript Examples

A guided tour of the language, ordered from easiest to most complex.
Each file is self-contained and heavily commented — read them in order.

## How to run

Build the compiler once, then run any example **from the repository root**
(the compiler links the C++ stdlib by relative path):

```bash
./build.sh                                  # one-time build

# Compile AND run in one step (recommended)
./build/cscript -r example/01_hello.csc

# Or compile to a named executable, then run it
./build/cscript -o hello example/01_hello.csc
./hello
```

Run everything in sequence:

```bash
for f in example/*.csc example/14_modules/main.csc; do
    [[ "$f" == *"math_utils"* ]] && continue
    echo "=== $f ==="; ./build/cscript -r "$f"
done
```

## The progression

| # | File | Concepts |
|---|------|----------|
| 01 | `01_hello.csc` | `print`, `println`, `console.log` |
| 02 | `02_variables.csc` | `let`/`const`, `string`/`i32`/`f64`/`boolean`, type inference |
| 03 | `03_operators.csc` | arithmetic, comparison, logical, `+=`, `++`, `===` |
| 04 | `04_control_flow.csc` | `if`/`else if`/`else`, `switch`/`case`/`default`, fallthrough |
| 05 | `05_loops.csc` | `while`, `for`, `do-while`, `for...of`, `break`/`continue` |
| 06 | `06_functions.csc` | parameters, returns, recursion, generics `<T>` |
| 07 | `07_strings.csc` | concatenation, template literals, stdlib string helpers |
| 08 | `08_arrays.csc` | literals, `.length`, `.push()`/`.pop()`/`.shift()`, iteration |
| 09 | `09_objects.csc` | object literals, property access/assignment, nesting, printing |
| 10 | `10_interfaces.csc` | `interface`, `extends`, compile-time structural checks |
| 11 | `11_methods_and_this.csc` | object methods, `this`, shorthand syntax |
| 12 | `12_destructuring.csc` | `let { a, b } = obj` |
| 13 | `13_exceptions.csc` | `try`/`catch`/`finally`, `throw`, rethrowing |
| 14 | `14_modules/` | `import`/`export` — run `14_modules/main.csc` |
| 15 | `15_json.csc` | `JSON.stringify`, `JSON.parse`, stdlib JSON helpers |
| 16 | `16_typescript_compat.csc` | `console.log`, `Math.*`, `===`, `number` — near-plain TS |
| 17 | `17_cpp_stdlib.csc` | C++ stdlib: strings, file I/O, random |
| 18 | `18_bfs_graph.csc` | **capstone**: generics + `Map`/`Set` + BFS algorithm |

`18_bfs_graph.ts` is the same algorithm in real TypeScript (`node example/18_bfs_graph.ts`) —
diff it against the `.csc` file to see how close the languages are.

## Notes

- **Booleans print as `1`/`0`** (they are native integers).
- Example 14 must be run via its entry point: `./build/cscript -r example/14_modules/main.csc`.
- Example 17 writes a small `demo_output.txt` into the current directory.
- The commented-out lines in `02` (const reassignment) and `10` (interface
  violations) are intentional — uncomment them to see compile-time errors.
