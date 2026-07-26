// EXPECT: Type mismatch in declaration of 'x'
class Rock { size: i32 = 1; }
class Bullet { speed: i32 = 1; }
let x: Rock | null = new Bullet();
println(x.size);
