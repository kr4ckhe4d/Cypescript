// Union types, and `implements` on a class.
//
// A union is representable when its members share a representation — pointers
// or numerics. Mixing the two has no single machine type, so `string | i32` is
// rejected rather than silently reinterpreted.

interface Drawable { x: f64; draw(): void; }
interface Named { name: string; }

// A class can now declare what it satisfies, and the compiler verifies it
class Sprite implements Drawable, Named {
    x: f64 = 3.5;
    name: string = "sprite";
    draw(): void { println(`${this.name} at ${this.x}`); }
}

let s: Sprite = new Sprite();
s.draw();

// A class still satisfies an interface structurally, declared or not
let d: Drawable = new Sprite();
println("structural still works");

// --- Nullable handles: the union that matters most ---
class Node { value: i32 = 0; }

function findNode(present: boolean): Node | null {
    if (present) { return new Node(); }
    return null;
}

let found: Node | null = findNode(true);
if (found != null) { println(found.value); }

let missing: Node | null = findNode(false);
if (missing == null) { println("not found"); }

// A subclass fits its parent's union
class Leaf extends Node { depth: i32 = 2; }
let anyNode: Node | null = new Leaf();
println(anyNode.value);

// Unions work as parameters
function describe(n: Node | null): string {
    if (n == null) { return "nothing"; }
    return "something";
}
println(describe(null));
println(describe(new Node()));

// --- Numeric unions share a representation too ---
let scalar: i32 | f64 = 5;
println(scalar);
scalar = 2.5;
println(scalar);

// --- `|` is still bitwise-or outside a type position ---
println(5 | 3);
let flag: boolean = false;
if (flag || true) { println("logical or intact"); }
