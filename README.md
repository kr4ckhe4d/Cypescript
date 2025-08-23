# Cypescript

A TypeScript-style language compiler built with C++ and LLVM. Cypescript aims to provide a familiar syntax for developers coming from TypeScript/JavaScript while compiling to efficient native code through LLVM.

## Features

- **TypeScript-inspired syntax** with type annotations
- **Variable declarations** with `let`, `const`, and type inference
- **Built-in types**: `string`, `i32`, `f64`, `boolean`
- **Function calls** (currently `print` function)
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

### 4. Manual Usage

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

### Function Calls

```typescript
print("Hello, World!");
print(42);
print(message);
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
│   └── hello.csc     # Example Cypescript code
├── build.sh          # Build script
├── test.sh           # Test script
├── setup-macos.sh    # macOS setup script
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

## Roadmap

### Current Features ✅
- [x] Lexical analysis with comprehensive token support
- [x] Basic parsing for variable declarations and function calls
- [x] LLVM IR code generation
- [x] String and integer literal support
- [x] Type annotations
- [x] Error handling and reporting

### Planned Features 🚧
- [ ] Arithmetic expressions (`+`, `-`, `*`, `/`)
- [ ] Boolean expressions and logic operators
- [ ] Control flow (`if`/`else`, `while`, `for`)
- [ ] Function definitions
- [ ] Arrays and objects
- [ ] Type inference
- [ ] Module system
- [ ] Standard library functions

## Contributing

This is a learning project, but contributions are welcome! Areas that need work:

1. **Parser improvements** - Add support for expressions and control flow
2. **Type system** - Implement proper type checking
3. **Standard library** - Add more built-in functions
4. **Error messages** - Improve error reporting with line numbers
5. **Optimization** - Add LLVM optimization passes

## License

MIT License - feel free to use this project for learning and experimentation.

## Acknowledgments

- Built with [LLVM](https://llvm.org/) compiler infrastructure
- Inspired by TypeScript syntax and semantics
- Thanks to the LLVM community for excellent documentation and examples