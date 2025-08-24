#!/usr/bin/env python3
# Python Intensive Computational Benchmark
# Equivalent to Cypescript intensive_benchmark.csc

import time

print("=== Python Intensive Computational Benchmark ===")

# Benchmark 1: Nested Loop Computation
print("Benchmark 1: Nested Loop Matrix Multiplication Simulation")
matrix_size = 200
matrix_iterations = 100

print(f"Matrix size: {matrix_size}x{matrix_size}")
print(f"Iterations: {matrix_iterations}")

total_operations = 0
nested_start = time.perf_counter()

for iteration in range(matrix_iterations):
    for i in range(matrix_size):
        for j in range(matrix_size):
            for k in range(matrix_size):
                # Simulate matrix multiplication operation
                result = (i * j + k) % 1000
                total_operations += result

nested_end = time.perf_counter()
nested_time = (nested_end - nested_start) * 1000

print("Nested loop benchmark complete")
print(f"Total operations: {total_operations}")
print(f"Nested loop time: {nested_time:.2f}ms")

# Benchmark 2: Prime Number Generation
print("Benchmark 2: Prime Number Generation")
prime_limit = 10000
prime_count = 0

print(f"Finding primes up to: {prime_limit:,}")

prime_start = time.perf_counter()

for num in range(2, prime_limit + 1):
    is_prime = True
    
    # Check if number is prime
    for divisor in range(2, int(num**0.5) + 1):
        if num % divisor == 0:
            is_prime = False
            break
    
    if is_prime:
        prime_count += 1

prime_end = time.perf_counter()
prime_time = (prime_end - prime_start) * 1000

print("Prime generation benchmark complete")
print(f"Primes found: {prime_count}")
print(f"Prime generation time: {prime_time:.2f}ms")

# Benchmark 3: Fibonacci Sequence Generation
print("Benchmark 3: Fibonacci Sequence Generation")
fib_count = 35
fib_iterations = 1000

print(f"Fibonacci numbers to calculate: {fib_count}")
print(f"Iterations: {fib_iterations:,}")

fib_total = 0
fib_start = time.perf_counter()

for iteration in range(fib_iterations):
    for n in range(fib_count + 1):
        # Calculate nth Fibonacci number iteratively
        if n == 0:
            fib_result = 0
        elif n == 1:
            fib_result = 1
        else:
            a, b = 0, 1
            for i in range(2, n + 1):
                fib_result = a + b
                a, b = b, fib_result
        
        fib_total += fib_result

fib_end = time.perf_counter()
fib_time = (fib_end - fib_start) * 1000

print("Fibonacci benchmark complete")
print(f"Total Fibonacci sum: {fib_total}")
print(f"Fibonacci time: {fib_time:.2f}ms")

# Benchmark 4: Array Sorting Simulation (Bubble Sort)
print("Benchmark 4: Array Sorting Simulation")
sort_size = 1000
sort_iterations = 50

print(f"Array size: {sort_size}")
print(f"Iterations: {sort_iterations}")

test_array = [64, 34, 25, 12, 22, 11, 90, 88, 76, 50, 42, 30, 18, 95, 70]
test_size = len(test_array)
sort_operations = 0

sort_start = time.perf_counter()

for iteration in range(sort_iterations):
    # Bubble sort implementation
    arr = test_array.copy()
    for i in range(test_size - 1):
        for j in range(test_size - i - 1):
            if arr[j] > arr[j + 1]:
                # Swap elements
                arr[j], arr[j + 1] = arr[j + 1], arr[j]
                sort_operations += 1

sort_end = time.perf_counter()
sort_time = (sort_end - sort_start) * 1000

print("Sorting benchmark complete")
print(f"Sort operations: {sort_operations}")
print(f"Sorting time: {sort_time:.2f}ms")

# Summary
total_time = nested_time + prime_time + fib_time + sort_time
print("=== Python Intensive Benchmark Summary ===")
print(f"Nested loops: {nested_time:.2f}ms")
print(f"Prime generation: {prime_time:.2f}ms")
print(f"Fibonacci: {fib_time:.2f}ms")
print(f"Sorting: {sort_time:.2f}ms")
print(f"Total time: {total_time:.2f}ms")
print("=== Intensive Benchmark Complete ===")
