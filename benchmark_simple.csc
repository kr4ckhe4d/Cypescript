let iterations: i32 = 100000000;
let sum: i32 = 0;

println("Starting Cscript simple benchmark (100,000,000 iterations)...");

for (let i: i32 = 0; i < iterations; i = i + 1) {
    sum = sum + 1;
}

print("Benchmark completed! Sum: ");
println(sum);
