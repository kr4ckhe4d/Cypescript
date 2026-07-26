// `.prop`, `[i]` and `.method()` can follow any expression, not just a plain
// variable. `f().prop` used to be a parse error: only the variable path chained,
// so every other producer — calls, `new`, literals, parenthesised expressions —
// stopped dead at the following dot.

class Box {
    value: i32 = 0;
    label: string = "box";
    constructor(v: i32) { this.value = v; }
    bump(): i32 { this.value += 1; return this.value; }
}

function make(v: i32): Box { return new Box(v); }

// On a call result
println(make(41).value);
println(make(3).bump());

// On a `new` expression
println(new Box(7).value);
println(new Box(9).bump());

// Feeding an annotated slot, so the type is checked through the chain
let n: i32 = make(5).value;
println(n);

// On literals and parenthesised expressions
println([5, 6, 7][1]);
println((make(2)).value);

// Interleaved with the non-null assertion
let m = new Map<string, string[]>();
m.set("k", ["x", "y", "z"]);
let got: string[] = m.get("k")!;
println(got.length);

// Compound assignment through a call result — needs both this and the
// evaluate-the-target-once fix to be correct
class Counter { hits: i32 = 0; }
let lookups: i32 = 0;
function pick(c: Counter): Counter { lookups += 1; return c; }

let c: Counter = new Counter();
pick(c).hits += 5;
println(c.hits);       // 5
println(lookups);      // 1 — the call is evaluated once, not twice
