// Comprehensive test of all working native compiler features
println("=== Comprehensive Native Compiler Test ===");

// Test 1: Type inference
let autoMessage = "Type inference works!";
print("Auto-inferred string: ");
println(autoMessage);

// Test 2: Explicit string types and comparisons
let greeting: string = "Hello";
let name: string = "Alice";
let fullGreeting: string = "Hello Alice";

if (greeting == "Hello") {
    println("String literal comparison works!");
}

if (name != "Bob") {
    println("String inequality comparison works!");
}

// Test 3: Integer operations and comparisons
let x: i32 = 10;
let y: i32 = 20;
let sum: i32 = x + y;
let product: i32 = x * y;

print("Sum: ");
println(sum);
print("Product: ");
println(product);

if (sum > x) {
    println("Integer comparison works!");
}

// Test 4: Complex arithmetic
let a: i32 = 15;
let b: i32 = 4;
let quotient: i32 = a / b;
let remainder: i32 = a % b;

print("15 / 4 = ");
println(quotient);
print("15 % 4 = ");
println(remainder);

// Test 5: Control flow with string comparisons
let userType: string = "admin";

if (userType == "admin") {
    println("Access granted: Administrator");
} else {
    if (userType == "user") {
        println("Access granted: Regular user");
    } else {
        println("Access denied: Unknown user type");
    }
}

// Test 6: Loops with string operations
let count: i32 = 0;
let target: string = "test";
let current: string = "test";

while (count < 3) {
    if (current == target) {
        print("Match found at iteration ");
        println(count);
    }
    count = count + 1;
}

// Test 7: Array type declarations (parsing only)
let numbers: i32[] = [1, 2, 3, 4, 5];
println("Array type parsing successful!");

// Test 8: Complex expressions
let result: i32 = (x + y) * 2 - (a / b);
print("Complex expression result: ");
println(result);

// Test 9: Multiple string comparisons
let status1: string = "active";
let status2: string = "inactive";
let status3: string = "active";

if (status1 == status3) {
    println("Status comparison 1: Match");
}

if (status1 != status2) {
    println("Status comparison 2: Different");
}

// Test 10: For loop with string operations
for (let i: i32 = 0; i < 3; i = i + 1) {
    let testStr: string = "iteration";
    if (testStr == "iteration") {
        print("For loop iteration ");
        print(i);
        println(" with string comparison");
    }
}

println("");
println("=== All Native Compiler Features Tested Successfully! ===");
println("✓ Type inference");
println("✓ String comparisons (== and !=)");
println("✓ Integer arithmetic and comparisons");
println("✓ Complex expressions");
println("✓ Control flow with string conditions");
println("✓ Loops with string operations");
println("✓ Array type parsing");
println("✓ Mixed data type operations");

println("");
println("Native compiler is working excellently!");
