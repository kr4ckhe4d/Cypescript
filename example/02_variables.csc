// 02 — Variables and Types
// let = mutable, const = immutable (reassigning a const is a compile error).
// Types: string, i32, f64, boolean. `number` also works (compiles to f64).

let message: string = "typed string";
let count: i32 = 42;
let pi: f64 = 3.14159;
let active: boolean = true;

println(message);   // typed string
println(count);     // 42
println(pi);        // 3.14159
println(active);    // 1  (booleans print as 1/0)

// Type inference — annotations are optional
let inferredInt = 7;
let inferredStr = "no annotation needed";
let inferredFloat = 2.5;
println(inferredInt);
println(inferredStr);
println(inferredFloat);

// const cannot be reassigned:
const LIMIT: i32 = 100;
println(LIMIT);
// LIMIT = 200;   // <- uncomment for a compile error
