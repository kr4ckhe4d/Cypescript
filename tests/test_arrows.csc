// Tests: arrow functions, closures, and callback array methods

// Arrow forms: typed, inferred return, single untyped param, block body
let double = (x: i32): i32 => x * 2;
println(double(21));                 // 42

let add = (a: i32, b: i32) => a + b;
println(add(3, 4));                  // 7

let shout = (s: string): string => s + "!";
println(shout("hey"));               // hey!

let clamp = n => {
    if (n > 10) {
        return 10;
    }
    return n;
};
println(clamp(50));                  // 10
println(clamp(5));                   // 5

// Closures capture by value (snapshot at creation)
let base: i32 = 10;
let addBase = (x: i32): i32 => x + base;
println(addBase(5));                 // 15
base = 999;
println(addBase(5));                 // 15 (snapshot, not live)

// Captured object pointers share the underlying object
let state = { count: 0 };
let bump = (): i32 => {
    state.count += 1;
    return state.count;
};
println(bump());                     // 1
println(bump());                     // 2
println(state.count);                // 2

// Callback array methods
let numbers: i32[] = [1, 2, 3, 4, 5, 6];

let doubled = numbers.map(x => x * 2);
println(doubled[0]);                 // 2
println(doubled[5]);                 // 12
println(doubled.length);             // 6

let evens = numbers.filter(x => x % 2 == 0);
println(evens.length);               // 3
println(evens[2]);                   // 6

let sum = numbers.reduce((acc, x) => acc + x, 0);
println(sum);                        // 21

let firstBig = numbers.find(x => x > 4);
println(firstBig);                   // 5

let total: i32 = 0;
let counter = { hits: 0 };
numbers.forEach(x => {
    counter.hits += 1;
});
println(counter.hits);               // 6

// map to strings, closures via variables, aliasing
let labels = numbers.map((x: i32): string => "n" + x);
println(labels[0]);                  // n1

let isOdd = (x: i32): boolean => x % 2 != 0;
let odds = numbers.filter(isOdd);
println(odds.length);                // 3

let isOddAlias = isOdd;
println(isOddAlias(3));              // 1
