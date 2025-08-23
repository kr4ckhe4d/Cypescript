#!/bin/bash

# Enhanced Cypescript Compiler with Custom C++ Libraries
# This script allows you to compile Cypescript with custom C++ files

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Check arguments
if [[ $# -lt 2 ]]; then
    echo -e "${RED}Usage: $0 <cypescript-file> <output-name> [custom-cpp-files...]${NC}"
    echo -e "Example: $0 example.csc my_program"
    echo -e "Example: $0 example.csc my_program src/custom_math_lib.cpp src/my_lib.cpp"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_NAME="$2"
shift 2
CUSTOM_CPP_FILES=("$@")

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

echo -e "${BLUE}🚀 Compiling Cypescript with Custom C++ Integration: $INPUT_FILE${NC}"

# Step 1: Compile the standard C++ library
echo -e "${BLUE}📚 Compiling C++ standard library...${NC}"
if g++ -c src/cypescript_stdlib.cpp -o cypescript_stdlib.o -std=c++11; then
    echo -e "${GREEN}✓ C++ standard library compiled successfully${NC}"
else
    echo -e "${RED}❌ Failed to compile C++ standard library${NC}"
    exit 1
fi

# Step 2: Compile custom C++ libraries if provided
CUSTOM_OBJECTS=()
if [[ ${#CUSTOM_CPP_FILES[@]} -gt 0 ]]; then
    echo -e "${BLUE}🔧 Compiling custom C++ libraries...${NC}"
    for cpp_file in "${CUSTOM_CPP_FILES[@]}"; do
        if [[ ! -f "$cpp_file" ]]; then
            echo -e "${RED}❌ Custom C++ file not found: $cpp_file${NC}"
            exit 1
        fi
        
        object_file="${cpp_file%.cpp}.o"
        echo -e "${BLUE}   Compiling: $cpp_file${NC}"
        
        if g++ -c "$cpp_file" -o "$object_file" -std=c++11; then
            echo -e "${GREEN}   ✓ $cpp_file compiled successfully${NC}"
            CUSTOM_OBJECTS+=("$object_file")
        else
            echo -e "${RED}   ❌ Failed to compile $cpp_file${NC}"
            exit 1
        fi
    done
    echo -e "${GREEN}✓ All custom C++ libraries compiled${NC}"
fi

# Step 3: Compile Cypescript to LLVM IR
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
elif [[ -f "/opt/homebrew/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/bin/llc"
elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
elif [[ -f "/usr/local/bin/llc" ]]; then
    LLC_CMD="/usr/local/bin/llc"
else
    echo -e "${RED}❌ LLC not found. Please install LLVM.${NC}"
    exit 1
fi

# Step 4: Compile LLVM IR to object file
echo -e "${BLUE}🔧 Compiling LLVM IR to object file...${NC}"
if $LLC_CMD -filetype=obj -relocation-model=pic output.ll -o cypescript_program.o; then
    echo -e "${GREEN}✓ Object file created${NC}"
else
    echo -e "${RED}❌ Failed to compile LLVM IR${NC}"
    exit 1
fi

# Step 5: Link with C++ libraries
echo -e "${BLUE}🔗 Linking with C++ libraries...${NC}"
LINK_COMMAND="clang cypescript_program.o cypescript_stdlib.o"

# Add custom object files to link command
for obj_file in "${CUSTOM_OBJECTS[@]}"; do
    LINK_COMMAND="$LINK_COMMAND $obj_file"
done

LINK_COMMAND="$LINK_COMMAND -o $OUTPUT_NAME -lstdc++"

if eval $LINK_COMMAND; then
    echo -e "${GREEN}✓ Executable linked successfully${NC}"
else
    echo -e "${RED}❌ Failed to link executable${NC}"
    exit 1
fi

# Step 6: Cleanup intermediate files
echo -e "${BLUE}🧹 Cleaning up intermediate files...${NC}"
rm -f output.ll cypescript_program.o cypescript_stdlib.o
for obj_file in "${CUSTOM_OBJECTS[@]}"; do
    rm -f "$obj_file"
done

echo -e "${GREEN}🎉 Compilation complete!${NC}"
echo -e "${YELLOW}📍 Executable: ${NC}$OUTPUT_NAME"
echo -e "${YELLOW}🚀 Run with: ${NC}./$OUTPUT_NAME"

# Show summary of linked libraries
echo -e "${BLUE}📚 Linked libraries:${NC}"
echo -e "   • Cypescript standard library (cypescript_stdlib.cpp)"
if [[ ${#CUSTOM_CPP_FILES[@]} -gt 0 ]]; then
    for cpp_file in "${CUSTOM_CPP_FILES[@]}"; do
        echo -e "   • Custom library: $cpp_file"
    done
fi
