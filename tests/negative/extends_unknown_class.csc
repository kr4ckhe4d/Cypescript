// EXPECT: extends unknown class 'Missing'
class Orphan extends Missing {
    x: i32 = 0;
}
let o: Orphan = new Orphan();
println(o.x);
