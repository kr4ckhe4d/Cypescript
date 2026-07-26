// EXPECT: Type mismatch in declaration of 'n'
// A class field's declared type is known, so reading it is checked too.
class User { name: string = "anon"; }
let u: User = new User();
let n: i32 = u.name;
println(n);
