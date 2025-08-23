// Test array.length property
let numbers: i32[] = [10, 20, 30, 40, 50];
let names: string[] = ["Alice", "Bob", "Charlie"];
let empty: i32[] = [];

println("=== Array Length Test ===");

// Test basic length access
println("Numbers array length:");
println(numbers.length);

println("Names array length:");
println(names.length);

println("Empty array length:");
println(empty.length);

// Test length in expressions
let total: i32 = numbers.length + names.length;
println("Total length:");
println(total);

// Test length in loops (the main use case!)
println("Numbers using length in loop:");
for (let i: i32 = 0; i < numbers.length; i = i + 1) {
    println(numbers[i]);
}

println("Names using length in loop:");
for (let j: i32 = 0; j < names.length; j = j + 1) {
    println(names[j]);
}

// Test length in conditionals
if (numbers.length > 3) {
    println("Numbers array has more than 3 elements!");
}

if (names.length == 3) {
    println("Names array has exactly 3 elements!");
}

println("=== Test Complete ===");
