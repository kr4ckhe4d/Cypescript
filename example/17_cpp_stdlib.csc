// 17 — C++ Standard Library Integration
// The stdlib (src/cypescript_stdlib.cpp) is linked automatically:
// string helpers, file I/O, random numbers, JSON utilities, math.

// String helpers
let text: string = "Hello World";
println(string_reverse(text));          // dlroW olleH
println(string_upper(text));            // HELLO WORLD
println(string_lower(text));            // hello world
println(string_find(text, "World"));    // 6
println(string_concat(text, " again")); // Hello World again

// Math helpers (also reachable as Math.* — see example 16)
println(math_sqrt(2.0));                // 1.41421
println(math_abs_i32(-42));             // 42

// File I/O
file_write("demo_output.txt", "written by Cypescript");
if (file_exists("demo_output.txt")) {
    println(file_read("demo_output.txt")); // written by Cypescript
}

// Random numbers (seeded for reproducibility)
random_seed(42);
let roll: i32 = random_int(1, 6);
println(`dice roll: ${roll}`);

// You can also add your own C++ functions — see "C++ Integration" in the README:
//   ./compile-with-custom-cpp.sh my_program.csc output src/my_lib.cpp
