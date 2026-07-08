// Count primes below 1,000,000 by trial division
function countPrimes(limit: number): number {
    let count: number = 0;
    for (let n: number = 2; n < limit; n++) {
        let isPrime: boolean = true;
        for (let d: number = 2; d * d <= n; d++) {
            if (n % d === 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            count++;
        }
    }
    return count;
}

console.log(countPrimes(1000000));
