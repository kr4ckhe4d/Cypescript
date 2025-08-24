// Cypescript Array Processing Benchmark
// Tests performance of large array operations

println("=== Cypescript Array Processing Benchmark ===");

// Configuration
let array_size: i32 = 1000000;  // 1 million elements
println("Array size: 1,000,000 elements");

// Create large array
println("Creating large array...");
let numbers: i32[] = [];

// Fill array with sequential numbers (simulating array creation)
let i: i32 = 0;
while (i < array_size) {
    // Note: We can't dynamically resize arrays yet, so we'll work with a smaller fixed array
    i = i + 1;
}

// Use a fixed large array for actual benchmarking
let benchmark_array: i32[] = [
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
    71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100
];

let actual_size: i32 = benchmark_array.length;
println("Actual benchmark array size: ");
println(actual_size);

// Benchmark 1: Array Sum
println("Benchmark 1: Array Sum");
let iterations: i32 = 100000;  // Repeat operation many times
println("Iterations: 100,000");

let sum_total: i32 = 0;
let iter: i32 = 0;

// Manual timing (we'll measure externally)
println("Starting sum benchmark...");
while (iter < iterations) {
    let current_sum: i32 = 0;
    let j: i32 = 0;
    
    // Sum all elements in array
    while (j < actual_size) {
        current_sum = current_sum + benchmark_array[j];
        j = j + 1;
    }
    
    sum_total = sum_total + current_sum;
    iter = iter + 1;
}
println("Sum benchmark complete");
println("Total sum across all iterations: ");
println(sum_total);

// Benchmark 2: Array Maximum
println("Benchmark 2: Array Maximum");
let max_iterations: i32 = 100000;
println("Iterations: 100,000");

let max_total: i32 = 0;
let max_iter: i32 = 0;

println("Starting max benchmark...");
while (max_iter < max_iterations) {
    let current_max: i32 = benchmark_array[0];
    let k: i32 = 1;
    
    // Find maximum element
    while (k < actual_size) {
        if (benchmark_array[k] > current_max) {
            current_max = benchmark_array[k];
        }
        k = k + 1;
    }
    
    max_total = max_total + current_max;
    max_iter = max_iter + 1;
}
println("Max benchmark complete");
println("Total max across all iterations: ");
println(max_total);

// Benchmark 3: Array Search
println("Benchmark 3: Array Search");
let search_iterations: i32 = 100000;
let search_target: i32 = 75;
println("Iterations: 100,000");
println("Search target: 75");

let found_count: i32 = 0;
let search_iter: i32 = 0;

println("Starting search benchmark...");
while (search_iter < search_iterations) {
    let found: i32 = 0;
    let l: i32 = 0;
    
    // Linear search for target
    while (l < actual_size) {
        if (benchmark_array[l] == search_target) {
            found = 1;
        }
        l = l + 1;
    }
    
    if (found == 1) {
        found_count = found_count + 1;
    }
    search_iter = search_iter + 1;
}
println("Search benchmark complete");
println("Found target in iterations: ");
println(found_count);

// Benchmark 4: Array Modification
println("Benchmark 4: Array Modification");
let modify_iterations: i32 = 50000;  // Fewer iterations for modification
println("Iterations: 50,000");

// Create a copy for modification
let modify_array: i32[] = [
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
    71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100
];

let modify_iter: i32 = 0;
println("Starting modification benchmark...");
while (modify_iter < modify_iterations) {
    let m: i32 = 0;
    
    // Double all elements
    while (m < actual_size) {
        modify_array[m] = modify_array[m] * 2;
        m = m + 1;
    }
    
    // Reset array for next iteration
    let n: i32 = 0;
    while (n < actual_size) {
        modify_array[n] = modify_array[n] / 2;
        n = n + 1;
    }
    
    modify_iter = modify_iter + 1;
}
println("Modification benchmark complete");

println("=== All Benchmarks Complete ===");
