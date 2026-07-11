// Tests: class declarations, constructors, methods, fields, defaults

class Point {
    x: i32;
    y: i32;

    constructor(x: i32, y: i32) {
        this.x = x;
        this.y = y;
    }

    manhattan(): i32 {
        return this.x + this.y;
    }

    scale(factor: i32): void {
        this.x *= factor;
        this.y *= factor;
    }
}

class Greeter {
    name: string = "world";
    punctuation: string = "!";

    greet(): string {
        return "hello " + this.name + this.punctuation;
    }
}

class Counter {
    value: i32;

    constructor(start: i32) {
        this.value = start;
    }

    increment(): i32 {
        this.value += 1;
        return this.value;
    }
}

let p = new Point(3, 4);
println(p.x);            // 3
println(p.y);            // 4
println(p.manhattan());  // 7
p.scale(2);
println(p.manhattan());  // 14

// Direct field assignment
p.y = 100;
println(p.y);            // 100

// Field defaults (no constructor)
let g = new Greeter();
println(g.greet());      // hello world!
g.name = "cypescript";
println(g.greet());      // hello cypescript!

// Class name as a type annotation
let origin: Point = new Point(0, 0);
println(origin.manhattan()); // 0

// Multiple instances share layout but not state
let c1 = new Counter(10);
let c2 = new Counter(100);
println(c1.increment()); // 11
println(c2.increment()); // 101
println(c1.increment()); // 12
