// Tests: function-type parameters — passing closures to regular functions

function apply(f: (i32) => i32, x: i32): i32 {
    return f(x);
}

function twice(f: (x: i32) => i32, x: i32): i32 {
    return f(f(x));
}

function combine(f: (i32, i32) => i32, a: i32, b: i32): i32 {
    return f(a, b);
}

function describe(fmt: (string) => string, name: string): string {
    return fmt(name);
}

let inc = (n: i32): i32 => n + 1;
println(apply(inc, 41));                  // 42
println(apply(n => n * 10, 5));           // 50 (inline arrow argument)
println(twice(inc, 40));                  // 42

println(combine((a: i32, b: i32): i32 => a * b, 6, 7));  // 42
println(combine((a, b) => a - b, 50, 8));                // 42

println(describe((s: string): string => "<<" + s + ">>", "cy"));  // <<cy>>

// Closures with captures passed as parameters
let offset: i32 = 100;
let addOffset = (n: i32): i32 => n + offset;
println(apply(addOffset, 11));            // 111

// Aliased closure through a variable keeps working
let g = inc;
println(apply(g, 1));                     // 2
