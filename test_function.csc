// Test basic function declaration and call
function add(a: i32, b: i32): i32 {
    return a + b;
}

function greet(): void {
    println("Hello from function!");
}

let result: i32 = add(5, 3);
println(result);
greet();
