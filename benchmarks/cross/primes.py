# Count primes below 1,000,000 by trial division
def count_primes(limit):
    count = 0
    for n in range(2, limit):
        is_prime = True
        d = 2
        while d * d <= n:
            if n % d == 0:
                is_prime = False
                break
            d += 1
        if is_prime:
            count += 1
    return count


print(count_primes(1000000))
