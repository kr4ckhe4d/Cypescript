// Property access is checked against the shape the compiler knows: a class's
// declared members, an interface's members, or the literal a variable was
// initialized from. Unknown types stay unchecked, so dynamic values still work.

class Point {
    x: i32 = 1;
    y: i32 = 2;
    sum(): i32 { return this.x + this.y; }
}
class Point3 extends Point { z: i32 = 3; }

let p: Point3 = new Point3();
println(p.x);          // inherited field
println(p.z);          // own field
println(p.sum());      // inherited method

interface Named { name: string; }
interface Tagged extends Named { tag: i32; }
let t: Tagged = { name: "a", tag: 7 };
println(t.name);       // inherited interface member
println(t.tag);

let config = { host: "localhost", port: 8080, nested: { depth: 2 } };
println(config.host);
println(config.nested.depth);

let numbers: i32[] = [1, 2, 3];
println(numbers.length);

// A dynamic value is not policed
let parsed = JSON.parse("{\"anything\": 1}");
println(parsed.whatever);
