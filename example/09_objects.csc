// 09 — Objects
// Object literals, property access/assignment, nesting, printing, JSON-style output.

let user = {
    name: "Alice Johnson",
    age: 28,
    role: "Developer",
    active: true
};

// Property access
println(user.name);     // Alice Johnson
println(user.age);      // 28
println(user.active);   // 1

// Property assignment
user.age = 29;
user.name = "Alice J.";
println(user.age);      // 29

// Printing an object emits its JSON representation
println(user);          // {"name":"Alice J.","age":29,...}

// Nested objects
let company = {
    name: "TechCorp",
    employee: {
        name: "Bob",
        age: 35
    }
};
println(company.employee.name);  // Bob
println(company.employee.age);   // 35
