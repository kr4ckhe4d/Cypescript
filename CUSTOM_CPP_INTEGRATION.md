# Custom C++ Integration Guide

This guide shows how to extend Cypescript with your own custom C++ libraries and functions.

## Overview

Cypescript's C++ integration allows you to:
- **Add custom C++ functions** that can be called from Cypescript
- **Use existing C++ libraries** in your Cypescript programs
- **Extend functionality** beyond the built-in standard library
- **Maintain native performance** with zero-cost interop

## Quick Start

### 1. Create Your C++ Library

Create a `.cpp` file with your custom functions using `extern "C"`:

```cpp
// src/my_custom_lib.cpp
#include <cmath>
#include <vector>

extern "C" {
    // Your custom functions here
    int my_add(int a, int b) {
        return a + b;
    }
    
    double my_sqrt(double x) {
        return std::sqrt(x);
    }
}
```

### 2. Add Functions to Parser

Edit `src/Parser.cpp` and add your functions to `isKnownFunction()`:

```cpp
bool Parser::isKnownFunction(const std::string& name) {
    // ... existing functions ...
    
    // Your custom functions
    if (name == "my_add" || name == "my_sqrt") {
        return true;
    }
    
    return false;
}
```

### 3. Add Functions to CodeGen

Edit `src/CodeGen.cpp` and add LLVM declarations to `getOrDeclareExternalFunction()`:

```cpp
llvm::FunctionCallee CodeGen::getOrDeclareExternalFunction(const std::string& name) {
    // ... existing functions ...
    
    // Your custom functions
    else if (name == "my_add") {
        return m_module->getOrInsertFunction("my_add",
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context),
            llvm::Type::getInt32Ty(m_context));
    }
    else if (name == "my_sqrt") {
        return m_module->getOrInsertFunction("my_sqrt",
            llvm::Type::getDoubleTy(m_context),
            llvm::Type::getDoubleTy(m_context));
    }
    
    return nullptr;
}
```

### 4. Rebuild Compiler

```bash
./build.sh
```

### 5. Compile with Custom Library

```bash
./compile-with-custom-cpp.sh my_program.csc output_name src/my_custom_lib.cpp
```

### 6. Use in Cypescript

```typescript
let result: i32 = my_add(5, 3);
println(result); // 8
```

## Complete Example

### Custom Math Library (`src/custom_math_lib.cpp`)

```cpp
#include <cmath>
#include <vector>
#include <algorithm>

extern "C" {
    // Advanced Math Functions
    int math_gcd(int a, int b) {
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    int math_lcm(int a, int b) {
        return (a * b) / math_gcd(a, b);
    }
    
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
    
    // Statistics Functions
    double stats_mean(int* arr, int size) {
        if (size == 0) return 0.0;
        int sum = 0;
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
        return (double)sum / size;
    }
    
    double stats_median(int* arr, int size) {
        if (size == 0) return 0.0;
        
        std::vector<int> sorted(arr, arr + size);
        std::sort(sorted.begin(), sorted.end());
        
        if (size % 2 == 0) {
            return (sorted[size/2 - 1] + sorted[size/2]) / 2.0;
        } else {
            return sorted[size/2];
        }
    }
}
```

### Cypescript Usage (`example/custom_demo.csc`)

```typescript
println("=== Custom Math Demo ===");

// Advanced math functions
let a: i32 = 48;
let b: i32 = 18;
let gcd: i32 = math_gcd(a, b);
println("GCD: ");
println(gcd);

let lcm: i32 = math_lcm(a, b);
println("LCM: ");
println(lcm);

// Prime checking
let num: i32 = 17;
let is_prime: i32 = math_is_prime(num);
if (is_prime == 1) {
    println("17 is prime!");
}

// Fibonacci
let fib: i32 = math_fibonacci(10);
println("10th Fibonacci: ");
println(fib);

println("=== Demo Complete ===");
```

### Compilation

```bash
./compile-with-custom-cpp.sh example/custom_demo.csc my_program src/custom_math_lib.cpp
./my_program
```

## Type Mapping

### Cypescript → C++ Types

| Cypescript Type | C++ Type | LLVM Type |
|----------------|----------|-----------|
| `i32` | `int` | `llvm::Type::getInt32Ty()` |
| `string` | `const char*` | `llvm::PointerType::get(llvm::Type::getInt8Ty(), 0)` |
| `f64` | `double` | `llvm::Type::getDoubleTy()` |
| `boolean` | `int` (0/1) | `llvm::Type::getInt32Ty()` |
| `i32[]` | `int*` | `llvm::PointerType::get(llvm::Type::getInt32Ty(), 0)` |
| `void` | `void` | `llvm::Type::getVoidTy()` |

### Function Signatures

```cpp
// Single parameter
int my_function(int x);
// LLVM: getOrInsertFunction("my_function", Int32Ty, Int32Ty)

// Multiple parameters
int add_three(int a, int b, int c);
// LLVM: getOrInsertFunction("add_three", Int32Ty, Int32Ty, Int32Ty, Int32Ty)

// String parameter
int string_length(const char* str);
// LLVM: getOrInsertFunction("string_length", Int32Ty, PointerType::get(Int8Ty, 0))

// Array parameter
int array_sum(int* arr, int size);
// LLVM: getOrInsertFunction("array_sum", Int32Ty, PointerType::get(Int32Ty, 0), Int32Ty)

// Void return
void print_message(const char* msg);
// LLVM: getOrInsertFunction("print_message", VoidTy, PointerType::get(Int8Ty, 0))
```

