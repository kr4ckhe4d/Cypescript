# Cypescript - A TypeScript-style language compiler built with C++ and LLVM
# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project Name and Version - Enable both C and CXX for LLVM dependencies
project(Cypescript
    VERSION 1.1.0
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

# --- Cypescript Runtime Library ---
# Precompiled so the installed compiler does not need the stdlib sources.
add_library(cypescript_stdlib STATIC
    src/cypescript_stdlib.cpp
)
set_target_properties(cypescript_stdlib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "cypescript"          # -> libcypescript.a
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# --- Cypescript Game Runtime ---
# A flat C shim over raylib so Cypescript programs can open a window and draw.
#
# raylib is vendored by default: CMake downloads and builds it from source, so
# `cscript` ships a self-contained game runtime on macOS, Linux and Windows with
# no system package to install. Set -DCYPESCRIPT_VENDOR_RAYLIB=OFF to link a
# system raylib instead (faster incremental builds if you already have one).
#
# raylib is zlib/libpng licensed, which is compatible with this project's MIT
# license. See THIRD_PARTY.md for the attribution notice we redistribute.
option(CYPESCRIPT_VENDOR_RAYLIB "Download and build raylib from source" ON)
option(CYPESCRIPT_BUILD_GAME_RUNTIME "Build the game runtime (needs raylib)" ON)

set(CYPESCRIPT_GAME_AVAILABLE FALSE)

if(CYPESCRIPT_BUILD_GAME_RUNTIME)
    set(RAYLIB_TARGET "")

    if(CYPESCRIPT_VENDOR_RAYLIB)
        include(FetchContent)
        message(STATUS "Vendoring raylib (set -DCYPESCRIPT_VENDOR_RAYLIB=OFF to use a system one)")

        # Build only what a game needs: a static lib, no examples, no games.
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)

        FetchContent_Declare(raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG        5.5
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
        )
        FetchContent_MakeAvailable(raylib)
        set(RAYLIB_TARGET raylib)
        set(CYPESCRIPT_GAME_AVAILABLE TRUE)
    else()
        find_path(RAYLIB_INCLUDE_DIR raylib.h
            HINTS /opt/homebrew/opt/raylib/include /usr/local/opt/raylib/include
                  /usr/local/include /usr/include
        )
        find_library(RAYLIB_LIBRARY NAMES raylib
            HINTS /opt/homebrew/opt/raylib/lib /usr/local/opt/raylib/lib
                  /usr/local/lib /usr/lib
        )
        if(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY)
            message(STATUS "Found system raylib: ${RAYLIB_LIBRARY}")
            set(CYPESCRIPT_GAME_AVAILABLE TRUE)
        else()
            message(STATUS "raylib not found — skipping the game runtime "
                           "(re-run with -DCYPESCRIPT_VENDOR_RAYLIB=ON to bundle it)")
        endif()
    endif()
endif()

if(CYPESCRIPT_GAME_AVAILABLE)
    add_library(cypescript_game STATIC
        runtime/game/cyps_game.c
    )
    set_target_properties(cypescript_game PROPERTIES
        C_STANDARD 11
        OUTPUT_NAME "cypescript_game"     # -> libcypescript_game.a
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    if(RAYLIB_TARGET)
        target_link_libraries(cypescript_game PUBLIC ${RAYLIB_TARGET})
        # Put the vendored libraylib.a beside our own libs, where cscript's
        # automatic -L already points, so `link "raylib";` resolves with no
        # system package and no machine-specific paths.
        add_custom_command(TARGET cypescript_game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${RAYLIB_TARGET}> ${CMAKE_BINARY_DIR}
            COMMENT "Staging vendored raylib next to the Cypescript runtime"
        )
    else()
        target_include_directories(cypescript_game PRIVATE ${RAYLIB_INCLUDE_DIR})
    endif()
    # raylib's headers use designated initialisers and compound literals freely
    if(NOT MSVC)
        target_compile_options(cypescript_game PRIVATE -Wno-missing-field-initializers)
    endif()
endif()

# --- Platform link requirements for games ---
# Baked into lib/game.csc at configure time: raylib's system dependencies differ
# per OS and Cypescript has no conditional compilation.
set(CYPESCRIPT_GAME_LINK_DIRECTIVES "link \"cypescript_game\";\nlink \"raylib\";")
if(APPLE)
    string(APPEND CYPESCRIPT_GAME_LINK_DIRECTIVES
        "\nlink framework \"Cocoa\";"
        "\nlink framework \"IOKit\";"
        "\nlink framework \"CoreVideo\";"
        "\nlink framework \"CoreAudio\";"
        "\nlink framework \"OpenGL\";")
    if(NOT CYPESCRIPT_VENDOR_RAYLIB)
        # System raylib lives outside clang's default search path on macOS
        set(CYPESCRIPT_GAME_LINK_DIRECTIVES
            "link path \"/opt/homebrew/lib\";\nlink path \"/usr/local/lib\";\n${CYPESCRIPT_GAME_LINK_DIRECTIVES}")
    endif()
elseif(WIN32)
    string(APPEND CYPESCRIPT_GAME_LINK_DIRECTIVES
        "\nlink \"opengl32\";"
        "\nlink \"gdi32\";"
        "\nlink \"winmm\";"
        "\nlink \"shell32\";")
elseif(UNIX)
    string(APPEND CYPESCRIPT_GAME_LINK_DIRECTIVES
        "\nlink \"GL\";"
        "\nlink \"m\";"
        "\nlink \"pthread\";"
        "\nlink \"dl\";"
        "\nlink \"rt\";"
        "\nlink \"X11\";")
endif()

# Generated into the build tree; cscript finds it next to its own binary.
configure_file(
    ${CMAKE_SOURCE_DIR}/lib/game.csc.in
    ${CMAKE_BINARY_DIR}/lib/game.csc
    @ONLY
)

# --- Our Compiler Executable ---
add_executable(cscript
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/CodeGen.cpp
    src/ObjectOptimizer.cpp  # Phase 1 optimization: direct struct property access
    src/Optimizer.cpp        # Phase 3 optimization: AST constant folding + dead branches
    src/Semantic.cpp         # Semantic analysis: scoped checks with source positions
)

# Set target properties
set_target_properties(cscript PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "cscript"
)
add_dependencies(cscript cypescript_stdlib)
target_compile_definitions(cscript PRIVATE CYPESCRIPT_VERSION="${PROJECT_VERSION}")

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

# --- Install target ---
# Layout: <prefix>/bin/cscript + <prefix>/lib/libcypescript.a
# cscript locates the runtime library relative to its own path (or CYPESCRIPT_HOME).
install(TARGETS cscript
    RUNTIME DESTINATION bin
)
install(TARGETS cypescript_stdlib
    ARCHIVE DESTINATION lib
)
if(CYPESCRIPT_GAME_AVAILABLE)
    install(TARGETS cypescript_game
        ARCHIVE DESTINATION lib
    )
endif()

# Bundled Cypescript modules, importable by bare name: import { } from "game";
# cscript locates this directory relative to its own binary (or CYPESCRIPT_HOME).
install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/
    DESTINATION lib/cypescript
    FILES_MATCHING PATTERN "*.csc"
)

# The vendored raylib static library ships alongside, so installed games link
# against the exact build the runtime was compiled against.
if(CYPESCRIPT_GAME_AVAILABLE AND CYPESCRIPT_VENDOR_RAYLIB)
    install(FILES $<TARGET_FILE:raylib> DESTINATION lib)
    # raylib's zlib license requires its notice to travel with binaries
    install(FILES ${CMAKE_SOURCE_DIR}/THIRD_PARTY.md
            DESTINATION share/cypescript)
endif()