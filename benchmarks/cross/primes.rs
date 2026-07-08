// Count primes below 1,000,000 by trial division
fn count_primes(limit: i32) -> i32 {
    let mut count = 0;
    let mut n = 2;
    while n < limit {
        let mut is_prime = true;
        let mut d = 2;
        while d * d <= n {
            if n % d == 0 {
                is_prime = false;
                break;
            }
            d += 1;
        }
        if is_prime {
            count += 1;
        }
        n += 1;
    }
    count
}

fn main() {
    println!("{}", count_primes(1000000));
}
