// Comprehensive string array test
let names: string[] = ["Alice", "Bob", "Charlie", "Diana", "Eve"];
let cities: string[] = ["New York", "London", "Tokyo"];

println("=== String Array Test ===");

// Test basic access
println("First name: " + names[0]);
println("Last name: " + names[4]);

// Test in loops
println("\nAll names:");
for (let i: i32 = 0; i < 5; i = i + 1) {
    print("Name ");
    print(i);
    print(": ");
    println(names[i]);
}

println("\nAll cities:");
for (let j: i32 = 0; j < 3; j = j + 1) {
    print("City ");
    print(j);
    print(": ");
    println(cities[j]);
}

// Test in conditionals
if (names[0] == "Alice") {
    println("\n✓ String comparison works!");
}

if (cities[1] == "London") {
    println("✓ Multiple arrays work!");
}

println("\n=== Test Complete ===");
