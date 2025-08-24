// Cypescript C++ Integration Benchmark
// Uses C++ functions for maximum performance

println("=== Cypescript C++ Integration Benchmark ===");

// Create benchmark array
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

let array_size: i32 = benchmark_array.length;
println("Array size: ");
println(array_size);

// Benchmark 1: C++ Array Sum
println("Benchmark 1: C++ Array Sum");
let sum_iterations: i32 = 100000;
println("Iterations: 100,000");

let sum_total: i32 = 0;
let sum_iter: i32 = 0;

println("Starting C++ sum benchmark...");
while (sum_iter < sum_iterations) {
    // Use C++ function for array sum (much faster)
    let current_sum: i32 = array_sum_i32(benchmark_array, array_size);
    sum_total = sum_total + current_sum;
    sum_iter = sum_iter + 1;
}
println("C++ sum benchmark complete");
println("Total sum across all iterations: ");
println(sum_total);

// Benchmark 2: C++ Array Max
println("Benchmark 2: C++ Array Max");
let max_iterations: i32 = 100000;
println("Iterations: 100,000");

let max_total: i32 = 0;
let max_iter: i32 = 0;

println("Starting C++ max benchmark...");
while (max_iter < max_iterations) {
    // Use C++ function for array max (much faster)
    let current_max: i32 = array_max_i32(benchmark_array, array_size);
    max_total = max_total + current_max;
    max_iter = max_iter + 1;
}
println("C++ max benchmark complete");
println("Total max across all iterations: ");
println(max_total);

// Benchmark 3: C++ Array Min
println("Benchmark 3: C++ Array Min");
let min_iterations: i32 = 100000;
println("Iterations: 100,000");

let min_total: i32 = 0;
let min_iter: i32 = 0;

println("Starting C++ min benchmark...");
while (min_iter < min_iterations) {
    // Use C++ function for array min (much faster)
    let current_min: i32 = array_min_i32(benchmark_array, array_size);
    min_total = min_total + current_min;
    min_iter = min_iter + 1;
}
println("C++ min benchmark complete");
println("Total min across all iterations: ");
println(min_total);

// Benchmark 4: String Operations
println("Benchmark 4: String Operations");
let string_iterations: i32 = 50000;
println("Iterations: 50,000");

let test_string: string = "Hello World Benchmark Test String";
let string_iter: i32 = 0;

println("Starting string benchmark...");
while (string_iter < string_iterations) {
    // Use C++ string functions
    let upper: string = string_upper(test_string);
    let lower: string = string_lower(upper);
    let reversed: string = string_reverse(lower);
    
    // Get string length (C++ function)
    let length: i32 = string_length(reversed);
    
    string_iter = string_iter + 1;
}
println("String benchmark complete");

// Benchmark 5: Mathematical Operations
println("Benchmark 5: Mathematical Operations");
let math_iterations: i32 = 100000;
println("Iterations: 100,000");

let math_iter: i32 = 0;
let math_total: i32 = 0;

println("Starting math benchmark...");
while (math_iter < math_iterations) {
    // Use custom C++ math functions if available
    let gcd_result: i32 = math_gcd(48, 18);
    let fib_result: i32 = math_fibonacci(10);
    let prime_check: i32 = math_is_prime(17);
    
    math_total = math_total + gcd_result + fib_result + prime_check;
    math_iter = math_iter + 1;
}
println("Math benchmark complete");
println("Total math results: ");
println(math_total);

println("=== C++ Integration Benchmark Complete ===");
