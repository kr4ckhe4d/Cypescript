// EXPECT: Type mismatch in argument 1 of 'double'
function double(x: i32): i32 { return x * 2; }
println(double("not a number"));
