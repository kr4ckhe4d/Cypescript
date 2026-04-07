let a: i32 = 10;
let b: i32 = 0;

if (a || b) {
    println("a or b is truthy");
}

if (!b) {
    println("b is falsy");
}

let c: string = "hello";
let d: string = "";

if (c || d) {
    println("c is truthy");
}

let e: string = JSON.parse("null");
if (!e) {
    println("e is null");
}
