// Simple comprehensive string array test
let names: string[] = ["Alice", "Bob", "Charlie", "Diana", "Eve"];
let cities: string[] = ["New York", "London", "Tokyo"];

println("=== String Array Test ===");

// Test basic access
println("First name:");
println(names[0]);
println("Last name:");
println(names[4]);

// Test in loops
println("All names:");
for (let i: i32 = 0; i < 5; i = i + 1) {
    println(names[i]);
}

println("All cities:");
for (let j: i32 = 0; j < 3; j = j + 1) {
    println(cities[j]);
}

// Test in conditionals
if (names[0] == "Alice") {
    println("✓ String comparison works!");
}

if (cities[1] == "London") {
    println("✓ Multiple arrays work!");
}

println("=== Test Complete ===");
