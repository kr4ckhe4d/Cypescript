#!/bin/bash

# Cypescript Build Script
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔨 Building Cypescript Compiler...${NC}"

# Check if LLVM is available
if ! command -v llvm-config &> /dev/null; then
    echo -e "${YELLOW}⚠️  llvm-config not found in PATH${NC}"
    
    # Try common Homebrew locations
    if [[ -f "/opt/homebrew/bin/llvm-config" ]]; then
        export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
        echo -e "${GREEN}✓ Found LLVM in Homebrew (Apple Silicon)${NC}"
    elif [[ -f "/usr/local/bin/llvm-config" ]]; then
        export PATH="/usr/local/opt/llvm/bin:$PATH"
        echo -e "${GREEN}✓ Found LLVM in Homebrew (Intel)${NC}"
    else
        echo -e "${RED}❌ LLVM not found. Please run ./setup-macos.sh first${NC}"
        exit 1
    fi
fi

# Show LLVM version
LLVM_VERSION=$(llvm-config --version 2>/dev/null || echo "unknown")
echo -e "${GREEN}✓ Using LLVM version: ${LLVM_VERSION}${NC}"

# Clean previous build
if [[ -d "build" ]]; then
    echo -e "${YELLOW}🧹 Cleaning previous build...${NC}"
    rm -rf build/
fi

# Configure with CMake
echo -e "${BLUE}⚙️  Configuring with CMake...${NC}"
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
echo -e "${BLUE}🔧 Compiling...${NC}"
cmake --build build --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Check if build was successful
if [[ -f "build/cscript" ]]; then
    echo -e "${GREEN}✅ Build successful!${NC}"
    echo -e "${GREEN}📍 Executable: build/cscript${NC}"
    
    # Show file size
    SIZE=$(ls -lh build/cscript | awk '{print $5}')
    echo -e "${BLUE}📊 Binary size: ${SIZE}${NC}"
    
    echo ""
    echo -e "${YELLOW}Next steps:${NC}"
    echo -e "  Test the compiler: ${BLUE}./test.sh${NC}"
    echo -e "  Run manually:      ${BLUE}./build/cscript example/hello.csc${NC}"
else
    echo -e "${RED}❌ Build failed!${NC}"
    exit 1
fi
