// Benchmark: Simple loop (100M iterations) with data dependency
let iterations: i32 = 100000000;
let sum: i32 = 0;

for (let i: i32 = 0; i < iterations; i = i + 1) {
    sum = sum + (i % 7);
}

print("Sum: ");
println(sum);
