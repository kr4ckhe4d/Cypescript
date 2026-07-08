// 08 — Arrays
// Literals, indexing, .length, push/pop/shift, iteration, element assignment.

let numbers: i32[] = [10, 25, 7, 42, 18];

println(numbers[0]);        // 10
println(numbers.length);    // 5

// Element assignment
numbers[2] = 700;
println(numbers[2]);        // 700

// Sum and max the classic way
let sum: i32 = 0;
let max: i32 = numbers[0];
for (let i: i32 = 0; i < numbers.length; i++) {
    sum += numbers[i];
    if (numbers[i] > max) {
        max = numbers[i];
    }
}
println(sum);               // 795
println(max);               // 700

// Dynamic arrays: push adds to the end, shift removes from the front,
// pop removes from the end
let queue: string[] = [];
queue.push("first");
queue.push("second");
queue.push("third");
println(queue.length);      // 3
println(queue.shift());     // first
println(queue.pop());       // third
println(queue.length);      // 1

// for...of iteration
for (const n of numbers) {
    println(n);
}
