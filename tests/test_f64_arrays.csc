// Tests: f64[] arrays — literals, access, mutation, iteration, callbacks

let temps: f64[] = [36.6, 37.2, 39.1, 36.9];
println(temps.length);   // 4
println(temps[2]);       // 39.1

// Element assignment
temps[0] = 35.0;
println(temps[0]);       // 35

// push / pop / shift
temps.push(40.5);
println(temps.length);   // 5
println(temps.pop());    // 40.5
println(temps.shift());  // 35
println(temps.length);   // 3

// Iteration and accumulation
let sum: f64 = 0.0;
for (const t of temps) {
    sum += t;
}
println(sum);            // 37.2 + 39.1 + 36.9 = 113.2

// Int literals coerce into an annotated f64 array
let halves: f64[] = [1, 2, 3];
println(halves[1] / 2.0);  // 1

// Callback methods on f64 arrays
let doubled = temps.map((t: f64): f64 => t * 2.0);
println(doubled[0]);       // 74.4

let hot = temps.filter((t: f64): boolean => t > 37.0);
println(hot.length);       // 2

let total = temps.reduce((acc: f64, t: f64): f64 => acc + t, 0.0);
println(total);            // 113.2

let firstHot = temps.find((t: f64): boolean => t > 38.0);
println(firstHot);         // 39.1

// f64 -> i32 map (rounds toward zero via conversion)
let whole = temps.map((t: f64): i32 => t - 0.5);
println(whole[0]);         // 36
