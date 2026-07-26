// EXPECT: Cannot reassign const variable 'limit'
const limit: i32 = 10;
limit++;
println(limit);
