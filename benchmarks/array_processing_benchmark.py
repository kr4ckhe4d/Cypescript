#!/usr/bin/env python3
# Python Array Processing Benchmark
# Equivalent to Cypescript array_processing_benchmark.csc

import time

print("=== Python Array Processing Benchmark ===")

# Configuration
array_size = 1000000  # 1 million elements
print(f"Array size: {array_size:,} elements")

# Create large array
print("Creating large array...")
test_array = list(range(1, 101))  # 1-100 for comparison with Cypescript
actual_size = len(test_array)
print(f"Actual benchmark array size: {actual_size}")

# Benchmark 1: Array Sum
print("Benchmark 1: Array Sum")
iterations = 100000  # 100,000 iterations
print(f"Iterations: {iterations:,}")

sum_start = time.perf_counter()
sum_total = 0
for iter_count in range(iterations):
    current_sum = sum(test_array)
    sum_total += current_sum
sum_end = time.perf_counter()
sum_time = (sum_end - sum_start) * 1000  # Convert to milliseconds

print("Sum benchmark complete")
print(f"Total sum across all iterations: {sum_total}")
print(f"Sum benchmark time: {sum_time:.2f}ms")

# Benchmark 2: Array Maximum
print("Benchmark 2: Array Maximum")
max_iterations = 100000
print(f"Iterations: {max_iterations:,}")

max_start = time.perf_counter()
max_total = 0
for iter_count in range(max_iterations):
    current_max = max(test_array)
    max_total += current_max
max_end = time.perf_counter()
max_time = (max_end - max_start) * 1000

print("Max benchmark complete")
print(f"Total max across all iterations: {max_total}")
print(f"Max benchmark time: {max_time:.2f}ms")

# Benchmark 3: Array Search
print("Benchmark 3: Array Search")
search_iterations = 100000
search_target = 75
print(f"Iterations: {search_iterations:,}")
print(f"Search target: {search_target}")

search_start = time.perf_counter()
found_count = 0
for iter_count in range(search_iterations):
    found = search_target in test_array
    if found:
        found_count += 1
search_end = time.perf_counter()
search_time = (search_end - search_start) * 1000

print("Search benchmark complete")
print(f"Found target in iterations: {found_count}")
print(f"Search benchmark time: {search_time:.2f}ms")

# Benchmark 4: Array Modification
print("Benchmark 4: Array Modification")
modify_iterations = 50000  # Fewer iterations for modification
print(f"Iterations: {modify_iterations:,}")

# Create a copy for modification
modify_array = test_array.copy()

modify_start = time.perf_counter()
for iter_count in range(modify_iterations):
    # Double all elements
    for i in range(len(modify_array)):
        modify_array[i] *= 2
    
    # Reset array for next iteration
    for i in range(len(modify_array)):
        modify_array[i] //= 2
modify_end = time.perf_counter()
modify_time = (modify_end - modify_start) * 1000

print("Modification benchmark complete")
print(f"Modification benchmark time: {modify_time:.2f}ms")

# Summary
total_time = sum_time + max_time + search_time + modify_time
print("=== Python Benchmark Summary ===")
print(f"Sum benchmark: {sum_time:.2f}ms")
print(f"Max benchmark: {max_time:.2f}ms")
print(f"Search benchmark: {search_time:.2f}ms")
print(f"Modification benchmark: {modify_time:.2f}ms")
print(f"Total time: {total_time:.2f}ms")
print("=== All Benchmarks Complete ===")
