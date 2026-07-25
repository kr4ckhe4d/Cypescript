// Bitwise operators, hex/binary literals, and null — all of which used to be
// parse errors or silently wrong.

println(5 & 3);
println(5 | 3);
println(5 ^ 3);
println(1 << 4);
println(256 >> 4);
println(~0);

// Literal bases: 0xFF used to parse as 0 (std::stoll defaulted to base 10)
println(0xFF);
println(0xDEAD);
println(0b1010);
println(0755);

// Precedence: & binds tighter than |, both looser than comparison
println(1 | 2 & 2);
println((1 | 2) & 2);

// Packing a color, the reason this was needed
let color: i32 = (17 << 16) | (34 << 8) | 51;
println(color);

// Shifts must not be confused with nested generics
let m = new Map<string, Set<i32>>();
println("generics still close");

// null as an expression
let handle: ptr = null;
if (handle == null) { println("null compares equal"); }
