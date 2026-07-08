// 12 — Object Destructuring
// Pull properties out of an object into local bindings.

let user = {
    name: "Alice",
    age: 28,
    active: true
};

let { name, age } = user;
println(name);    // Alice
println(age);     // 28

// const destructuring makes the bindings immutable
const { active } = user;
println(active);  // 1

// Bindings are copies — reassigning them does not touch the object
let point = { x: 10, y: 20 };
let { x, y } = point;
x = 999;
println(x);        // 999
println(point.x);  // 10

// Great with template literals
println(`${name} is ${age} years old`);
