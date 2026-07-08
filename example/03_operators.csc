// 03 — Operators
// Arithmetic, comparison, logical, compound assignment, increment/decrement.

let a: i32 = 10;
let b: i32 = 3;

// Arithmetic
println(a + b);  // 13
println(a - b);  // 7
println(a * b);  // 30
println(a / b);  // 3   (integer division)
println(a % b);  // 1

// Floats promote automatically when mixed with ints
println(a / 4.0); // 2.5

// Comparison (=== and !== also accepted)
println(a > b);   // 1
println(a == 10); // 1
println(a !== b); // 1

// Logical with short-circuiting
let inRange: boolean = a > 0 && a < 100;
println(inRange); // 1
println(!inRange); // 0

// Compound assignment and increment/decrement
let x: i32 = 5;
x += 10;  // 15
x -= 3;   // 12
x *= 2;   // 24
x /= 6;   // 4
x %= 3;   // 1
x++;      // 2
x--;      // 1
println(x); // 1
