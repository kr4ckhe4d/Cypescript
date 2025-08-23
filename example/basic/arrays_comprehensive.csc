// Comprehensive Array Test - Native Compiler
println("=== Comprehensive Array Operations Test ===");

// Test 1: Integer array creation and basic operations
println("Test 1: Integer Arrays");
let numbers: i32[] = [1, 2, 3, 4, 5];
println("Array created successfully!");

// Test 2: Array access
println("Test 2: Array Access");
let first: i32 = numbers[0];
let second: i32 = numbers[1];
let third: i32 = numbers[2];

print("First element: ");
println(first);
print("Second element: ");
println(second);
print("Third element: ");
println(third);

// Test 3: Array operations in expressions
println("Test 3: Array Operations in Expressions");
let sum: i32 = numbers[0] + numbers[1] + numbers[2];
print("Sum of first three elements: ");
println(sum);

let product: i32 = numbers[0] * numbers[1];
print("Product of first two elements: ");
println(product);

// Test 4: Array access in control flow
println("Test 4: Array Access in Control Flow");
if (numbers[0] == 1) {
    println("First element is 1 - correct!");
}

if (numbers[1] > numbers[0]) {
    println("Second element is greater than first - correct!");
}

// Test 5: Array access in loops
println("Test 5: Array Access in Loops");
println("Printing array elements using loops:");

// Using for loop with array access
for (let i: i32 = 0; i < 5; i = i + 1) {
    print("Element ");
    print(i);
    print(": ");
    println(numbers[i]);
}

// Test 6: String arrays
println("Test 6: String Arrays");
let names: string[] = ["Alice", "Bob", "Charlie"];
println("String array created successfully!");

let firstName: string = names[0];
let secondName: string = names[1];

print("First name: ");
println(firstName);
print("Second name: ");
println(secondName);

// Test 7: String array comparisons
println("Test 7: String Array Comparisons");
if (names[0] == "Alice") {
    println("First name is Alice - correct!");
}

if (names[1] != "Charlie") {
    println("Second name is not Charlie - correct!");
}

// Test 8: Mixed array operations
println("Test 8: Mixed Array Operations");
let scores: i32[] = [95, 87, 92, 78, 88];
let studentNames: string[] = ["Alice", "Bob", "Carol", "David", "Eve"];

// Find highest score
let maxScore: i32 = scores[0];
let maxIndex: i32 = 0;

for (let i: i32 = 1; i < 5; i = i + 1) {
    if (scores[i] > maxScore) {
        maxScore = scores[i];
        maxIndex = i;
    }
}

print("Highest score: ");
println(maxScore);
print("Student with highest score: ");
println(studentNames[maxIndex]);

// Test 9: Array calculations
println("Test 9: Array Calculations");
let totalScore: i32 = 0;
for (let i: i32 = 0; i < 5; i = i + 1) {
    totalScore = totalScore + scores[i];
}

let averageScore: i32 = totalScore / 5;
print("Total score: ");
println(totalScore);
print("Average score: ");
println(averageScore);

// Test 10: Complex array expressions
println("Test 10: Complex Array Expressions");
let result1: i32 = (numbers[0] + numbers[1]) * numbers[2];
let result2: i32 = numbers[4] - (numbers[1] + numbers[2]);

print("(numbers[0] + numbers[1]) * numbers[2] = ");
println(result1);
print("numbers[4] - (numbers[1] + numbers[2]) = ");
println(result2);

// Test 11: Array bounds testing (within valid range)
println("Test 11: Array Bounds Testing");
let lastElement: i32 = numbers[4];
print("Last element of numbers array: ");
println(lastElement);

let lastScore: i32 = scores[4];
print("Last score: ");
println(lastScore);

// Test 12: Array comparison operations
println("Test 12: Array Comparison Operations");
if (scores[0] > scores[3]) {
    print(studentNames[0]);
    print(" scored higher than ");
    println(studentNames[3]);
}

if (scores[2] >= 90) {
    print(studentNames[2]);
    println(" has an excellent score!");
}

println("");
println("=== Array Operations Summary ===");
println("✓ Integer array creation and access");
println("✓ String array creation and access");
println("✓ Array operations in expressions");
println("✓ Array access in control flow");
println("✓ Array access in loops");
println("✓ Array comparisons");
println("✓ Complex array calculations");
println("✓ Mixed data type arrays");

println("");
println("All array operations completed successfully!");
println("Native compiler array support is working excellently!");
