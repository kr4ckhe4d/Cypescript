#!/bin/bash

# Cypescript Test Script
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}🧪 Testing Cypescript Compiler...${NC}"

# Check if compiler exists
if [[ ! -f "build/cscript" ]]; then
    echo -e "${RED}❌ Compiler not found. Run ./build.sh first${NC}"
    exit 1
fi

# Check if example file exists
if [[ ! -f "example/hello.csc" ]]; then
    echo -e "${YELLOW}⚠️  Example file not found, creating one...${NC}"
    mkdir -p example
    cat > example/hello.csc << 'EOF'
let message: string = "Hello, Cypescript!";
print(message);

let number: i32 = 42;
print(number);

print("Direct string test");
print(123);
EOF
    echo -e "${GREEN}✓ Created example/hello.csc${NC}"
fi

echo -e "${CYAN}📄 Source code:${NC}"
echo "----------------------------------------"
cat example/hello.csc
echo "----------------------------------------"
echo ""

# Test 1: Basic compilation
echo -e "${BLUE}🔍 Test 1: Basic Compilation${NC}"
if ./build/cscript example/hello.csc; then
    echo -e "${GREEN}✅ Compilation successful${NC}"
else
    echo -e "${RED}❌ Compilation failed${NC}"
    exit 1
fi

# Check if LLVM IR was generated
if [[ -f "output.ll" ]]; then
    echo -e "${GREEN}✓ LLVM IR generated (output.ll)${NC}"
    SIZE=$(ls -lh output.ll | awk '{print $5}')
    echo -e "${BLUE}📊 IR file size: ${SIZE}${NC}"
else
    echo -e "${RED}❌ LLVM IR not generated${NC}"
    exit 1
fi

echo ""

# Test 2: Verbose compilation
echo -e "${BLUE}🔍 Test 2: Verbose Compilation${NC}"
./build/cscript -v --print-tokens --print-ast example/hello.csc -o test_output.ll

echo ""

# Test 3: Compile to executable (if llc is available)
echo -e "${BLUE}🔍 Test 3: Full Compilation Pipeline${NC}"

# Check for llc
LLC_CMD=""
if command -v llc &> /dev/null; then
    LLC_CMD="llc"
elif command -v llc-16 &> /dev/null; then
    LLC_CMD="llc-16"
elif command -v llc-17 &> /dev/null; then
    LLC_CMD="llc-17"
elif command -v llc-18 &> /dev/null; then
    LLC_CMD="llc-18"
elif [[ -f "/opt/homebrew/bin/llc" ]]; then
    LLC_CMD="/opt/homebrew/bin/llc"
elif [[ -f "/usr/local/bin/llc" ]]; then
    LLC_CMD="/usr/local/bin/llc"
fi

if [[ -n "$LLC_CMD" ]]; then
    echo -e "${GREEN}✓ Found LLC: $LLC_CMD${NC}"
    
    # Compile LLVM IR to object file
    echo -e "${CYAN}🔧 Compiling IR to object file...${NC}"
    $LLC_CMD -filetype=obj -relocation-model=pic output.ll -o output.o
    
    if [[ -f "output.o" ]]; then
        echo -e "${GREEN}✓ Object file generated${NC}"
        
        # Link to executable
        echo -e "${CYAN}🔗 Linking executable...${NC}"
        clang output.o -o my_program
        
        if [[ -f "my_program" ]]; then
            echo -e "${GREEN}✅ Executable generated successfully!${NC}"
            
            # Run the program
            echo -e "${CYAN}🚀 Running the program:${NC}"
            echo "----------------------------------------"
            ./my_program
            echo "----------------------------------------"
            echo -e "${GREEN}✅ Program executed successfully!${NC}"
        else
            echo -e "${RED}❌ Failed to create executable${NC}"
        fi
    else
        echo -e "${RED}❌ Failed to create object file${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  LLC not found, skipping executable generation${NC}"
    echo -e "${BLUE}💡 To complete the pipeline manually:${NC}"
    echo -e "   1. llc -filetype=obj -relocation-model=pic output.ll -o output.o"
    echo -e "   2. clang output.o -o my_program"
    echo -e "   3. ./my_program"
fi

echo ""

# Test 4: Error handling
echo -e "${BLUE}🔍 Test 4: Error Handling${NC}"
echo 'let invalid syntax here' > example/error_test.csc
echo -e "${CYAN}Testing with invalid syntax...${NC}"
if ./build/cscript example/error_test.csc 2>/dev/null; then
    echo -e "${YELLOW}⚠️  Expected compilation to fail${NC}"
else
    echo -e "${GREEN}✅ Error handling works correctly${NC}"
fi
rm -f example/error_test.csc

echo ""
echo -e "${GREEN}🎉 All tests completed!${NC}"

# Show generated files
echo -e "${BLUE}📁 Generated files:${NC}"
ls -la *.ll *.o my_program 2>/dev/null || echo "No generated files found"

echo ""
echo -e "${YELLOW}💡 Tips:${NC}"
echo -e "  • Use ${CYAN}./build/cscript --help${NC} for all options"
echo -e "  • Edit ${CYAN}example/hello.csc${NC} to test different features"
echo -e "  • Check ${CYAN}output.ll${NC} to see generated LLVM IR"
