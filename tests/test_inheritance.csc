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

// --- DISPATCH IS STATIC, NOT VIRTUAL ---
// The method chosen depends on the type the compiler knows at the call site,
// not on the object's runtime class. There are no vtables. Reaching a Square
// through a `Shape`-typed slot therefore runs Shape's area(), not Square's.
let viaParent: Shape = shapes[0];
println(viaParent.area());   // 0 — Shape's area, even though it is a Square
