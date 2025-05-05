# Cypescript

A simple compiler for the MyLang language, built as a learning project using C++ and the LLVM compiler infrastructure. Currently, it supports a basic `print("string");` command.

## Setup and Installation

These instructions assume a Debian/Ubuntu-based Linux system. Adapt package names for other distributions (e.g., `llvm-devel`, `clang-devel` on Fedora/CentOS; `llvm` via Homebrew on macOS).

```
# Update package lists
sudo apt update

# Install core LLVM/Clang development files and tools
# - llvm-X-dev: Essential C++ headers and libraries for LLVM API
# - clang-X: C/C++ compiler (use this for linking for best compatibility)
# - lld-X: LLVM's linker (optional, clang/gcc use system 'ld' by default)
sudo apt install llvm-16-dev clang-16 lld-16 # Install dev files and tools

# Install LLVM runtime command-line tools (e.g., llc) matching the version
sudo apt install llvm

# Install other potential LLVM dependencies
sudo apt install libzstd-dev

# Check the installed LLVM tool (llc) version
llc-16 --version

# Check the installed Clang version
clang-16 --version
```

**Configure cmake project and build**
```rm -rf build/ && cmake -B build && cmake --build build```

**Running and Compiling Workflow**
```
# 1. Generate the llvm bitcode (or IR) from your source file
./build/cscript ./example/hello.csc

# 2. Compile the generated LLVM IR (.ll) to an object file (.o)
llc-16 -filetype=obj -relocation-model=pic output.ll -o output.o
**For llc versions below 15: llc --opaque-pointers -filetype=obj -relocation-model=pic output.ll -o output.o**

# 3. Link the object file (.o) into a final executable
clang output.o -o my_program
# (Alternatively: gcc output.o -o my_program)

# 4. Run the compiled program
./my_program
```