// EXPECT: mixes pointer and numeric members
// A union is only representable when its members share a machine representation.
let x: string | i32 = 5;
println(x);
