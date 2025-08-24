// ARM NEON-Optimized Array Processing Benchmark
// Tests the performance improvement from NEON vectorization on Apple Silicon

println("=== ARM NEON-Optimized Array Processing Benchmark ===");

// Check if NEON is available
let neon_support: i32 = neon_available();
print("NEON support available: ");
if (neon_support == 1) {
    println("Yes");
} else {
    println("No (fallback to scalar)");
}

// Create large test array
let test_array: i32[] = [
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

let array_size: i32 = test_array.length;
println("Array size: ");
println(array_size);

// Benchmark 1: NEON Array Sum vs Regular Array Sum
println("Benchmark 1: NEON Array Sum vs Regular Array Sum");
let iterations: i32 = 100000;
println("Iterations: 100,000");

// Regular array sum benchmark
println("Running regular array sum...");
let regular_sum_total: i32 = 0;
let iter: i32 = 0;
while (iter < iterations) {
    let current_sum: i32 = array_sum_i32(test_array, array_size);
    regular_sum_total = regular_sum_total + current_sum;
    iter = iter + 1;
}
println("Regular sum complete. Total: ");
println(regular_sum_total);

// NEON array sum benchmark
println("Running NEON array sum...");
let neon_sum_total: i32 = 0;
let neon_iter: i32 = 0;
while (neon_iter < iterations) {
    let current_sum: i32 = neon_array_sum_i32(test_array, array_size);
    neon_sum_total = neon_sum_total + current_sum;
    neon_iter = neon_iter + 1;
}
println("NEON sum complete. Total: ");
println(neon_sum_total);

// Verify results match
if (regular_sum_total == neon_sum_total) {
    println("✓ Results match - NEON implementation correct");
} else {
    println("✗ Results don't match - potential NEON bug");
}

// Benchmark 2: NEON Array Max vs Regular Array Max
println("Benchmark 2: NEON Array Max vs Regular Array Max");
let max_iterations: i32 = 100000;
println("Iterations: 100,000");

// Regular array max benchmark
println("Running regular array max...");
let regular_max_total: i32 = 0;
let max_iter: i32 = 0;
while (max_iter < max_iterations) {
    let current_max: i32 = array_max_i32(test_array, array_size);
    regular_max_total = regular_max_total + current_max;
    max_iter = max_iter + 1;
}
println("Regular max complete. Total: ");
println(regular_max_total);

// NEON array max benchmark
println("Running NEON array max...");
let neon_max_total: i32 = 0;
let neon_max_iter: i32 = 0;
while (neon_max_iter < max_iterations) {
    let current_max: i32 = neon_array_max_i32(test_array, array_size);
    neon_max_total = neon_max_total + current_max;
    neon_max_iter = neon_max_iter + 1;
}
println("NEON max complete. Total: ");
println(neon_max_total);

// Verify results match
if (regular_max_total == neon_max_total) {
    println("✓ Results match - NEON implementation correct");
} else {
    println("✗ Results don't match - potential NEON bug");
}

// Benchmark 3: NEON Array Min vs Regular Array Min
println("Benchmark 3: NEON Array Min vs Regular Array Min");
let min_iterations: i32 = 100000;
println("Iterations: 100,000");

// Regular array min benchmark
println("Running regular array min...");
let regular_min_total: i32 = 0;
let min_iter: i32 = 0;
while (min_iter < min_iterations) {
    let current_min: i32 = array_min_i32(test_array, array_size);
    regular_min_total = regular_min_total + current_min;
    min_iter = min_iter + 1;
}
println("Regular min complete. Total: ");
println(regular_min_total);

// NEON array min benchmark
println("Running NEON array min...");
let neon_min_total: i32 = 0;
let neon_min_iter: i32 = 0;
while (neon_min_iter < min_iterations) {
    let current_min: i32 = neon_array_min_i32(test_array, array_size);
    neon_min_total = neon_min_total + current_min;
    neon_min_iter = neon_min_iter + 1;
}
println("NEON min complete. Total: ");
println(neon_min_total);

// Verify results match
if (regular_min_total == neon_min_total) {
    println("✓ Results match - NEON implementation correct");
} else {
    println("✗ Results don't match - potential NEON bug");
}

// Benchmark 4: NEON Array Count Equal
println("Benchmark 4: NEON Array Count Equal");
let count_iterations: i32 = 50000;
let target_value: i32 = 50;
println("Iterations: 50,000");
println("Target value: 50");

println("Running NEON count equal...");
let count_total: i32 = 0;
let count_iter: i32 = 0;
while (count_iter < count_iterations) {
    let count_result: i32 = neon_array_count_equal_i32(test_array, array_size, target_value);
    count_total = count_total + count_result;
    count_iter = count_iter + 1;
}
println("NEON count complete. Total matches: ");
println(count_total);

// Show theoretical NEON performance ratio
println("ARM NEON Performance Information:");
println("NEON processes 4 integers in parallel (vs 8 for x86 AVX2)");
println("Expected speedup on Apple Silicon: 4x for array operations");

println("=== NEON Benchmark Complete ===");
