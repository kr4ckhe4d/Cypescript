// EXPECT: Type mismatch in return value: expected 'i32', got 'string'
function count(): i32 {
    return "three";
}
println(count());
