// 10 — Interfaces
// Structural typing checked at compile time. A missing or wrongly-typed
// property in the object literal is a compile error.

interface Point {
    x: i32;
    y: i32;
}

interface User {
    name: string;
    age: i32;
}

// Interface inheritance
interface Admin extends User {
    level: i32;
}

let origin: Point = { x: 0, y: 0 };
let p: Point = { x: 3, y: 4 };
println(p.x + p.y);     // 7

let user: User = { name: "Alice", age: 28 };
println(user.name);     // Alice

let admin: Admin = { name: "Bob", age: 35, level: 9 };
println(admin.level);   // 9

// Extra properties beyond the interface are allowed:
let tagged: User = { name: "Carol", age: 41, city: "Berlin" };
println(tagged.city);   // Berlin

// These would be compile errors — try uncommenting:
// let bad1: User = { name: "Dave" };              // missing 'age'
// let bad2: User = { name: 123, age: 28 };        // 'name' is not a string
