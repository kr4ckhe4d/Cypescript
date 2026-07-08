#!/bin/bash

# Cypescript Compile and Run Script
# This script compiles a .csc file and runs the resulting executable

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Check arguments
if [[ $# -eq 0 ]]; then
    echo -e "${RED}Usage: $0 <cypescript-file> [output-name]${NC}"
    echo -e "Example: $0 example/01_hello.csc"
    echo -e "Example: $0 example/01_hello.csc my_program"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_NAME="${2:-my_program}"

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

echo -e "${BLUE}🚀 Compiling and running: $INPUT_FILE${NC}"

# Step 1: Compile to LLVM IR
echo -e "${BLUE}📝 Generating LLVM IR...${NC}"
./build/cscript "$INPUT_FILE" -o "${OUTPUT_NAME}.ll"

# Find LLC
LLC_CMD=""
if command -v llc &> /dev/null; then
    LLC_CMD="llc"
elif [[ -f "/opt/homebrew/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/bin/llc"
elif [[ -f "/usr/local/bin/llc" ]]; then
    LLC_CMD="/usr/local/bin/llc"
else
    echo -e "${RED}❌ LLC not found. Please install LLVM.${NC}"
    exit 1
fi

# Step 2: Compile IR to object file
echo -e "${BLUE}🔧 Compiling to object file...${NC}"
$LLC_CMD -filetype=obj -relocation-model=pic "${OUTPUT_NAME}.ll" -o "${OUTPUT_NAME}.o"

# Step 3: Link to executable
echo -e "${BLUE}🔗 Linking executable...${NC}"
clang "${OUTPUT_NAME}.o" -o "$OUTPUT_NAME"

# Step 4: Run the program
echo -e "${GREEN}✅ Compilation successful!${NC}"
echo -e "${BLUE}🏃 Running $OUTPUT_NAME:${NC}"
echo "----------------------------------------"
./"$OUTPUT_NAME"
echo "----------------------------------------"

# Cleanup option
echo ""
read -p "Clean up intermediate files? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -f "${OUTPUT_NAME}.ll" "${OUTPUT_NAME}.o"
    echo -e "${GREEN}✓ Cleaned up intermediate files${NC}"
fi

echo -e "${GREEN}🎉 Done!${NC}"