// Test: User-defined functions
function add(a: i32, b: i32): i32 {
    return a + b;
}

function factorial(n: i32): i32 {
    let result: i32 = 1;
    let counter: i32 = 1;
    while (counter <= n) {
        result = result * counter;
        counter = counter + 1;
    }
    return result;
}

function greet(name: string): void {
    print("Hello, ");
    println(name);
}

println(add(5, 3));
println(factorial(5));
greet("Alice");
