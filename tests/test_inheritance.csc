// `class extends`: a subclass lays its parent's fields out first, so the two
// share a struct prefix. Methods are inherited and can be overridden.

class Animal {
    name: string = "animal";
    legs: i32 = 4;

    describe(): void { println(`${this.name} has ${this.legs} legs`); }
    speak(): void { println("..."); }
}

class Dog extends Animal {
    speak(): void { println("woof"); }
}

let d: Dog = new Dog();
d.describe();          // inherited method, inherited fields
d.speak();             // overridden
println(d.legs);       // inherited field

// --- Constructors are inherited, and chains can be several deep ---
class Shape {
    label: string = "shape";
    sides: i32 = 0;
    constructor(label: string, sides: i32) { this.label = label; this.sides = sides; }
    area(): f64 { return 0.0; }
}
class Polygon extends Shape {
    filled: boolean = false;
}
class Square extends Polygon {
    size: f64 = 2.0;
    area(): f64 { return this.size * this.size; }
}

let sq: Square = new Square("square", 4);   // Shape's constructor, two levels up
println(sq.label);
println(sq.sides);
println(sq.filled);
println(sq.area());

// A subclass field shadows nothing; both are reachable
sq.size = 3.0;
println(sq.area());

// --- Subclass instances live in parent-typed arrays ---
let shapes: Shape[] = [];
shapes.push(new Square("a", 4));
shapes.push(new Shape("b", 3));
println(shapes.length);
println(shapes[0].label);
println(shapes[1].sides);

// --- DISPATCH IS VIRTUAL ---
// The method chosen follows the object's runtime class, not the type at the
// call site. Reaching a Square through a `Shape`-typed slot runs Square's
// area(). (This test previously asserted the opposite — the limitation it
// pinned down was removed when virtual dispatch landed.)
// shapes[0] is a Square that kept the default size of 2.0
let viaParent: Shape = shapes[0];
println(viaParent.area());   // 4 — Square's area, reached through a Shape slot

function areaOf(s: Shape): f64 { return s.area(); }
println(areaOf(new Square("x", 4)));   // 4 — dispatches through a parameter too
