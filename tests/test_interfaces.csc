// Tests: interface declarations, structural typing, extends

interface User {
    name: string;
    age: i32;
}

interface Admin extends User {
    level: i32;
}

let user: User = { name: "Alice", age: 28 };
println(user.name); // Alice
println(user.age);  // 28

let admin: Admin = { name: "Bob", age: 35, level: 9 };
println(admin.name);  // Bob
println(admin.level); // 9

// Interfaces in function parameter positions (objects are passed by pointer)
interface Point {
    x: i32;
    y: i32;
}

let origin: Point = { x: 0, y: 0 };
let p: Point = { x: 3, y: 4 };
println(p.x + p.y); // 7

// Extra properties beyond the interface are allowed
let detailed: User = { name: "Carol", age: 41, city: "Berlin" };
println(detailed.city); // Berlin
