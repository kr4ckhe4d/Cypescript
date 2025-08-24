// SIMD-Optimized Array Operations Library
// Uses AVX2 instructions for 8x parallel integer operations

#include <immintrin.h>
#include <cstring>
#include <algorithm>
#include <cmath>

extern "C" {
    // SIMD-optimized array sum (8x faster than scalar)
    int simd_array_sum_i32(int* arr, int size) {
        if (size == 0) return 0;
        
        // Use SIMD for bulk of the array (process 8 integers at once)
        int simd_size = (size / 8) * 8;
        __m256i sum_vec = _mm256_setzero_si256();
        
        for (int i = 0; i < simd_size; i += 8) {
            __m256i data = _mm256_loadu_si256((__m256i*)&arr[i]);
            sum_vec = _mm256_add_epi32(sum_vec, data);
        }
        
        // Extract sum from SIMD register
        int result[8];
        _mm256_storeu_si256((__m256i*)result, sum_vec);
        int total = result[0] + result[1] + result[2] + result[3] + 
                   result[4] + result[5] + result[6] + result[7];
        
        // Handle remaining elements (scalar)
        for (int i = simd_size; i < size; i++) {
            total += arr[i];
        }
        
        return total;
    }
    
    // SIMD-optimized array maximum
    int simd_array_max_i32(int* arr, int size) {
        if (size == 0) return 0;
        if (size == 1) return arr[0];
        
        int max_val = arr[0];
        
        // Use SIMD for bulk processing
        int simd_size = (size / 8) * 8;
        if (simd_size >= 8) {
            __m256i max_vec = _mm256_set1_epi32(arr[0]);
            
            for (int i = 0; i < simd_size; i += 8) {
                __m256i data = _mm256_loadu_si256((__m256i*)&arr[i]);
                max_vec = _mm256_max_epi32(max_vec, data);
            }
            
            // Extract maximum from SIMD register
            int result[8];
            _mm256_storeu_si256((__m256i*)result, max_vec);
            max_val = std::max({result[0], result[1], result[2], result[3],
                               result[4], result[5], result[6], result[7]});
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; i++) {
            max_val = std::max(max_val, arr[i]);
        }
        
        return max_val;
    }
    
    // SIMD-optimized array minimum
    int simd_array_min_i32(int* arr, int size) {
        if (size == 0) return 0;
        if (size == 1) return arr[0];
        
        int min_val = arr[0];
        
        // Use SIMD for bulk processing
        int simd_size = (size / 8) * 8;
        if (simd_size >= 8) {
            __m256i min_vec = _mm256_set1_epi32(arr[0]);
            
            for (int i = 0; i < simd_size; i += 8) {
                __m256i data = _mm256_loadu_si256((__m256i*)&arr[i]);
                min_vec = _mm256_min_epi32(min_vec, data);
            }
            
            // Extract minimum from SIMD register
            int result[8];
            _mm256_storeu_si256((__m256i*)result, min_vec);
            min_val = std::min({result[0], result[1], result[2], result[3],
                               result[4], result[5], result[6], result[7]});
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; i++) {
            min_val = std::min(min_val, arr[i]);
        }
        
        return min_val;
    }
    
    // SIMD-optimized array multiplication (scale all elements)
    void simd_array_multiply_i32(int* arr, int size, int multiplier) {
        if (size == 0) return;
        
        // Use SIMD for bulk processing
        int simd_size = (size / 8) * 8;
        __m256i mult_vec = _mm256_set1_epi32(multiplier);
        
        for (int i = 0; i < simd_size; i += 8) {
            __m256i data = _mm256_loadu_si256((__m256i*)&arr[i]);
            __m256i result = _mm256_mullo_epi32(data, mult_vec);
            _mm256_storeu_si256((__m256i*)&arr[i], result);
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; i++) {
            arr[i] *= multiplier;
        }
    }
    
    // SIMD-optimized array addition (add value to all elements)
    void simd_array_add_i32(int* arr, int size, int addend) {
        if (size == 0) return;
        
        // Use SIMD for bulk processing
        int simd_size = (size / 8) * 8;
        __m256i add_vec = _mm256_set1_epi32(addend);
        
        for (int i = 0; i < simd_size; i += 8) {
            __m256i data = _mm256_loadu_si256((__m256i*)&arr[i]);
            __m256i result = _mm256_add_epi32(data, add_vec);
            _mm256_storeu_si256((__m256i*)&arr[i], result);
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; i++) {
            arr[i] += addend;
        }
    }
    
    // SIMD-optimized dot product
    long long simd_dot_product_i32(int* arr1, int* arr2, int size) {
        if (size == 0) return 0;
        
        long long total = 0;
        
        // Use SIMD for bulk processing
        int simd_size = (size / 8) * 8;
        __m256i sum_vec = _mm256_setzero_si256();
        
        for (int i = 0; i < simd_size; i += 8) {
            __m256i data1 = _mm256_loadu_si256((__m256i*)&arr1[i]);
            __m256i data2 = _mm256_loadu_si256((__m256i*)&arr2[i]);
            __m256i product = _mm256_mullo_epi32(data1, data2);
            sum_vec = _mm256_add_epi32(sum_vec, product);
        }
        
        // Extract sum from SIMD register
        int result[8];
        _mm256_storeu_si256((__m256i*)result, sum_vec);
        total = result[0] + result[1] + result[2] + result[3] + 
               result[4] + result[5] + result[6] + result[7];
        
        // Handle remaining elements
        for (int i = simd_size; i < size; i++) {
            total += (long long)arr1[i] * arr2[i];
        }
        
        return total;
    }
    
    // SIMD-optimized array comparison (count equal elements)
    int simd_array_count_equal_i32(int* arr, int size, int target) {
        if (size == 0) return 0;
        
        int count = 0;
        
        // Use SIMD for bulk processing
        int simd_size = (size / 8) * 8;
        __m256i target_vec = _mm256_set1_epi32(target);
        
        for (int i = 0; i < simd_size; i += 8) {
            __m256i data = _mm256_loadu_si256((__m256i*)&arr[i]);
            __m256i cmp = _mm256_cmpeq_epi32(data, target_vec);
            
            // Count the number of matches
            int mask = _mm256_movemask_epi8(cmp);
            count += __builtin_popcount(mask) / 4; // Each match is 4 bytes
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; i++) {
            if (arr[i] == target) count++;
        }
        
        return count;
    }
    
    // Performance comparison function
    double simd_performance_ratio() {
        // Theoretical SIMD speedup for integer operations
        return 8.0; // AVX2 processes 8 32-bit integers in parallel
    }
}
