// 19 — Arrow Functions and Closures
// Arrows are expressions: assign them to variables or pass them as callbacks.
// Captures are snapshotted BY VALUE when the closure is created — but a
// captured object/array pointer still refers to the same underlying data.

// Forms
let double = (x: i32): i32 => x * 2;      // typed, expression body
let add = (a: i32, b: i32) => a + b;      // return type inferred
let clamp = n => {                        // single untyped param (defaults i32),
    if (n > 10) { return 10; }            // block body with explicit returns
    return n;
};

println(double(21));   // 42
println(add(3, 4));    // 7
println(clamp(50));    // 10

// Closures capture by value (a snapshot at creation time)
let base: i32 = 10;
let addBase = (x: i32): i32 => x + base;
println(addBase(5));   // 15
base = 999;
println(addBase(5));   // still 15 — the closure holds a snapshot

// For shared mutable state, capture an OBJECT: the pointer is snapshotted,
// so mutations through it are visible everywhere.
let counter = { count: 0 };
let increment = (): i32 => {
    counter.count += 1;
    return counter.count;
};
println(increment());     // 1
println(increment());     // 2
println(counter.count);   // 2

// Callback array methods — the TypeScript classics
let numbers: i32[] = [1, 2, 3, 4, 5, 6];

let doubled = numbers.map(x => x * 2);            // [2,4,6,8,10,12]
let evens = numbers.filter(x => x % 2 == 0);      // [2,4,6]
let sum = numbers.reduce((acc, x) => acc + x, 0); // 21
let firstBig = numbers.find(x => x > 4);          // 5
numbers.forEach(x => { print(x); print(" "); });
println("");

println(doubled[3]);      // 8
println(evens.length);    // 3
println(sum);             // 21
println(firstBig);        // 5

// map can change the element type
let labels = numbers.map((x: i32): string => `item-${x}`);
println(labels[0]);       // item-1

// Closures are first-class: store them, alias them, pass them by name
let isOdd = (x: i32): boolean => x % 2 != 0;
println(numbers.filter(isOdd).length); // 3
