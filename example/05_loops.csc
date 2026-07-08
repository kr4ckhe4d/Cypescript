// 05 — Loops
// while, for, do-while, for...of — plus break and continue.

// while
let countdown: i32 = 3;
while (countdown > 0) {
    println(countdown);
    countdown--;
}
println("liftoff!");

// classic for (i++ and i += n both work)
let sum: i32 = 0;
for (let i: i32 = 1; i <= 10; i++) {
    sum += i;
}
println(sum); // 55

// do-while runs the body at least once
let attempts: i32 = 0;
do {
    attempts++;
} while (attempts < 3);
println(attempts); // 3

// for...of iterates arrays
let fruits: string[] = ["apple", "banana", "cherry"];
for (const fruit of fruits) {
    println(fruit);
}

// break and continue
let firstMultipleOf7Over50: i32 = 0;
for (let n: i32 = 51; n < 100; n++) {
    if (n % 7 != 0) {
        continue;   // skip non-multiples
    }
    firstMultipleOf7Over50 = n;
    break;          // stop at the first hit
}
println(firstMultipleOf7Over50); // 56
