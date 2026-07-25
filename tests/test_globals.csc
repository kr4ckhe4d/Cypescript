// Module-level variables are visible inside functions. Previously this failed
// with "Use of undefined variable" — functions only saw their own parameters.

const SPEED: f64 = 4.0;
const LABEL: string = "hits";
let counter: i32 = 0;
let history: i32[] = [];

// Reading a module-level const
function scaled(x: f64): f64 { return x * SPEED; }

// Mutating a module-level variable
function bump(): i32 {
    counter += 1;
    return counter;
}

// Module-level arrays and strings work too
function record(value: i32): void { history.push(value); }
function label(): string { return LABEL; }

println(scaled(2.0));

bump();
bump();
bump();
println(counter);

record(10);
record(20);
println(history.length);
println(history[1]);
println(label());

// Locals still shadow globals rather than clobbering them
function shadowed(): i32 {
    let counter: i32 = 100;
    return counter;
}
println(shadowed());
println(counter);
