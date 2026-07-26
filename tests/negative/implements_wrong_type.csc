// EXPECT: but interface 'Sized' requires 'f64'
interface Sized { size: f64; }
class Wrong implements Sized {
    size: string = "big";
}
let w: Wrong = new Wrong();
println(w.size);
