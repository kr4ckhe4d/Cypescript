// EXPECT: Type mismatch in declaration of 'n'
class Point { x: i32 = 0; }
let p: Point = new Point();
let n: i32 = p;
println(n);
