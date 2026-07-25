// && and || build a phi from both arms, so the two sides must agree on type.
// Mixing an i32 boolean field with an i1 comparison used to emit invalid IR:
// "PHI node operands are not the same type as the result".

class Entity { alive: boolean = true; hp: i32 = 10; radius: f64 = 2.5; }

let e: Entity = new Entity();

// boolean field && comparison  (i32 && i1)
if (e.alive && e.radius > 1.0) { println("bool && comparison"); }

// comparison && boolean field  (i1 && i32)
if (e.hp > 5 && e.alive) { println("comparison && bool"); }

// same again for ||
if (e.alive || e.hp > 100) { println("bool || comparison"); }
if (e.hp > 100 || e.alive) { println("comparison || bool"); }

// chained, mixed both ways
if (e.alive && e.hp > 5 && e.radius < 10.0) { println("chained mixed"); }

// short-circuiting still holds: the right side must not run when skipped
let calls: i32 = 0;
function bump(): boolean { calls += 1; return true; }
if (false && bump()) { println("unreachable"); }
println(calls);
if (true || bump()) { println("or short-circuits"); }
println(calls);

// The value-returning || idiom still yields a usable pointer rather than a
// boolean — this is what `map.get(k) || []` relies on.
let chosen: string = "primary" || "fallback";
println(chosen);

let missing: Entity = null;
let fallback: Entity = new Entity();
let resolved: Entity = missing || fallback;
println(resolved.hp);
