// Comprehensive Cypescript Example
// Demonstrates all implemented features: loops, arithmetic, comparisons, control flow

println("=== Cypescript Comprehensive Demo ===");

// Factorial calculation using while loop
let n: i32 = 5;
let factorial: i32 = 1;
let counter: i32 = 1;

print("Calculating factorial of ");
println(n);

while (counter <= n) {
    factorial = factorial * counter;
    print("Step ");
    print(counter);
    print(": ");
    println(factorial);
    counter = counter + 1;
}

print("Final result: ");
print(n);
print("! = ");
println(factorial);

// Prime number detection using for loop
println("=== Prime Number Detection ===");
let testNum: i32 = 17;
let isPrime: i32 = 1; // 1 = true, 0 = false

print("Testing if ");
print(testNum);
println(" is prime...");

if (testNum <= 1) {
    isPrime = 0;
} else {
    for (let divisor: i32 = 2; divisor * divisor <= testNum; divisor = divisor + 1) {
        let remainder: i32 = testNum % divisor;
        if (remainder == 0) {
            isPrime = 0;
            print("Found divisor: ");
            println(divisor);
        }
    }
}

if (isPrime == 1) {
    print(testNum);
    println(" is PRIME!");
} else {
    print(testNum);
    println(" is NOT prime");
}

// Fibonacci sequence using do-while
println("=== Fibonacci Sequence ===");
let limit: i32 = 10;
let fib1: i32 = 0;
let fib2: i32 = 1;
let fibCount: i32 = 0;

print("First ");
print(limit);
println(" Fibonacci numbers:");

do {
    if (fibCount == 0) {
        print("F(");
        print(fibCount);
        print(") = ");
        println(fib1);
    } else {
        if (fibCount == 1) {
            print("F(");
            print(fibCount);
            print(") = ");
            println(fib2);
        } else {
            let nextFib: i32 = fib1 + fib2;
            print("F(");
            print(fibCount);
            print(") = ");
            println(nextFib);
            fib1 = fib2;
            fib2 = nextFib;
        }
    }
    fibCount = fibCount + 1;
} while (fibCount < limit);

// Complex nested loops and conditions
println("=== Nested Loops and Conditions ===");
println("Finding perfect squares up to 50:");

for (let i: i32 = 1; i <= 7; i = i + 1) {
    let square: i32 = i * i;
    if (square <= 50) {
        print(i);
        print("² = ");
        println(square);
    }
}

// Arithmetic progression
println("=== Arithmetic Progression ===");
let start: i32 = 5;
let diff: i32 = 3;
let terms: i32 = 6;

print("Arithmetic progression: start=");
print(start);
print(", difference=");
println(diff);

for (let term: i32 = 0; term < terms; term = term + 1) {
    let value: i32 = start + term * diff;
    print("Term ");
    print(term + 1);
    print(": ");
    println(value);
}

println("=== Comprehensive Demo Complete ===");
