// Cypescript Control Flow Test
// Testing comparison operators and if/else statements

let a: i32 = 10;
let b: i32 = 5;

// Test equality
if (a == 10) {
    print("a equals 10");
}

// Test inequality
if (b != 10) {
    print("b does not equal 10");
}

// Test less than
if (b < a) {
    print("b is less than a");
}

// Test greater than
if (a > b) {
    print("a is greater than b");
}

// Test if/else
if (a > 20) {
    print("a is greater than 20");
} else {
    print("a is not greater than 20");
}

// Complex condition with arithmetic
let result: i32 = a + b;
if (result == 15) {
    print("Sum is 15");
} else {
    print("Sum is not 15");
}

// Nested conditions
if (a > 5) {
    if (b < 10) {
        print("a > 5 and b < 10");
    }
}

// Test all comparison operators
let x: i32 = 7;
let y: i32 = 7;

if (x == y) {
    print("x equals y");
}

if (x <= y) {
    print("x less than or equal to y");
}

if (x >= y) {
    print("x greater than or equal to y");
}

// Test with expressions
if (2 + 3 == 5) {
    print("Math works!");
}

if (10 / 2 > 3) {
    print("Division and comparison work!");
}
