function isPrime(n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 === 0) return 0;
    let d = 3;
    while (d * d <= n) { if (n % d === 0) return 0; d += 2; }
    return 1;
}
let count = 0;
for (let i = 2; i < 500000; i++) { count += isPrime(i); }
console.log(`Primes found: ${count}`);
