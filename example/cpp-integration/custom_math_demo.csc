// Custom C++ Integration Demo
// This demonstrates using custom C++ libraries with Cypescript

println("=== Custom C++ Math Library Demo ===");

// Advanced Math Functions
println("Advanced Math Functions:");
let a: i32 = 48;
let b: i32 = 18;
let gcd_result: i32 = math_gcd(a, b);
print("GCD of 48 and 18: ");
println(gcd_result);

let lcm_result: i32 = math_lcm(a, b);
print("LCM of 48 and 18: ");
println(lcm_result);

let test_num: i32 = 17;
let is_prime: i32 = math_is_prime(test_num);
print("Is 17 prime? ");
if (is_prime == 1) {
    println("Yes");
} else {
    println("No");
}

let fib_n: i32 = 10;
let fib_result: i32 = math_fibonacci(fib_n);
print("10th Fibonacci number: ");
println(fib_result);

let fact_n: i32 = 6;
let fact_result: i32 = math_factorial(fact_n);
print("6! = ");
println(fact_result);

// Statistics Functions
println("Statistics Functions:");
let numbers: i32[] = [10, 20, 30, 40, 50, 15, 25, 35];
let array_size: i32 = numbers.length;

// Note: These functions return doubles, but we'll need to add f64 support
// For now, we can call them but can't store the results in Cypescript
// This demonstrates the limitation and future enhancement needed

println("Array: [10, 20, 30, 40, 50, 15, 25, 35]");
println("Array size: ");
println(array_size);

// We can call the functions, but can't store f64 results yet
// This shows what will be possible when f64 support is added:
// let mean: f64 = stats_mean(numbers, array_size);
// let median: f64 = stats_median(numbers, array_size);
// let stddev: f64 = stats_stddev(numbers, array_size);

println("Mean, median, and standard deviation functions available");
println("(f64 return types require future language support)");

// Integer-only math functions work perfectly
println("Prime checking:");
for (let i: i32 = 2; i <= 20; i = i + 1) {
    let prime_check: i32 = math_is_prime(i);
    if (prime_check == 1) {
        print("Prime: ");
        println(i);
    }
}

println("Fibonacci sequence (first 10):");
for (let i: i32 = 0; i < 10; i = i + 1) {
    let fib: i32 = math_fibonacci(i);
    print("F(");
    print(i);
    print(") = ");
    println(fib);
}

println("=== Custom C++ Integration Complete ===");
