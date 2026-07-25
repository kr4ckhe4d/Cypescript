// Foreign function interface: `declare function` binds any C symbol directly.
// These all come from libc/libm, which are already linked — no link directive needed.

declare function atan2(y: f64, x: f64): f64;
declare function strlen(s: string): i64;
declare function abs(x: i32): i32;
declare function fmax(a: f64, b: f64): f64;

// f64 arguments and return
println(atan2(1.0, 1.0));

// A C function returning i64, printed as a number
println(strlen("cypescript"));

// i32 in, i32 out
println(abs(0 - 42));

// Return values flow into expressions like any other call
let biggest: f64 = fmax(2.5, 7.25);
println(biggest);
println(fmax(atan2(0.0, 1.0), 3.5));

// Declared functions work inside user-defined functions too
function widen(text: string): i32 {
    let n: i64 = strlen(text);
    return n;
}
println(widen("abcde"));

// Opaque `ptr` handles: allocate in C, hand the pointer back to C.
// This is how a game holds a Texture* or Sound* without the compiler
// knowing anything about the type.
declare function malloc(size: i64): ptr;
declare function free(block: ptr): void;
declare function memset(block: ptr, value: i32, size: i64): ptr;

let block: ptr = malloc(64);
memset(block, 0, 64);
free(block);
println("ptr handle round-tripped through C");
