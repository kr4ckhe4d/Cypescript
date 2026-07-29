// Test: shift() advances a head offset instead of erasing from the front.
//
// Every other operation therefore has to speak in positions relative to the
// first live element, not to the start of the underlying storage. This asserts
// the interactions where an off-by-one would hide: indexing, assigning,
// appending, popping, for...of and removeAt, each after elements have been
// shifted off the front.
//
// The dead prefix is reclaimed in one bulk erase once it outgrows the live
// region, so the second block drains far enough to trigger that and checks the
// live elements did not move.

const q: i32[] = [];
q.push(10);
q.push(20);
q.push(30);
q.push(40);
q.push(50);

println(q.shift());
println(q.shift());
println(q.length);

// Indexing is relative to the first live element, not to the storage
println(q[0]);
println(q[2]);

q[0] = 99;
println(q[0]);

// Appending after a shift lands after the live tail
q.push(60);
println(q.length);
println(q[3]);
println(q.pop());

// for...of walks the live region only
let joined: string = "";
for (const n of q) {
    joined = joined + n + ",";
}
println(joined);

q.removeAt(0);
println(q.length);
println(q[0]);

// Enough shifts to cross the reclaim threshold and bulk-erase the prefix
const big: i32[] = [];
for (let i: i32 = 0; i < 100; i = i + 1) {
    big.push(i);
}
for (let i: i32 = 0; i < 60; i = i + 1) {
    let discarded: i32 = big.shift();
}
println(big.length);
println(big[0]);
println(big[39]);

// Text takes the same path, and hands back a copy the caller owns
const s: string[] = [];
s.push("a");
s.push("b");
s.push("c");
println(s.shift());
println(s[0]);
println(s.length);
s.push("d");
println(s[2]);
