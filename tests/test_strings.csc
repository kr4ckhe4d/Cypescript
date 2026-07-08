// Tests: string concatenation with +, mixed-type concat, template literals

let first: string = "Hello";
let second: string = "World";
let joined: string = first + ", " + second + "!";
println(joined); // Hello, World!

// Concatenating numbers and booleans
let age: i32 = 28;
println("Age: " + age);          // Age: 28
let pi: f64 = 3.14;
println("Pi is about " + pi);    // Pi is about 3.14

// Template literals with interpolation
let name: string = "Alice";
println(`Hello ${name}!`);                       // Hello Alice!
println(`Next year: ${age + 1}`);                // Next year: 29
println(`${name} is ${age} years old`);          // Alice is 28 years old

// Template literal with only an expression
println(`${1 + 2 + 3}`); // 6

// String equality still works
if (first == "Hello") {
    println("comparison ok");
}

// Concatenation in a loop
let acc: string = "";
for (let i: i32 = 0; i < 3; i++) {
    acc = acc + "x";
}
println(acc); // xxx
