// Functions Preview - What user-defined functions will look like
// This is a preview of planned functionality - not yet implemented

println("=== Functions Preview ===");

// Basic function with return value
function add(a: i32, b: i32): i32 {
    return a + b;
}

// Function with string parameters
function greet(name: string, age: i32): string {
    print("Hello ");
    print(name);
    print(", you are ");
    print(age);
    println(" years old");
    return "greeting_complete";
}

// Void function (no return value)
function printHeader(): void {
    println("=== Program Start ===");
}

// Function with local variables
function calculate(x: i32): i32 {
    let temp: i32 = x * 2;
    let result: i32 = temp + 10;
    return result;
}

// Recursive function
function factorial(n: i32): i32 {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

// Function overloading (same name, different parameters)
function process(value: i32): i32 {
    return value * 2;
}

function process(text: string): string {
    return string_upper(text);  // Using C++ integration
}

// Main program using functions
printHeader();

let sum: i32 = add(10, 5);
println("10 + 5 = ");
println(sum);

let greeting: string = greet("Alice", 25);
println("Greeting result: ");
println(greeting);

let calculated: i32 = calculate(7);
println("Calculate(7) = ");
println(calculated);

let fact: i32 = factorial(5);
println("5! = ");
println(fact);

// Function overloading examples
let doubled: i32 = process(21);        // Calls i32 version
let upper: string = process("hello");   // Calls string version

println("Doubled: ");
println(doubled);
println("Uppercase: ");
println(upper);

println("=== Functions Demo Complete ===");

// Implementation Notes:
// 1. Functions will have their own scope for local variables
// 2. Parameters are passed by value
// 3. Return type must match declared type
// 4. Recursive calls are supported
// 5. Function overloading based on parameter types
// 6. LLVM will generate efficient native code for all functions
