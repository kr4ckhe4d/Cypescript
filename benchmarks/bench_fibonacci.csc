// Benchmark: Fibonacci computation using functions
function fibonacci(n: i32): i32 {
    let a: i32 = 0;
    let b: i32 = 1;
    for (let i: i32 = 0; i < n; i = i + 1) {
        let temp: i32 = a + b;
        a = b;
        b = temp;
    }
    return a;
}

let iterations: i32 = 10000000;
let sum: i32 = 0;

for (let i: i32 = 0; i < iterations; i = i + 1) {
    sum = sum + fibonacci(20 + i % 5);
}

print("Sum: ");
println(sum);
