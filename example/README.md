# Cypescript Examples

This directory contains **categorized examples** demonstrating different aspects of the Cypescript language.

## 🎯 **NEW!** User-Defined Functions Demo

### 🔥 [`functions_demo.csc`](functions_demo.csc) - **Phase 1 Functions Implementation**
**Comprehensive demonstration of user-defined functions!**

**Compilation:**
```bash
./build/cscript example/functions_demo.csc
llc -filetype=obj -relocation-model=pic output.ll -o output.o
clang output.o -o functions_demo
./functions_demo
```

**Features Demonstrated:**
- ✅ **Function declarations** with parameters and return types
- ✅ **Return statements** with expressions
- ✅ **Local variables** and proper scoping
- ✅ **Function calls** in expressions and statements
- ✅ **Void functions** for side effects
- ✅ **Nested function calls** and complex logic
- ✅ **Integration** with existing language features

### 🔥 [`json_demo.csc`](json_demo.csc) - **Native JSON Support**
**Comprehensive demonstration of native JSON operations!**

**Compilation:**
```bash
./compile-with-cpp.sh example/json_demo.csc json_demo
./json_demo
```

**Features Demonstrated:**
- ✅ **JSON object creation** and manipulation
- ✅ **String, number, and boolean** properties
- ✅ **Data retrieval** and type conversion
- ✅ **JSON validation** and error checking
- ✅ **Pretty-printing and minification**
- ✅ **Real-world configuration** examples

**Sample Output:**
```
🚀 Cypescript Functions Demo - Phase 1
================================
Testing with x=15, y=25
add(x, y) = 40
multiply(x, y) = 375
factorial(5) = 120
power(2, 8) = 256
Hello, Alice! You are 28 years old.
Rectangle area (12 x 8) = 96
complexMath(5, 10, 3) = 36
✅ All function tests completed successfully!
```

## 📁 Directory Structure

### 🔧 [`basic/`](basic/) - Native Compiler Examples
Examples that work with the **basic compilation pipeline**.

**Compilation:**
```bash
./compile-run.sh example/basic/hello.csc
```

**Features:** Core language (variables, arithmetic, control flow, loops, arrays)

### 🚀 [`cpp-integration/`](cpp-integration/) - C++ Integration Examples  
Examples demonstrating **C++ library integration**.

**Compilation:**
```bash
./compile-with-cpp.sh example/cpp-integration/cpp_integration_basic.csc my_program
```

**Features:** All basic features + 20+ C++ functions (string, array, file I/O, utilities)

### 🌐 [`browser-only/`](browser-only/) - Browser Interpreter Examples
Examples using **advanced features** only available in the browser.

**Usage:**
```bash
./launch-docs.sh  # Then use the browser playground
```

**Features:** Objects, booleans, floating-point numbers, complex data structures

## 🎯 Quick Start Guide

### 1. **New to Cypescript?** Start with Basic Examples
```bash
./compile-run.sh example/basic/hello.csc
./compile-run.sh example/basic/arithmetic.csc
./compile-run.sh example/basic/control_flow.csc
```

### 2. **Want Functions?** Try the Functions Demo
```bash
# Compile and run the comprehensive functions example
./build/cscript example/functions_demo.csc
llc -filetype=obj -relocation-model=pic output.ll -o output.o
clang output.o -o functions_demo
./functions_demo
```

### 3. **Want JSON?** Try the JSON Demo
```bash
# Compile and run the comprehensive JSON example
./compile-with-cpp.sh example/json_demo.csc json_demo
./json_demo
```

### 4. **Want More Power?** Try C++ Integration
```bash
./compile-with-cpp.sh example/cpp-integration/cpp_integration_basic.csc demo
./demo
```

### 3. **Need Custom Libraries?** Use Custom C++ Integration
```bash
./compile-with-custom-cpp.sh example/cpp-integration/custom_math_demo.csc demo src/custom_math_lib.cpp
./demo
```

