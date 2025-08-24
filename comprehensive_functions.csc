// Comprehensive Function Test for Cypescript
// Demonstrates Phase 1 function implementation

// Basic arithmetic function
function add(a: i32, b: i32): i32 {
    return a + b;
}

// Function with multiple operations
function calculate(x: i32, y: i32): i32 {
    let temp: i32 = x * 2;
    let result: i32 = temp + y;
    return result;
}

// Void function with side effects
function printHeader(): void {
    println("=== Cypescript Function Demo ===");
}

// Function that calls other functions
function complexCalculation(a: i32, b: i32, c: i32): i32 {
    let sum1: i32 = add(a, b);
    let sum2: i32 = add(sum1, c);
    return calculate(sum2, 10);
}

// Void function with parameters
function printResult(label: string, value: i32): void {
    print(label);
    print(": ");
    println(value);
}

// Main program execution
printHeader();

let x: i32 = 5;
let y: i32 = 3;
let z: i32 = 7;

printResult("x", x);
printResult("y", y);
printResult("z", z);

let sum: i32 = add(x, y);
printResult("add(x, y)", sum);

let calc: i32 = calculate(x, y);
printResult("calculate(x, y)", calc);

let complex: i32 = complexCalculation(x, y, z);
printResult("complexCalculation(x, y, z)", complex);

println("Function implementation successful!");
