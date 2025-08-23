// Test array assignment operations
let numbers: i32[] = [1, 2, 3, 4, 5];
let names: string[] = ["Alice", "Bob", "Charlie"];

println("=== Array Assignment Test ===");

// Test initial values
println("Initial numbers:");
for (let i: i32 = 0; i < 5; i = i + 1) {
    println(numbers[i]);
}

println("Initial names:");
for (let j: i32 = 0; j < 3; j = j + 1) {
    println(names[j]);
}

// Test integer array assignment
numbers[0] = 100;
numbers[2] = 300;
numbers[4] = 500;

println("After integer assignments:");
for (let k: i32 = 0; k < 5; k = k + 1) {
    println(numbers[k]);
}

// Test string array assignment
names[0] = "NewAlice";
names[1] = "NewBob";
names[2] = "NewCharlie";

println("After string assignments:");
for (let m: i32 = 0; m < 3; m = m + 1) {
    println(names[m]);
}

println("=== Test Complete ===");
