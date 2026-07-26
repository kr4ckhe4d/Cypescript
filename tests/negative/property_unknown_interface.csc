// EXPECT: Object has no property 'email'
interface User { name: string; age: i32; }
let u: User = { name: "a", age: 1 };
println(u.email);
