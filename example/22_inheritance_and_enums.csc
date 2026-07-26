// 22 — Classes with inheritance, and enums
//
// Subclasses inherit fields and methods, override them, and dispatch virtually:
// the implementation that runs follows the object's runtime class, not the type
// the compiler sees at the call site.

enum Kind { Circle, Square, Cube }

class Shape {
    label: string = "shape";
    kind: Kind = Kind.Circle;

    constructor(label: string, kind: Kind) {
        this.label = label;
        this.kind = kind;
    }

    area(): f64 { return 0.0; }

    // Inherited by every subclass, and calls a method they override
    describe(): void { println(`${this.label} (kind ${this.kind}) area=${this.area()}`); }
}

class Circle extends Shape {
    radius: f64 = 1.0;
    constructor(r: f64) {
        super("circle", Kind.Circle);      // the parent's constructor
        this.radius = r;
    }
    area(): f64 { return Math.PI * this.radius * this.radius; }
}

class Square extends Shape {
    side: f64 = 1.0;
    constructor(s: f64) {
        super("square", Kind.Square);
        this.side = s;
    }
    area(): f64 { return this.side * this.side; }
}

class Cube extends Square {
    constructor(s: f64) {
        super(s);                          // Square's constructor
        this.label = "cube";
        this.kind = Kind.Cube;
    }
    // Reuses the implementation it overrides
    area(): f64 { return 6.0 * super.area(); }
}

let shapes: Shape[] = [];
shapes.push(new Circle(2.0));
shapes.push(new Square(3.0));
shapes.push(new Cube(2.0));

// Each element runs its own area(), through a Shape-typed binding
for (const s of shapes) { s.describe(); }

let total: f64 = 0.0;
let i: i32 = 0;
while (i < shapes.length) {
    total += shapes[i].area();
    i += 1;
}
println(`total area ${total}`);

// A grid of enum values — nested arrays hold handles, so rows are real arrays
let board: Kind[][] = [];
let r: i32 = 0;
while (r < 2) {
    let row: Kind[] = [];
    row.push(Kind.Circle);
    row.push(Kind.Cube);
    board.push(row);
    r += 1;
}
println(board[1][1]);
