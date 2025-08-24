// ARM NEON-Optimized Array Operations Library
// Uses ARM NEON instructions for 4x parallel integer operations on Apple Silicon

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif
#include <cstring>
#include <algorithm>
#include <cmath>

extern "C" {
    // NEON-optimized array sum (4x faster than scalar on ARM)
    int neon_array_sum_i32(int* arr, int size) {
        if (size == 0) return 0;
        
#ifdef __ARM_NEON
        // Use NEON for bulk of the array (process 4 integers at once)
        int neon_size = (size / 4) * 4;
        int32x4_t sum_vec = vdupq_n_s32(0);
        
        for (int i = 0; i < neon_size; i += 4) {
            int32x4_t data = vld1q_s32(&arr[i]);
            sum_vec = vaddq_s32(sum_vec, data);
        }
        
        // Extract sum from NEON register
        int32_t result[4];
        vst1q_s32(result, sum_vec);
        int total = result[0] + result[1] + result[2] + result[3];
        
        // Handle remaining elements (scalar)
        for (int i = neon_size; i < size; i++) {
            total += arr[i];
        }
        
        return total;
#else
        // Fallback to scalar implementation
        int total = 0;
        for (int i = 0; i < size; i++) {
            total += arr[i];
        }
        return total;
#endif
    }
    
    // NEON-optimized array maximum
    int neon_array_max_i32(int* arr, int size) {
        if (size == 0) return 0;
        if (size == 1) return arr[0];
        
        int max_val = arr[0];
        
#ifdef __ARM_NEON
        // Use NEON for bulk processing
        int neon_size = (size / 4) * 4;
        if (neon_size >= 4) {
            int32x4_t max_vec = vdupq_n_s32(arr[0]);
            
            for (int i = 0; i < neon_size; i += 4) {
                int32x4_t data = vld1q_s32(&arr[i]);
                max_vec = vmaxq_s32(max_vec, data);
            }
            
            // Extract maximum from NEON register
            int32_t result[4];
            vst1q_s32(result, max_vec);
            max_val = std::max({result[0], result[1], result[2], result[3]});
        }
        
        // Handle remaining elements
        for (int i = neon_size; i < size; i++) {
            max_val = std::max(max_val, arr[i]);
        }
#else
        // Fallback to scalar implementation
        for (int i = 1; i < size; i++) {
            max_val = std::max(max_val, arr[i]);
        }
#endif
        
        return max_val;
    }
    
    // NEON-optimized array minimum
    int neon_array_min_i32(int* arr, int size) {
        if (size == 0) return 0;
        if (size == 1) return arr[0];
        
        int min_val = arr[0];
        
#ifdef __ARM_NEON
        // Use NEON for bulk processing
        int neon_size = (size / 4) * 4;
        if (neon_size >= 4) {
            int32x4_t min_vec = vdupq_n_s32(arr[0]);
            
            for (int i = 0; i < neon_size; i += 4) {
                int32x4_t data = vld1q_s32(&arr[i]);
                min_vec = vminq_s32(min_vec, data);
            }
            
            // Extract minimum from NEON register
            int32_t result[4];
            vst1q_s32(result, min_vec);
            min_val = std::min({result[0], result[1], result[2], result[3]});
        }
        
        // Handle remaining elements
        for (int i = neon_size; i < size; i++) {
            min_val = std::min(min_val, arr[i]);
        }
#else
        // Fallback to scalar implementation
        for (int i = 1; i < size; i++) {
            min_val = std::min(min_val, arr[i]);
        }
#endif
        
        return min_val;
    }
    
    // NEON-optimized array multiplication (scale all elements)
    void neon_array_multiply_i32(int* arr, int size, int multiplier) {
        if (size == 0) return;
        
#ifdef __ARM_NEON
        // Use NEON for bulk processing
        int neon_size = (size / 4) * 4;
        int32x4_t mult_vec = vdupq_n_s32(multiplier);
        
        for (int i = 0; i < neon_size; i += 4) {
            int32x4_t data = vld1q_s32(&arr[i]);
            int32x4_t result = vmulq_s32(data, mult_vec);
            vst1q_s32(&arr[i], result);
        }
        
        // Handle remaining elements
        for (int i = neon_size; i < size; i++) {
            arr[i] *= multiplier;
        }
#else
        // Fallback to scalar implementation
        for (int i = 0; i < size; i++) {
            arr[i] *= multiplier;
        }
#endif
    }
    
    // NEON-optimized array addition (add value to all elements)
    void neon_array_add_i32(int* arr, int size, int addend) {
        if (size == 0) return;
        
#ifdef __ARM_NEON
        // Use NEON for bulk processing
        int neon_size = (size / 4) * 4;
        int32x4_t add_vec = vdupq_n_s32(addend);
        
        for (int i = 0; i < neon_size; i += 4) {
            int32x4_t data = vld1q_s32(&arr[i]);
            int32x4_t result = vaddq_s32(data, add_vec);
            vst1q_s32(&arr[i], result);
        }
        
        // Handle remaining elements
        for (int i = neon_size; i < size; i++) {
            arr[i] += addend;
        }
#else
        // Fallback to scalar implementation
        for (int i = 0; i < size; i++) {
            arr[i] += addend;
        }
#endif
    }
    
    // NEON-optimized dot product
    long long neon_dot_product_i32(int* arr1, int* arr2, int size) {
        if (size == 0) return 0;
        
        long long total = 0;
        
#ifdef __ARM_NEON
        // Use NEON for bulk processing
        int neon_size = (size / 4) * 4;
        int64x2_t sum_vec = vdupq_n_s64(0);
        
        for (int i = 0; i < neon_size; i += 4) {
            int32x4_t data1 = vld1q_s32(&arr1[i]);
            int32x4_t data2 = vld1q_s32(&arr2[i]);
            
            // Multiply and accumulate (widening to 64-bit)
            int64x2_t product_lo = vmull_s32(vget_low_s32(data1), vget_low_s32(data2));
            int64x2_t product_hi = vmull_s32(vget_high_s32(data1), vget_high_s32(data2));
            
            sum_vec = vaddq_s64(sum_vec, product_lo);
            sum_vec = vaddq_s64(sum_vec, product_hi);
        }
        
        // Extract sum from NEON register
        int64_t result[2];
        vst1q_s64(result, sum_vec);
        total = result[0] + result[1];
        
        // Handle remaining elements
        for (int i = neon_size; i < size; i++) {
            total += (long long)arr1[i] * arr2[i];
        }
#else
        // Fallback to scalar implementation
        for (int i = 0; i < size; i++) {
            total += (long long)arr1[i] * arr2[i];
        }
#endif
        
        return total;
    }
    
    // NEON-optimized array comparison (count equal elements)
    int neon_array_count_equal_i32(int* arr, int size, int target) {
        if (size == 0) return 0;
        
        int count = 0;
        
#ifdef __ARM_NEON
        // Use NEON for bulk processing
        int neon_size = (size / 4) * 4;
        int32x4_t target_vec = vdupq_n_s32(target);
        
        for (int i = 0; i < neon_size; i += 4) {
            int32x4_t data = vld1q_s32(&arr[i]);
            uint32x4_t cmp = vceqq_s32(data, target_vec);
            
            // Count the number of matches
            uint32_t result[4];
            vst1q_u32(result, cmp);
            for (int j = 0; j < 4; j++) {
                if (result[j] != 0) count++;
            }
        }
        
        // Handle remaining elements
        for (int i = neon_size; i < size; i++) {
            if (arr[i] == target) count++;
        }
#else
        // Fallback to scalar implementation
        for (int i = 0; i < size; i++) {
            if (arr[i] == target) count++;
        }
#endif
        
        return count;
    }
    
    // Performance comparison function
    double neon_performance_ratio() {
        // Theoretical NEON speedup for integer operations on ARM
#ifdef __ARM_NEON
        return 4.0; // ARM NEON processes 4 32-bit integers in parallel
#else
        return 1.0; // No SIMD available
#endif
    }
    
    // Check if NEON is available
    int neon_available() {
#ifdef __ARM_NEON
        return 1;
#else
        return 0;
#endif
    }
}
