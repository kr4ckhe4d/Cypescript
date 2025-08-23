# Basic Examples

These examples work with the **basic compilation pipeline** using `./compile-run.sh` or manual compilation.

## Compilation Methods

### Option 1: Using compile-run.sh (Recommended)
```bash
./compile-run.sh example/basic/hello.csc
./compile-run.sh example/basic/arithmetic.csc
```

### Option 2: Manual Compilation
```bash
# 1. Compile to LLVM IR
./build/cscript example/basic/hello.csc

# 2. Compile to object file
llc -filetype=obj -relocation-model=pic output.ll -o output.o

# 3. Link executable
clang output.o -o my_program

# 4. Run
./my_program
```

## Supported Features

These examples use only the **core language features**:
- ✅ Variables (`let` with type annotations)
- ✅ Basic types (`string`, `i32`)
- ✅ Arithmetic operations (`+`, `-`, `*`, `/`, `%`)
- ✅ Comparison operations (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- ✅ Control flow (`if`/`else`)
- ✅ Loops (`while`, `for`, `do-while`)
- ✅ Arrays (`i32[]`, `string[]`)
- ✅ Array operations (access, assignment, `.length`)
- ✅ Built-in functions (`print`, `println`)

## Example Files

### Core Language Features
- **`hello.csc`** - Basic variables and printing
- **`arithmetic.csc`** - Arithmetic operations and precedence
- **`control_flow.csc`** - If/else statements and comparisons
- **`while_loop.csc`** - While loop examples
- **`all_loops.csc`** - All loop types (while, for, do-while)
- **`comprehensive.csc`** - Complex algorithms (factorial, prime, fibonacci)

### Array Examples
- **`array_length_test.csc`** - Array `.length` property usage
- **`array_assignment_test.csc`** - Array element assignment
- **`arrays_final_test.csc`** - Comprehensive array operations
- **`arrays_simple_test.csc`** - Basic array creation and access
- **`arrays_comprehensive.csc`** - Advanced array usage patterns

### String and Array Integration
- **`string_array_simple.csc`** - String arrays
- **`string_array_comprehensive.csc`** - Advanced string array operations
- **`arrays_string_test.csc`** - String array testing

### Test and Debug Files
- **`comprehensive_native_test.csc`** - Native compiler feature validation
- **`complex_native_compatible.csc`** - Complex example for native compiler
- **`complex_native_simple.csc`** - Simplified complex example
- **`print_vs_println.csc`** - Print function comparison
- **`test_fixes.csc`** - Various language feature tests
- **`loop_edge_cases.csc`** - Edge cases for loop constructs

## Performance

These examples compile to **native machine code** through LLVM, providing:
- ⚡ **Native performance** (no interpreter overhead)
- 🔧 **LLVM optimizations** (dead code elimination, constant folding, etc.)
- 📦 **Standalone executables** (no runtime dependencies)
