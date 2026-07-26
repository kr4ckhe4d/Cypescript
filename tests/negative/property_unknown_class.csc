// EXPECT: Class 'Point' has no member 'z'
class Point { x: i32 = 0; y: i32 = 0; }
let p: Point = new Point();
println(p.z);
