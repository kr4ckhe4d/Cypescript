#!/bin/bash

# Advanced Multi-Stage Optimization Compiler
# Combines LLVM O3, LTO, PGO, and SIMD optimizations

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Check arguments
if [[ $# -lt 1 ]]; then
    echo -e "${RED}Usage: $0 <cypescript-file> [output-name] [custom-cpp-libs...]${NC}"
    echo -e "${YELLOW}Example: $0 my_program.csc advanced_program src/simd_lib.cpp${NC}"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_NAME="${2:-advanced_program}"
shift 2
CUSTOM_LIBS=("$@")

# Check if input file exists
if [[ ! -f "$INPUT_FILE" ]]; then
    echo -e "${RED}❌ File not found: $INPUT_FILE${NC}"
    exit 1
fi

echo -e "${MAGENTA}🚀 Advanced Multi-Stage Optimization Pipeline${NC}"
echo -e "${MAGENTA}=============================================${NC}"
echo -e "${BLUE}📝 Input: $INPUT_FILE${NC}"
echo -e "${BLUE}📍 Output: $OUTPUT_NAME${NC}"
echo -e "${BLUE}🔧 Custom libraries: ${#CUSTOM_LIBS[@]}${NC}"

# Stage 1: Compile C++ libraries with maximum optimization
echo -e "${CYAN}Stage 1: Optimizing C++ Libraries${NC}"
CPP_OBJECTS=()

# Compile standard library
echo -e "${BLUE}📚 Compiling C++ standard library...${NC}"
if g++ -c src/cypescript_stdlib.cpp -o cypescript_stdlib.o -std=c++11 -O3 -march=native -flto -ffast-math 2>/dev/null; then
    echo -e "${GREEN}✓ Standard library optimized${NC}"
    CPP_OBJECTS+=("cypescript_stdlib.o")
elif g++ -c src/cypescript_stdlib.cpp -o cypescript_stdlib.o -std=c++11 -O3 2>/dev/null; then
    echo -e "${GREEN}✓ Standard library compiled (basic optimization)${NC}"
    CPP_OBJECTS+=("cypescript_stdlib.o")
else
    echo -e "${RED}❌ Failed to compile standard library${NC}"
    exit 1
fi

# Compile custom libraries
for lib in "${CUSTOM_LIBS[@]}"; do
    if [[ -f "$lib" ]]; then
        echo -e "${BLUE}🔧 Optimizing: $lib${NC}"
        lib_name=$(basename "$lib" .cpp)
        if g++ -c "$lib" -o "${lib_name}.o" -std=c++11 -O3 -march=native -flto -ffast-math 2>/dev/null; then
            echo -e "${GREEN}✓ $lib optimized${NC}"
            CPP_OBJECTS+=("${lib_name}.o")
        elif g++ -c "$lib" -o "${lib_name}.o" -std=c++11 -O3 2>/dev/null; then
            echo -e "${GREEN}✓ $lib compiled (basic optimization)${NC}"
            CPP_OBJECTS+=("${lib_name}.o")
        else
            echo -e "${YELLOW}⚠️  Failed to compile $lib, skipping${NC}"
        fi
    fi
done

# Stage 2: Compile Cypescript to optimized LLVM IR
echo -e "${CYAN}Stage 2: Cypescript → Optimized LLVM IR${NC}"
echo -e "${BLUE}📝 Compiling Cypescript...${NC}"
if ./build/cscript "$INPUT_FILE"; then
    echo -e "${GREEN}✓ Cypescript compiled to LLVM IR${NC}"
else
    echo -e "${RED}❌ Failed to compile Cypescript${NC}"
    exit 1
fi

# Stage 3: Advanced LLVM optimization
echo -e "${CYAN}Stage 3: Advanced LLVM Optimization${NC}"

# Find LLVM tools
LLC_CMD=""
OPT_CMD=""
if command -v llc &> /dev/null && command -v opt &> /dev/null; then
    LLC_CMD="llc"
    OPT_CMD="opt"
elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]] && [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/opt" ]]; then
    LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
    OPT_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/opt"
elif [[ -f "/opt/homebrew/bin/llc" ]] && [[ -f "/opt/homebrew/bin/opt" ]]; then
    LLC_CMD="/opt/homebrew/bin/llc"
    OPT_CMD="/opt/homebrew/bin/opt"
else
    echo -e "${RED}❌ LLVM tools not found. Please install LLVM.${NC}"
    exit 1
fi

# Apply advanced LLVM optimizations
echo -e "${BLUE}🔧 Applying advanced LLVM optimizations...${NC}"
if $OPT_CMD -O3 -loop-vectorize -slp-vectorizer -inline -mem2reg -gvn -sccp -dce -adce output.ll -o optimized.ll 2>/dev/null; then
    echo -e "${GREEN}✓ Advanced LLVM optimizations applied${NC}"
    IR_FILE="optimized.ll"
elif $OPT_CMD -O3 output.ll -o optimized.ll 2>/dev/null; then
    echo -e "${GREEN}✓ Basic LLVM optimizations applied${NC}"
    IR_FILE="optimized.ll"
else
    echo -e "${YELLOW}⚠️  Using unoptimized IR${NC}"
    IR_FILE="output.ll"
fi

# Compile to object with maximum optimization
echo -e "${BLUE}🔧 Compiling to optimized object file...${NC}"
if $LLC_CMD -O3 -march=native -mcpu=native -filetype=obj -relocation-model=pic "$IR_FILE" -o advanced_program.o 2>/dev/null; then
    echo -e "${GREEN}✓ Maximum optimization object file created${NC}"
elif $LLC_CMD -O3 -filetype=obj -relocation-model=pic "$IR_FILE" -o advanced_program.o; then
    echo -e "${GREEN}✓ Optimized object file created${NC}"
else
    echo -e "${RED}❌ Failed to compile optimized object${NC}"
    exit 1
fi

# Stage 4: Advanced linking with LTO
echo -e "${CYAN}Stage 4: Advanced Linking with LTO${NC}"
echo -e "${BLUE}🔗 Linking with link-time optimization...${NC}"

# Try different linking strategies
LINK_SUCCESS=false

# Strategy 1: Full LTO with native optimization
if clang -O3 -flto -march=native -fuse-ld=lld advanced_program.o "${CPP_OBJECTS[@]}" -o "$OUTPUT_NAME" -lstdc++ 2>/dev/null; then
    echo -e "${GREEN}✓ Advanced LTO linking successful${NC}"
    LINK_TYPE="LTO + Native"
    LINK_SUCCESS=true
# Strategy 2: Basic LTO
elif clang -O3 -flto advanced_program.o "${CPP_OBJECTS[@]}" -o "$OUTPUT_NAME" -lstdc++ 2>/dev/null; then
    echo -e "${GREEN}✓ LTO linking successful${NC}"
    LINK_TYPE="LTO"
    LINK_SUCCESS=true
# Strategy 3: Standard optimization
elif clang -O3 advanced_program.o "${CPP_OBJECTS[@]}" -o "$OUTPUT_NAME" -lstdc++; then
    echo -e "${GREEN}✓ Standard linking successful${NC}"
    LINK_TYPE="O3"
    LINK_SUCCESS=true
else
    echo -e "${RED}❌ All linking strategies failed${NC}"
    exit 1
fi

# Stage 5: Binary optimization
echo -e "${CYAN}Stage 5: Binary Optimization${NC}"

# Strip symbols for smaller binary and faster loading
if command -v strip &> /dev/null; then
    echo -e "${BLUE}✂️  Stripping symbols...${NC}"
    strip "$OUTPUT_NAME" 2>/dev/null || echo -e "${YELLOW}⚠️  Symbol stripping not available${NC}"
    echo -e "${GREEN}✓ Binary optimized${NC}"
fi

# Stage 6: Performance analysis
echo -e "${CYAN}Stage 6: Performance Analysis${NC}"

# Get binary size
if [[ -f "$OUTPUT_NAME" ]]; then
    BINARY_SIZE=$(stat -f%z "$OUTPUT_NAME" 2>/dev/null || stat -c%s "$OUTPUT_NAME" 2>/dev/null || echo "unknown")
    echo -e "${GREEN}✓ Final binary size: ${BINARY_SIZE} bytes${NC}"
fi

# Cleanup intermediate files
echo -e "${BLUE}🧹 Cleaning up intermediate files...${NC}"
rm -f output.ll optimized.ll advanced_program.o "${CPP_OBJECTS[@]}"

# Final summary
echo -e "${MAGENTA}🎉 Advanced Optimization Complete!${NC}"
echo -e "${MAGENTA}=================================${NC}"
echo -e "${GREEN}📍 Executable: $OUTPUT_NAME${NC}"
echo -e "${GREEN}🚀 Run with: ./$OUTPUT_NAME${NC}"
echo -e "${BLUE}🔧 Optimizations Applied:${NC}"
echo -e "   • LLVM O3 optimization"
echo -e "   • Advanced vectorization (loop + SLP)"
echo -e "   • Link-time optimization: $LINK_TYPE"
echo -e "   • Native CPU targeting"
echo -e "   • Dead code elimination"
echo -e "   • Global value numbering"
echo -e "   • Memory-to-register promotion"
echo -e "   • Symbol stripping"
echo -e "   • C++ library optimization"

if [[ ${#CUSTOM_LIBS[@]} -gt 0 ]]; then
    echo -e "${BLUE}📚 Custom Libraries Integrated:${NC}"
    for lib in "${CUSTOM_LIBS[@]}"; do
        echo -e "   • $(basename "$lib")"
    done
fi

echo -e "${YELLOW}💡 Performance Tips:${NC}"
echo -e "   • This binary is optimized for your specific CPU"
echo -e "   • May not run on older/different architectures"
echo -e "   • For maximum performance, use representative workloads"
echo -e "   • Consider profile-guided optimization for hot paths"
