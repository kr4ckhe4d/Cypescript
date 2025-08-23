// Cypescript Standard Library - C++ Integration
// This provides essential functions that Cypescript can call

#include <cmath>
#include <string>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <chrono>

extern "C" {
    // ===================
    // MATH FUNCTIONS
    // ===================
    
    double math_sqrt(double x) {
        return std::sqrt(x);
    }
    
    double math_pow(double base, double exp) {
        return std::pow(base, exp);
    }
    
    double math_abs_f64(double x) {
        return std::abs(x);
    }
    
    int math_abs_i32(int x) {
        return std::abs(x);
    }
    
    double math_sin(double x) {
        return std::sin(x);
    }
    
    double math_cos(double x) {
        return std::cos(x);
    }
    
    double math_tan(double x) {
        return std::tan(x);
    }
    
    double math_log(double x) {
        return std::log(x);
    }
    
    double math_exp(double x) {
        return std::exp(x);
    }
    
    // ===================
    // STRING FUNCTIONS
    // ===================
    
    const char* string_reverse(const char* str) {
        if (!str) return nullptr;
        std::string s(str);
        std::reverse(s.begin(), s.end());
        char* result = new char[s.length() + 1];
        strcpy(result, s.c_str());
        return result;
    }
    
    const char* string_upper(const char* str) {
        if (!str) return nullptr;
        std::string s(str);
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        char* result = new char[s.length() + 1];
        strcpy(result, s.c_str());
        return result;
    }
    
    const char* string_lower(const char* str) {
        if (!str) return nullptr;
        std::string s(str);
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        char* result = new char[s.length() + 1];
        strcpy(result, s.c_str());
        return result;
    }
    
    int string_length(const char* str) {
        return str ? strlen(str) : 0;
    }
    
    const char* string_substring(const char* str, int start, int length) {
        if (!str) return nullptr;
        std::string s(str);
        if (start < 0 || start >= (int)s.length()) return nullptr;
        
        std::string sub = s.substr(start, length);
        char* result = new char[sub.length() + 1];
        strcpy(result, sub.c_str());
        return result;
    }
    
    int string_find(const char* str, const char* substr) {
        if (!str || !substr) return -1;
        std::string s(str);
        std::string sub(substr);
        size_t pos = s.find(sub);
        return pos == std::string::npos ? -1 : (int)pos;
    }
    
    const char* string_concat(const char* str1, const char* str2) {
        if (!str1 && !str2) return nullptr;
        if (!str1) str1 = "";
        if (!str2) str2 = "";
        
        std::string result = std::string(str1) + std::string(str2);
        char* cstr = new char[result.length() + 1];
        strcpy(cstr, result.c_str());
        return cstr;
    }
    
    // ===================
    // ARRAY FUNCTIONS
    // ===================
    
    int array_sum_i32(int* arr, int size) {
        if (!arr || size <= 0) return 0;
        int sum = 0;
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
        return sum;
    }
    
    int array_max_i32(int* arr, int size) {
        if (!arr || size <= 0) return 0;
        int max = arr[0];
        for (int i = 1; i < size; i++) {
            if (arr[i] > max) max = arr[i];
        }
        return max;
    }
    
    int array_min_i32(int* arr, int size) {
        if (!arr || size <= 0) return 0;
        int min = arr[0];
        for (int i = 1; i < size; i++) {
            if (arr[i] < min) min = arr[i];
        }
        return min;
    }
    
    void array_sort_i32(int* arr, int size) {
        if (!arr || size <= 0) return;
        std::sort(arr, arr + size);
    }
    
    void array_reverse_i32(int* arr, int size) {
        if (!arr || size <= 0) return;
        std::reverse(arr, arr + size);
    }
    
    // ===================
    // FILE I/O FUNCTIONS
    // ===================
    
    const char* file_read(const char* filename) {
        if (!filename) return nullptr;
        
        std::ifstream file(filename);
        if (!file.is_open()) return nullptr;
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        char* result = new char[content.length() + 1];
        strcpy(result, content.c_str());
        return result;
    }
    
    int file_write(const char* filename, const char* content) {
        if (!filename || !content) return 0;
        
        std::ofstream file(filename);
        if (!file.is_open()) return 0;
        
        file << content;
        file.close();
        return 1; // success
    }
    
    int file_exists(const char* filename) {
        if (!filename) return 0;
        std::ifstream file(filename);
        return file.good() ? 1 : 0;
    }
    
    // ===================
    // UTILITY FUNCTIONS
    // ===================
    
    void sleep_ms(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
    
    int random_int(int min, int max) {
        if (min > max) std::swap(min, max);
        return min + (rand() % (max - min + 1));
    }
    
    double random_double() {
        return (double)rand() / RAND_MAX;
    }
    
    void random_seed(int seed) {
        srand(seed);
    }
    
    // Memory management helper
    void free_string(const char* str) {
        delete[] str;
    }
}
