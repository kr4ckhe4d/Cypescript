# Cypescript - A TypeScript-style language compiler built with C++ and LLVM
# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project Name and Version - Enable both C and CXX for LLVM dependencies
project(Cypescript 
    VERSION 1.0.0
    DESCRIPTION "A TypeScript-style language compiler"
    LANGUAGES C CXX
)

# Set C++ Standard (LLVM typically requires C++17 or newer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3)
    endif()
endif()

# --- Find LLVM ---
# On macOS with Homebrew, LLVM might be in /opt/homebrew/opt/llvm or /usr/local/opt/llvm
if(APPLE)
    # Try Homebrew paths first
    set(LLVM_HINTS
        /opt/homebrew/opt/llvm/lib/cmake/llvm
        /usr/local/opt/llvm/lib/cmake/llvm
    )
    find_path(LLVM_CMAKE_DIR
        NAMES LLVMConfig.cmake
        PATHS ${LLVM_HINTS}
        NO_DEFAULT_PATH
    )
    if(LLVM_CMAKE_DIR)
        set(LLVM_DIR ${LLVM_CMAKE_DIR})
        message(STATUS "Found LLVM CMake files at: ${LLVM_DIR}")
    endif()
endif()

# Find LLVM (try different versions)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Separate LLVM version components
string(REGEX MATCH "^([0-9]+)" LLVM_VERSION_MAJOR ${LLVM_PACKAGE_VERSION})

# --- Configure LLVM ---
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST UNIX_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# --- Our Compiler Executable ---
add_executable(cscript
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/CodeGen.cpp
)

# Set target properties
set_target_properties(cscript PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "cscript"
)

# --- Link against LLVM Libraries ---
# LLVM components we need (simplified for compatibility)
set(LLVM_COMPONENTS 
    Core 
    Support 
    AsmPrinter
    BitWriter
    Analysis
    Target
    MC
)

# Map components to library names
llvm_map_components_to_libnames(LLVM_LIBS ${LLVM_COMPONENTS})

# Link our compiler executable against the LLVM libraries
target_link_libraries(cscript PRIVATE ${LLVM_LIBS})

# Add LLVM compile flags if available
if(LLVM_CXXFLAGS)
    target_compile_options(cscript PRIVATE ${LLVM_CXXFLAGS})
endif()

# Handle LLVM link flags
if(LLVM_LDFLAGS)
    set_target_properties(cscript PROPERTIES LINK_FLAGS ${LLVM_LDFLAGS})
endif()

# --- Platform-specific settings ---
if(APPLE)
    # macOS specific settings
    target_link_libraries(cscript PRIVATE "-framework CoreFoundation")
endif()

# --- Debug Information ---
message(STATUS "=== Cypescript Build Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "LLVM Version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM Include Dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM Components: ${LLVM_COMPONENTS}")
message(STATUS "Target: cscript")
message(STATUS "=====================================")

# --- Install target (optional) ---
install(TARGETS cscript
    RUNTIME DESTINATION bin
)