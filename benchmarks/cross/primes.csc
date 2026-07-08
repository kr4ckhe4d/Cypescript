// Count primes below 1,000,000 by trial division
function countPrimes(limit: i32): i32 {
    let count: i32 = 0;
    for (let n: i32 = 2; n < limit; n++) {
        let isPrime: boolean = true;
        for (let d: i32 = 2; d * d <= n; d++) {
            if (n % d == 0) {
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

println(countPrimes(1000000));