### 4. **Learning Complex Concepts?** Use Browser Examples
```bash
./launch-docs.sh
# Copy code from browser-only/ examples into the playground
```

## 📊 Feature Compatibility Matrix

| Feature | Basic | C++ Integration | Custom C++ | Browser Only |
|---------|-------|----------------|------------|--------------|
| Variables (`let`) | ✅ | ✅ | ✅ | ✅ |
| Basic types (`string`, `i32`) | ✅ | ✅ | ✅ | ✅ |
| Arithmetic (`+`, `-`, `*`, `/`, `%`) | ✅ | ✅ | ✅ | ✅ |
| Comparisons (`==`, `!=`, `<`, etc.) | ✅ | ✅ | ✅ | ✅ |
| Control flow (`if`/`else`) | ✅ | ✅ | ✅ | ✅ |
| Loops (`while`, `for`, `do-while`) | ✅ | ✅ | ✅ | ✅ |
| Arrays (`i32[]`, `string[]`) | ✅ | ✅ | ✅ | ✅ |
| Array operations (`.length`, `[index]`) | ✅ | ✅ | ✅ | ✅ |
| Built-in functions (`print`, `println`) | ✅ | ✅ | ✅ | ✅ |
| **C++ functions** (20+ functions) | ❌ | ✅ | ✅ | ❌ |
| **Custom C++ libraries** | ❌ | ❌ | ✅ | ❌ |
| **Object literals** (`{ key: value }`) | ❌ | ❌ | ❌ | ✅ |
| **Boolean literals** (`true`, `false`) | ❌ | ❌ | ❌ | ✅ |
| **Float literals** (`3.14`) | ❌ | ❌ | ❌ | ✅ |

## 🏗️ Build Scripts Reference

### `./compile-run.sh` - Basic Compilation
- **Input:** `.csc` files using core features only
- **Output:** Native executable via LLVM
- **Performance:** Maximum (native machine code)
- **Dependencies:** LLVM, Clang

### `./compile-with-cpp.sh` - C++ Integration
- **Input:** `.csc` files using C++ functions
- **Output:** Native executable linked with C++ library
- **Performance:** Maximum (native + zero-cost C++ interop)
- **Dependencies:** LLVM, Clang, G++

### `./compile-with-custom-cpp.sh` - Custom C++ Libraries
- **Input:** `.csc` files + custom `.cpp` files
- **Output:** Native executable with custom C++ integration
- **Performance:** Maximum (native + custom libraries)
- **Dependencies:** LLVM, Clang, G++
- **Usage:** `./compile-with-custom-cpp.sh program.csc output lib1.cpp lib2.cpp`

### `./launch-docs.sh` - Browser Interpreter
- **Input:** Any `.csc` code (full language support)
- **Output:** Interactive execution in browser
- **Performance:** Good (JavaScript interpreter)
- **Dependencies:** Python 3, Web browser

## 🎓 Learning Path

1. **Start Here:** `basic/hello.csc` - Learn basic syntax
2. **Core Features:** `basic/arithmetic.csc`, `basic/control_flow.csc`
3. **Data Structures:** `basic/array_length_test.csc`
4. **Algorithms:** `basic/comprehensive.csc`
5. **System Integration:** `cpp-integration/cpp_integration_basic.csc`
6. **Advanced Concepts:** `browser-only/game_system.csc`

## 🔧 Troubleshooting

### "Parsing failed" Errors
- Check if you're using unsupported features (objects, booleans, floats)
- Try the appropriate build script for your example category

### "Function not found" Errors  
- C++ functions require `./compile-with-cpp.sh`
- Object methods require browser interpreter

### "LLC not found" Errors
- Install LLVM: `./setup-macos.sh`
- Check PATH includes LLVM binaries

## 📚 Additional Resources

- **Language Documentation:** `./launch-docs.sh`
- **Build System:** See main `README.md`
- **C++ Integration Guide:** `src/cypescript_stdlib.cpp`
