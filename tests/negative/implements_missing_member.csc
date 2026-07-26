// EXPECT: does not implement 'draw' required by interface 'Drawable'
interface Drawable { x: f64; draw(): void; }
class Broken implements Drawable {
    x: f64 = 0.0;
}
let b: Broken = new Broken();
println(b.x);
