// Custom Math Library - Example of extending Cypescript with custom C++ code
// This demonstrates how to add your own C++ functions to Cypescript

#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

extern "C" {
    // ===================
    // ADVANCED MATH FUNCTIONS
    // ===================
    
    // Calculate the greatest common divisor
    int math_gcd(int a, int b) {
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    // Calculate the least common multiple
    int math_lcm(int a, int b) {
        return (a * b) / math_gcd(a, b);
    }
    
    // Check if a number is prime
    int math_is_prime(int n) {
        if (n <= 1) return 0;
        if (n <= 3) return 1;
        if (n % 2 == 0 || n % 3 == 0) return 0;
        
        for (int i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) {
                return 0;
            }
        }
        return 1;
    }
    
    // Calculate nth Fibonacci number
    int math_fibonacci(int n) {
        if (n <= 1) return n;
        
        int a = 0, b = 1;
        for (int i = 2; i <= n; i++) {
            int temp = a + b;
            a = b;
            b = temp;
        }
        return b;
    }
    
    // Calculate factorial
    int math_factorial(int n) {
        if (n <= 1) return 1;
        int result = 1;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }
    
    // ===================
    // STATISTICS FUNCTIONS
    // ===================
    
    // Calculate mean of an array
    double stats_mean(int* arr, int size) {
        if (size == 0) return 0.0;
        int sum = 0;
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
        return (double)sum / size;
    }
    
    // Calculate median of an array
    double stats_median(int* arr, int size) {
        if (size == 0) return 0.0;
        
        // Create a copy and sort it
        std::vector<int> sorted(arr, arr + size);
        std::sort(sorted.begin(), sorted.end());
        
        if (size % 2 == 0) {
            return (sorted[size/2 - 1] + sorted[size/2]) / 2.0;
        } else {
            return sorted[size/2];
        }
    }
    
    // Calculate standard deviation
    double stats_stddev(int* arr, int size) {
        if (size <= 1) return 0.0;
        
        double mean = stats_mean(arr, size);
        double sum_sq_diff = 0.0;
        
        for (int i = 0; i < size; i++) {
            double diff = arr[i] - mean;
            sum_sq_diff += diff * diff;
        }
        
        return std::sqrt(sum_sq_diff / (size - 1));
    }
    
    // ===================
    // GEOMETRY FUNCTIONS
    // ===================
    
    // Calculate distance between two points
    double geom_distance(double x1, double y1, double x2, double y2) {
        double dx = x2 - x1;
        double dy = y2 - y1;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    // Calculate area of a circle
    double geom_circle_area(double radius) {
        return 3.14159265359 * radius * radius;
    }
    
    // Calculate area of a rectangle
    double geom_rectangle_area(double width, double height) {
        return width * height;
    }
    
    // Calculate area of a triangle
    double geom_triangle_area(double base, double height) {
        return 0.5 * base * height;
    }
}
