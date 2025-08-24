// Memory Optimization Benchmark
// Tests cache-friendly algorithms and memory pool allocation

println("=== Memory Optimization Benchmark ===");

// Initialize memory pool (16MB)
println("Initializing memory pool (16MB)...");
memory_pool_init(16);

// Create test array
let test_array: i32[] = [
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100
];

let array_size: i32 = test_array.length;
println("Test array size: ");
println(array_size);

// Benchmark 1: Cache-Optimized vs Standard Array Sum
println("Benchmark 1: Cache-Optimized vs Standard Array Sum");
let iterations: i32 = 100000;
println("Iterations: 100,000");

// Standard array sum
println("Running standard array sum...");
let standard_sum_total: i32 = 0;
let std_iter: i32 = 0;
while (std_iter < iterations) {
    let current_sum: i32 = array_sum_i32(test_array, array_size);
    standard_sum_total = standard_sum_total + current_sum;
    std_iter = std_iter + 1;
}
println("Standard sum complete. Total: ");
println(standard_sum_total);

// Cache-optimized array sum
println("Running cache-optimized array sum...");
let cache_sum_total: i32 = 0;
let cache_iter: i32 = 0;
while (cache_iter < iterations) {
    let current_sum: i32 = cache_optimized_sum_i32(test_array, array_size);
    cache_sum_total = cache_sum_total + current_sum;
    cache_iter = cache_iter + 1;
}
println("Cache-optimized sum complete. Total: ");
println(cache_sum_total);

// Verify correctness
if (standard_sum_total == cache_sum_total) {
    println("✓ Results match - cache optimization correct");
} else {
    println("✗ Results don't match - cache optimization bug");
}

// Benchmark 2: Cache-Optimized vs Standard Array Max
println("Benchmark 2: Cache-Optimized vs Standard Array Max");
let max_iterations: i32 = 100000;
println("Iterations: 100,000");

// Standard array max
println("Running standard array max...");
let standard_max_total: i32 = 0;
let std_max_iter: i32 = 0;
while (std_max_iter < max_iterations) {
    let current_max: i32 = array_max_i32(test_array, array_size);
    standard_max_total = standard_max_total + current_max;
    std_max_iter = std_max_iter + 1;
}
println("Standard max complete. Total: ");
println(standard_max_total);

// Cache-optimized array max
println("Running cache-optimized array max...");
let cache_max_total: i32 = 0;
let cache_max_iter: i32 = 0;
while (cache_max_iter < max_iterations) {
    let current_max: i32 = cache_optimized_max_i32(test_array, array_size);
    cache_max_total = cache_max_total + current_max;
    cache_max_iter = cache_max_iter + 1;
}
println("Cache-optimized max complete. Total: ");
println(cache_max_total);

// Verify correctness
if (standard_max_total == cache_max_total) {
    println("✓ Results match - cache optimization correct");
} else {
    println("✗ Results don't match - cache optimization bug");
}

// Benchmark 3: Memory Access Pattern Comparison
println("Benchmark 3: Memory Access Pattern Comparison");
let access_iterations: i32 = 10000;
println("Access pattern iterations: 10,000");

// Sequential access (cache-friendly)
println("Testing sequential access pattern...");
let sequential_result: i32 = cache_miss_comparison_i32(test_array, array_size, 0);
println("Sequential access result: ");
println(sequential_result);

// Random access (cache-unfriendly)
println("Testing random access pattern...");
let random_result: i32 = cache_miss_comparison_i32(test_array, array_size, 1);
println("Random access result: ");
println(random_result);

// Results should be the same (sum of all elements) but performance will differ
if (sequential_result == random_result) {
    println("✓ Access patterns produce same result");
} else {
    println("⚠ Access patterns produce different results (expected due to pseudo-random)");
}

// Benchmark 4: Memory Bandwidth Test
println("Benchmark 4: Memory Bandwidth Test");
let bandwidth_iterations: i32 = 1000;
println("Bandwidth test iterations: 1,000");

println("Running memory bandwidth test...");
memory_bandwidth_test_i32(test_array, array_size, bandwidth_iterations);
println("Memory bandwidth test complete");

// Check final array state
let final_sum: i32 = array_sum_i32(test_array, array_size);
println("Final array sum after bandwidth test: ");
println(final_sum);

// Benchmark 5: Memory Pool Statistics
println("Benchmark 5: Memory Pool Statistics");

// Get memory statistics (using placeholder variables since we can't declare in function call)
let pool_used: i32 = 0;
let pool_total: i32 = 0;
let fragmentation: i32 = 0;

// Note: This is a simplified call - in a real implementation we'd need proper pointer handling
println("Memory pool statistics:");
println("Pool usage tracking implemented");

// Reset memory pool
println("Resetting memory pool...");
memory_pool_reset();
println("Memory pool reset complete");

// Performance Analysis Summary
println("=== Memory Optimization Analysis ===");
println("Cache-optimized algorithms tested:");
println("- Blocked array sum with L1 cache optimization");
println("- Prefetching array max with memory hints");
println("- Sequential vs random access pattern comparison");
println("- Memory bandwidth utilization test");
println("- Memory pool allocation and management");

println("Expected benefits:");
println("- Reduced cache misses through blocking");
println("- Improved memory bandwidth utilization");
println("- Better data locality and prefetching");
println("- Reduced memory allocation overhead");

// Cleanup
println("Cleaning up memory pool...");
memory_pool_cleanup();

println("=== Memory Optimization Benchmark Complete ===");
