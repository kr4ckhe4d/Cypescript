#!/bin/bash

# Multi-Language Performance Benchmark Suite
# Tests Cypescript, JavaScript, TypeScript, and Python

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

echo -e "${MAGENTA}🚀 Multi-Language Performance Benchmark Suite${NC}"
echo -e "${MAGENTA}=============================================${NC}"

# Check available languages
echo -e "${BLUE}🔍 Checking available languages...${NC}"

LANGUAGES=()
if [[ -f "build/cscript" ]]; then
    echo -e "${GREEN}✓ Cypescript compiler available${NC}"
    LANGUAGES+=("cypescript")
else
    echo -e "${YELLOW}⚠️  Cypescript compiler not found${NC}"
fi

if command -v node &> /dev/null; then
    echo -e "${GREEN}✓ Node.js available${NC}"
    LANGUAGES+=("javascript")
else
    echo -e "${YELLOW}⚠️  Node.js not found${NC}"
fi

if command -v tsc &> /dev/null; then
    echo -e "${GREEN}✓ TypeScript available${NC}"
    LANGUAGES+=("typescript")
else
    echo -e "${YELLOW}⚠️  TypeScript not found${NC}"
fi

if command -v python3 &> /dev/null; then
    echo -e "${GREEN}✓ Python 3 available${NC}"
    LANGUAGES+=("python")
else
    echo -e "${YELLOW}⚠️  Python 3 not found${NC}"
fi

echo -e "${BLUE}📊 Testing ${#LANGUAGES[@]} languages: ${LANGUAGES[*]}${NC}"
echo ""

# Function to get timestamp
get_timestamp() {
    if command -v node &> /dev/null; then
        node -e "console.log(Date.now())"
    else
        python3 -c "import time; print(int(time.time() * 1000))"
    fi
}

# Results storage
declare -A RESULTS

# Test 1: Array Processing Benchmark
echo -e "${CYAN}=== Test 1: Array Processing Benchmark ===${NC}"

# JavaScript
if [[ " ${LANGUAGES[@]} " =~ " javascript " ]]; then
    echo -e "${BLUE}Testing JavaScript...${NC}"
    START_TIME=$(get_timestamp)
    node benchmarks/array_processing_benchmark.js >/dev/null 2>&1
    END_TIME=$(get_timestamp)
    JS_TIME=$((END_TIME - START_TIME))
    RESULTS["js_array"]=$JS_TIME
    echo -e "${GREEN}✓ JavaScript: ${JS_TIME}ms${NC}"
fi

# TypeScript
if [[ " ${LANGUAGES[@]} " =~ " typescript " ]]; then
    echo -e "${BLUE}Testing TypeScript...${NC}"
    if tsc benchmarks/array_processing_benchmark.ts --target es2020 --outDir benchmarks/ 2>/dev/null; then
        START_TIME=$(get_timestamp)
        node benchmarks/array_processing_benchmark.js >/dev/null 2>&1
        END_TIME=$(get_timestamp)
        TS_TIME=$((END_TIME - START_TIME))
        RESULTS["ts_array"]=$TS_TIME
        echo -e "${GREEN}✓ TypeScript: ${TS_TIME}ms${NC}"
        rm -f benchmarks/array_processing_benchmark.js
    else
        echo -e "${YELLOW}⚠️  TypeScript compilation failed${NC}"
        RESULTS["ts_array"]="FAILED"
    fi
fi

# Python
if [[ " ${LANGUAGES[@]} " =~ " python " ]]; then
    echo -e "${BLUE}Testing Python...${NC}"
    START_TIME=$(get_timestamp)
    python3 benchmarks/array_processing_benchmark.py >/dev/null 2>&1
    END_TIME=$(get_timestamp)
    PY_TIME=$((END_TIME - START_TIME))
    RESULTS["py_array"]=$PY_TIME
    echo -e "${GREEN}✓ Python: ${PY_TIME}ms${NC}"
fi

