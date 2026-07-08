// 14 — Modules: the imported file
// `export` marks declarations for use by importers.
// Modules are inlined once at compile time (cycles are handled).

export const GREETING: string = "hello from math_utils";

export function square(x: i32): i32 {
    return x * x;
}

export function cube(x: i32): i32 {
    return x * square(x);   // modules can use their own exports
}
