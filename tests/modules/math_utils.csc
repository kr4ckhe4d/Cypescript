// Helper module for test_modules.csc (not run directly by the test suite)

export function square(x: i32): i32 {
    return x * x;
}

export function cube(x: i32): i32 {
    return x * square(x);
}

export const MODULE_GREETING: string = "hello from math_utils";
