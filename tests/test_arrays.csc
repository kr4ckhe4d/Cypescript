// Test: Arrays and array.length
let numbers: i32[] = [10, 25, 7, 42, 18];
println(numbers.length);
println(numbers[0]);
println(numbers[4]);

let sum: i32 = 0;
for (let i: i32 = 0; i < numbers.length; i = i + 1) {
    sum = sum + numbers[i];
}
println(sum);
