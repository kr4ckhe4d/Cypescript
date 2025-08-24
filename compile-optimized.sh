#!/bin/bash

# Optimized Cypescript Compiler with Maximum Performance
# Uses aggressive LLVM optimization flags

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
OUTPUT_NAME="${2:-optimized_program}"

# Check if input file exists
if [[ ! -f "$INPUT_FILE" ]]; then
    echo -e "${RED}❌ File not found: $INPUT_FILE${NC}"
    exit 1
fi

# Check if compiler exists
if [[ ! -f "build/cscript" ]]; then
    echo -e "${YELLOW}⚠️  Compiler not found, building...${NC}"
    ./build.sh
fi

echo -e "${BLUE}🚀 Optimized Cypescript Compilation: $INPUT_FILE${NC}"

# Step 1: Compile Cypescript to LLVM IR
echo -e "${BLUE}📝 Compiling Cypescript to LLVM IR...${NC}"
if ./build/cscript "$INPUT_FILE"; then
    echo -e "${GREEN}✓ Cypescript compiled to LLVM IR${NC}"
else
    echo -e "${RED}❌ Failed to compile Cypescript file${NC}"
    exit 1
fi

# Find LLC with optimization support
LLC_CMD=""
if command -v llc &> /dev/null; then
    LLC_CMD="llc"
elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
elif [[ -f "/opt/homebrew/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/bin/llc"
elif [[ -f "/usr/local/bin/llc" ]]; then
    LLC_CMD="/usr/local/bin/llc"
else
    echo -e "${RED}❌ LLC not found. Please install LLVM.${NC}"
    exit 1
fi

# Step 2: Optimize LLVM IR and compile to object file with maximum optimization
echo -e "${BLUE}🔧 Compiling with maximum optimization (-O3)...${NC}"

# Try different optimization levels, starting with most aggressive
if $LLC_CMD -O3 -filetype=obj -relocation-model=pic output.ll -o optimized_program.o; then
    echo -e "${GREEN}✓ Highly optimized object file created (O3)${NC}"
    OPT_LEVEL="O3"
elif $LLC_CMD -O2 -filetype=obj -relocation-model=pic output.ll -o optimized_program.o; then
    echo -e "${GREEN}✓ Optimized object file created (O2)${NC}"
    OPT_LEVEL="O2"
elif $LLC_CMD -O1 -filetype=obj -relocation-model=pic output.ll -o optimized_program.o; then
    echo -e "${GREEN}✓ Basic optimized object file created (O1)${NC}"
    OPT_LEVEL="O1"
else
    echo -e "${YELLOW}⚠️  Falling back to unoptimized compilation...${NC}"
    $LLC_CMD -filetype=obj -relocation-model=pic output.ll -o optimized_program.o
    OPT_LEVEL="O0"
fi

# Step 3: Link with optimization flags
echo -e "${BLUE}🔗 Linking with optimization flags...${NC}"

# Try optimized linking, fall back to basic if needed
if clang -O3 -flto optimized_program.o -o "$OUTPUT_NAME" 2>/dev/null; then
    echo -e "${GREEN}✓ Executable linked with LTO optimization${NC}"
    LINK_TYPE="LTO enabled"
elif clang -O3 optimized_program.o -o "$OUTPUT_NAME"; then
    echo -e "${GREEN}✓ Executable linked with basic optimization${NC}"
    LINK_TYPE="O3 optimization"
else
    echo -e "${RED}❌ Failed to link executable${NC}"
    exit 1
fi

# Step 4: Cleanup intermediate files
echo -e "${BLUE}🧹 Cleaning up intermediate files...${NC}"
rm -f output.ll optimized_program.o

echo -e "${GREEN}🎉 Optimized compilation complete!${NC}"
echo -e "${YELLOW}📍 Executable: ${NC}$OUTPUT_NAME"
echo -e "${YELLOW}🚀 Run with: ${NC}./$OUTPUT_NAME"

# Show optimization details
echo -e "${BLUE}⚡ Optimizations Applied:${NC}"
echo -e "   • LLVM optimization level: $OPT_LEVEL"
echo -e "   • Link-time optimization: $LINK_TYPE"
echo -e "   • Dead code elimination"
echo -e "   • Register allocation optimization"

# Show file size comparison if original exists
if [[ -f "my_program" ]]; then
    ORIGINAL_SIZE=$(stat -f%z my_program 2>/dev/null || stat -c%s my_program 2>/dev/null || echo "unknown")
    OPTIMIZED_SIZE=$(stat -f%z "$OUTPUT_NAME" 2>/dev/null || stat -c%s "$OUTPUT_NAME" 2>/dev/null || echo "unknown")
    
    if [[ "$ORIGINAL_SIZE" != "unknown" && "$OPTIMIZED_SIZE" != "unknown" ]]; then
        echo -e "${BLUE}📊 Binary Size Comparison:${NC}"
        echo -e "   • Unoptimized: ${ORIGINAL_SIZE} bytes"
        echo -e "   • Optimized: ${OPTIMIZED_SIZE} bytes"
        
        if [[ $OPTIMIZED_SIZE -lt $ORIGINAL_SIZE ]]; then
            REDUCTION=$((100 - (OPTIMIZED_SIZE * 100 / ORIGINAL_SIZE)))
            echo -e "   • ${GREEN}Size reduction: ${REDUCTION}%${NC}"
        fi
    fi
fi
