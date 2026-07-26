// Virtual dispatch: the method chosen follows the object's runtime class, not
// the type the compiler sees at the call site.

class Shape {
    label: string = "shape";
    area(): f64 { return 0.0; }
    // Inherited, and calls a virtual method — so this dispatches per subclass
    describe(): void { println(`${this.label} area=${this.area()}`); }
}

class Circle extends Shape {
    radius: f64 = 1.0;
    constructor(r: f64) { this.label = "circle"; this.radius = r; }
    area(): f64 { return 3.14159 * this.radius * this.radius; }
}

class Square extends Shape {
    side: f64 = 1.0;
    constructor(s: f64) { this.label = "square"; this.side = s; }
    area(): f64 { return this.side * this.side; }
}

// Three levels deep: Cube overrides Square's override
class Cube extends Square {
    constructor(s: f64) { this.label = "cube"; this.side = s; }
    area(): f64 { return 6.0 * this.side * this.side; }
}

let shapes: Shape[] = [];
shapes.push(new Circle(2.0));
shapes.push(new Square(3.0));
shapes.push(new Cube(2.0));
shapes.push(new Shape());

// Through a parent-typed for...of binding
for (const s of shapes) { s.describe(); }

// Through parent-typed indexing
let total: f64 = 0.0;
let i: i32 = 0;
while (i < shapes.length) {
    total += shapes[i].area();
    i += 1;
}
println(total);

// Through a parent-typed parameter
function areaOf(s: Shape): f64 { return s.area(); }
println(areaOf(new Cube(1.0)));
println(areaOf(new Shape()));

// Through a parent-typed local
let asShape: Shape = new Circle(1.0);
println(asShape.area());

// A method a subclass does NOT override still resolves to the parent's
class Silent extends Shape { }
let quiet: Shape = new Silent();
println(quiet.area());
