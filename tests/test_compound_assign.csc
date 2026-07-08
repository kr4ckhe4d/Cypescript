// Tests: +=, -=, *=, /=, %=, ++, --

let x: i32 = 10;
x += 5;
println(x); // 15
x -= 3;
println(x); // 12
x *= 2;
println(x); // 24
x /= 4;
println(x); // 6
x %= 4;
println(x); // 2

x++;
println(x); // 3
x--;
x--;
println(x); // 1

// In for loops
let sum: i32 = 0;
for (let i: i32 = 0; i < 5; i++) {
    sum += i;
}
println(sum); // 10

// Compound assignment on object properties
let counter = { value: 100 };
counter.value += 11;
println(counter.value); // 111
counter.value--;
println(counter.value); // 110

// f64 compound assignment
let f: f64 = 1.5;
f *= 4.0;
println(f); // 6
