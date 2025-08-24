#!/bin/bash

# Cypescript vs JavaScript/TypeScript Benchmark Runner

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}🚀 Cypescript vs JavaScript/TypeScript Benchmark Suite${NC}"
echo -e "${CYAN}====================================================${NC}"

# Check if we're in the right directory
if [[ ! -f "../build/cscript" ]]; then
    echo -e "${RED}❌ Cypescript compiler not found. Please run ./build.sh first${NC}"
    exit 1
fi

# Check if Node.js is available
if ! command -v node &> /dev/null; then
    echo -e "${RED}❌ Node.js not found. Please install Node.js to run JavaScript benchmarks${NC}"
    exit 1
fi

# Check if TypeScript is available
if ! command -v tsc &> /dev/null; then
    echo -e "${YELLOW}⚠️  TypeScript compiler not found. Installing locally...${NC}"
    npm install -g typescript 2>/dev/null || echo -e "${YELLOW}⚠️  Could not install TypeScript globally. Skipping TypeScript benchmark.${NC}"
fi

echo ""
echo -e "${BLUE}📊 Running Benchmarks...${NC}"
echo ""

# Benchmark 1: Cypescript (Native Compilation)
echo -e "${GREEN}1. Cypescript (Native LLVM Compilation)${NC}"
echo "Compiling Cypescript benchmark..."

# Compile Cypescript benchmark
if ../build/cscript array_processing_benchmark.csc -o cypescript_benchmark.ll; then
    echo "✓ Cypescript compilation successful"
else
    echo -e "${RED}❌ Cypescript compilation failed${NC}"
    exit 1
fi

# Find LLC and compile to native
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
    echo -e "${RED}❌ LLC not found. Please install LLVM${NC}"
    exit 1
fi

# Compile to native
if $LLC_CMD -filetype=obj -relocation-model=pic cypescript_benchmark.ll -o cypescript_benchmark.o; then
    echo "✓ LLVM IR compiled to object file"
else
    echo -e "${RED}❌ LLVM compilation failed${NC}"
    exit 1
fi

# Link executable
if clang cypescript_benchmark.o -o cypescript_benchmark; then
    echo "✓ Native executable created"
else
    echo -e "${RED}❌ Linking failed${NC}"
    exit 1
fi

echo "Running Cypescript benchmark..."
echo "----------------------------------------"
CYPESCRIPT_START=$(node -e "console.log(Date.now())")
./cypescript_benchmark
CYPESCRIPT_END=$(node -e "console.log(Date.now())")
CYPESCRIPT_TIME=$((CYPESCRIPT_END - CYPESCRIPT_START))
echo "----------------------------------------"
echo -e "${GREEN}Cypescript Total Time: ${CYPESCRIPT_TIME}ms${NC}"
echo ""

# Benchmark 2: JavaScript (Node.js)
echo -e "${GREEN}2. JavaScript (Node.js V8 Engine)${NC}"
echo "Running JavaScript benchmark..."
echo "----------------------------------------"
JS_START=$(node -e "console.log(Date.now())")
node array_processing_benchmark.js
JS_END=$(node -e "console.log(Date.now())")
JS_TIME=$((JS_END - JS_START))
echo "----------------------------------------"
echo -e "${GREEN}JavaScript Total Time: ${JS_TIME}ms${NC}"
echo ""

# Benchmark 3: TypeScript (if available)
if command -v tsc &> /dev/null; then
    echo -e "${GREEN}3. TypeScript (Compiled to JavaScript)${NC}"
    echo "Compiling TypeScript benchmark..."
    
    if tsc array_processing_benchmark.ts --target es2020 --outDir . 2>/dev/null; then
        echo "✓ TypeScript compilation successful"
        echo "Running TypeScript benchmark..."
        echo "----------------------------------------"
        TS_START=$(node -e "console.log(Date.now())")
        node array_processing_benchmark.js
        TS_END=$(node -e "console.log(Date.now())")
        TS_TIME=$((TS_END - TS_START))
        echo "----------------------------------------"
        echo -e "${GREEN}TypeScript Total Time: ${TS_TIME}ms${NC}"
    else
        echo -e "${YELLOW}⚠️  TypeScript compilation failed, skipping...${NC}"
        TS_TIME="N/A"
    fi
else
    echo -e "${YELLOW}⚠️  TypeScript not available, skipping...${NC}"
    TS_TIME="N/A"
fi

echo ""
echo -e "${CYAN}📈 BENCHMARK RESULTS${NC}"
echo -e "${CYAN}===================${NC}"
echo ""
echo -e "${BLUE}Language/Runtime${NC}                ${BLUE}Total Time${NC}"
echo "----------------------------------------"
echo -e "${GREEN}Cypescript (Native LLVM)${NC}        ${CYPESCRIPT_TIME}ms"
echo -e "${GREEN}JavaScript (Node.js V8)${NC}         ${JS_TIME}ms"
if [[ "$TS_TIME" != "N/A" ]]; then
    echo -e "${GREEN}TypeScript → JavaScript${NC}         ${TS_TIME}ms"
fi
echo ""

# Calculate performance ratios
if [[ "$TS_TIME" != "N/A" ]]; then
    SPEEDUP_VS_JS=$(node -e "console.log((${JS_TIME} / ${CYPESCRIPT_TIME}).toFixed(2))")
    SPEEDUP_VS_TS=$(node -e "console.log((${TS_TIME} / ${CYPESCRIPT_TIME}).toFixed(2))")
    
    echo -e "${CYAN}🏆 PERFORMANCE ANALYSIS${NC}"
    echo -e "${CYAN}======================${NC}"
    echo -e "${GREEN}Cypescript is ${SPEEDUP_VS_JS}x faster than JavaScript${NC}"
    echo -e "${GREEN}Cypescript is ${SPEEDUP_VS_TS}x faster than TypeScript${NC}"
else
    SPEEDUP_VS_JS=$(node -e "console.log((${JS_TIME} / ${CYPESCRIPT_TIME}).toFixed(2))")
    
    echo -e "${CYAN}🏆 PERFORMANCE ANALYSIS${NC}"
    echo -e "${CYAN}======================${NC}"
    echo -e "${GREEN}Cypescript is ${SPEEDUP_VS_JS}x faster than JavaScript${NC}"
fi

echo ""
echo -e "${BLUE}💡 Notes:${NC}"
echo "• Cypescript compiles to native machine code via LLVM"
echo "• JavaScript runs on Node.js V8 JIT compiler"
echo "• TypeScript compiles to JavaScript (same runtime performance)"
echo "• Results may vary based on system and optimization levels"
echo ""

# Cleanup
echo -e "${BLUE}🧹 Cleaning up...${NC}"
rm -f cypescript_benchmark.ll cypescript_benchmark.o cypescript_benchmark
rm -f array_processing_benchmark.js  # Remove compiled TypeScript output
echo "✓ Cleanup complete"

echo -e "${CYAN}🎉 Benchmark Complete!${NC}"
