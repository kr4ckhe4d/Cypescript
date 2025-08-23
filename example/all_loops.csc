// Comprehensive Loop Testing Example
// Tests all implemented loop constructs: while, for, do-while

println("=== All Loop Types Demo ===");

// 1. Traditional For Loop
println("=== For Loop Test ===");
println("Counting 1 to 5 with for loop:");

for (let i: i32 = 1; i <= 5; i = i + 1) {
    print("For loop iteration: ");
    println(i);
}

// 2. While Loop (we already tested this)
println("=== While Loop Test ===");
println("Counting down from 3:");

let countdown: i32 = 3;
while (countdown > 0) {
    print("While countdown: ");
    println(countdown);
    countdown = countdown - 1;
}

// 3. Do-While Loop
println("=== Do-While Loop Test ===");
println("Do-while executes at least once:");

let doCounter: i32 = 0;
do {
    print("Do-while iteration: ");
    println(doCounter);
    doCounter = doCounter + 1;
} while (doCounter < 3);

// 4. Nested Loops
println("=== Nested Loops Test ===");
println("Multiplication table (2x2):");

for (let row: i32 = 1; row <= 2; row = row + 1) {
    for (let col: i32 = 1; col <= 2; col = col + 1) {
        let product: i32 = row * col;
        print("Row ");
        print(row);
        print(" Col ");
        print(col);
        print(" = ");
        println(product);
    }
}

// 5. Complex Loop with Conditions
println("=== Complex Loop Logic ===");
println("Finding even numbers 1-10:");

for (let num: i32 = 1; num <= 10; num = num + 1) {
    let remainder: i32 = num % 2;
    if (remainder == 0) {
        print("Even number found: ");
        println(num);
    }
}

println("=== All Loops Demo Complete ===");
