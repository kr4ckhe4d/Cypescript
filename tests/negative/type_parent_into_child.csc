// EXPECT: Type mismatch in declaration of 'sq'
// A subclass fits its parent's slot, but not the other way round.
class Shape { sides: i32 = 0; }
class Square extends Shape { size: f64 = 1.0; }
let sq: Square = new Shape();
println(sq.sides);
