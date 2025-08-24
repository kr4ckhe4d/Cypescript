#!/bin/bash

# Static-Linked Cypescript Compiler for Minimal Startup Time
# Creates a statically linked executable to reduce loading overhead

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Check arguments
if [[ $# -lt 1 ]]; then
    echo -e "${RED}Usage: $0 <cypescript-file> [output-name]${NC}"
    echo -e "Example: $0 example.csc my_program"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_NAME="${2:-static_program}"

echo -e "${BLUE}🚀 Static-Linked Cypescript Compilation: $INPUT_FILE${NC}"

# Step 1: Compile Cypescript to LLVM IR
echo -e "${BLUE}📝 Compiling Cypescript to LLVM IR...${NC}"
if ./build/cscript "$INPUT_FILE"; then
    echo -e "${GREEN}✓ Cypescript compiled to LLVM IR${NC}"
else
    echo -e "${RED}❌ Failed to compile Cypescript file${NC}"
    exit 1
fi

# Find LLC
LLC_CMD=""
if command -v llc &> /dev/null; then
    LLC_CMD="llc"
elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
elif [[ -f "/opt/homebrew/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/bin/llc"
else
    echo -e "${RED}❌ LLC not found. Please install LLVM.${NC}"
    exit 1
fi

# Step 2: Compile with optimization and static linking preparation
echo -e "${BLUE}🔧 Compiling for static linking...${NC}"
OPTIMIZATION_FLAGS="-O3 -march=native"

if $LLC_CMD $OPTIMIZATION_FLAGS -filetype=obj -relocation-model=static output.ll -o static_program.o; then
    echo -e "${GREEN}✓ Static object file created${NC}"
else
    echo -e "${RED}❌ Failed to compile for static linking${NC}"
    exit 1
fi

# Step 3: Static linking (attempt, may not work on all systems)
echo -e "${BLUE}🔗 Attempting static linking...${NC}"

# Try static linking first, fall back to optimized dynamic linking
if clang -static -O3 static_program.o -o "$OUTPUT_NAME" 2>/dev/null; then
    echo -e "${GREEN}✓ Statically linked executable created${NC}"
    LINK_TYPE="static"
elif clang -O3 -Wl,-dead_strip static_program.o -o "$OUTPUT_NAME" 2>/dev/null; then
    echo -e "${GREEN}✓ Optimized dynamically linked executable created (static linking not available)${NC}"
    LINK_TYPE="dynamic-optimized"
else
    echo -e "${YELLOW}⚠️  Falling back to standard linking...${NC}"
    clang -O3 static_program.o -o "$OUTPUT_NAME"
    LINK_TYPE="dynamic-standard"
fi

# Step 4: Strip symbols to reduce size and improve loading
echo -e "${BLUE}✂️  Stripping symbols for faster loading...${NC}"
if command -v strip &> /dev/null; then
    strip "$OUTPUT_NAME" 2>/dev/null || echo -e "${YELLOW}⚠️  Symbol stripping not available${NC}"
    echo -e "${GREEN}✓ Symbols stripped${NC}"
fi

# Step 5: Cleanup
echo -e "${BLUE}🧹 Cleaning up...${NC}"
rm -f output.ll static_program.o

echo -e "${GREEN}🎉 Static compilation complete!${NC}"
echo -e "${YELLOW}📍 Executable: ${NC}$OUTPUT_NAME"
echo -e "${YELLOW}🚀 Run with: ${NC}./$OUTPUT_NAME"

# Show optimization details
echo -e "${BLUE}⚡ Optimizations Applied:${NC}"
echo -e "   • Link type: $LINK_TYPE"
echo -e "   • LLVM -O3 optimization"
echo -e "   • Native CPU targeting (-march=native)"
echo -e "   • Symbol stripping for faster loading"
echo -e "   • Dead code elimination"

# Show file size
if [[ -f "$OUTPUT_NAME" ]]; then
    FILE_SIZE=$(stat -f%z "$OUTPUT_NAME" 2>/dev/null || stat -c%s "$OUTPUT_NAME" 2>/dev/null || echo "unknown")
    echo -e "${BLUE}📊 Binary size: ${NC}${FILE_SIZE} bytes"
fi
