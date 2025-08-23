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
- **Built-in types**: `string`, `i32`, `f64`, `boolean`, arrays (`i32[]`), objects
- **Complete arithmetic operations** (`+`, `-`, `*`, `/`, `%`)
- **Comparison operators** (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- **Control flow** with `if`/`else` statements and nesting
- **All loop constructs**: `while`, `for`, `do-while`
- **Variable assignments** and complex expressions
- **Arrays and objects** with access syntax (`arr[index]`, `obj.property`)
- **Array length property** (`arr.length`) for dynamic programming
- **Function calls** (currently `print` and `println` functions)
- **C++ Integration** with 20+ standard library functions
- **Custom C++ Libraries** - Import and use your own C++ code
- **String and numeric literals**
- **Comments** (single-line `//` and multi-line `/* */`)
- **Comprehensive error handling** and reporting
- **LLVM IR generation** for efficient native code compilation
- **VSCode Extension** with syntax highlighting and IntelliSense

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

### 5. Install VSCode Extension (Optional)

For the best development experience, install the Cypescript VSCode extension:

```bash
cd vscode-extension/
./install.sh
```

This provides:
- **Syntax highlighting** for `.csc` files
- **IntelliSense** with auto-completion for all language features
- **Build integration** (`Ctrl+F5` to compile and run, `Ctrl+Shift+F5` for C++ integration)
- **Code snippets** for common patterns and C++ functions
- **Error diagnostics** and hover documentation
- **Function support** (planned feature indicators)

### 6. Manual Usage

#### Basic Compilation
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

#### C++ Integration Compilation
For programs using C++ library functions, use the enhanced compilation script:

```bash
# Compile with C++ integration
./compile-with-cpp.sh example/cpp_integration_basic.csc my_program

# This automatically:
# 1. Compiles the C++ standard library
# 2. Compiles your Cypescript code to LLVM IR
# 3. Links everything together into a native executable
# 4. Creates a ready-to-run program
```

The C++ integration script handles the complete compilation pipeline:
- ✅ **C++ Library Compilation**: Builds the Cypescript standard library
- ✅ **LLVM IR Generation**: Converts Cypescript to optimized IR
- ✅ **Native Linking**: Creates high-performance executables
- ✅ **Automatic Cleanup**: Removes intermediate files

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

### C++ Integration Functions

Cypescript provides seamless integration with C++ through a comprehensive standard library:

#### String Functions
```typescript
let text: string = "Hello World";
let reversed: string = string_reverse(text);        // "dlroW olleH"
let upper: string = string_upper(text);             // "HELLO WORLD"
let lower: string = string_lower(text);             // "hello world"
let length: i32 = string_length(text);              // 11
let substr: string = string_substring(text, 0, 5);  // "Hello"
let pos: i32 = string_find(text, "World");          // 6
let concat: string = string_concat("Hello", " C++"); // "Hello C++"
```

#### Array Functions
```typescript
let numbers: i32[] = [10, 5, 8, 3, 12, 7];
let sum: i32 = array_sum_i32(numbers, numbers.length);  // 45
let max: i32 = array_max_i32(numbers, numbers.length);  // 12
let min: i32 = array_min_i32(numbers, numbers.length);  // 3
```

#### File I/O Functions
```typescript
let success: i32 = file_write("data.txt", "Hello from Cypescript!");
let exists: i32 = file_exists("data.txt");           // 1 (true)
let content: string = file_read("data.txt");         // "Hello from Cypescript!"
```

#### Utility Functions
```typescript
random_seed(42);                                     // Seed random generator
let rand1: i32 = random_int(1, 100);                // Random number 1-100
let rand2: i32 = random_int(1, 100);                // Another random number
```

### Arrays and Objects

```typescript
// Arrays
let numbers: i32[] = [1, 2, 3, 4, 5];
print("First number: ");
println(numbers[0]);

let names: string[] = ["Alice", "Bob", "Charlie"];
println(names);

// Array length property
println("Array length: ");
println(numbers.length); // 5

// Dynamic loops using length
for (let i: i32 = 0; i < numbers.length; i = i + 1) {
    println(numbers[i]);
}

// Objects
let person = { name: "Alice", age: 25, active: true };
print("Name: ");
println(person.name);
print("Age: ");
println(person.age);

// Nested structures
let data = { 
    values: [10, 20, 30], 
    info: { year: 2024, valid: true } 
};
println(data.values[1]); // 20
println(data.info.year); // 2024
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

### Basic Cypescript Programs

For simple programs without C++ integration:

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

### C++ Integration Programs (Recommended)

For programs using C++ library functions, use the automated script:

```bash
# One-command compilation with C++ integration
./compile-with-cpp.sh example/cpp_integration_basic.csc my_program

# Then run the program
./my_program
```

The automated script provides:
- ✅ **Automatic C++ Library Compilation**: Builds the Cypescript standard library
- ✅ **LLVM IR Generation**: Converts Cypescript to optimized IR
- ✅ **Native Linking**: Creates high-performance executables
- ✅ **Error Handling**: Clear error messages and status reporting
- ✅ **Cleanup**: Removes intermediate files automatically

For programs using C++ library functions:

```bash
# One-command compilation with C++ integration
./compile-with-cpp.sh example/cpp_integration_basic.csc my_program

# Then run the program
./my_program
```

#### Manual C++ Integration Steps

If you prefer manual compilation:

```bash
# 1. Compile the C++ standard library
g++ -c src/cypescript_stdlib.cpp -o cypescript_stdlib.o -std=c++11

# 2. Compile Cypescript to LLVM IR
./build/cscript example/cpp_integration_basic.csc

# 3. Compile LLVM IR to object file
llc -filetype=obj -relocation-model=pic output.ll -o cypescript_program.o

# 4. Link with C++ library
clang cypescript_program.o cypescript_stdlib.o -o my_program -lstdc++

# 5. Run the program
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

### Complex Data Structures Example
```typescript
// Company Management System
let company = {
    name: "TechCorp Industries",
    departments: [
        {
            name: "Engineering",
            employees: [
                { 
                    name: "Alice Johnson", 
                    role: "Senior Developer",
                    skills: ["JavaScript", "Python", "React"],
                    projects: ["WebApp", "API Gateway"]
                },
                { 
                    name: "Bob Smith", 
                    role: "DevOps Engineer",
                    skills: ["Docker", "Kubernetes", "AWS"],
                    projects: ["Infrastructure", "CI/CD"]
                }
            ]
        }
    ]
};

// Access nested data
print("Company: ");
println(company.name);
print("Department: ");
println(company.departments[0].name);
print("Lead Developer: ");
println(company.departments[0].employees[0].name);
print("Primary Skill: ");
println(company.departments[0].employees[0].skills[0]);
print("Main Project: ");
println(company.departments[0].employees[0].projects[0]);
```

### C++ Integration Example
```typescript
// Comprehensive C++ Integration Demo
println("=== C++ Integration Demo ===");

// String processing
let text: string = "Hello World";
let reversed: string = string_reverse(text);
let upper: string = string_upper(text);
println("Original: " + text);
println("Reversed: " + reversed);
println("Uppercase: " + upper);

// Array operations
let numbers: i32[] = [10, 5, 8, 3, 12, 7];
let sum: i32 = array_sum_i32(numbers, numbers.length);
let max: i32 = array_max_i32(numbers, numbers.length);
println("Array sum: " + sum);
println("Array max: " + max);

// Custom math functions (with custom C++ library)
let gcd_result: i32 = math_gcd(48, 18);
let fib_result: i32 = math_fibonacci(10);
println("GCD(48,18): " + gcd_result);
println("Fibonacci(10): " + fib_result);

// File I/O
file_write("data.txt", "Hello from Cypescript!");
let content: string = file_read("data.txt");
println("File content: " + content);

// Random numbers
random_seed(42);
let rand: i32 = random_int(1, 100);
println("Random number: " + rand);
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
│   ├── Token.h       # Token definitions
│   └── cypescript_stdlib.cpp # C++ standard library
├── example/
│   ├── README.md     # Example organization guide
│   ├── basic/        # Basic examples (./compile-run.sh)
│   │   ├── hello.csc # Basic variables and printing
│   │   ├── arithmetic.csc # Arithmetic operations
│   │   ├── control_flow.csc # If/else statements
│   │   ├── all_loops.csc # All loop types
│   │   ├── array_length_test.csc # Array operations
│   │   └── comprehensive.csc # Complex algorithms
│   ├── cpp-integration/ # C++ integration examples (./compile-with-cpp.sh)
│   │   ├── cpp_integration_basic.csc # Comprehensive C++ demo
│   │   ├── cpp_integration_simple.csc # Simple C++ demo
│   │   ├── cpp_integration_test.csc # C++ function test suite
│   │   └── custom_math_demo.csc # Custom C++ library demo
│   └── browser-only/ # Browser interpreter examples (./launch-docs.sh)
│       ├── game_system.csc # RPG management system
│       ├── complex_data_structures.csc # E-commerce system
│       └── functions_preview.csc # Preview of planned functions
├── docs/             # Web documentation
│   ├── index.html    # Interactive docs
│   ├── styles.css    # Documentation styling
│   ├── script.js     # UI functionality
│   └── cypescript-interpreter.js # Browser interpreter
├── build.sh          # Build script
├── test.sh           # Test script
├── setup-macos.sh    # macOS setup script
├── launch-docs.sh    # Documentation launcher
├── compile-run.sh    # Basic compilation script
├── compile-with-cpp.sh # C++ integration compiler
├── compile-with-custom-cpp.sh # Custom C++ libraries compiler
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

## C++ Integration

Cypescript provides seamless integration with C++ through a comprehensive standard library, enabling access to the entire C++ ecosystem while maintaining language safety and simplicity.

### Quick Start with C++ Integration

```bash
# Compile a Cypescript program with C++ functions
./compile-with-cpp.sh example/cpp-integration/cpp_integration_basic.csc my_program

# Run the compiled program
./my_program
```

### Custom C++ Libraries

You can easily extend Cypescript with your own C++ libraries:

```bash
# Compile with custom C++ libraries
./compile-with-custom-cpp.sh my_program.csc output src/my_custom_lib.cpp src/another_lib.cpp
```

**Example Custom Library:**
```cpp
// src/my_math_lib.cpp
extern "C" {
    int math_gcd(int a, int b) {
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    int math_fibonacci(int n) {
        if (n <= 1) return n;
        int a = 0, b = 1;
        for (int i = 2; i <= n; i++) {
            int temp = a + b;
            a = b;
            b = temp;
        }
        return b;
    }
}
```

**Use in Cypescript:**
```typescript
let gcd_result: i32 = math_gcd(48, 18);  // Returns 6
let fib_10: i32 = math_fibonacci(10);    // Returns 55
```

### Available C++ Functions

#### String Functions
- `string_reverse(str)` - Reverse a string
- `string_upper(str)` - Convert to uppercase
- `string_lower(str)` - Convert to lowercase
- `string_length(str)` - Get string length
- `string_substring(str, start, length)` - Extract substring
- `string_find(str, substr)` - Find substring position
- `string_concat(str1, str2)` - Concatenate strings

#### Array Functions
- `array_sum_i32(arr, size)` - Sum array elements
- `array_max_i32(arr, size)` - Find maximum element
- `array_min_i32(arr, size)` - Find minimum element

#### File I/O Functions
- `file_read(filename)` - Read file contents
- `file_write(filename, content)` - Write to file
- `file_exists(filename)` - Check if file exists

#### Utility Functions
- `random_seed(seed)` - Seed random generator
- `random_int(min, max)` - Generate random integer
- `random_double()` - Generate random double

### Example Usage

```typescript
// String processing
let text: string = "Hello World";
let reversed: string = string_reverse(text);
println(reversed); // "dlroW olleH"

// Array operations
let numbers: i32[] = [10, 5, 8, 3, 12];
let sum: i32 = array_sum_i32(numbers, numbers.length);
println(sum); // 38

// File operations
file_write("data.txt", "Hello from Cypescript!");
let content: string = file_read("data.txt");
println(content); // "Hello from Cypescript!"

// Random numbers
random_seed(42);
let rand: i32 = random_int(1, 100);
println(rand); // Random number between 1-100
```

### Compilation Process

The C++ integration compilation process:

1. **Compiles C++ standard library** (`src/cypescript_stdlib.cpp`)
2. **Compiles Cypescript to LLVM IR** (your `.csc` file)
3. **Links everything together** into a native executable
4. **Optimizes with LLVM** for maximum performance

### Extending with New Functions

To add new C++ functions:

1. **Add the C++ function** to `src/cypescript_stdlib.cpp` or create a custom library
2. **Declare it in the parser** (`src/Parser.cpp` - `isKnownFunction`)
3. **Add LLVM declaration** (`src/CodeGen.cpp` - `getOrDeclareExternalFunction`)

Example:
```cpp
// In cypescript_stdlib.cpp or custom library
extern "C" {
    int my_function(int x) {
        return x * 2;
    }
}
```

```typescript
// In Cypescript
let result: i32 = my_function(21); // Returns 42
```

**📚 For detailed custom C++ integration guide, see [`CUSTOM_CPP_INTEGRATION.md`](CUSTOM_CPP_INTEGRATION.md)**

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
- [x] Built-in `print` and `println` functions
- [x] String and integer literal support
- [x] Boolean literals (`true`, `false`)
- [x] Single-line (`//`) and multi-line (`/* */`) comments
- [x] Arrays with literal syntax (`[1, 2, 3]`) and access (`arr[index]`)
- [x] Array assignment operations (`arr[index] = value`)
- [x] Array length property (`arr.length`)
- [x] Objects with literal syntax (`{ key: value }`) and access (`obj.property`) - Web only
- [x] Nested data structures (arrays of objects, objects with arrays) - Web only
- [x] **C++ Integration** with 20+ standard library functions
  - [x] String functions (reverse, upper, lower, length, substring, find, concat)
  - [x] Array functions (sum, max, min for i32 arrays)
  - [x] File I/O functions (read, write, exists)
  - [x] Utility functions (random numbers, seeding)
- [x] LLVM IR code generation (for basic features)
- [x] Native executable compilation (for basic features)
- [x] Comprehensive error handling and reporting
- [x] Interactive web documentation with runnable examples

### 🚧 Planned Features
- [ ] **User-defined functions** with parameters and return values
  - [ ] Function declarations: `function add(a: i32, b: i32): i32 { return a + b; }`
  - [ ] Function calls: `let result: i32 = add(5, 3);`
  - [ ] Return statements and type checking
  - [ ] Local variable scoping within functions
  - [ ] Function overloading support
- [ ] **Enhanced string operations**
  - [ ] String concatenation operator (`+`)
  - [ ] Escape sequences (`\n`, `\t`, `\"`, `\\`)
  - [ ] String interpolation/template literals
- [ ] **Advanced control flow**
  - [ ] `break` and `continue` statements in loops
  - [ ] `for...in` and `for...of` loops for arrays
  - [ ] Switch/case statements
- [ ] **Enhanced type system**
  - [ ] Boolean literals (`true`, `false`) in native compiler
  - [ ] Floating-point support (`f64` literals and arithmetic)
  - [ ] Type inference improvements
  - [ ] Generic types and functions
- [ ] **Data structures** (native compiler support)
  - [ ] Objects and structures with property access
  - [ ] Nested data structures
  - [ ] Dynamic arrays and collections
- [ ] **Module system and imports**
  - [ ] File-based modules: `import { function } from "./module.csc"`
  - [ ] Export declarations: `export function myFunc() { }`
  - [ ] Standard library modules
- [ ] **Exception handling**
  - [ ] Try/catch blocks: `try { } catch (error) { }`
  - [ ] Throw statements: `throw "Error message"`
  - [ ] Error types and handling
- [ ] **Standard library expansion**
  - [ ] More built-in functions and utilities
  - [ ] File system operations
  - [ ] Network/HTTP capabilities
  - [ ] JSON parsing and serialization

## Function Implementation Roadmap

### **Phase 1: Basic Function Support**
```typescript
// Function declaration
function add(a: i32, b: i32): i32 {
    return a + b;
}

// Function call
let result: i32 = add(5, 3);
println(result); // 8
```

**Implementation Requirements:**
- [ ] **Lexer**: Add `function` and `return` keywords
- [ ] **Parser**: Parse function declarations and calls
- [ ] **AST**: Add `FunctionDeclarationNode` and `FunctionCallNode`
- [ ] **CodeGen**: Generate LLVM function definitions and calls
- [ ] **Symbol Table**: Track function names and signatures
- [ ] **Type Checking**: Validate parameter and return types

### **Phase 2: Advanced Function Features**
```typescript
// Multiple parameters
function greet(name: string, age: i32): string {
    print("Hello ");
    print(name);
    print(", you are ");
    print(age);
    println(" years old");
    return "greeting_complete";
}

// Void functions
function printHeader(): void {
    println("=== Program Start ===");
}

// Local variables
function calculate(x: i32): i32 {
    let temp: i32 = x * 2;
    let result: i32 = temp + 10;
    return result;
}
```

**Implementation Requirements:**
- [ ] **Local Scoping**: Variables local to function scope
- [ ] **Void Functions**: Functions that don't return values
- [ ] **Multiple Parameters**: Support for 0-N parameters
- [ ] **Stack Management**: Proper LLVM stack frame handling

### **Phase 3: Function Overloading**
```typescript
// Same name, different signatures
function process(value: i32): i32 {
    return value * 2;
}

function process(text: string): string {
    return string_upper(text);
}

// Usage
let num: i32 = process(42);        // Calls i32 version
let str: string = process("hello"); // Calls string version
```

**Implementation Requirements:**
- [ ] **Name Mangling**: Generate unique LLVM function names
- [ ] **Overload Resolution**: Choose correct function based on parameters
- [ ] **Type System**: Enhanced type matching for overloads

### **Phase 4: Recursive Functions**
```typescript
// Factorial example
function factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

let result: i32 = factorial(5); // 120
```

**Implementation Requirements:**
- [ ] **Call Stack**: Proper recursive call handling
- [ ] **Tail Recursion**: Optimization for tail-recursive functions
- [ ] **Stack Overflow**: Protection against infinite recursion

## Contributing

This is a learning project, but contributions are welcome! Areas that need work:

1. **Language features** - Implement planned features (especially functions!)
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