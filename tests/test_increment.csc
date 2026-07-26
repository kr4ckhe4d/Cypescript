// `++` and `--` are expressions, not just statements: they yield a value, so
// they can appear inside a larger expression. Postfix yields the value before
// the step, prefix the value after, and the target is stepped exactly once.

let i: i32 = 0;
println(i++);            // 0, then i is 1
println(i);              // 1
println(++i);            // 2
println(i--);            // 2, then i is 1
println(--i);            // 0

// The classic use: index and step in one go
let arr: i32[] = [10, 20, 30];
let k: i32 = 0;
println(arr[k++]);       // 10
println(arr[k++]);       // 20
println(k);              // 2

// Two cursors walking at once
let src: i32[] = [1, 2, 3];
let dst: i32[] = [0, 0, 0];
let s: i32 = 0;
let d: i32 = 0;
while (s < 3) { dst[d++] = src[s++]; }
println(dst[0] + dst[1] + dst[2]);   // 6

// Stepping an element, not a variable
arr[0]++;
println(arr[0]);         // 11

// The target is evaluated once, so an index with a side effect runs once
let calls: i32 = 0;
function next(): i32 { calls = calls + 1; return 1; }
arr[next()]++;
println(calls);          // 1
println(arr[1]);         // 21

// As a loop condition
let n: i32 = 3;
while (n-- > 0) { println(n); }   // 2, 1, 0

// A for loop's update clause is the same node
let total: i32 = 0;
for (let f: i32 = 0; f < 4; f++) { total += f; }
println(total);          // 6

// Non-i32 targets keep their own type rather than being narrowed
let real: f64 = 1.5;
real++;
println(real);           // 2.5

let wide: i64 = 9;
wide++;
println(wide);           // 10

// Buffer elements step inline, like every other buffer access
let buf = new Buffer<i32>(4);
buf[2] = 5;
println(buf[2]++);       // 5
println(buf[2]);         // 6

// Class fields and object properties
class Counter { hits: i32 = 0; }
let c: Counter = new Counter();
c.hits++;
println(c.hits++);       // 1
println(c.hits);         // 2

let config = { count: 7 };
config.count++;
println(config.count);   // 8
