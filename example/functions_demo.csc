// Cypescript Functions Demo - Phase 1 Implementation
// Demonstrates user-defined function capabilities

// ===== Basic Function Declarations =====

// Simple arithmetic functions
function add(a: i32, b: i32): i32 {
    return a + b;
}

function multiply(x: i32, y: i32): i32 {
    return x * y;
}

// Function with local variables and loops
function factorial(n: i32): i32 {
    let result: i32 = 1;
    let counter: i32 = 1;
    
    while (counter <= n) {
        result = multiply(result, counter);
        counter = counter + 1;
    }
    
    return result;
}

// Void functions for output
function printSeparator(): void {
    println("================================");
}

function greetUser(name: string, age: i32): void {
    print("Hello, ");
    print(name);
    print("! You are ");
    print(age);
    println(" years old.");
}

// Function that calls other functions
function calculateArea(length: i32, width: i32): i32 {
    return multiply(length, width);
}

// Complex calculation with multiple function calls
function complexMath(a: i32, b: i32, c: i32): i32 {
    let sum1: i32 = add(a, b);
    let sum2: i32 = add(sum1, c);
    let doubled: i32 = multiply(sum2, 2);
    return doubled;
}

// Mathematical utility function
function power(base: i32, exponent: i32): i32 {
    let result: i32 = 1;
    let counter: i32 = 0;
    
    while (counter < exponent) {
        result = multiply(result, base);
        counter = counter + 1;
    }
    
    return result;
}

// ===== Main Program Execution =====

printSeparator();
println("🚀 Cypescript Functions Demo - Phase 1");
printSeparator();

// Test basic arithmetic
let x: i32 = 15;
let y: i32 = 25;

print("Testing with x=");
print(x);
print(", y=");
println(y);

let sum: i32 = add(x, y);
print("add(x, y) = ");
println(sum);

let product: i32 = multiply(x, y);
print("multiply(x, y) = ");
println(product);

printSeparator();

// Test factorial calculation
let n: i32 = 5;
let fact: i32 = factorial(n);
print("factorial(");
print(n);
print(") = ");
println(fact);

// Test power function
let base: i32 = 2;
let exp: i32 = 8;
let pow_result: i32 = power(base, exp);
print("power(");
print(base);
print(", ");
print(exp);
print(") = ");
println(pow_result);

printSeparator();

// Test user greeting
greetUser("Alice", 28);
greetUser("Bob", 35);

printSeparator();

// Test area calculation
let length: i32 = 12;
let width: i32 = 8;
let area: i32 = calculateArea(length, width);
print("Rectangle area (");
print(length);
print(" x ");
print(width);
print(") = ");
println(area);

// Test complex math
let a: i32 = 5;
let b: i32 = 10;
let c: i32 = 3;
let complex: i32 = complexMath(a, b, c);
print("complexMath(");
print(a);
print(", ");
print(b);
print(", ");
print(c);
print(") = ");
println(complex);

printSeparator();
println("✅ All function tests completed successfully!");
println("🎯 Phase 1 function implementation working!");
println("📈 Features demonstrated:");
println("   • Function declarations with parameters");
println("   • Return statements and values");
println("   • Local variables and scoping");
println("   • Function calls in expressions");
println("   • Void functions");
println("   • Nested function calls");
printSeparator();
