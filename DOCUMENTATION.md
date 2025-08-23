# Cypescript Language Reference

**Version:** 1.0  
**Last Updated:** August 2025  
**Target Platform:** Native (via LLVM)

## Table of Contents

1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Language Syntax](#language-syntax)
4. [Data Types](#data-types)
5. [Variables and Constants](#variables-and-constants)
6. [Operators](#operators)
7. [Control Flow Statements](#control-flow-statements)
8. [Loop Statements](#loop-statements)
9. [Functions](#functions)
10. [Comments](#comments)
11. [Compiler Reference](#compiler-reference)
12. [Error Handling](#error-handling)
13. [Examples](#examples)
14. [Language Grammar](#language-grammar)

---

## Overview

Cypescript is a TypeScript-inspired programming language that compiles to efficient native code through LLVM. It combines familiar TypeScript syntax with static typing and high-performance compilation, making it ideal for systems programming while maintaining developer-friendly syntax.

### Key Features

- **TypeScript-inspired syntax** with explicit type annotations
- **Static typing** with compile-time type checking
- **LLVM-based compilation** for optimal performance
- **Native code generation** with no runtime dependencies
- **Comprehensive control flow** including all major loop constructs
- **Memory-efficient** variable management
- **Cross-platform** compilation support

### Design Philosophy

Cypescript bridges the gap between high-level language expressiveness and low-level performance. It provides:

- **Familiar syntax** for JavaScript/TypeScript developers
- **Predictable performance** through ahead-of-time compilation
- **Type safety** without runtime overhead
- **Zero-cost abstractions** where possible

---

## Getting Started

### System Requirements

- **Operating System:** macOS (Intel/Apple Silicon), Linux, Windows
- **Dependencies:** CMake 3.15+, LLVM 15+, Clang
- **Memory:** 4GB RAM minimum, 8GB recommended
- **Storage:** 2GB free space for toolchain

### Installation

#### macOS Installation

```bash
# Clone the repository
git clone https://github.com/your-org/cypescript.git
cd cypescript

# Run setup script
./setup-macos.sh

# Build the compiler
./build.sh
```

#### Manual Installation

```bash
# Install dependencies
brew install cmake llvm

# Configure environment
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Your First Program

Create a file named `hello.csc`:

```typescript
// hello.csc
let message: string = "Hello, Cypescript!";
print(message);

let answer: i32 = 42;
print("The answer is: ");
print(answer);
```

Compile and run:

```bash
# Compile to LLVM IR
./build/cscript hello.csc

# Compile to native executable
llc -filetype=obj -relocation-model=pic output.ll -o output.o
clang output.o -o hello

# Run
./hello
```

---

## Language Syntax

### Case Sensitivity

Cypescript is **case-sensitive**. `myVariable` and `MyVariable` are different identifiers.

### Identifiers

Valid identifier rules:
- Must start with a letter (a-z, A-Z) or underscore (_)
- Can contain letters, digits (0-9), and underscores
- Cannot be a reserved keyword

**Valid identifiers:**
```typescript
myVariable
_privateVar
counter1
MAX_SIZE
```

**Invalid identifiers:**
```typescript
1variable    // Cannot start with digit
my-variable  // Hyphens not allowed
class        // Reserved keyword
```

### Reserved Keywords

```
let       const     var       function  if        else
while     for       do        return    true      false
null      undefined string    i32       f64       boolean
number
```

### Whitespace and Line Endings

- Whitespace (spaces, tabs, newlines) is generally ignored
- Statements must end with semicolons (`;`)
- Blocks are delimited by braces (`{` `}`)

---

## Data Types

Cypescript supports several built-in data types with explicit type annotations.

### Primitive Types

#### Integer Types

| Type | Description | Range | Size |
|------|-------------|-------|------|
| `i32` | 32-bit signed integer | -2,147,483,648 to 2,147,483,647 | 4 bytes |

**Example:**
```typescript
let age: i32 = 25;
let temperature: i32 = -10;
let maxValue: i32 = 2147483647;
```

#### Floating-Point Types

| Type | Description | Precision | Size |
|------|-------------|-----------|------|
| `f64` | 64-bit double precision | ~15-17 decimal digits | 8 bytes |

**Example:**
```typescript
let pi: f64 = 3.14159265359;
let price: f64 = 19.99;
```

#### String Type

| Type | Description | Encoding |
|------|-------------|----------|
| `string` | UTF-8 string literal | UTF-8 |

**Example:**
```typescript
let name: string = "Alice";
let greeting: string = "Hello, World!";
let empty: string = "";
```

#### Boolean Type

| Type | Description | Values |
|------|-------------|--------|
| `boolean` | Logical true/false | `true`, `false` |

**Example:**
```typescript
let isActive: boolean = true;
let isComplete: boolean = false;
```

### Type Compatibility

Cypescript uses **strict typing** with no implicit conversions between types.

```typescript
let num: i32 = 42;
let str: string = "hello";

// This would cause a compile error:
// num = str;  // Error: Cannot assign string to i32
```

---

## Variables and Constants

### Variable Declarations

Variables are declared using the `let` keyword with explicit type annotations.

**Syntax:**
```typescript
let variableName: type = initialValue;
```

**Examples:**
```typescript
let counter: i32 = 0;
let message: string = "Hello";
let isReady: boolean = true;
let temperature: f64 = 98.6;
```

### Variable Assignment

After declaration, variables can be reassigned using the assignment operator (`=`).

```typescript
let score: i32 = 100;
score = 95;           // Valid reassignment
score = score + 5;    // Using current value
```

### Constants

Constants are declared using the `const` keyword (parser support implemented, full semantics pending).

```typescript
const MAX_USERS: i32 = 1000;
const APP_NAME: string = "Cypescript App";
```

### Scope Rules

Variables have **block scope** - they are accessible within the block where they are declared.

```typescript
{
    let localVar: i32 = 10;
    // localVar is accessible here
}
// localVar is not accessible here
```

**Loop variable scope:**
```typescript
for (let i: i32 = 0; i < 10; i = i + 1) {
    // i is accessible within the loop
}
// i is not accessible here
```

---

## Operators

### Arithmetic Operators

| Operator | Description | Example | Result Type |
|----------|-------------|---------|-------------|
| `+` | Addition | `5 + 3` | Same as operands |
| `-` | Subtraction | `5 - 3` | Same as operands |
| `*` | Multiplication | `5 * 3` | Same as operands |
| `/` | Division | `10 / 3` | Same as operands |
| `%` | Modulo | `10 % 3` | Same as operands |

**Examples:**
```typescript
let a: i32 = 10;
let b: i32 = 3;

let sum: i32 = a + b;        // 13
let difference: i32 = a - b; // 7
let product: i32 = a * b;    // 30
let quotient: i32 = a / b;   // 3 (integer division)
let remainder: i32 = a % b;  // 1
```

### Comparison Operators

| Operator | Description | Example | Result Type |
|----------|-------------|---------|-------------|
| `==` | Equal to | `a == b` | `boolean` |
| `!=` | Not equal to | `a != b` | `boolean` |
| `<` | Less than | `a < b` | `boolean` |
| `<=` | Less than or equal | `a <= b` | `boolean` |
| `>` | Greater than | `a > b` | `boolean` |
| `>=` | Greater than or equal | `a >= b` | `boolean` |

**Examples:**
```typescript
let x: i32 = 5;
let y: i32 = 10;

let isEqual: boolean = x == y;     // false
let isNotEqual: boolean = x != y;  // true
let isLess: boolean = x < y;       // true
let isGreater: boolean = x > y;    // false
```

### Assignment Operators

| Operator | Description | Example | Equivalent |
|----------|-------------|---------|------------|
| `=` | Simple assignment | `a = 5` | N/A |

**Examples:**
```typescript
let value: i32 = 10;
value = 20;              // Simple assignment
value = value + 5;       // Using current value
```

### Operator Precedence

Operators are evaluated in the following order (highest to lowest precedence):

1. **Parentheses** `()`
2. **Multiplicative** `*`, `/`, `%`
3. **Additive** `+`, `-`
4. **Comparison** `<`, `<=`, `>`, `>=`
5. **Equality** `==`, `!=`
6. **Assignment** `=`

**Examples:**
```typescript
let result: i32 = 2 + 3 * 4;        // 14 (not 20)
let result2: i32 = (2 + 3) * 4;     // 20
let condition: boolean = 5 + 3 > 2 * 4;  // false (8 > 8)
```

---

## Control Flow Statements

### If Statements

Execute code conditionally based on boolean expressions.

**Syntax:**
```typescript
if (condition) {
    // statements
}
```

**If-Else Syntax:**
```typescript
if (condition) {
    // statements when true
} else {
    // statements when false
}
```

**Examples:**
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

### Nested If Statements

```typescript
let temperature: i32 = 75;
let humidity: i32 = 60;

if (temperature > 70) {
    if (humidity < 50) {
        print("Warm and dry");
    } else {
        print("Warm and humid");
    }
} else {
    print("Cool weather");
}
```

### Complex Conditions

```typescript
let age: i32 = 25;
let hasLicense: boolean = true;

if (age >= 18) {
    if (hasLicense == true) {
        print("Can drive");
    } else {
        print("Need license");
    }
}
```

---

## Loop Statements

Cypescript supports all major loop constructs for iterative execution.

### While Loops

Execute statements while a condition remains true.

**Syntax:**
```typescript
while (condition) {
    // statements
}
```

**Example:**
```typescript
let count: i32 = 0;
while (count < 5) {
    print("Count: ");
    print(count);
    count = count + 1;
}
```

### For Loops

Traditional C-style for loops with initialization, condition, and increment.

**Syntax:**
```typescript
for (initialization; condition; increment) {
    // statements
}
```

**Examples:**
```typescript
// Basic for loop
for (let i: i32 = 0; i < 10; i = i + 1) {
    print("Iteration: ");
    print(i);
}

// For loop with existing variable
let counter: i32 = 0;
for (counter = 5; counter < 8; counter = counter + 1) {
    print("Counter: ");
    print(counter);
}

// For loop with no initialization
let preInit: i32 = 10;
for (; preInit > 0; preInit = preInit - 1) {
    print("Countdown: ");
    print(preInit);
}
```

### Do-While Loops

Execute statements at least once, then continue while condition is true.

**Syntax:**
```typescript
do {
    // statements
} while (condition);
```

**Example:**
```typescript
let input: i32 = 0;
do {
    print("Processing: ");
    print(input);
    input = input + 1;
} while (input < 3);
```

### Nested Loops

Loops can be nested to create complex iteration patterns.

**Example:**
```typescript
// Multiplication table
for (let row: i32 = 1; row <= 3; row = row + 1) {
    for (let col: i32 = 1; col <= 3; col = col + 1) {
        let product: i32 = row * col;
        print("Row ");
        print(row);
        print(" Col ");
        print(col);
        print(" = ");
        print(product);
    }
}
```

### Loop Best Practices

1. **Always ensure termination conditions**
2. **Initialize loop variables properly**
3. **Avoid infinite loops**
4. **Use meaningful variable names**

**Good:**
```typescript
for (let studentIndex: i32 = 0; studentIndex < totalStudents; studentIndex = studentIndex + 1) {
    // Process student
}
```

**Avoid:**
```typescript
for (let i: i32 = 0; i > 0; i = i + 1) {  // Infinite loop!
    // This will never execute
}
```

---

## Functions

### Built-in Functions

#### print()

Outputs values to the console.

**Syntax:**
```typescript
print(expression);
```

**Supported Types:**
- `string` - Prints the string content
- `i32` - Prints the integer value
- `f64` - Prints the floating-point value
- `boolean` - Prints "true" or "false"

**Examples:**
```typescript
print("Hello, World!");           // String output
print(42);                        // Integer output
print(3.14159);                   // Float output (when implemented)
print(true);                      // Boolean output (when implemented)

let name: string = "Alice";
print(name);                      // Variable output
```

### User-Defined Functions

*Note: User-defined functions are planned for future implementation.*

---

## Comments

Cypescript supports both single-line and multi-line comments.

### Single-Line Comments

Use `//` for single-line comments.

```typescript
// This is a single-line comment
let value: i32 = 42;  // Comment at end of line
```

### Multi-Line Comments

Use `/* */` for multi-line comments.

```typescript
/*
 * This is a multi-line comment
 * that spans several lines
 */
let result: i32 = calculateValue();

/*
 * TODO: Implement error handling
 * FIXME: Handle edge cases
 */
```

### Comment Best Practices

1. **Explain why, not what**
2. **Keep comments up-to-date**
3. **Use comments for complex logic**
4. **Document assumptions and constraints**

**Good:**
```typescript
// Calculate compound interest using daily compounding
let interest: f64 = principal * rate * time;
```

**Less helpful:**
```typescript
// Set x to 5
let x: i32 = 5;
```

---

## Compiler Reference

### Command Line Interface

**Basic Usage:**
```bash
./build/cscript [options] <input-file>
```

**Options:**
- `-v, --verbose` - Enable verbose output
- `--print-tokens` - Display lexical analysis tokens
- `--print-ast` - Display abstract syntax tree
- `-o <file>` - Specify output file name
- `--help` - Show help information

**Examples:**
```bash
# Basic compilation
./build/cscript program.csc

# Verbose compilation with debugging
./build/cscript -v --print-tokens --print-ast program.csc

# Custom output file
./build/cscript -o my_output.ll program.csc
```

### Compilation Process

1. **Lexical Analysis** - Source code → Tokens
2. **Syntax Analysis** - Tokens → Abstract Syntax Tree (AST)
3. **Code Generation** - AST → LLVM IR
4. **Optimization** - LLVM optimizations (optional)
5. **Object Generation** - LLVM IR → Object file
6. **Linking** - Object file → Executable

**Complete Build Process:**
```bash
# Step 1: Cypescript to LLVM IR
./build/cscript program.csc

# Step 2: LLVM IR to object file
llc -filetype=obj -relocation-model=pic output.ll -o output.o

# Step 3: Link to executable
clang output.o -o program

# Step 4: Run
./program
```

### Build Scripts

**Automated compilation:**
```bash
# Build compiler
./build.sh

# Run tests
./test.sh

# Compile and run a program
./compile-run.sh program.csc
```

---

## Error Handling

### Compile-Time Errors

#### Syntax Errors

**Undefined Variable:**
```typescript
let x: i32 = y;  // Error: 'y' is not defined
```

**Type Mismatch:**
```typescript
let num: i32 = "hello";  // Error: Cannot assign string to i32
```

**Missing Semicolon:**
```typescript
let value: i32 = 42  // Error: Expected ';' after statement
print(value);
```

#### Parser Errors

**Unexpected Token:**
```
Parse Error: Expected ')' after if condition. Found IDENTIFIER ('x') instead.
```

**Invalid Statement:**
```
Parse Error: Unexpected token at start of statement: NUMBER ('42')
```

### Runtime Errors

Since Cypescript compiles to native code, runtime errors are typically:
- **Segmentation faults** (memory access violations)
- **Arithmetic errors** (division by zero, overflow)
- **System errors** (file access, memory allocation)

### Error Recovery

The compiler attempts to provide helpful error messages with context:

```
Parsing failed: Expected ';' after assignment
  --> program.csc:5:20
   |
 5 | let value: i32 = 42
   |                    ^ Expected semicolon here
```

---

## Examples

### Basic Examples

#### Hello World
```typescript
// hello.csc
print("Hello, Cypescript!");
```

#### Variables and Arithmetic
```typescript
// arithmetic.csc
let a: i32 = 10;
let b: i32 = 20;
let sum: i32 = a + b;
let product: i32 = a * b;

print("Sum: ");
print(sum);
print("Product: ");
print(product);
```

### Intermediate Examples

#### Factorial Calculation
```typescript
// factorial.csc
let n: i32 = 5;
let factorial: i32 = 1;
let counter: i32 = 1;

print("Calculating factorial of ");
print(n);

while (counter <= n) {
    factorial = factorial * counter;
    counter = counter + 1;
}

print("Result: ");
print(factorial);
```

#### Fibonacci Sequence
```typescript
// fibonacci.csc
let limit: i32 = 10;
let fib1: i32 = 0;
let fib2: i32 = 1;
let count: i32 = 0;

print("Fibonacci sequence:");

while (count < limit) {
    if (count == 0) {
        print(fib1);
    } else {
        if (count == 1) {
            print(fib2);
        } else {
            let next: i32 = fib1 + fib2;
            print(next);
            fib1 = fib2;
            fib2 = next;
        }
    }
    count = count + 1;
}
```

### Advanced Examples

#### Prime Number Detection
```typescript
// prime.csc
let testNum: i32 = 17;
let divisor: i32 = 2;
let isPrime: i32 = 1;  // 1 = true, 0 = false

print("Testing if ");
print(testNum);
print(" is prime...");

if (testNum <= 1) {
    isPrime = 0;
} else {
    while (divisor * divisor <= testNum) {
        let remainder: i32 = testNum % divisor;
        if (remainder == 0) {
            isPrime = 0;
            print("Found divisor: ");
            print(divisor);
        }
        divisor = divisor + 1;
    }
}

if (isPrime == 1) {
    print("PRIME!");
} else {
    print("Not prime");
}
```

#### Nested Loop Patterns
```typescript
// patterns.csc
print("Triangle pattern:");

for (let row: i32 = 1; row <= 5; row = row + 1) {
    for (let col: i32 = 1; col <= row; col = col + 1) {
        print("*");
    }
    print(""); // New line (conceptually)
}
```

---

## Language Grammar

### Formal Grammar (EBNF)

```ebnf
Program ::= Statement*

Statement ::= VariableDeclaration
           | AssignmentStatement  
           | IfStatement
           | WhileStatement
           | ForStatement
           | DoWhileStatement
           | FunctionCall
           | Block

VariableDeclaration ::= 'let' IDENTIFIER ':' Type '=' Expression ';'

AssignmentStatement ::= IDENTIFIER '=' Expression ';'

IfStatement ::= 'if' '(' Expression ')' Block ('else' Block)?

WhileStatement ::= 'while' '(' Expression ')' Block

ForStatement ::= 'for' '(' (VariableDeclaration | AssignmentStatement | ';') 
                          Expression? ';' 
                          AssignmentStatement? ')' Block

DoWhileStatement ::= 'do' Block 'while' '(' Expression ')' ';'

Block ::= '{' Statement* '}'

Expression ::= ComparisonExpression

ComparisonExpression ::= AdditionExpression (('==' | '!=' | '<' | '<=' | '>' | '>=') AdditionExpression)*

AdditionExpression ::= MultiplicationExpression (('+' | '-') MultiplicationExpression)*

MultiplicationExpression ::= PrimaryExpression (('*' | '/' | '%') PrimaryExpression)*

PrimaryExpression ::= IDENTIFIER
                   | INTEGER_LITERAL
                   | STRING_LITERAL
                   | BOOLEAN_LITERAL
                   | '(' Expression ')'

FunctionCall ::= IDENTIFIER '(' (Expression (',' Expression)*)? ')' ';'

Type ::= 'i32' | 'f64' | 'string' | 'boolean'

IDENTIFIER ::= [a-zA-Z_][a-zA-Z0-9_]*
INTEGER_LITERAL ::= [0-9]+
STRING_LITERAL ::= '"' [^"]* '"'
BOOLEAN_LITERAL ::= 'true' | 'false'
```

### Lexical Elements

**Keywords:**
```
let const var function if else while for do return
true false null undefined string i32 f64 boolean number
```

**Operators:**
```
+ - * / % == != < <= > >= = 
```

**Delimiters:**
```
( ) { } [ ] ; : , .
```

**Literals:**
- **Integer:** `42`, `0`, `-10`
- **String:** `"hello"`, `"world"`, `""`
- **Boolean:** `true`, `false`

---

## Appendices

### Appendix A: Reserved Words

Complete list of reserved keywords that cannot be used as identifiers:

```
let       const     var       function  if        else
while     for       do        return    true      false  
null      undefined string    i32       f64       boolean
number    break     continue  switch    case      default
class     interface extends   implements public   private
protected static    abstract  readonly  enum      namespace
module    import    export    async     await     yield
```

### Appendix B: Operator Precedence Table

| Precedence | Operator | Associativity | Description |
|------------|----------|---------------|-------------|
| 1 (Highest) | `()` | Left-to-right | Parentheses |
| 2 | `*` `/` `%` | Left-to-right | Multiplicative |
| 3 | `+` `-` | Left-to-right | Additive |
| 4 | `<` `<=` `>` `>=` | Left-to-right | Relational |
| 5 | `==` `!=` | Left-to-right | Equality |
| 6 (Lowest) | `=` | Right-to-left | Assignment |

### Appendix C: ASCII Character Set

Cypescript source files should use UTF-8 encoding. All ASCII characters are supported in string literals and comments.

### Appendix D: Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| Variable access | O(1) | Direct memory access |
| Arithmetic operations | O(1) | Native CPU instructions |
| Function calls | O(1) | Direct call, no overhead |
| Loop iteration | O(n) | Where n is iteration count |

---

## Version History

### Version 1.0 (August 2025)
- Initial release
- Basic data types (i32, f64, string, boolean)
- Variable declarations and assignments
- Arithmetic and comparison operators
- Control flow (if/else statements)
- Loop constructs (while, for, do-while)
- LLVM-based code generation
- Native executable output

### Planned Features

#### Version 1.1
- User-defined functions
- Function parameters and return values
- Local variable scoping
- Function overloading

#### Version 1.2
- Arrays and array operations
- String manipulation functions
- Enhanced type system

#### Version 1.3
- Objects and structures
- Member access operators
- Constructor functions

#### Version 2.0
- Modules and imports
- Generic types
- Advanced memory management
- Standard library

---

## Support and Community

### Getting Help

- **Documentation:** This reference guide
- **Examples:** See `/example` directory
- **Issues:** GitHub issue tracker
- **Discussions:** GitHub discussions

### Contributing

Cypescript is open source. Contributions are welcome in:
- Language features
- Compiler optimizations
- Documentation improvements
- Example programs
- Testing and bug reports

### License

MIT License - See LICENSE file for details.

---

*This documentation is maintained by the Cypescript development team. For the most current information, please refer to the official repository.*
