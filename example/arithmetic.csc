// Cypescript Arithmetic Test
// Testing basic arithmetic operations

// Simple arithmetic
let a: i32 = 10;
let b: i32 = 5;

// Addition
let sum: i32 = a + b;
print(sum);

// Subtraction  
let diff: i32 = a - b;
print(diff);

// Multiplication
let product: i32 = a * b;
print(product);

// Division
let quotient: i32 = a / b;
print(quotient);

// Modulo
let remainder: i32 = a % b;
print(remainder);

// Complex expressions with operator precedence
let result1: i32 = 2 + 3 * 4;  // Should be 14 (not 20)
print(result1);

let result2: i32 = (2 + 3) * 4;  // Should be 20
print(result2);

// Nested arithmetic
let x: i32 = 100;
let y: i32 = 20;
let z: i32 = 3;

let complex: i32 = x + y * z - 10 / 2;  // 100 + 60 - 5 = 155
print(complex);

// Direct literal arithmetic
print(7 + 3);
print(15 - 8);
print(6 * 7);
print(20 / 4);
print(17 % 5);