## Advanced Examples

### 1. Image Processing Library

```cpp
// src/image_lib.cpp
extern "C" {
    // Simple image blur (conceptual)
    void image_blur(int* pixels, int width, int height, int radius) {
        // Image processing logic here
    }
    
    int image_get_pixel(int* pixels, int width, int x, int y) {
        return pixels[y * width + x];
    }
    
    void image_set_pixel(int* pixels, int width, int x, int y, int color) {
        pixels[y * width + x] = color;
    }
}
```

### 2. Network Library

```cpp
// src/network_lib.cpp
#include <curl/curl.h>

extern "C" {
    int http_get(const char* url, char* response_buffer, int buffer_size) {
        // HTTP GET implementation using libcurl
        return 0; // Success
    }
    
    int http_post(const char* url, const char* data) {
        // HTTP POST implementation
        return 0;
    }
}
```

### 3. Database Library

```cpp
// src/database_lib.cpp
#include <sqlite3.h>

extern "C" {
    int db_connect(const char* filename) {
        // SQLite connection logic
        return 1; // Connection ID
    }
    
    int db_query(int connection_id, const char* sql) {
        // Execute SQL query
        return 0; // Success
    }
    
    void db_close(int connection_id) {
        // Close database connection
    }
}
```

## Best Practices

### 1. Memory Management

```cpp
extern "C" {
    // ✅ Good: Return simple values
    int calculate_result(int x) {
        return x * 2;
    }
    
    // ⚠️ Careful: Returning allocated memory
    const char* create_string() {
        char* result = new char[100];
        strcpy(result, "Hello");
        return result; // Caller must free this!
    }
    
    // ✅ Better: Use static buffers for simple cases
    const char* get_status() {
        static char buffer[256];
        strcpy(buffer, "Ready");
        return buffer;
    }
}
```

### 2. Error Handling

```cpp
extern "C" {
    // Return error codes
    int safe_divide(int a, int b, int* result) {
        if (b == 0) {
            return -1; // Error: division by zero
        }
        *result = a / b;
        return 0; // Success
    }
    
    // Use sentinel values
    int find_element(int* arr, int size, int target) {
        for (int i = 0; i < size; i++) {
            if (arr[i] == target) {
                return i; // Found at index i
            }
        }
        return -1; // Not found
    }
}
```

### 3. Performance Optimization

```cpp
extern "C" {
    // ✅ Pass arrays by pointer + size
    int process_array(int* data, int size) {
        // Efficient: direct memory access
        for (int i = 0; i < size; i++) {
            data[i] *= 2;
        }
        return 0;
    }
    
    // ✅ Use const for read-only parameters
    int calculate_checksum(const int* data, int size) {
        int sum = 0;
        for (int i = 0; i < size; i++) {
            sum += data[i];
        }
        return sum;
    }
}
```

## Compilation Scripts

### Basic Custom Compilation

```bash
#!/bin/bash
# compile-my-lib.sh

CPP_FILES="src/my_lib1.cpp src/my_lib2.cpp"
./compile-with-custom-cpp.sh $1 $2 $CPP_FILES
```

### Advanced Compilation with External Libraries

```bash
#!/bin/bash
# compile-with-external-libs.sh

# Compile custom C++ files
g++ -c src/my_network_lib.cpp -o my_network_lib.o -std=c++11 -lcurl
g++ -c src/my_database_lib.cpp -o my_database_lib.o -std=c++11 -lsqlite3

# Compile Cypescript
./build/cscript $1

# Link everything
llc -filetype=obj -relocation-model=pic output.ll -o cypescript_program.o
clang cypescript_program.o cypescript_stdlib.o my_network_lib.o my_database_lib.o -o $2 -lstdc++ -lcurl -lsqlite3
```

## Troubleshooting

### Common Issues

1. **Function not found**: Make sure function is added to both Parser and CodeGen
2. **Linking errors**: Check that all required libraries are linked
3. **Type mismatches**: Verify LLVM type declarations match C++ signatures
4. **Memory issues**: Be careful with string and array memory management

### Debugging Tips

```bash
# Check if function is recognized by parser
./build/cscript -v --print-tokens your_file.csc

# Check LLVM IR generation
./build/cscript your_file.csc
cat output.ll | grep your_function_name

# Check linking
nm your_program | grep your_function_name
```

## Future Enhancements

- **Automatic binding generation** from C++ headers
- **Template function support** for generic programming
- **Class/object integration** for OOP features
- **Exception handling** across language boundaries
- **Callback functions** from C++ to Cypescript

## Contributing

To add new custom C++ integration features:

1. Create your C++ library file
2. Add functions to Parser (`isKnownFunction`)
3. Add LLVM declarations to CodeGen (`getOrDeclareExternalFunction`)
4. Test with example Cypescript programs
5. Update documentation and examples

This system makes Cypescript incredibly extensible while maintaining native performance!
