// Tests: object destructuring

let user = { name: "Alice", age: 28, active: true };

let { name, age } = user;
println(name); // Alice
println(age);  // 28

const { active } = user;
println(active); // 1

// Destructured values are copies of properties
let point = { x: 10, y: 20 };
let { x, y } = point;
println(x + y); // 30

// Use destructured bindings in expressions and template literals
println(`${name} is ${age}`); // Alice is 28
