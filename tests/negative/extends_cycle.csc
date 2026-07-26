// EXPECT: Inheritance cycle
class A extends B { a: i32 = 1; }
class B extends A { b: i32 = 2; }
let x: A = new A();
println(x.a);