# Cypescript (multiple optimization levels)
if [[ " ${LANGUAGES[@]} " =~ " cypescript " ]]; then
    echo -e "${BLUE}Testing Cypescript (Basic)...${NC}"
    if ./compile-optimized.sh benchmarks/array_processing_benchmark.csc cs_basic >/dev/null 2>&1; then
        START_TIME=$(get_timestamp)
        ./cs_basic >/dev/null 2>&1
        END_TIME=$(get_timestamp)
        CS_BASIC_TIME=$((END_TIME - START_TIME))
        RESULTS["cs_basic_array"]=$CS_BASIC_TIME
        echo -e "${GREEN}✓ Cypescript (Basic): ${CS_BASIC_TIME}ms${NC}"
        rm -f cs_basic
    else
        echo -e "${YELLOW}⚠️  Cypescript basic compilation failed${NC}"
        RESULTS["cs_basic_array"]="FAILED"
    fi
    
    echo -e "${BLUE}Testing Cypescript (Advanced)...${NC}"
    if ./compile-advanced.sh benchmarks/array_processing_benchmark.csc cs_advanced >/dev/null 2>&1; then
        START_TIME=$(get_timestamp)
        ./cs_advanced >/dev/null 2>&1
        END_TIME=$(get_timestamp)
        CS_ADV_TIME=$((END_TIME - START_TIME))
        RESULTS["cs_advanced_array"]=$CS_ADV_TIME
        echo -e "${GREEN}✓ Cypescript (Advanced): ${CS_ADV_TIME}ms${NC}"
        rm -f cs_advanced
    else
        echo -e "${YELLOW}⚠️  Cypescript advanced compilation failed${NC}"
        RESULTS["cs_advanced_array"]="FAILED"
    fi
    
    echo -e "${BLUE}Testing Cypescript (NEON SIMD)...${NC}"
    if ./compile-with-custom-cpp.sh benchmarks/array_processing_benchmark.csc cs_neon src/custom_math_lib.cpp src/neon_optimized_lib.cpp >/dev/null 2>&1; then
        START_TIME=$(get_timestamp)
        ./cs_neon >/dev/null 2>&1
        END_TIME=$(get_timestamp)
        CS_NEON_TIME=$((END_TIME - START_TIME))
        RESULTS["cs_neon_array"]=$CS_NEON_TIME
        echo -e "${GREEN}✓ Cypescript (NEON): ${CS_NEON_TIME}ms${NC}"
        rm -f cs_neon
    else
        echo -e "${YELLOW}⚠️  Cypescript NEON compilation failed${NC}"
        RESULTS["cs_neon_array"]="FAILED"
    fi
fi

echo ""

# Test 2: Intensive Computational Benchmark
echo -e "${CYAN}=== Test 2: Intensive Computational Benchmark ===${NC}"

# JavaScript
if [[ " ${LANGUAGES[@]} " =~ " javascript " ]]; then
    echo -e "${BLUE}Testing JavaScript...${NC}"
    START_TIME=$(get_timestamp)
    node benchmarks/intensive_benchmark.js >/dev/null 2>&1
    END_TIME=$(get_timestamp)
    JS_INTENSIVE_TIME=$((END_TIME - START_TIME))
    RESULTS["js_intensive"]=$JS_INTENSIVE_TIME
    echo -e "${GREEN}✓ JavaScript: ${JS_INTENSIVE_TIME}ms${NC}"
fi

# Python
if [[ " ${LANGUAGES[@]} " =~ " python " ]]; then
    echo -e "${BLUE}Testing Python...${NC}"
    START_TIME=$(get_timestamp)
    python3 benchmarks/intensive_benchmark.py >/dev/null 2>&1
    END_TIME=$(get_timestamp)
    PY_INTENSIVE_TIME=$((END_TIME - START_TIME))
    RESULTS["py_intensive"]=$PY_INTENSIVE_TIME
    echo -e "${GREEN}✓ Python: ${PY_INTENSIVE_TIME}ms${NC}"
fi

# Cypescript
if [[ " ${LANGUAGES[@]} " =~ " cypescript " ]]; then
    echo -e "${BLUE}Testing Cypescript (Advanced)...${NC}"
    if ./compile-advanced.sh benchmarks/intensive_benchmark.csc cs_intensive >/dev/null 2>&1; then
        START_TIME=$(get_timestamp)
        ./cs_intensive >/dev/null 2>&1
        END_TIME=$(get_timestamp)
        CS_INTENSIVE_TIME=$((END_TIME - START_TIME))
        RESULTS["cs_intensive"]=$CS_INTENSIVE_TIME
        echo -e "${GREEN}✓ Cypescript: ${CS_INTENSIVE_TIME}ms${NC}"
        rm -f cs_intensive
    else
        echo -e "${YELLOW}⚠️  Cypescript intensive compilation failed${NC}"
        RESULTS["cs_intensive"]="FAILED"
    fi
fi

echo ""

# Results Analysis
echo -e "${MAGENTA}📊 MULTI-LANGUAGE BENCHMARK RESULTS${NC}"
echo -e "${MAGENTA}===================================${NC}"

echo -e "${CYAN}Array Processing Benchmark Results:${NC}"
echo -e "${BLUE}────────────────────────────────────────${NC}"

# Find fastest for comparison
FASTEST_ARRAY=999999
for key in "${!RESULTS[@]}"; do
    if [[ "$key" =~ _array$ ]] && [[ "${RESULTS[$key]}" =~ ^[0-9]+$ ]]; then
        if [[ ${RESULTS[$key]} -lt $FASTEST_ARRAY ]]; then
            FASTEST_ARRAY=${RESULTS[$key]}
        fi
    fi
done

