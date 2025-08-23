# Cypescript Quick Reference

## Data Types
```typescript
let integer: i32 = 42;
let decimal: f64 = 3.14;
let text: string = "Hello";
let flag: boolean = true;
```

## Variables & Assignment
```typescript
let variable: i32 = 10;    // Declaration
variable = 20;             // Assignment
```

## Arithmetic Operators
```typescript
let sum: i32 = a + b;      // Addition
let diff: i32 = a - b;     // Subtraction  
let prod: i32 = a * b;     // Multiplication
let quot: i32 = a / b;     // Division
let rem: i32 = a % b;      // Modulo
```

## Comparison Operators
```typescript
a == b    // Equal
a != b    // Not equal
a < b     // Less than
a <= b    // Less than or equal
a > b     // Greater than
a >= b    // Greater than or equal
```

## Control Flow
```typescript
// If-else
if (condition) {
    // statements
} else {
    // statements
}

// Nested conditions
if (condition1) {
    if (condition2) {
        // nested logic
    }
}
```

## Loops

### While Loop
```typescript
while (condition) {
    // statements
}
```

### For Loop
```typescript
for (let i: i32 = 0; i < 10; i = i + 1) {
    // statements
}

// With existing variable
for (i = 0; i < 10; i = i + 1) {
    // statements
}
```

### Do-While Loop
```typescript
do {
    // statements
} while (condition);
```

## Functions
```typescript
print("Hello World");      // String output
print(42);                 // Integer output
print(variable);           // Variable output
```

## Comments
```typescript
// Single line comment

/*
 * Multi-line
 * comment
 */
```

## Compilation
```bash
# Compile Cypescript to LLVM IR
./build/cscript program.csc

# Compile to native executable
llc -filetype=obj -relocation-model=pic output.ll -o output.o
clang output.o -o program

# Run
./program
```

## Common Patterns

### Factorial
```typescript
let n: i32 = 5;
let result: i32 = 1;
for (let i: i32 = 1; i <= n; i = i + 1) {
    result = result * i;
}
```

### Fibonacci
```typescript
let a: i32 = 0;
let b: i32 = 1;
for (let i: i32 = 0; i < 10; i = i + 1) {
    print(a);
    let temp: i32 = a + b;
    a = b;
    b = temp;
}
```

### Prime Check
```typescript
let num: i32 = 17;
let isPrime: i32 = 1;
for (let i: i32 = 2; i * i <= num; i = i + 1) {
    if (num % i == 0) {
        isPrime = 0;
    }
}
```

## Operator Precedence (High to Low)
1. `()` - Parentheses
2. `*` `/` `%` - Multiplicative
3. `+` `-` - Additive  
4. `<` `<=` `>` `>=` - Relational
5. `==` `!=` - Equality
6. `=` - Assignment

## Reserved Keywords
```
let const var function if else while for do return
true false null undefined string i32 f64 boolean
```

## File Extension
- Source files: `.csc`
- LLVM IR output: `.ll`
- Object files: `.o`
