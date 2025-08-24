// Memory-Optimized Array Operations Library
// Implements cache-friendly algorithms and memory pool allocation

#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

// Memory pool for efficient allocation
static char* memory_pool = nullptr;
static size_t pool_size = 0;
static size_t pool_offset = 0;

extern "C" {
    // Initialize memory pool for cache-friendly allocations
    void memory_pool_init(int size_mb) {
        if (memory_pool) {
            free(memory_pool);
        }
        
        pool_size = size_mb * 1024 * 1024; // Convert MB to bytes
        memory_pool = (char*)aligned_alloc(64, pool_size); // 64-byte aligned for cache lines
        pool_offset = 0;
        
        // Pre-fault pages to avoid page faults during computation
        for (size_t i = 0; i < pool_size; i += 4096) {
            memory_pool[i] = 0;
        }
    }
    
    // Allocate from memory pool (cache-aligned)
    void* memory_pool_alloc(int size) {
        if (!memory_pool || pool_offset + size > pool_size) {
            return nullptr;
        }
        
        // Align to cache line boundary (64 bytes)
        size_t aligned_size = (size + 63) & ~63;
        void* ptr = memory_pool + pool_offset;
        pool_offset += aligned_size;
        
        return ptr;
    }
    
    // Reset memory pool for reuse
    void memory_pool_reset() {
        pool_offset = 0;
    }
    
    // Cache-optimized array sum with blocking
    int cache_optimized_sum_i32(int* arr, int size) {
        if (size == 0) return 0;
        
        const int BLOCK_SIZE = 1024; // Fits in L1 cache
        int total = 0;
        
        // Process in cache-friendly blocks
        for (int block_start = 0; block_start < size; block_start += BLOCK_SIZE) {
            int block_end = std::min(block_start + BLOCK_SIZE, size);
            int block_sum = 0;
            
#ifdef __ARM_NEON
            // NEON optimization within each block
            int neon_size = ((block_end - block_start) / 4) * 4;
            int32x4_t sum_vec = vdupq_n_s32(0);
            
            for (int i = block_start; i < block_start + neon_size; i += 4) {
                int32x4_t data = vld1q_s32(&arr[i]);
                sum_vec = vaddq_s32(sum_vec, data);
            }
            
            // Extract sum from NEON register
            int32_t result[4];
            vst1q_s32(result, sum_vec);
            block_sum = result[0] + result[1] + result[2] + result[3];
            
            // Handle remaining elements in block
            for (int i = block_start + neon_size; i < block_end; i++) {
                block_sum += arr[i];
            }
#else
            // Scalar version with loop unrolling
            int i = block_start;
            for (; i < block_end - 3; i += 4) {
                block_sum += arr[i] + arr[i+1] + arr[i+2] + arr[i+3];
            }
            for (; i < block_end; i++) {
                block_sum += arr[i];
            }
#endif
            
            total += block_sum;
        }
        
        return total;
    }
    
    // Cache-optimized array maximum with prefetching
    int cache_optimized_max_i32(int* arr, int size) {
        if (size == 0) return 0;
        if (size == 1) return arr[0];
        
        int max_val = arr[0];
        const int PREFETCH_DISTANCE = 64; // Prefetch 64 elements ahead
        
        // Prefetch initial data
        __builtin_prefetch(&arr[0], 0, 3); // Read, high temporal locality
        
        for (int i = 1; i < size; i++) {
            // Prefetch future data
            if (i + PREFETCH_DISTANCE < size) {
                __builtin_prefetch(&arr[i + PREFETCH_DISTANCE], 0, 3);
            }
            
            if (arr[i] > max_val) {
                max_val = arr[i];
            }
        }
        
        return max_val;
    }
    
    // Memory-efficient array copy with streaming stores
    void memory_efficient_copy_i32(int* dest, int* src, int size) {
        if (size == 0) return;
        
#ifdef __ARM_NEON
        // Use NEON for efficient copying
        int neon_size = (size / 4) * 4;
        
        for (int i = 0; i < neon_size; i += 4) {
            int32x4_t data = vld1q_s32(&src[i]);
            vst1q_s32(&dest[i], data);
        }
        
        // Handle remaining elements
        for (int i = neon_size; i < size; i++) {
            dest[i] = src[i];
        }
#else
        // Use memcpy for optimized copying
        memcpy(dest, src, size * sizeof(int));
#endif
    }
    
    // Cache-aware matrix transpose (for better memory access patterns)
    void cache_aware_transpose_i32(int* matrix, int* result, int rows, int cols) {
        const int TILE_SIZE = 32; // Cache-friendly tile size
        
        for (int i = 0; i < rows; i += TILE_SIZE) {
            for (int j = 0; j < cols; j += TILE_SIZE) {
                // Process tile
                int max_i = std::min(i + TILE_SIZE, rows);
                int max_j = std::min(j + TILE_SIZE, cols);
                
                for (int ii = i; ii < max_i; ii++) {
                    for (int jj = j; jj < max_j; jj++) {
                        result[jj * rows + ii] = matrix[ii * cols + jj];
                    }
                }
            }
        }
    }
    
    // Memory bandwidth optimized array operations
    void memory_bandwidth_test_i32(int* arr, int size, int iterations) {
        // Sequential access pattern (cache-friendly)
        for (int iter = 0; iter < iterations; iter++) {
            for (int i = 0; i < size; i++) {
                arr[i] = arr[i] * 2 + 1;
            }
        }
    }
    
    // Cache miss analysis - random vs sequential access
    long long cache_miss_comparison_i32(int* arr, int size, int pattern) {
        long long total = 0;
        
        if (pattern == 0) {
            // Sequential access (cache-friendly)
            for (int i = 0; i < size; i++) {
                total += arr[i];
            }
        } else {
            // Random access (cache-unfriendly)
            for (int i = 0; i < size; i++) {
                int index = (i * 7919) % size; // Pseudo-random pattern
                total += arr[index];
            }
        }
        
        return total;
    }
    
    // Memory-optimized string operations
    int memory_optimized_string_compare(const char* str1, const char* str2) {
        // Compare 8 bytes at a time when possible
        const char* p1 = str1;
        const char* p2 = str2;
        
        // Align to 8-byte boundary for better performance
        while (((uintptr_t)p1 & 7) && *p1 && *p1 == *p2) {
            p1++;
            p2++;
        }
        
        // Compare 8 bytes at a time
        while (*p1 && *p2) {
            if (((uintptr_t)p1 & 7) == 0 && ((uintptr_t)p2 & 7) == 0) {
                // Both aligned, compare 8 bytes
                uint64_t v1 = *(uint64_t*)p1;
                uint64_t v2 = *(uint64_t*)p2;
                
                if (v1 != v2) {
                    // Find the differing byte
                    for (int i = 0; i < 8; i++) {
                        if (p1[i] != p2[i]) {
                            return p1[i] - p2[i];
                        }
                        if (p1[i] == 0) break;
                    }
                }
                
                // Check for null terminator in the 8 bytes
                for (int i = 0; i < 8; i++) {
                    if (p1[i] == 0) {
                        return 0; // Strings are equal
                    }
                }
                
                p1 += 8;
                p2 += 8;
            } else {
                // Not aligned, compare byte by byte
                if (*p1 != *p2) {
                    return *p1 - *p2;
                }
                p1++;
                p2++;
            }
        }
        
        return *p1 - *p2;
    }
    
    // Get memory optimization statistics
    void get_memory_stats(int* pool_used_mb, int* pool_total_mb, int* fragmentation_percent) {
        if (memory_pool) {
            *pool_used_mb = (int)(pool_offset / (1024 * 1024));
            *pool_total_mb = (int)(pool_size / (1024 * 1024));
            *fragmentation_percent = (int)((pool_offset * 100) / pool_size);
        } else {
            *pool_used_mb = 0;
            *pool_total_mb = 0;
            *fragmentation_percent = 0;
        }
    }
    
    // Cleanup memory pool
    void memory_pool_cleanup() {
        if (memory_pool) {
            free(memory_pool);
            memory_pool = nullptr;
            pool_size = 0;
            pool_offset = 0;
        }
    }
}
