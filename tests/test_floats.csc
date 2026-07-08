// Tests: f64 literals, arithmetic, comparisons, and mixing with i32

let pi: f64 = 3.14159;
let radius: f64 = 2.0;
let area: f64 = pi * radius * radius;
println(area); // 12.5664

let sum: f64 = 1.5 + 2.25;
println(sum); // 3.75

let quotient: f64 = 7.0 / 2.0;
println(quotient); // 3.5

// Mixed int/float promotes to f64
let mixed: f64 = 2 * pi;
println(mixed); // 6.28318

// Comparisons
if (pi > 3.0) {
    println("pi > 3");
}
if (1.5 <= 1.5) {
    println("le works");
}

// Unary minus
let neg: f64 = -2.5;
println(neg); // -2.5

// f64 function parameters and return values
function circleArea(r: f64): f64 {
    return 3.14159 * r * r;
}
println(circleArea(3.0)); // 28.2743

// f64 in loops
let acc: f64 = 0.0;
for (let i: i32 = 0; i < 4; i++) {
    acc = acc + 0.5;
}
println(acc); // 2
