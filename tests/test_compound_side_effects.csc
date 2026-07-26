// Compound assignment must evaluate its target exactly once. It used to be
// desugared by re-parsing the left-hand side — `a[f()] += v` became
// `a[f()] = a[f()] + v`, which called f() twice and could read one slot while
// writing another.

let calls: i32 = 0;
function nextIndex(): i32 {
    calls += 1;
    return calls;
}

// --- Array element with a side-effecting index ---
let a: i32[] = [10, 20, 30];
a[nextIndex()] += 100;      // index evaluated once -> a[1]
println(a[0]);              // 10
println(a[1]);              // 120
println(a[2]);              // 30
println(calls);             // 1

// Every operator, still one evaluation each
a[nextIndex()] -= 5;        // a[2] = 25
println(a[2]);
println(calls);             // 2

let b: i32[] = [2, 3, 4, 5];
let k: i32 = 0;
b[k] *= 10;
b[1] /= 3;
b[2] %= 3;
println(b[0]); println(b[1]); println(b[2]);

// ++ and -- on an element
let c: i32[] = [7, 7];
c[0]++;
c[1]--;
println(c[0]); println(c[1]);

// --- f64 elements ---
let f: f64[] = [1.5, 2.5];
f[0] += 0.25;
f[1] *= 2.0;
println(f[0]); println(f[1]);

// --- String elements concatenate ---
let s: string[] = ["a", "b"];
s[0] += "bc";
println(s[0]);

// --- Object property whose object expression has a side effect ---
class Counter { hits: i32 = 0; }
let boxes: Counter[] = [];
boxes.push(new Counter());
boxes.push(new Counter());

let lookups: i32 = 0;
function pick(): i32 {
    lookups += 1;
    return 1;
}
boxes[pick()].hits += 5;    // pick() evaluated once -> boxes[1]
println(boxes[0].hits);     // 0
println(boxes[1].hits);     // 5
println(lookups);           // 1

let box: Counter = boxes[1];
box.hits *= 3;
println(box.hits);          // 15
box.hits++;
println(box.hits);          // 16

// --- Plain variables still behave ---
let n: i32 = 10;
n += 5;
n -= 3;
n *= 2;
n /= 4;
println(n);                 // 6
