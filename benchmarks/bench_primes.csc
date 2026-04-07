// Benchmark: Prime counting up to 500K
function isPrime(n: i32): i32 {
    if (n <= 1) { return 0; }
    if (n <= 3) { return 1; }
    if (n % 2 == 0) { return 0; }
    let d: i32 = 3;
    while (d * d <= n) {
        if (n % d == 0) { return 0; }
        d = d + 2;
    }
    return 1;
}

let count: i32 = 0;
let limit: i32 = 500000;

for (let i: i32 = 2; i < limit; i = i + 1) {
    count = count + isPrime(i);
}

print("Primes found: ");
println(count);
