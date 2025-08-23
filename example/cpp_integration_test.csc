// Comprehensive C++ Integration Test
// This demonstrates calling C++ functions from Cypescript

println("=== C++ Integration Test ===");

// Math functions (integer only - f64 not yet supported)
println("Math Functions:");
let abs_result: i32 = math_abs_i32(42);  // Using positive number since -42 not supported
println("abs(42) = ");
println(abs_result);

// String functions
println("String Functions:");
let text: string = "Hello World";
let reversed: string = string_reverse(text);
println("Original: ");
println(text);
println("Reversed: ");
println(reversed);

let upper: string = string_upper(text);
println("Uppercase: ");
println(upper);

let lower: string = string_lower(text);
println("Lowercase: ");
println(lower);

let length: i32 = string_length(text);
println("Length: ");
println(length);

let substring: string = string_substring(text, 0, 5);
println("Substring(0,5): ");
println(substring);

let find_pos: i32 = string_find(text, "World");
println("Find 'World': ");
println(find_pos);

let concat: string = string_concat("Hello", " C++");
println("Concat result: ");
println(concat);

// Array functions
println("Array Functions:");
let numbers: i32[] = [10, 5, 8, 3, 12, 7];
let sum: i32 = array_sum_i32(numbers, numbers.length);
println("Array sum: ");
println(sum);

let max: i32 = array_max_i32(numbers, numbers.length);
println("Array max: ");
println(max);

let min: i32 = array_min_i32(numbers, numbers.length);
println("Array min: ");
println(min);

// File I/O functions
println("File I/O Functions:");
let write_result: i32 = file_write("test.txt", "Hello from Cypescript!");
println("File write result: ");
println(write_result);

let exists: i32 = file_exists("test.txt");
println("File exists: ");
println(exists);

let content: string = file_read("test.txt");
println("File content: ");
println(content);

// Utility functions
println("Utility Functions:");
random_seed(42);
let rand1: i32 = random_int(1, 100);
println("Random int (1-100): ");
println(rand1);

let rand2: i32 = random_int(1, 100);
println("Another random int: ");
println(rand2);

println("=== Test Complete ===");
