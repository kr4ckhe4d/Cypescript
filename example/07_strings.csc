// 07 — Strings and Template Literals
// Concatenation with +, interpolation with backticks, and stdlib helpers.

let firstName: string = "Ada";
let lastName: string = "Lovelace";

// + concatenates strings, and mixes in numbers/booleans automatically
let full: string = firstName + " " + lastName;
println(full);                       // Ada Lovelace
println("Born in " + 1815);          // Born in 1815

// Template literals: `${expression}` is evaluated and interpolated
let age: i32 = 28;
println(`${firstName} is ${age} years old`);
println(`Next year: ${age + 1}`);

// Escape sequences
println("Tab:\tNewline follows:\nQuoted: \"hi\"");

// String comparison
if (firstName == "Ada") {
    println("comparison works");
}

// C++ stdlib string helpers (linked automatically)
println(string_upper(full));         // ADA LOVELACE
println(string_reverse(firstName));  // adA
println(string_length(full));        // 12
println(string_substring(full, 0, 3)); // Ada
