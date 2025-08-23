// Test the native compiler fixes
println("=== Testing Native Compiler Fixes ===");

// Test 1: Type inference (no explicit type annotation)
let message = "Hello, World!";
print("Message: ");
println(message);

// Test 2: String comparison
let name1: string = "Alice";
let name2: string = "Alice";
let name3: string = "Bob";

if (name1 == name2) {
    println("String comparison works: Alice == Alice");
}

if (name1 != name3) {
    println("String inequality works: Alice != Bob");
}

// Test 3: Array type parsing (basic)
let numbers: i32[] = [1, 2, 3, 4, 5];
println("Array parsing successful!");

println("=== All Tests Completed ===");
