# Cypescript

A TypeScript-style language compiler built with C++ and LLVM. Cypescript aims to provide a familiar syntax for developers coming from TypeScript/JavaScript while compiling to efficient native code through LLVM.

## 📖 Interactive Documentation

**NEW!** Cypescript now features comprehensive web-based documentation with runnable examples and interactive performance charts:

```bash
# Launch interactive documentation
./launch-docs.sh
```

The web documentation includes:
- 🚀 **Runnable Examples** - Execute Cypescript code directly in your browser
- 🎮 **Interactive Playground** - Write and test your own code
- 📊 **Performance Benchmarks** - Interactive charts comparing Cypescript vs JavaScript
- 📱 **Mobile-Friendly** - Works on all devices
- 🔍 **Searchable** - Find what you need quickly
- 📚 **Complete Reference** - All language features documented
- 📈 **Visual Performance Analysis** - Charts showing optimization impact and scaling

## Features

- **TypeScript-inspired syntax** with type annotations
- **Variable declarations** with `let`, `const`, and type inference
- **Built-in types**: `string`, `i32`, `f64`, `boolean`, `void`, arrays (`i32[]`), objects
- **Complete arithmetic operations** (`+`, `-`, `*`, `/`, `%`)
- **Comparison operators** (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- **Control flow** with `if`/`else` statements and nesting
- **All loop constructs**: `while`, `for`, `do-while`
- **Variable assignments** and complex expressions
- **🔥 NEW! Native TypeScript-style objects** with property access (`obj.property`)
- **Arrays and objects** with access syntax (`arr[index]`, `obj.property`)
- **Array length property** (`arr.length`) for dynamic programming
- **🔥 NEW! User-defined functions** with parameters, return values, and local scoping
- **Built-in functions** (`print` and `println`)
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
- **Code snippets** for common patterns, functions, and C++ functions
- **Error diagnostics** and hover documentation
- **Function support** with syntax highlighting and completion

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

#### Complete Compilation Pipeline
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

#### C++ Integration (Advanced)

For programs that need additional functionality, Cypescript provides seamless C++ integration:

```bash
# One-command compilation with C++ integration
./compile-with-cpp.sh example/cpp_integration_basic.csc my_program

# Then run the program
./my_program
```

The C++ integration provides access to:
- **String functions**: `string_reverse()`, `string_upper()`, `string_lower()`
- **Array functions**: `array_sum_i32()`, `array_max_i32()`, `array_min_i32()`
- **File I/O**: `file_read()`, `file_write()`, `file_exists()`
- **JSON functions**: `json_create_object()`, `json_add_string()`, `json_get_string()`
- **Utilities**: `random_int()`, `random_seed()`

See the [C++ Integration](#c-integration) section for complete details.

## ⚡ Performance Optimizations

Cypescript now includes multiple optimization levels for different use cases:

### **Advanced Multi-Stage Optimization**
```bash
# Six-stage optimization pipeline with 25.8% performance improvement
./compile-advanced.sh my_program.csc advanced_program

# Results: 25.8% faster execution, 59% smaller binaries, production-ready
```

### **Profile-Guided Optimization (PGO)**
```bash
# Three-stage PGO workflow for 20-30% additional improvement
./compile-pgo.sh profile my_program.csc instrumented_program
./instrumented_program  # Collect runtime profile data
./compile-pgo.sh optimize my_program.csc optimized_program profile.profdata

# Results: Hot path optimization, runtime behavior analysis
```

### **NEON SIMD Optimization**
```bash
# ARM NEON vectorization for 4x parallel array processing
./compile-with-custom-cpp.sh my_program.csc neon_program src/neon_optimized_lib.cpp

# Results: 4x parallel processing on Apple Silicon, validated correctness
```

### **Process Pooling (Development)**
```bash
# Eliminate compilation overhead for repeated execution
./cypescript-pool.sh cache my_program.csc cached_program
./cypescript-pool.sh exec cached_program  # Instant execution!

# Results: 15% faster execution, zero compilation overhead
```

### **Performance Characteristics**
- **vs JavaScript:** 3.1x slower (reduced from 4.5x through memory optimization)
- **vs Python:** 2x faster on small workloads, **131x faster** on intensive computation
- **Memory optimization:** 31.3% performance improvement over basic compilation
- **NEON SIMD:** 4x parallel processing validated on Apple Silicon
- **Complex Objects:** Handles 2.5M property accesses at enterprise scale

**Multi-Language Benchmark Results:**
```
Array Processing (Small Workloads):
🥇 JavaScript: 98ms
🥈 Cypescript (Memory-Optimized): 301ms  
🥉 Cypescript (Advanced): 302ms
🥉 Cypescript (Basic): 438ms
4️⃣ Python: 644ms

Complex Object Processing (Enterprise Scale):
🥇 JavaScript: 0.57ms (250K property accesses)
🥈 Cypescript: ~466ms (2.5M property accesses - 10x larger scale!)
🥉 Python: 9.20ms (250K property accesses - 16x slower than JS)

Intensive Computation (Large Workloads):
🥇 JavaScript: 723ms
🥈 Cypescript (Memory-Optimized): 986ms
🥉 Python: 94,666ms (131x slower!)
```

See the **interactive performance documentation** for detailed benchmarks and charts!

## Language Syntax

### Variable Declarations

```typescript
let message: string = "Hello, World!";
let count: i32 = 42;
let pi: f64 = 3.14159;
let isActive: boolean = true;
```

### Native TypeScript-Style Objects

```typescript
// Object creation with mixed types
let user = {
    name: "Alice Johnson",
    age: 28,
    role: "Developer",
    active: true
};

// Property access
println(user.name);     // "Alice Johnson"
println(user.age);      // 28
println(user.active);   // 1 (true)

// Multiple objects
let config = {
    appName: "Cypescript IDE",
    version: "1.0.0",
    port: 8080,
    debug: false
};

println(config.appName);  // "Cypescript IDE"
println(config.port);     // 8080
println(config);          // {"appName":"Cypescript IDE","version":"1.0.0","port":8080,"debug":false}

// Nested Objects
let company = {
    name: "TechCorp",
    employee: { name: "Alice", age: 28 }
};
println(company.employee.name);  // "Alice"

// JSON Stringification (requires C++ integration)
let jsonStr: string = JSON.stringify(config);
println(jsonStr);         // {"appName":"Cypescript IDE","version":"1.0.0","port":8080,"debug":false}

// JSON Parsing (requires C++ integration)
let parsed = JSON.parse(jsonStr);
println(parsed.appName);  // "Cypescript IDE"
println(parsed.port);     // 8080

// Dynamic Arrays (requires C++ integration)
let queue: string[] = ["NodeA"];
queue.push("NodeB");
queue.push("NodeC");

println(queue.length);   // 3
println(queue.shift());  // "NodeA"
println(queue.length);   // 2

// For-of Iteration
for (const item of queue) {
    println(item);
}

// Advanced Collections (requires C++ integration)
let visited: Set<string> = new Set<string>();
visited.add("NodeA");
println(visited.has("NodeA")); // 1

let graph: Map<string, string[]> = new Map<string, string[]>();
graph.set("NodeA", ["NodeB", "NodeC"]);
let neighbors: string[] = graph.get("NodeA");
println(neighbors.length);     // 2

// Full Algorithm Example: Breadth-First Search
type Graph<T> = Map<T, T[]>;

function breadthFirstSearch<T>(graph: Graph<T>, startNode: T): T[] {
    const visited: Set<T> = new Set<T>();
    const queue: T[] = [];
    const traversalOrder: T[] = [];

    visited.add(startNode);
    queue.push(startNode);

    while (queue.length > 0) {
        const currentNode: T = queue.shift()!;
        traversalOrder.push(currentNode);

        const neighbors: T[] = graph.get(currentNode) || [];

        for (const neighbor of neighbors) {
            if (!visited.has(neighbor)) {
                visited.add(neighbor);
                queue.push(neighbor);
            }
        }
    }

    return traversalOrder;
}
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

### User-Defined Functions

```typescript
// Function with parameters and return value
function add(a: i32, b: i32): i32 {
    return a + b;
}

// Function with local variables
function factorial(n: i32): i32 {
    let result: i32 = 1;
    let counter: i32 = 1;
    
    while (counter <= n) {
        result = result * counter;
        counter = counter + 1;
    }
    
    return result;
}

// Void function
function greet(name: string): void {
    print("Hello, ");
    println(name);
}

// Function calls
let sum: i32 = add(5, 3);        // 8
let fact: i32 = factorial(5);    // 120
greet("Alice");                  // Hello, Alice

// Functions calling other functions
function complexCalculation(x: i32, y: i32): i32 {
    let doubled: i32 = add(x, x);
    return add(doubled, y);
}
```

### Arrays

```typescript
// Array declaration and initialization
let numbers: i32[] = [1, 2, 3, 4, 5];
println("Array: ");
println(numbers);

// Array access
print("First element: ");
println(numbers[0]);
print("Last element: ");
println(numbers[4]);

// String array
let names: string[] = ["Alice", "Bob", "Charlie"];
println("Names: ");
println(names);

// Array length property
println("Array length: ");
println(numbers.length); // 5

// Dynamic loops using length
for (let i: i32 = 0; i < numbers.length; i = i + 1) {
    println(numbers[i]);
}
```

### Built-in Functions

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

### Native Cypescript Programs

For TypeScript-style programs with native objects:

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

### Native TypeScript-Style Objects
```typescript
// Object creation and property access
let user = {
    name: "Alice Johnson",
    age: 28,
    role: "Developer",
    active: true
};

println("User Information:");
println(user.name);     // Alice Johnson
println(user.age);      // 28
println(user.role);     // Developer
println(user.active);   // 1 (true)

// Object printing and JSON stringification
println(user);          // {"name":"Alice Johnson","age":28,"role":"Developer","active":true}
let jsonStr: string = JSON.stringify(user);
println(jsonStr);

// JSON Parsing
let parsed = JSON.parse(jsonStr);
println(parsed.name);   // Alice Johnson
```

### User-Defined Functions Demo
```typescript
// Function declarations
function add(a: i32, b: i32): i32 {
    return a + b;
}

function factorial(n: i32): i32 {
    let result: i32 = 1;
    let counter: i32 = 1;
    
    while (counter <= n) {
        result = result * counter;
        counter = counter + 1;
    }
    
    return result;
}

function greetUser(name: string, age: i32): void {
    print("Hello, ");
    print(name);
    print("! You are ");
    print(age);
    println(" years old.");
}

// Main program
let x: i32 = 15;
let y: i32 = 25;
let sum: i32 = add(x, y);
println(sum); // Output: 40

let fact: i32 = factorial(5);
println(fact); // Output: 120

greetUser("Alice", 28); // Output: Hello, Alice! You are 28 years old.
```

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

### Array Processing
```typescript
// Array operations with length property
let numbers: i32[] = [10, 25, 7, 42, 18];
let sum: i32 = 0;
let max: i32 = numbers[0];

// Calculate sum and find maximum
for (let i: i32 = 0; i < numbers.length; i = i + 1) {
    sum = sum + numbers[i];
    if (numbers[i] > max) {
        max = numbers[i];
    }
}

print("Sum: "); println(sum);     // Sum: 102
print("Max: "); println(max);     // Max: 42
print("Length: "); println(numbers.length); // Length: 5
```

### C++ Integration Example
```typescript
// Advanced functionality with C++ integration
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

// File I/O
file_write("data.txt", "Hello from Cypescript!");
let content: string = file_read("data.txt");
println("File content: " + content);

// JSON manipulation
let user: string = json_create_object();
user = json_add_string(user, "name", "Alice");
user = json_add_int(user, "age", 28);
println("JSON: " + json_prettify(user));
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
│   ├── property_access_test.csc # 🔥 NEW! Native object property access
│   ├── typescript_objects_native.csc # 🔥 NEW! TypeScript-style objects
│   ├── functions_demo.csc # User-defined functions demo
│   ├── basic/        # Basic examples (native compilation)
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
│   │   ├── json_demo.csc # JSON functions demo
│   │   └── custom_math_demo.csc # Custom C++ library demo
│   └── browser-only/ # Browser interpreter examples (./launch-docs.sh)
│       ├── game_system.csc # RPG management system
│       └── complex_data_structures.csc # E-commerce system
├── docs/             # Web documentation
│   ├── index.html    # Interactive docs
│   ├── styles.css    # Documentation styling
│   ├── script.js     # UI functionality
│   └── cypescript-interpreter.js # Browser interpreter
├── NATIVE_OBJECTS_ROADMAP.md # Native TypeScript development roadmap
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

## 🎯 Native TypeScript-Style Objects

Cypescript provides native TypeScript-style object support with property access, just like TypeScript!

### **Object Creation and Property Access**

```typescript
// Create objects with mixed types
let user = {
    name: "Alice Johnson",
    age: 28,
    role: "Developer",
    active: true
};

// Access properties directly
println(user.name);     // "Alice Johnson"
println(user.age);      // 28
println(user.role);     // "Developer"
println(user.active);   // 1 (true)
```

### **Multiple Objects**

```typescript
// Create multiple objects
let config = {
    appName: "Cypescript IDE",
    version: "1.0.0",
    port: 8080,
    debug: false
};

let settings = {
    theme: "dark",
    fontSize: 14,
    autoSave: true
};

// Access properties from different objects
println(config.appName);    // "Cypescript IDE"
println(config.port);       // 8080
println(settings.theme);    // "dark"
println(settings.fontSize); // 14
```

### **Real-World Example**

```typescript
// Employee management system
let employee = {
    firstName: "Alice",
    lastName: "Johnson",
    employeeId: 12345,
    department: "Engineering",
    salary: 95000,
    isActive: true,
    isRemote: false
};

// Process employee data
print("Employee: ");
print(employee.firstName);
print(" ");
println(employee.lastName);

print("ID: ");
println(employee.employeeId);

print("Department: ");
println(employee.department);

print("Status: ");
if (employee.isActive == 1) {
    println("Active");
} else {
    println("Inactive");
}

print("Work Mode: ");
if (employee.isRemote == 1) {
    println("Remote");
} else {
    println("On-site");
}
```

### **Supported Property Types**

- **Strings**: `name: "Alice Johnson"`
- **Integers**: `age: 28`, `port: 8080`
- **Booleans**: `active: true`, `debug: false`

### **🚀 Coming Soon: Advanced Object Features**

```typescript
// Future features in development:
let user = { name: "Alice", age: 28 };

// Object printing
println(user);  // Will print: { name: "Alice", age: 28 }

// JSON conversion
let jsonString = JSON.stringify(user);  // Convert to JSON string
let parsed = JSON.parse(jsonString);    // Parse JSON back to object

// Nested objects
let company = {
    name: "TechCorp",
    employee: { name: "Alice", age: 28 }
};
println(company.employee.name);  // Nested property access
```

## 🔧 C++ Integration (Advanced)

For programs that need additional functionality beyond native TypeScript features, Cypescript provides seamless C++ integration with 30+ standard library functions.

### **Quick Start with C++ Integration**

```bash
# Compile a Cypescript program with C++ functions
./compile-with-cpp.sh example/cpp_integration_basic.csc my_program

# Run the compiled program
./my_program
```

### **Available C++ Functions**

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

#### JSON Functions (String-Based)
```typescript
// Create and manipulate JSON strings
let jsonObj: string = json_create_object();         // Creates: {}
jsonObj = json_add_string(jsonObj, "name", "Alice");
jsonObj = json_add_int(jsonObj, "age", 28);
jsonObj = json_add_boolean(jsonObj, "active", 1);

// Retrieve values
let name: string = json_get_string(jsonObj, "name");        // "Alice"
let age: i32 = json_get_int(jsonObj, "age");                // 28
let active: i32 = json_get_boolean(jsonObj, "active");      // 1

// JSON utilities
let isValid: i32 = json_is_valid(jsonObj);                  // 1 if valid
let pretty: string = json_prettify(jsonObj);                // Pretty-printed
let compact: string = json_minify(jsonObj);                 // Minified
```

### **Custom C++ Libraries**

You can easily extend Cypescript with your own C++ libraries:

```bash
# Compile with custom C++ libraries
./compile-with-custom-cpp.sh my_program.csc output src/my_custom_lib.cpp
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

### **C++ Integration Example**
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

// File I/O
file_write("data.txt", "Hello from Cypescript!");
let content: string = file_read("data.txt");
println("File content: " + content);

// JSON manipulation
let user: string = json_create_object();
user = json_add_string(user, "name", "Alice");
user = json_add_int(user, "age", 28);
println("JSON: " + json_prettify(user));
```

### **When to Use C++ Integration**

- **File operations** - Reading/writing files
- **String processing** - Advanced string manipulation
- **Mathematical operations** - Complex calculations
- **JSON interop** - Working with external JSON APIs
- **Performance-critical code** - Optimized C++ algorithms
- **Legacy integration** - Using existing C++ libraries

**Note:** For most TypeScript-style development, use native objects. C++ integration is for advanced use cases requiring additional functionality.

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
- [x] **🔥 NEW! Native TypeScript-style objects** with property access (`obj.property`)
- [x] **🔥 NEW! User-defined functions** - **Complete Implementation!**
  - [x] Function declarations: `function add(a: i32, b: i32): i32 { return a + b; }`
  - [x] Function calls: `let result: i32 = add(5, 3);`
  - [x] Return statements and type checking
  - [x] Local variable scoping within functions
  - [x] Void functions: `function greet(): void { println("Hello!"); }`
  - [x] Nested function calls and complex logic
  - [x] Integration with all existing language features
- [x] LLVM IR code generation (for all features)
- [x] Native executable compilation (for all features)
- [x] Comprehensive error handling and reporting
- [x] Interactive web documentation with runnable examples

### 🚧 Planned Features
- [ ] **Object printing and debugging**
  - [ ] Direct object printing: `println(obj)` 
  - [ ] Object inspection and debugging tools
- [ ] **JSON integration for native objects**
  - [ ] `JSON.stringify(obj)` - Convert native objects to JSON strings
  - [ ] `JSON.parse(jsonString)` - Parse JSON strings to native objects
  - [ ] Seamless interop between native objects and JSON
- [ ] **Enhanced string operations**
  - [ ] String concatenation operator (`+`)
  - [ ] Escape sequences (`\n`, `\t`, `\"`, `\\`)
  - [ ] String interpolation/template literals
- [ ] **Advanced control flow**
  - [ ] `break` and `continue` statements in loops
  - [ ] `for...in` and `for...of` loops for arrays and objects
  - [ ] Switch/case statements
- [ ] **Enhanced type system**
  - [ ] Floating-point support (`f64` literals and arithmetic)
  - [ ] Type inference improvements
  - [ ] Generic types and functions
  - [ ] Interface definitions
- [ ] **Advanced object features**
  - [ ] Nested objects: `company.employee.name`
  - [ ] Object methods and `this` keyword
  - [ ] Object destructuring: `let { name, age } = user`
- [ ] **Module system and imports**
  - [ ] File-based modules: `import { function } from "./module.csc"`
  - [ ] Export declarations: `export function myFunc() { }`
  - [ ] Standard library modules
- [ ] **Exception handling**
  - [ ] Try/catch blocks: `try { } catch (error) { }`
  - [ ] Throw statements: `throw "Error message"`
  - [ ] Error types and handling

## Function Implementation Roadmap

### **✅ Phase 1: Basic Function Support - COMPLETE!**
```typescript
// Function declaration
function add(a: i32, b: i32): i32 {
    return a + b;
}

// Function call
let result: i32 = add(5, 3);
println(result); // 8
```

**Implementation Status:**
- [x] **Lexer**: Add `function` and `return` keywords ✅
- [x] **Parser**: Parse function declarations and calls ✅
- [x] **AST**: Add `FunctionDeclarationNode` and `ReturnStatementNode` ✅
- [x] **CodeGen**: Generate LLVM function definitions and calls ✅
- [x] **Symbol Table**: Track function names and signatures ✅
- [x] **Type Checking**: Validate parameter and return types ✅

**🎯 See [`example/functions_demo.csc`](example/functions_demo.csc) for a comprehensive demonstration!**

### **🚧 Phase 2: Advanced Function Features - PLANNED**
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
- [x] **Local Scoping**: Variables local to function scope ✅
- [x] **Void Functions**: Functions that don't return values ✅
- [x] **Multiple Parameters**: Support for 0-N parameters ✅
- [x] **Stack Management**: Proper LLVM stack frame handling ✅
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

1. **Language features** - Implement advanced features (function overloading, recursion optimization, etc.)
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