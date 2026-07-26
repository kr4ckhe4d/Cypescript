// EXPECT: Type mismatch in declaration of 'bad': expected 'i32', got 'string'
// Assigning a string to an i32 used to compile clean and print a truncated
// pointer. This is the headline case for the type checker.
let s: string = "hi";
let bad: i32 = s;
println(bad);
