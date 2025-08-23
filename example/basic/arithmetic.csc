// Cypescript Arithmetic Test
// Testing basic arithmetic operations

println("=== Arithmetic Operations Demo ===");

// Simple arithmetic
let a: i32 = 10;
let b: i32 = 5;

print("a = ");
println(a);
print("b = ");
println(b);

// Addition
let sum: i32 = a + b;
print("a + b = ");
println(sum);

// Subtraction  
let diff: i32 = a - b;
print("a - b = ");
println(diff);

// Multiplication
let product: i32 = a * b;
print("a * b = ");
println(product);

// Division
let quotient: i32 = a / b;
print("a / b = ");
println(quotient);

// Modulo
let remainder: i32 = a % b;
print("a % b = ");
println(remainder);

// Complex expressions with operator precedence
println("Operator Precedence:");
let result1: i32 = 2 + 3 * 4;  // Should be 14 (not 20)
print("2 + 3 * 4 = ");
println(result1);

let result2: i32 = (2 + 3) * 4;  // Should be 20
print("(2 + 3) * 4 = ");
println(result2);

// Nested arithmetic
let x: i32 = 100;
let y: i32 = 20;
let z: i32 = 3;

let complex: i32 = x + y * z - 10 / 2;  // 100 + 60 - 5 = 155
print("100 + 20 * 3 - 10 / 2 = ");
println(complex);

// Direct literal arithmetic
println("Direct Calculations:");
print("7 + 3 = ");
println(7 + 3);
print("15 - 8 = ");
println(15 - 8);
print("6 * 7 = ");
println(6 * 7);
print("20 / 4 = ");
println(20 / 4);
print("17 % 5 = ");
println(17 % 5);

println("Arithmetic demo complete!");
