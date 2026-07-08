// 16 — TypeScript Compatibility
// This file is (almost) plain TypeScript. The goal: run .ts scripts
// natively with minute changes. These idioms work as-is:

// console.log with multiple arguments (space-joined, like JS)
const language: string = "Cypescript";
console.log("Hello from", language, "!");

// number type (compiles to f64)
const radius: number = 2.5;
const area: number = Math.floor(3.14159 * radius * radius);
console.log("area (floored):", area);

// Math.* functions
console.log("sqrt(144) =", Math.sqrt(144));
console.log("2^10 =", Math.pow(2, 10));
console.log("abs(-5.5) =", Math.abs(-5.5));

// Strict equality operators
let x: i32 = 5;
if (x === 5 && x !== 3) {
    console.log("strict equality works");
}

// Template literals, const, interfaces, destructuring — all TS-style
interface Config {
    host: string;
    port: i32;
}
const config: Config = { host: "localhost", port: 8080 };
const { host, port } = config;
console.log(`listening on ${host}:${port}`);

// What still needs changing when porting real TS (see README):
//   - arrow functions -> named functions
//   - classes         -> object literals with methods
//   - npm imports     -> local ./file imports only
//   - i32 for hot integer loops (number = f64 is slower)
