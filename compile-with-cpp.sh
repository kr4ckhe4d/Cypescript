#!/bin/bash

# Enhanced Cypescript compilation script with C++ library support
# Usage: ./compile-with-cpp.sh <cypescript_file> [output_name]

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if file argument is provided
if [ $# -eq 0 ]; then
    echo -e "${RED}❌ Error: Please provide a Cypescript file to compile${NC}"
    echo "Usage: $0 <cypescript_file> [output_name]"
    exit 1
fi

CYPESCRIPT_FILE="$1"
OUTPUT_NAME="${2:-cypescript_program}"

# Check if input file exists
if [ ! -f "$CYPESCRIPT_FILE" ]; then
    echo -e "${RED}❌ Error: File '$CYPESCRIPT_FILE' not found${NC}"
    exit 1
fi

echo -e "${BLUE}🚀 Compiling Cypescript with C++ Integration: $CYPESCRIPT_FILE${NC}"

# Step 1: Compile the C++ standard library
echo -e "${BLUE}📚 Compiling C++ standard library...${NC}"
if ! g++ -c src/cypescript_stdlib.cpp -o cypescript_stdlib.o -std=c++11; then
    echo -e "${RED}❌ Failed to compile C++ standard library${NC}"
    exit 1
fi
echo -e "${GREEN}✓ C++ library compiled successfully${NC}"

# Step 2: Compile Cypescript to LLVM IR
echo -e "${BLUE}📝 Compiling Cypescript to LLVM IR...${NC}"
if ! ./build/cscript "$CYPESCRIPT_FILE"; then
    echo -e "${RED}❌ Failed to compile Cypescript file${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Cypescript compiled to LLVM IR${NC}"

# Step 3: Find LLVM tools
LLC_PATH=""
if command -v llc &> /dev/null; then
    LLC_PATH="llc"
elif [ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]; then
    LLC_PATH="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
elif [ -f "/usr/local/bin/llc" ]; then
    LLC_PATH="/usr/local/bin/llc"
else
    echo -e "${RED}❌ Error: llc not found. Please install LLVM${NC}"
    exit 1
fi

# Step 4: Compile LLVM IR to object file
echo -e "${BLUE}🔧 Compiling LLVM IR to object file...${NC}"
if ! "$LLC_PATH" -filetype=obj -relocation-model=pic output.ll -o cypescript_program.o; then
    echo -e "${RED}❌ Failed to compile LLVM IR to object file${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Object file created${NC}"

# Step 5: Link everything together
echo -e "${BLUE}🔗 Linking with C++ standard library...${NC}"
if ! clang cypescript_program.o cypescript_stdlib.o -o "$OUTPUT_NAME" -lstdc++; then
    echo -e "${RED}❌ Failed to link executable${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Executable linked successfully${NC}"

# Step 6: Clean up intermediate files
echo -e "${BLUE}🧹 Cleaning up intermediate files...${NC}"
rm -f cypescript_program.o output.ll

echo -e "${GREEN}🎉 Compilation complete!${NC}"
echo -e "${YELLOW}📍 Executable: ${NC}$OUTPUT_NAME"
echo -e "${YELLOW}🚀 Run with: ${NC}./$OUTPUT_NAME"

# Make the output executable
chmod +x "$OUTPUT_NAME"
