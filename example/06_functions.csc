// 06 — Functions
// Parameters, return values, void functions, recursion, and generics.

function add(a: i32, b: i32): i32 {
    return a + b;
}

function greet(name: string): void {
    println("Hello, " + name + "!");
}

// Recursion
function factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// Floats in signatures
function circleArea(radius: f64): f64 {
    return 3.14159 * radius * radius;
}

// Generic functions
function identity<T>(value: T): T {
    return value;
}

println(add(5, 3));          // 8
greet("Alice");              // Hello, Alice!
println(factorial(5));       // 120
println(circleArea(2.0));    // 12.5664
println(identity<string>("generics work"));

// Functions calling functions
function sumOfSquares(a: i32, b: i32): i32 {
    return add(a * a, b * b);
}
println(sumOfSquares(3, 4)); // 25
