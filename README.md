# Cypescript

A TypeScript-style language compiler built with C++ and LLVM. Cypescript aims to provide a familiar syntax for developers coming from TypeScript/JavaScript while compiling to efficient native code through LLVM.

## 📖 Interactive Documentation

**NEW!** Cypescript now features comprehensive web-based documentation with runnable examples:

```bash
# Launch interactive documentation
./launch-docs.sh
```

The web documentation includes:
- 🚀 **Runnable Examples** - Execute Cypescript code directly in your browser
- 🎮 **Interactive Playground** - Write and test your own code
- 📱 **Mobile-Friendly** - Works on all devices
- 🔍 **Searchable** - Find what you need quickly
- 📚 **Complete Reference** - All language features documented

## Features

- **TypeScript-inspired syntax** with type annotations
- **Variable declarations** with `let`, `const`, and type inference
- **Built-in types**: `string`, `i32`, `f64`, `boolean`
- **Complete arithmetic operations** (`+`, `-`, `*`, `/`, `%`)
- **Comparison operators** (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- **Control flow** with `if`/`else` statements and nesting
- **All loop constructs**: `while`, `for`, `do-while`
- **Variable assignments** and complex expressions
- **Function calls** (currently `print` and `println` functions)
- **String and numeric literals**
- **Comments** (single-line `//` and multi-line `/* */`)
- **Comprehensive error handling** and reporting
- **LLVM IR generation** for efficient native code compilation

## Quick Start

### 1. Setup (macOS)

Run the setup script to install dependencies:

```bash
./setup-macos.sh
```

This will install:
- Homebrew (if not present)
- CMake
- LLVM (latest version)
- Configure your shell environment

### 2. Build

```bash
./build.sh
```

### 3. Test

```bash
./test.sh
```

### 4. View Documentation

```bash
./launch-docs.sh
```

### 5. Manual Usage

```bash
# Compile a Cypescript file
./build/cscript example/hello.csc

# With verbose output and debugging
./build/cscript -v --print-tokens --print-ast example/hello.csc

# Specify output file
./build/cscript -o my_output.ll example/hello.csc

# Get help
./build/cscript --help
```

## Language Syntax

### Variable Declarations

```typescript
let message: string = "Hello, World!";
let count: i32 = 42;
let pi: f64 = 3.14159;
let isActive: boolean = true;
```

### Arithmetic Operations

```typescript
let a: i32 = 10;
let b: i32 = 3;

let sum: i32 = a + b;        // 13
let difference: i32 = a - b; // 7
let product: i32 = a * b;    // 30
let quotient: i32 = a / b;   // 3 (integer division)
let remainder: i32 = a % b;  // 1
```

### Control Flow

```typescript
let score: i32 = 85;

if (score >= 90) {
    print("Grade: A");
} else {
    if (score >= 80) {
        print("Grade: B");
    } else {
        print("Grade: C or below");
    }
}
```

### Loops

```typescript
// While loop
let count: i32 = 0;
while (count < 5) {
    print("Count: ");
    print(count);
    count = count + 1;
}

// For loop
for (let i: i32 = 0; i < 10; i = i + 1) {
    print("Iteration: ");
    print(i);
}

// Do-while loop
let attempts: i32 = 0;
do {
    print("Attempt: ");
    print(attempts);
    attempts = attempts + 1;
} while (attempts < 3);
```

### Function Calls

```typescript
print("Hello, World!");  // Output without newline
println("Hello, World!"); // Output with newline
print(42);
println(message);
```

### Comments

```typescript
// Single-line comment
let x: i32 = 10;

/*
 * Multi-line comment
 * Supports multiple lines
 */
let y: string = "test";
```

## Complete Compilation Pipeline

After generating LLVM IR with Cypescript:

```bash
# 1. Compile Cypescript to LLVM IR
./build/cscript example/hello.csc

# 2. Compile LLVM IR to object file
llc -filetype=obj -relocation-model=pic output.ll -o output.o

# 3. Link to create executable
clang output.o -o my_program

# 4. Run the program
./my_program
```

## Example Programs

### Factorial Calculator
```typescript
let n: i32 = 5;
let factorial: i32 = 1;
let counter: i32 = 1;

while (counter <= n) {
    factorial = factorial * counter;
    counter = counter + 1;
}

print("5! = ");
println(factorial); // Output: 120
```

### Prime Number Checker
```typescript
let testNum: i32 = 17;
let divisor: i32 = 2;
let isPrime: i32 = 1;

if (testNum <= 1) {
    isPrime = 0;
} else {
    while (divisor * divisor <= testNum) {
        if (testNum % divisor == 0) {
            isPrime = 0;
        }
        divisor = divisor + 1;
    }
}

if (isPrime == 1) {
    println("17 is prime!");
}
```

### Fibonacci Sequence
```typescript
let limit: i32 = 10;
let fib1: i32 = 0;
let fib2: i32 = 1;
let count: i32 = 0;

while (count < limit) {
    if (count == 0) {
        println(fib1);
    } else {
        if (count == 1) {
            println(fib2);
        } else {
            let nextFib: i32 = fib1 + fib2;
            println(nextFib);
            fib1 = fib2;
            fib2 = nextFib;
        }
    }
    count = count + 1;
}
```

## Development

### Project Structure

```
Cypescript/
├── src/
│   ├── main.cpp      # Compiler entry point
│   ├── Lexer.cpp/h   # Lexical analysis
│   ├── Parser.cpp/h  # Syntax analysis
│   ├── AST.h         # Abstract Syntax Tree
│   ├── CodeGen.cpp/h # LLVM IR generation
│   └── Token.h       # Token definitions
├── example/
│   ├── hello.csc     # Basic example
│   ├── arithmetic.csc # Arithmetic operations
│   ├── control_flow.csc # If/else statements
│   ├── while_loop.csc # While loops
│   ├── all_loops.csc # All loop types
│   └── comprehensive.csc # Complex algorithms
├── docs/             # Web documentation
│   ├── index.html    # Interactive docs
│   ├── styles.css    # Documentation styling
│   ├── script.js     # UI functionality
│   └── cypescript-interpreter.js # Browser interpreter
├── build.sh          # Build script
├── test.sh           # Test script
├── setup-macos.sh    # macOS setup script
├── launch-docs.sh    # Documentation launcher
└── CMakeLists.txt    # CMake configuration
```

### Building Manually

If you prefer manual building:

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run
./build/cscript example/hello.csc
```

### Debugging

Enable verbose output to see compilation stages:

```bash
./build/cscript -v --print-tokens --print-ast example/hello.csc
```

This will show:
- **Lexical Analysis**: All tokens generated
- **Syntax Analysis**: Abstract Syntax Tree
- **Code Generation**: LLVM IR output
- **Timing Information**: Performance metrics

## Requirements

- **macOS** (Intel or Apple Silicon)
- **CMake** 3.15+
- **LLVM** (any recent version, installed via Homebrew)
- **Clang** (for linking final executable)
- **Python 3** (for web documentation)

## Language Features Status

### ✅ Implemented Features
- [x] Lexical analysis with comprehensive token support
- [x] Variable declarations (`let`) with type annotations
- [x] Variable assignments with type checking
- [x] All arithmetic operators (`+`, `-`, `*`, `/`, `%`)
- [x] All comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- [x] Control flow with `if`/`else` statements
- [x] Nested conditional statements
- [x] While loops with complex conditions
- [x] Traditional for loops (`for (init; condition; increment)`)
- [x] Do-while loops (post-condition loops)
- [x] Nested loops of all types
- [x] Built-in `print` function
- [x] String and integer literal support
- [x] Boolean literals (`true`, `false`)
- [x] Single-line (`//`) and multi-line (`/* */`) comments
- [x] LLVM IR code generation
- [x] Native executable compilation
- [x] Comprehensive error handling and reporting
- [x] Interactive web documentation with runnable examples

### 🚧 Planned Features
- [ ] User-defined functions with parameters and return values
- [ ] Function overloading and local scoping
- [ ] Arrays and array operations
- [ ] String manipulation functions
- [ ] Objects and structures
- [ ] Enhanced type system with generics
- [ ] Module system and imports
- [ ] Standard library functions
- [ ] `break` and `continue` statements
- [ ] `for...in` and `for...of` loops
- [ ] Exception handling (`try`/`catch`)

## Contributing

This is a learning project, but contributions are welcome! Areas that need work:

1. **Language features** - Implement planned features
2. **Standard library** - Add more built-in functions
3. **Optimization** - Improve LLVM IR generation
4. **Error messages** - Better error reporting with line numbers
5. **Documentation** - Expand examples and tutorials
6. **Testing** - Add comprehensive test suite

## License

MIT License - feel free to use this project for learning and experimentation.

## Acknowledgments

- Built with [LLVM](https://llvm.org/) compiler infrastructure
- Inspired by TypeScript syntax and semantics
- Thanks to the LLVM community for excellent documentation and examples
- Web documentation powered by modern web technologies