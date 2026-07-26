// EXPECT: Type mismatch in declaration of 'x'
// Two unrelated classes are not interchangeable, even though both are handles.
class Rock { size: i32 = 1; }
class Bullet { speed: i32 = 1; }
let x: Rock = new Bullet();
println(x.size);
