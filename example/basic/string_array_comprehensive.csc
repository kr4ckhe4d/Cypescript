// Comprehensive string array test
let names: string[] = ["Alice", "Bob", "Charlie", "Diana", "Eve"];
let cities: string[] = ["New York", "London", "Tokyo"];

println("=== String Array Test ===");

// Test basic access
print("First name: ");
println(names[0]);
print("Last name: ");
println(names[4]);

// Test in loops
println("All names:");
for (let i: i32 = 0; i < 5; i = i + 1) {
    print("Name ");
    print(i);
    print(": ");
    println(names[i]);
}

println("All cities:");
for (let j: i32 = 0; j < 3; j = j + 1) {
    print("City ");
    print(j);
    print(": ");
    println(cities[j]);
}

// Test in conditionals
if (names[0] == "Alice") {
    println("String comparison works!");
}

if (cities[1] == "London") {
    println("Multiple arrays work!");
}

// Test array length
println("Names array length:");
println(names.length);
println("Cities array length:");
println(cities.length);

// Test more string comparisons
if (names[2] == "Charlie") {
    println("Third name is Charlie");
}

if (cities[0] != "Paris") {
    println("First city is not Paris");
}

println("=== Test Complete ===");
