# C++ Integration Examples

These examples demonstrate **C++ integration** capabilities using the enhanced compilation pipeline.

## Compilation Method

**Use the C++ integration script:**
```bash
./compile-with-cpp.sh example/cpp-integration/cpp_integration_basic.csc my_program
./my_program
```

This script automatically:
1. ✅ Compiles the C++ standard library (`src/cypescript_stdlib.cpp`)
2. ✅ Compiles Cypescript to LLVM IR
3. ✅ Links everything into a native executable
4. ✅ Cleans up intermediate files

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

## Example Files

- **`cpp_integration_basic.csc`** - Comprehensive demo of all C++ functions
- **`cpp_integration_simple.csc`** - Simplified demo focusing on key functions
- **`cpp_integration_test.csc`** - Complete test suite for all C++ integration features

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
