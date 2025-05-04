# sudo apt install libzstd-dev
# sudo apt install llvm-16-dev clang-16 lld-16 # Install dev files and tools
# sudo apt install llvm
# sudo apt install clang

# configure cmake project and build
# rm -rf build/ && cmake -B build && cmake --build build
# generate the llvm bitcode
# ./build/cscript ./example/hello.csc
# llc -filetype=obj -relocation-model=pic output.ll -o output.o
# clang output.o -o my_program
# ./my_program

# Minimum CMake version required
cmake_minimum_required(VERSION 3.15) # LLVM often needs a reasonably recent CMake

# Project Name
project(Cypescript LANGUAGES C CXX)

# Set C++ Standard (LLVM typically requires C++17 or newer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer explicit standard features

# --- Find LLVM ---
# You might need to adjust the version number (e.g., 17, 16)
# You might need to hint CMake where LLVM is if it's not in a standard path:
# set(LLVM_DIR "/path/to/your/llvm/lib/cmake/llvm") # Example hint

find_package(LLVM 16 REQUIRED CONFIG) # Adjust version (e.g. 16, 17) if needed

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# --- Configure LLVM ---
# Include LLVM header directories
include_directories(${LLVM_INCLUDE_DIRS})
# Add LLVM definitions (required by LLVM headers)
add_definitions(${LLVM_DEFINITIONS})

# --- Our Compiler Executable ---
add_executable(cscript
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/CodeGen.cpp
    # Add other .cpp files here later (AST.cpp if needed)
)

# --- Link against LLVM Libraries ---
# List the LLVM components you need. Start minimal.
# Core: Basic LLVM data structures.
# Support: Helper utilities (file handling, error handling, etc.).
# IR: Core Intermediate Representation classes.
# AsmPrinter: For printing LLVM IR text. (Useful for debugging)
# We'll add more like CodeGen, Target, MC later.
set(LLVM_COMPONENTS Core Support AsmPrinter)

llvm_map_components_to_libnames(LLVM_LIBS ${LLVM_COMPONENTS})

# Link our compiler executable against the LLVM libraries
target_link_libraries(cscript PRIVATE ${LLVM_LIBS})

# Add flags required by LLVM (handles RTTI, Exceptions based on LLVM build)
# Note: Check LLVM documentation or examples for the most current way,
# but this often works. Adapt if needed.
string(APPEND CMAKE_CXX_FLAGS " ${LLVM_CXX_FLAGS}")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${LLVM_LDFLAGS}")
string(APPEND CMAKE_EXE_LINKER_FLAGS " ${LLVM_LDFLAGS}")

# Optional: Ensure assertions are enabled in Debug builds for LLVM
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(_DEBUG NDEBUG=0)
endif()

# --- Output ---
message(STATUS "Compiler Target: cscript")
message(STATUS "LLVM Include Dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM Library Dirs: ${LLVM_LIBRARY_DIRS}") # Might be empty if using imported targets
message(STATUS "Linking against LLVM Components: ${LLVM_COMPONENTS}")
message(STATUS "LLVM Libs Variable: ${LLVM_LIBS}") # Shows the actual library names found