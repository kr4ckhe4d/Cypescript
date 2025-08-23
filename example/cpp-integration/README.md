# C++ Integration Examples

These examples demonstrate **C++ integration** capabilities using the enhanced compilation pipeline.

## Compilation Method

**Use the C++ integration script:**
```bash
./compile-with-cpp.sh example/cpp-integration/cpp_integration_basic.csc my_program
./my_program
```

**For custom C++ libraries:**
```bash
./compile-with-custom-cpp.sh example/cpp-integration/custom_math_demo.csc my_program src/custom_math_lib.cpp
./my_program
```

This script automatically:
1. ✅ Compiles the C++ standard library (`src/cypescript_stdlib.cpp`)
2. ✅ Compiles custom C++ libraries (if provided)
3. ✅ Compiles Cypescript to LLVM IR
4. ✅ Links everything into a native executable
5. ✅ Cleans up intermediate files

## Available C++ Functions

### String Functions
- `string_reverse(str)` - Reverse a string
- `string_upper(str)` - Convert to uppercase  
- `string_lower(str)` - Convert to lowercase
- `string_length(str)` - Get string length
- `string_substring(str, start, length)` - Extract substring
- `string_find(str, substr)` - Find substring position
- `string_concat(str1, str2)` - Concatenate strings

### Array Functions
- `array_sum_i32(arr, size)` - Sum array elements
- `array_max_i32(arr, size)` - Find maximum element
- `array_min_i32(arr, size)` - Find minimum element

### File I/O Functions
- `file_read(filename)` - Read file contents
- `file_write(filename, content)` - Write to file
- `file_exists(filename)` - Check if file exists

### Utility Functions
- `random_seed(seed)` - Seed random generator
- `random_int(min, max)` - Generate random integer
- `random_double()` - Generate random double

### Custom Math Functions (via custom libraries)
- `math_gcd(a, b)` - Greatest common divisor
- `math_lcm(a, b)` - Least common multiple
- `math_is_prime(n)` - Prime number checking
- `math_fibonacci(n)` - Nth Fibonacci number
- `math_factorial(n)` - Factorial calculation

## Example Files

- **`cpp_integration_basic.csc`** - Comprehensive demo of all C++ functions
- **`cpp_integration_simple.csc`** - Simplified demo focusing on key functions
- **`cpp_integration_test.csc`** - Complete test suite for all C++ integration features
- **`custom_math_demo.csc`** - Demonstration of custom C++ library integration

## Example Usage

```typescript
// String processing
let text: string = "Hello World";
let reversed: string = string_reverse(text);
println(reversed); // "dlroW olleH"

// Array operations
let numbers: i32[] = [10, 5, 8, 3, 12];
let sum: i32 = array_sum_i32(numbers, numbers.length);
println(sum); // 38

// File operations
file_write("data.txt", "Hello from Cypescript!");
let content: string = file_read("data.txt");
println(content); // "Hello from Cypescript!"

// Random numbers
random_seed(42);
let rand: i32 = random_int(1, 100);
println(rand); // Random number between 1-100

// Custom math functions (requires custom C++ library)
let gcd: i32 = math_gcd(48, 18);
println(gcd); // 6

let fib: i32 = math_fibonacci(10);
println(fib); // 55
```

## Performance Benefits

- 🚀 **Zero-cost interop** - Direct C++ function calls through LLVM
- ⚡ **Native performance** - No FFI overhead
- 🔧 **LLVM optimizations** - Inlining and optimization across language boundaries
- 📚 **Entire C++ ecosystem** - Easy to extend with any C++ library

## Extending with New Functions

To add new C++ functions:

1. **Add to `src/cypescript_stdlib.cpp`:**
```cpp
extern "C" {
    int my_function(int x) {
        return x * 2;
    }
}
```

2. **Add to parser** (`src/Parser.cpp` - `isKnownFunction`)
3. **Add LLVM declaration** (`src/CodeGen.cpp` - `getOrDeclareExternalFunction`)

Then use in Cypescript:
```typescript
let result: i32 = my_function(21); // Returns 42
```

## Custom C++ Libraries

For advanced custom C++ integration, see the comprehensive guide:
**📚 [`CUSTOM_CPP_INTEGRATION.md`](../../CUSTOM_CPP_INTEGRATION.md)**

This covers:
- Creating custom C++ libraries
- Advanced type mapping
- Memory management
- Performance optimization
- Real-world examples (networking, databases, image processing)
