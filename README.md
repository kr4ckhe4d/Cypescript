# Cypescript

[![CI](https://github.com/kr4ckhe4d/Cypescript/actions/workflows/ci.yml/badge.svg)](https://github.com/kr4ckhe4d/Cypescript/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/kr4ckhe4d/Cypescript)](https://github.com/kr4ckhe4d/Cypescript/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A TypeScript-style language compiler built with C++ and LLVM. Cypescript aims to provide a familiar syntax for developers coming from TypeScript/JavaScript while compiling to efficient native code through LLVM.

## Install

### macOS (Homebrew)

```bash
brew tap kr4ckhe4d/cypescript
brew install cypescript

# Then compile and run any .csc file:
echo 'println("hello, cypescript!");' > hello.csc
cscript -r hello.csc
```

Requires `clang++` (Xcode Command Line Tools) at compile time.

### Debian / Ubuntu

```bash
sudo apt-get install -y cmake llvm-dev clang
git clone https://github.com/kr4ckhe4d/Cypescript.git && cd Cypescript
bash packaging/build-deb.sh
sudo apt install ./cypescript_*_*.deb
```

(Or download the prebuilt `.deb` artifact from the latest
[CI run](https://github.com/kr4ckhe4d/Cypescript/actions).)

### Arch Linux

```bash
git clone https://github.com/kr4ckhe4d/Cypescript.git
cd Cypescript/packaging/arch
makepkg -si     # builds from the release tarball and runs the test suite
```

Linux support is CI-validated; macOS is the primary development platform.
To build from source on any platform, see [Quick Start](#quick-start).
Maintainers: the full release process is documented in [RELEASING.md](RELEASING.md).

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
- **Full floating-point support**: `f64` literals, arithmetic, comparisons, and automatic i32→f64 promotion
- **Complete arithmetic operations** (`+`, `-`, `*`, `/`, `%`)
- **Compound assignment & increment operators** (`+=`, `-=`, `*=`, `/=`, `%=`, `++`, `--`)
- **Comparison & logical operators** (`==`, `!=`, `<`, `<=`, `>`, `>=`, `&&`, `||`, `!`)
- **Control flow**: `if`/`else`/`else if` chains, `switch`/`case`/`default` (with fallthrough)
- **All loop constructs**: `while`, `for`, `do-while`, `for...of` — with `break` and `continue`
- **Exception handling**: `try`/`catch`/`finally` and `throw`
- **User-defined functions** with parameters, return values, and local scoping
- **Arrow functions & closures** (`(x: i32) => x * 2`, capture-by-value snapshots)
- **Function-type parameters** (`function apply(f: (i32) => i32, x: i32)`) — pass closures to functions
- **Classes** with fields, defaults, constructors, and methods (`new Point(3, 4)`)
- **Callback array methods**: `.map()`, `.filter()`, `.reduce()`, `.find()`, `.forEach()`
- **Semantic analysis pass**: undefined variables, const reassignment, `break`/`continue`
  placement, and function arity — all reported with line/column positions
- **Generic functions and type aliases** (`function bfs<T>(...)`, `type Graph<T> = Map<T, T[]>`)
- **Interfaces** with `extends` and compile-time structural type checking
- **Native TypeScript-style objects** with property access, property assignment, nested objects, and object printing
- **Object methods with `this`** (`add: function(x: i32): i32 { this.value += x; ... }` and shorthand `area(): i32 { ... }`)
- **Object destructuring** (`let { name, age } = user;`)
- **Module system**: `import { x } from "./file";` and `export` (compile-time inlining)
- **JSON integration**: `JSON.stringify(obj)` and `JSON.parse(str)` with native objects
- **Arrays**: literal syntax, index access, `.length`, `.push()`, `.pop()`, `.shift()`
- **Advanced collections** via C++ stdlib: `Map<K,V>`, `Set<T>` with `.get()`, `.set()`, `.has()`, `.add()`
- **String operations**: concatenation (`+`, including numbers), template literals (`` `Hi ${name}` ``), escape sequences
- **`const` keyword** for immutable bindings (reassignment is a compile error)
- **Built-in functions** (`print` and `println`)
- **Comments** (single-line `//` and multi-line `/* */`)
- **AST optimizer**: compile-time constant folding and dead-branch elimination (disable with `--no-fold`)
- **LLVM -O2 native compilation** (3–17x faster than Node.js)
- **C++ integration** with 30+ stdlib functions (strings, arrays, file I/O, JSON, random)
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

### 4. Run Benchmarks

```bash
./benchmarks/run_benchmarks.sh
```

### 5. View Documentation

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

> `cscript` is relocatable: it finds the precompiled runtime
> (`libcypescript.a`) next to its own binary, so you can run it from any
> directory. Set `CYPESCRIPT_HOME=<prefix>` to point at a custom install
> (expects `<prefix>/lib/libcypescript.a`). Producing executables requires
> `clang++` on your PATH (Xcode Command Line Tools on macOS).

```bash
# Compile straight to a native executable (named after the input file)
./build/cscript example/01_hello.csc
./hello

# Compile AND run in one step
./build/cscript -r example/01_hello.csc

# Choose the executable name
./build/cscript -o my_program example/01_hello.csc

# Emit LLVM IR only (give the output a .ll extension)
./build/cscript -o my_output.ll example/01_hello.csc

# Disable the AST optimizer (constant folding / dead branches)
./build/cscript --no-fold example/01_hello.csc

# With verbose output and debugging
./build/cscript -v --print-tokens --print-ast example/01_hello.csc

# Get help
./build/cscript --help
```

`cscript` performs the whole pipeline for you: module resolution (imports) →
lexing → parsing → AST optimization → LLVM IR → `clang++ -O2` link against the
Cypescript stdlib. Exceptions, dynamic arrays, `Map`/`Set`, JSON, and string
helpers are all part of the automatically linked stdlib — no extra flags needed.

#### Manual Pipeline (optional)

Only needed if you want to inspect or post-process the IR yourself:

```bash
# 1. Emit LLVM IR
./build/cscript -o output.ll example/01_hello.csc

# 2. Compile IR + stdlib to an executable
clang++ -O2 output.ll src/cypescript_stdlib.cpp -o my_program -std=c++17

# 3. Run the program
./my_program
```

#### C++ Integration (Advanced)

For programs that need additional functionality, Cypescript provides seamless C++ integration:

```bash
# One-command compilation with C++ integration
./compile-with-cpp.sh example/17_cpp_stdlib.csc my_program

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
- **vs JavaScript (Node.js):** Cypescript is **3x–17x faster** with `-O2` optimizations
- **Simple loops:** 8x faster than Node.js
- **Function-heavy code (Fibonacci):** 3x faster than Node.js
- **Nested loops (Matrix):** 17x faster than Node.js
- **Branch-heavy code (Primes):** 7x faster than Node.js
- **NEON SIMD:** 4x parallel processing validated on Apple Silicon

**Benchmark Results (Cypescript -O2 vs Node.js v22):**
```
Benchmark                    Cypescript     Node.js     Speedup
─────────────────────────────────────────────────────────────────
Simple Loop (100M)              17ms        142ms       8.3x 🔥
Fibonacci (10M calls)           63ms        189ms       3.0x 🔥
Matrix Multiply (300³)           6ms        101ms      16.8x 🔥
Prime Sieve (500K)              13ms         87ms       6.6x 🔥
```

Run benchmarks yourself:
```bash
./benchmarks/run_benchmarks.sh
```

### **Cross-Language Benchmarks (Cypescript vs TypeScript vs Python vs Rust)**

The same algorithm implemented identically in all four languages
(`benchmarks/cross/`), best of 3 wall-clock runs on Apple Silicon
(Node v26, Python 3.14, rustc 1.89):

```
Benchmark                     Cypescript   Node (TS)     Python       Rust
──────────────────────────────────────────────────────────────────────────
Primes < 1M (trial division)     0.051s      0.146s      1.777s     0.050s
Fibonacci fib(35) recursive      0.025s      0.125s      0.621s     0.026s
```

**Cypescript runs within ~2% of Rust, 2.9–5x faster than Node.js on the
identical TypeScript source, and 25–35x faster than Python.**

```bash
bash benchmarks/cross/run_cross_benchmarks.sh   # reproduce (best of 3)
```

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

### Floating-Point Arithmetic

```typescript
let pi: f64 = 3.14159;
let radius: f64 = 2.0;
let area: f64 = pi * radius * radius;   // 12.5664
let mixed: f64 = 2 * pi;                // i32 automatically promotes to f64

function circleArea(r: f64): f64 {
    return 3.14159 * r * r;
}
println(circleArea(3.0));               // 28.2743
```

### Strings and Template Literals

```typescript
let name: string = "Alice";
let age: i32 = 28;

// Concatenation works with strings, integers, floats, and booleans
let msg: string = "Name: " + name + ", age: " + age;

// Template literals with ${expression} interpolation
println(`Hello ${name}, next year you are ${age + 1}!`);
```

### Control Flow

```typescript
let score: i32 = 85;

// if / else if / else chains
if (score >= 90) {
    println("Grade: A");
} else if (score >= 80) {
    println("Grade: B");
} else {
    println("Grade: C or below");
}

// switch/case with fallthrough and default
let day: i32 = 6;
switch (day) {
    case 6:
    case 7:
        println("Weekend");
        break;
    default:
        println("Weekday");
}

// Also works with string conditions
switch (command) {
    case "start": println("starting"); break;
    case "stop":  println("stopping"); break;
    default:      println("unknown");
}
```

### break / continue

```typescript
let total: i32 = 0;
for (let i: i32 = 0; i < 10; i++) {
    if (i == 3) { continue; }  // skip 3
    if (i == 7) { break; }     // stop at 7
    total += i;
}
println(total); // 18
```

### Compound Assignment and Increment

```typescript
let x: i32 = 10;
x += 5;   // 15
x -= 3;   // 12
x *= 2;   // 24
x /= 4;   // 6
x %= 4;   // 2
x++;      // 3
x--;      // 2

// Works on object properties too
counter.value += 10;
```

### Interfaces

```typescript
interface User {
    name: string;
    age: i32;
}

interface Admin extends User {
    level: i32;
}

// Structurally checked at compile time — a missing or mistyped
// property is a compile error.
let user: User = { name: "Alice", age: 28 };
let admin: Admin = { name: "Bob", age: 35, level: 9 };
```

### Object Methods and `this`

```typescript
let calculator = {
    value: 0,
    add: function(x: i32): i32 {
        this.value = this.value + x;
        return this.value;
    },
    reset(): void {          // shorthand method syntax
        this.value = 0;
    }
};

calculator.add(5);           // 5
calculator.add(7);           // 12
calculator.reset();

// Direct property assignment
calculator.value = 42;
```

### Object Destructuring

```typescript
let user = { name: "Alice", age: 28, active: true };
let { name, age } = user;
println(name);  // Alice
println(age);   // 28
```

### Exception Handling

```typescript
function risky(n: i32): i32 {
    if (n < 0) {
        throw "negative input: " + n;
    }
    return n * 2;
}

try {
    println(risky(21));   // 42
    println(risky(-1));   // throws
} catch (e) {
    println("caught: " + e);
} finally {
    println("cleanup always runs");
}
```

### Modules (import / export)

```typescript
// math_utils.csc
export function square(x: i32): i32 {
    return x * x;
}
export const GREETING: string = "Hello";

// main.csc
import { square, GREETING } from "./math_utils";
println(square(7)); // 49
```

Imports are resolved at compile time by inlining each module once (cycles are detected and broken automatically).

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

## Running TypeScript Scripts (Compatibility)

Cypescript's goal is to run existing TypeScript scripts natively with only
minute changes. The following TypeScript constructs work **as-is**:

- `console.log(...)` / `console.error(...)` — including multiple arguments
  (`console.log("x =", x)`)
- `===` and `!==` (treated as `==` / `!=`)
- `Math.sqrt`, `Math.pow`, `Math.abs`, `Math.floor`, `Math.sin`, `Math.cos`, `Math.log`, `Math.exp`
- `let` / `const`, type annotations, `number` (compiles to `f64`), `string`, `boolean`
- Interfaces, generics, type aliases, template literals, destructuring
- `Map` / `Set`, arrays with `.push()` / `.pop()` / `.shift()` / `.length`, `for...of`
- `try` / `catch` / `finally` / `throw`, `switch`, `break` / `continue`
- Object literals with methods and `this`

- Arrow functions and closures (`x => x * 2`), including `.map`/`.filter`/`.reduce`/
  `.find`/`.forEach` with arrow callbacks — note: captures are **by-value snapshots**
  (capture an object to share mutable state, e.g. `let s = {n: 0}; () => s.n += 1`)

Typical minute changes when porting a `.ts` file:

- Classes with fields, constructors, and methods (`class Point { ... }`, `new Point(3, 4)`)
- Function-type parameters (`function apply(f: (i32) => i32, x: i32)`)

| TypeScript | Cypescript |
|---|---|
| `x++` as an *expression* (`arr[i++]`) | statement-level `x++` only |
| closures mutating captured primitives | capture an object instead (by-value snapshots) |
| `class A extends B` | flat classes only (no inheritance yet) |
| `import` from npm packages | only local `./file.csc` module imports |
| `number` for integer loops (slow) | use `i32` for integer math (fast) |

See `example/18_bfs_graph.csc` vs `example/18_bfs_graph.ts` — the BFS algorithm
is byte-for-byte almost identical, and both produce identical output.

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
│   ├── main.cpp      # Compiler entry point + module resolution
│   ├── Lexer.cpp/h   # Lexical analysis (incl. template literals)
│   ├── Parser.cpp/h  # Syntax analysis
│   ├── AST.h         # Abstract Syntax Tree
│   ├── CodeGen.cpp/h # LLVM IR generation
│   ├── Optimizer.cpp/h # AST constant folding + dead-branch elimination
│   ├── ObjectOptimizer.cpp/h # Direct struct property access (Phase 1)
│   ├── Token.h       # Token definitions
│   └── cypescript_stdlib.cpp # C++ standard library + exception runtime
├── tests/
│   ├── run_tests.sh  # Test suite runner
│   ├── test_variables.csc    # Variable declarations
│   ├── test_arithmetic.csc   # Arithmetic operations
│   ├── test_if_else.csc      # Control flow
│   ├── test_control_flow.csc # break/continue, else-if, switch
│   ├── test_for.csc          # For loops
│   ├── test_while.csc        # While loops
│   ├── test_do_while.csc     # Do-while loops
│   ├── test_floats.csc       # f64 arithmetic
│   ├── test_strings.csc      # Concatenation + template literals
│   ├── test_compound_assign.csc # +=, -=, ++, --
│   ├── test_functions.csc    # User-defined functions
│   ├── test_arrays.csc       # Arrays and array.length
│   ├── test_objects.csc      # Native objects
│   ├── test_interfaces.csc   # Interfaces + structural typing
│   ├── test_methods.csc      # Object methods and `this`
│   ├── test_destructuring.csc # Object destructuring
│   ├── test_exceptions.csc   # try/catch/finally/throw
│   ├── test_modules.csc      # import/export
│   └── modules/              # Helper modules for module tests
├── benchmarks/
│   ├── run_benchmarks.sh     # Benchmark suite runner
│   ├── benchmark_simple.csc  # Simple loop (100M iterations)
│   ├── benchmark_simple.ts   # Node.js comparison
│   ├── bench_fibonacci.csc   # Fibonacci (10M function calls)
│   ├── bench_fibonacci.ts    # Node.js comparison
│   ├── bench_matrix.csc      # Matrix multiply (300³ nested loops)
│   ├── bench_matrix.ts       # Node.js comparison
│   ├── bench_primes.csc      # Prime sieve (500K)
│   └── bench_primes.ts       # Node.js comparison
├── example/          # Guided tour, easiest → most complex
│   ├── README.md            # How to run + progression table
│   ├── 01_hello.csc         # print/println/console.log
│   ├── 02_variables.csc     # types, let/const, inference
│   ├── ...                  # operators, control flow, loops, functions,
│   │                        # strings, arrays, objects, interfaces,
│   │                        # methods/this, destructuring, exceptions
│   ├── 14_modules/          # import/export (run main.csc)
│   ├── 15_json.csc          # JSON.stringify/parse
│   ├── 16_typescript_compat.csc # near-plain TypeScript
│   ├── 17_cpp_stdlib.csc    # C++ stdlib integration
│   └── 18_bfs_graph.csc/.ts # capstone: BFS in both languages
├── docs/             # Web documentation
├── build.sh          # Build script
├── test.sh           # Test script (runs tests/run_tests.sh)
├── setup-macos.sh    # macOS setup script
├── launch-docs.sh    # Documentation launcher
├── compile-run.sh    # Basic compilation script
├── compile-with-cpp.sh # C++ integration compiler
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
./build/cscript example/01_hello.csc
```

### Debugging

Enable verbose output to see compilation stages:

```bash
./build/cscript -v --print-tokens --print-ast example/01_hello.csc
```

This will show:
- **Lexical Analysis**: All tokens generated
- **Syntax Analysis**: Abstract Syntax Tree
- **Code Generation**: LLVM IR output
- **Timing Information**: Performance metrics

## Requirements

- **macOS** (Intel or Apple Silicon)
- **CMake** 3.15+
- **LLVM** (any recent version, installed via Homebrew) — build-time only
- **clang++** (Xcode Command Line Tools) — required at compile time to link executables
- **Python 3** (for web documentation)

### Installing system-wide (optional)

```bash
cmake --install build --prefix /usr/local   # installs bin/cscript + lib/libcypescript.a
cscript --version
```

A Homebrew formula template lives in `packaging/cypescript.rb` for publishing
releases, and `.github/workflows/ci.yml` builds and tests every push.

## C++ Integration

Cypescript provides seamless integration with C++ through a comprehensive standard library, enabling access to the entire C++ ecosystem while maintaining language safety and simplicity.

### Quick Start with C++ Integration

```bash
# Compile a Cypescript program with C++ functions
./compile-with-cpp.sh example/17_cpp_stdlib.csc my_program

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

## 🔧 C++ Integration (Advanced)

For programs that need additional functionality beyond native TypeScript features, Cypescript provides seamless C++ integration with 30+ standard library functions.

### **Quick Start with C++ Integration**

```bash
# Compile a Cypescript program with C++ functions
./compile-with-cpp.sh example/17_cpp_stdlib.csc my_program

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
- [x] Variable declarations (`let`, `const`) with type annotations
- [x] Variable assignments with type checking
- [x] All arithmetic operators (`+`, `-`, `*`, `/`, `%`)
- [x] Compound assignment (`+=`, `-=`, `*=`, `/=`, `%=`) and increment/decrement (`++`, `--`)
- [x] All comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- [x] Logical operators (`&&`, `||`) with short-circuit evaluation
- [x] Unary operators (`!`, `-`)
- [x] Control flow with `if`/`else` statements and nesting
- [x] `else if` chains
- [x] Switch/case statements with fallthrough, `default`, and string conditions
- [x] `break` and `continue` statements in all loops (and `break` in switch)
- [x] While loops with complex conditions
- [x] Traditional for loops (`for (init; condition; increment)`, including `i++` and `i += n`)
- [x] Do-while loops (post-condition loops)
- [x] `for...of` loops for arrays and collections
- [x] Nested loops of all types
- [x] Floating-point arithmetic (`f64` literals, operations, comparisons, i32→f64 promotion)
- [x] Built-in `print` and `println` functions (strings, i32, f64, booleans, objects)
- [x] String literals with escape sequences (`\n`, `\t`, `\\`, `\"`)
- [x] String concatenation with `+` operator (strings, integers, floats, booleans)
- [x] String interpolation / template literals (`` `Hello ${name}` ``)
- [x] Integer and boolean literal support (`true`, `false`)
- [x] Single-line (`//`) and multi-line (`/* */`) comments
- [x] Arrays with literal syntax (`[1, 2, 3]`), index access, and assignment
- [x] Array `.length` property
- [x] Dynamic arrays with `.push()`, `.pop()`, and `.shift()` (via C++ stdlib)
- [x] Native TypeScript-style objects with property access (`obj.property`)
- [x] Object property assignment (`obj.property = value`)
- [x] Object methods and `this` keyword (function-expression and shorthand syntax)
- [x] Object destructuring (`let { name, age } = user`)
- [x] Interface definitions with `extends` and compile-time structural checking
- [x] Nested objects (`company.employee.name`)
- [x] Object printing (`println(obj)` outputs JSON representation)
- [x] `JSON.stringify(obj)` and `JSON.parse(str)` with native objects
- [x] `const` keyword for immutable bindings
- [x] User-defined functions with parameters, return values, and local scoping
- [x] Void functions and nested function calls
- [x] Generic functions (`function bfs<T>(...)`)
- [x] Type aliases (`type Graph<T> = Map<T, T[]>`)
- [x] `Map<K,V>` and `Set<T>` collections (via C++ stdlib)
- [x] Non-null assertion operator (`!`)
- [x] `new` expressions (`new Set<T>()`, `new Map<K,V>()`)
- [x] Method calls (`.get()`, `.set()`, `.has()`, `.add()`)
- [x] Exception handling (`try` / `catch` / `finally` / `throw`)
- [x] Module system (`import { x } from "./file"` / `export`, compile-time inlining)
- [x] AST constant folding and dead-branch elimination (`--no-fold` to disable)
- [x] LLVM IR code generation with `-O2` optimizations
- [x] Native executable compilation (3–17x faster than Node.js)
- [x] C++ integration with 30+ stdlib functions
- [x] Comprehensive error handling and reporting
- [x] Interactive web documentation with runnable examples

- [x] Arrow functions and closures (`(x: i32) => x * 2`; captures are by-value snapshots)
- [x] Function-type parameters (`f: (i32) => i32`) — closures passed to regular functions
- [x] Classes: fields with defaults, constructors, methods, `new`, class-name type annotations
- [x] Callback array methods: `.map()`, `.filter()`, `.reduce()`, `.find()`, `.forEach()`
- [x] `f64[]` arrays (literals, indexing, push/pop/shift, for...of, callback methods)
- [x] Line/column numbers in lexer/parser error messages
- [x] Semantic analysis pass (undefined vars, const reassignment, break/continue placement,
      function arity) with line/column positions

### 🚧 Planned Features
- [ ] Class inheritance (`extends` between classes) and `instanceof`
- [ ] Union types (`string | i32`) and type guards
- [ ] By-reference closure captures (currently by-value; capture an object for shared state)
- [ ] Full type checker (property/type mismatches currently surface at codegen without positions)
- [ ] Reference-counted heap objects (escape-safe object returns)
- [ ] JIT compilation and profile-guided runtime optimization (see OPTIMIZATION_ROADMAP.md)

## Contributing

This is a learning project, but contributions are welcome! Areas that need work:

1. **Language features** - Implement planned features (`break`/`continue`, `else if`, `switch`, interfaces)
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