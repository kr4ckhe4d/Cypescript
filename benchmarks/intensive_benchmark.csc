// Intensive Computational Benchmark
// Designed to favor native compilation over JIT

println("=== Intensive Computational Benchmark ===");

// Benchmark 1: Nested Loop Computation
println("Benchmark 1: Nested Loop Matrix Multiplication Simulation");
let matrix_size: i32 = 200;
let matrix_iterations: i32 = 100;

println("Matrix size: 200x200");
println("Iterations: 100");

let total_operations: i32 = 0;
let iter: i32 = 0;

println("Starting intensive nested loop benchmark...");
while (iter < matrix_iterations) {
    let i: i32 = 0;
    while (i < matrix_size) {
        let j: i32 = 0;
        while (j < matrix_size) {
            let k: i32 = 0;
            while (k < matrix_size) {
                // Simulate matrix multiplication operation
                let result: i32 = (i * j + k) % 1000;
                total_operations = total_operations + result;
                k = k + 1;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    iter = iter + 1;
}
println("Nested loop benchmark complete");
println("Total operations: ");
println(total_operations);

// Benchmark 2: Prime Number Generation
println("Benchmark 2: Prime Number Generation");
let prime_limit: i32 = 10000;
let prime_count: i32 = 0;

println("Finding primes up to: 10,000");

let num: i32 = 2;
println("Starting prime generation benchmark...");
while (num <= prime_limit) {
    let is_prime: i32 = 1;
    let divisor: i32 = 2;
    
    // Check if number is prime
    while (divisor * divisor <= num) {
        if (num % divisor == 0) {
            is_prime = 0;
        }
        divisor = divisor + 1;
    }
    
    if (is_prime == 1) {
        prime_count = prime_count + 1;
    }
    
    num = num + 1;
}
println("Prime generation benchmark complete");
println("Primes found: ");
println(prime_count);

// Benchmark 3: Fibonacci Sequence (Recursive-style with loops)
println("Benchmark 3: Fibonacci Sequence Generation");
let fib_count: i32 = 35;
let fib_iterations: i32 = 1000;

println("Fibonacci numbers to calculate: 35");
println("Iterations: 1,000");

let fib_total: i32 = 0;
let fib_iter: i32 = 0;

println("Starting Fibonacci benchmark...");
while (fib_iter < fib_iterations) {
    let n: i32 = 0;
    while (n <= fib_count) {
        // Calculate nth Fibonacci number iteratively
        let a: i32 = 0;
        let b: i32 = 1;
        let fib_result: i32 = 0;
        
        if (n == 0) {
            fib_result = 0;
        } else {
            if (n == 1) {
                fib_result = 1;
            } else {
                let fib_i: i32 = 2;
                while (fib_i <= n) {
                    fib_result = a + b;
                    a = b;
                    b = fib_result;
                    fib_i = fib_i + 1;
                }
            }
        }
        
        fib_total = fib_total + fib_result;
        n = n + 1;
    }
    fib_iter = fib_iter + 1;
}
println("Fibonacci benchmark complete");
println("Total Fibonacci sum: ");
println(fib_total);

// Benchmark 4: Array Sorting Simulation (Bubble Sort)
println("Benchmark 4: Array Sorting Simulation");
let sort_size: i32 = 1000;
let sort_iterations: i32 = 50;

println("Array size: 1,000");
println("Iterations: 50");

// Create array to sort
let sort_array: i32[] = [];
let sort_init: i32 = 0;

// We'll simulate with a smaller fixed array due to language limitations
let test_array: i32[] = [64, 34, 25, 12, 22, 11, 90, 88, 76, 50, 42, 30, 18, 95, 70];
let test_size: i32 = test_array.length;

let sort_iter: i32 = 0;
let sort_operations: i32 = 0;

println("Starting sorting benchmark...");
while (sort_iter < sort_iterations) {
    // Bubble sort implementation
    let i: i32 = 0;
    while (i < test_size - 1) {
        let j: i32 = 0;
        while (j < test_size - i - 1) {
            if (test_array[j] > test_array[j + 1]) {
                // Swap elements
                let temp: i32 = test_array[j];
                test_array[j] = test_array[j + 1];
                test_array[j + 1] = temp;
                sort_operations = sort_operations + 1;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    
    // Reset array for next iteration (reverse it)
    let reset_i: i32 = 0;
    let reset_j: i32 = test_size - 1;
    while (reset_i < reset_j) {
        let temp: i32 = test_array[reset_i];
        test_array[reset_i] = test_array[reset_j];
        test_array[reset_j] = temp;
        reset_i = reset_i + 1;
        reset_j = reset_j - 1;
    }
    
    sort_iter = sort_iter + 1;
}
println("Sorting benchmark complete");
println("Sort operations: ");
println(sort_operations);

println("=== Intensive Benchmark Complete ===");