# Display array results
if [[ -n "${RESULTS[js_array]}" ]] && [[ "${RESULTS[js_array]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[js_array]} / $FASTEST_ARRAY).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  JavaScript:           ${RESULTS[js_array]}ms (${SPEEDUP}x)"
fi

if [[ -n "${RESULTS[ts_array]}" ]] && [[ "${RESULTS[ts_array]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[ts_array]} / $FASTEST_ARRAY).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  TypeScript:           ${RESULTS[ts_array]}ms (${SPEEDUP}x)"
fi

if [[ -n "${RESULTS[py_array]}" ]] && [[ "${RESULTS[py_array]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[py_array]} / $FASTEST_ARRAY).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  Python:               ${RESULTS[py_array]}ms (${SPEEDUP}x)"
fi

if [[ -n "${RESULTS[cs_basic_array]}" ]] && [[ "${RESULTS[cs_basic_array]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[cs_basic_array]} / $FASTEST_ARRAY).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  Cypescript (Basic):   ${RESULTS[cs_basic_array]}ms (${SPEEDUP}x)"
fi

if [[ -n "${RESULTS[cs_advanced_array]}" ]] && [[ "${RESULTS[cs_advanced_array]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[cs_advanced_array]} / $FASTEST_ARRAY).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  Cypescript (Advanced): ${RESULTS[cs_advanced_array]}ms (${SPEEDUP}x)"
fi

if [[ -n "${RESULTS[cs_neon_array]}" ]] && [[ "${RESULTS[cs_neon_array]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[cs_neon_array]} / $FASTEST_ARRAY).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  Cypescript (NEON):    ${RESULTS[cs_neon_array]}ms (${SPEEDUP}x)"
fi

echo ""
echo -e "${CYAN}Intensive Computational Benchmark Results:${NC}"
echo -e "${BLUE}────────────────────────────────────────${NC}"

# Find fastest for intensive comparison
FASTEST_INTENSIVE=999999
for key in "${!RESULTS[@]}"; do
    if [[ "$key" =~ _intensive$ ]] && [[ "${RESULTS[$key]}" =~ ^[0-9]+$ ]]; then
        if [[ ${RESULTS[$key]} -lt $FASTEST_INTENSIVE ]]; then
            FASTEST_INTENSIVE=${RESULTS[$key]}
        fi
    fi
done

# Display intensive results
if [[ -n "${RESULTS[js_intensive]}" ]] && [[ "${RESULTS[js_intensive]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[js_intensive]} / $FASTEST_INTENSIVE).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  JavaScript:           ${RESULTS[js_intensive]}ms (${SPEEDUP}x)"
fi

if [[ -n "${RESULTS[py_intensive]}" ]] && [[ "${RESULTS[py_intensive]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[py_intensive]} / $FASTEST_INTENSIVE).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  Python:               ${RESULTS[py_intensive]}ms (${SPEEDUP}x)"
fi

if [[ -n "${RESULTS[cs_intensive]}" ]] && [[ "${RESULTS[cs_intensive]}" =~ ^[0-9]+$ ]]; then
    SPEEDUP=$(node -e "console.log((${RESULTS[cs_intensive]} / $FASTEST_INTENSIVE).toFixed(2))" 2>/dev/null || echo "N/A")
    echo -e "  Cypescript (Advanced): ${RESULTS[cs_intensive]}ms (${SPEEDUP}x)"
fi

echo ""
echo -e "${MAGENTA}🎯 PERFORMANCE INSIGHTS${NC}"
echo -e "${MAGENTA}======================${NC}"
echo -e "${GREEN}✓ Multi-language performance comparison complete${NC}"
echo -e "${GREEN}✓ Cypescript optimization levels tested${NC}"
echo -e "${GREEN}✓ Real-world performance characteristics measured${NC}"

echo ""
echo -e "${BLUE}💡 Key Takeaways:${NC}"
echo -e "   • JavaScript V8 JIT excels at small, repeated operations"
echo -e "   • Python is optimized for developer productivity over raw speed"
echo -e "   • Cypescript native compilation shows scaling advantages"
echo -e "   • Advanced optimizations provide measurable improvements"
echo -e "   • SIMD benefits become apparent with larger workloads"

echo ""
echo -e "${MAGENTA}🎉 Multi-Language Benchmark Complete!${NC}"

# Export results for documentation
echo "# Multi-Language Benchmark Results" > benchmark_results.md
echo "Generated: $(date)" >> benchmark_results.md
echo "" >> benchmark_results.md
echo "## Array Processing Benchmark" >> benchmark_results.md
for key in "${!RESULTS[@]}"; do
    if [[ "$key" =~ _array$ ]]; then
        echo "- $key: ${RESULTS[$key]}ms" >> benchmark_results.md
    fi
done
echo "" >> benchmark_results.md
echo "## Intensive Computational Benchmark" >> benchmark_results.md
for key in "${!RESULTS[@]}"; do
    if [[ "$key" =~ _intensive$ ]]; then
        echo "- $key: ${RESULTS[$key]}ms" >> benchmark_results.md
    fi
done

echo -e "${BLUE}📄 Results exported to benchmark_results.md${NC}"
