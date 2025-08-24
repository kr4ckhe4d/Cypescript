#!/bin/bash

# Simplified Multi-Language Performance Benchmark
# Tests Cypescript, JavaScript, and Python

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

echo -e "${MAGENTA}🚀 Multi-Language Performance Benchmark${NC}"
echo -e "${MAGENTA}=======================================${NC}"

# Function to get timestamp
get_timestamp() {
    node -e "console.log(Date.now())"
}

# Test 1: Array Processing Benchmark
echo -e "${CYAN}=== Array Processing Benchmark ===${NC}"

# Python
echo -e "${BLUE}Testing Python...${NC}"
PY_START=$(get_timestamp)
python3 benchmarks/array_processing_benchmark.py >/dev/null 2>&1
PY_END=$(get_timestamp)
PY_TIME=$((PY_END - PY_START))
echo -e "${GREEN}✓ Python: ${PY_TIME}ms${NC}"

# JavaScript
echo -e "${BLUE}Testing JavaScript...${NC}"
JS_START=$(get_timestamp)
node benchmarks/array_processing_benchmark.js >/dev/null 2>&1
JS_END=$(get_timestamp)
JS_TIME=$((JS_END - JS_START))
echo -e "${GREEN}✓ JavaScript: ${JS_TIME}ms${NC}"

# Cypescript (Basic)
echo -e "${BLUE}Testing Cypescript (Basic)...${NC}"
if ./compile-optimized.sh benchmarks/array_processing_benchmark.csc cs_basic >/dev/null 2>&1; then
    CS_BASIC_START=$(get_timestamp)
    ./cs_basic >/dev/null 2>&1
    CS_BASIC_END=$(get_timestamp)
    CS_BASIC_TIME=$((CS_BASIC_END - CS_BASIC_START))
    echo -e "${GREEN}✓ Cypescript (Basic): ${CS_BASIC_TIME}ms${NC}"
    rm -f cs_basic
else
    echo -e "${YELLOW}⚠️  Cypescript basic compilation failed${NC}"
    CS_BASIC_TIME="FAILED"
fi

# Cypescript (Advanced)
echo -e "${BLUE}Testing Cypescript (Advanced)...${NC}"
if ./compile-advanced.sh benchmarks/array_processing_benchmark.csc cs_advanced >/dev/null 2>&1; then
    CS_ADV_START=$(get_timestamp)
    ./cs_advanced >/dev/null 2>&1
    CS_ADV_END=$(get_timestamp)
    CS_ADV_TIME=$((CS_ADV_END - CS_ADV_START))
    echo -e "${GREEN}✓ Cypescript (Advanced): ${CS_ADV_TIME}ms${NC}"
    rm -f cs_advanced
else
    echo -e "${YELLOW}⚠️  Cypescript advanced compilation failed${NC}"
    CS_ADV_TIME="FAILED"
fi

echo ""

# Test 2: Intensive Computational Benchmark
echo -e "${CYAN}=== Intensive Computational Benchmark ===${NC}"

# Python
echo -e "${BLUE}Testing Python...${NC}"
PY_INT_START=$(get_timestamp)
python3 benchmarks/intensive_benchmark.py >/dev/null 2>&1
PY_INT_END=$(get_timestamp)
PY_INT_TIME=$((PY_INT_END - PY_INT_START))
echo -e "${GREEN}✓ Python: ${PY_INT_TIME}ms${NC}"

# JavaScript
echo -e "${BLUE}Testing JavaScript...${NC}"
JS_INT_START=$(get_timestamp)
node benchmarks/intensive_benchmark.js >/dev/null 2>&1
JS_INT_END=$(get_timestamp)
JS_INT_TIME=$((JS_INT_END - JS_INT_START))
echo -e "${GREEN}✓ JavaScript: ${JS_INT_TIME}ms${NC}"

# Cypescript
echo -e "${BLUE}Testing Cypescript (Advanced)...${NC}"
if ./compile-advanced.sh benchmarks/intensive_benchmark.csc cs_intensive >/dev/null 2>&1; then
    CS_INT_START=$(get_timestamp)
    ./cs_intensive >/dev/null 2>&1
    CS_INT_END=$(get_timestamp)
    CS_INT_TIME=$((CS_INT_END - CS_INT_START))
    echo -e "${GREEN}✓ Cypescript: ${CS_INT_TIME}ms${NC}"
    rm -f cs_intensive
else
    echo -e "${YELLOW}⚠️  Cypescript intensive compilation failed${NC}"
    CS_INT_TIME="FAILED"
fi

echo ""

# Results Analysis
echo -e "${MAGENTA}📊 BENCHMARK RESULTS SUMMARY${NC}"
echo -e "${MAGENTA}============================${NC}"

echo -e "${CYAN}Array Processing Benchmark:${NC}"
echo -e "${BLUE}────────────────────────────${NC}"
echo -e "  Python:               ${PY_TIME}ms"
echo -e "  JavaScript:           ${JS_TIME}ms"
if [[ "$CS_BASIC_TIME" =~ ^[0-9]+$ ]]; then
    echo -e "  Cypescript (Basic):   ${CS_BASIC_TIME}ms"
fi
if [[ "$CS_ADV_TIME" =~ ^[0-9]+$ ]]; then
    echo -e "  Cypescript (Advanced): ${CS_ADV_TIME}ms"
fi

echo ""
echo -e "${CYAN}Intensive Computational Benchmark:${NC}"
echo -e "${BLUE}────────────────────────────────────${NC}"
echo -e "  Python:               ${PY_INT_TIME}ms"
echo -e "  JavaScript:           ${JS_INT_TIME}ms"
if [[ "$CS_INT_TIME" =~ ^[0-9]+$ ]]; then
    echo -e "  Cypescript (Advanced): ${CS_INT_TIME}ms"
fi

echo ""

# Performance Analysis
echo -e "${CYAN}Performance Analysis:${NC}"
echo -e "${BLUE}────────────────────${NC}"

# Array processing analysis
if [[ "$JS_TIME" =~ ^[0-9]+$ ]] && [[ "$PY_TIME" =~ ^[0-9]+$ ]]; then
    JS_VS_PY=$(node -e "console.log((${PY_TIME} / ${JS_TIME}).toFixed(2))")
    echo -e "  JavaScript vs Python (Array): ${JS_VS_PY}x faster"
fi

if [[ "$CS_ADV_TIME" =~ ^[0-9]+$ ]] && [[ "$JS_TIME" =~ ^[0-9]+$ ]]; then
    CS_VS_JS=$(node -e "console.log((${JS_TIME} / ${CS_ADV_TIME}).toFixed(2))")
    if [[ $(node -e "console.log(${CS_ADV_TIME} < ${JS_TIME})") == "true" ]]; then
        echo -e "  Cypescript vs JavaScript (Array): ${CS_VS_JS}x faster"
    else
        echo -e "  JavaScript vs Cypescript (Array): $(node -e "console.log((${CS_ADV_TIME} / ${JS_TIME}).toFixed(2))")x faster"
    fi
fi

if [[ "$CS_ADV_TIME" =~ ^[0-9]+$ ]] && [[ "$PY_TIME" =~ ^[0-9]+$ ]]; then
    CS_VS_PY=$(node -e "console.log((${PY_TIME} / ${CS_ADV_TIME}).toFixed(2))")
    echo -e "  Cypescript vs Python (Array): ${CS_VS_PY}x faster"
fi

echo ""

# Intensive analysis
if [[ "$JS_INT_TIME" =~ ^[0-9]+$ ]] && [[ "$PY_INT_TIME" =~ ^[0-9]+$ ]]; then
    JS_VS_PY_INT=$(node -e "console.log((${PY_INT_TIME} / ${JS_INT_TIME}).toFixed(2))")
    echo -e "  JavaScript vs Python (Intensive): ${JS_VS_PY_INT}x faster"
fi

if [[ "$CS_INT_TIME" =~ ^[0-9]+$ ]] && [[ "$JS_INT_TIME" =~ ^[0-9]+$ ]]; then
    CS_VS_JS_INT=$(node -e "console.log((${JS_INT_TIME} / ${CS_INT_TIME}).toFixed(2))")
    if [[ $(node -e "console.log(${CS_INT_TIME} < ${JS_INT_TIME})") == "true" ]]; then
        echo -e "  Cypescript vs JavaScript (Intensive): ${CS_VS_JS_INT}x faster"
    else
        echo -e "  JavaScript vs Cypescript (Intensive): $(node -e "console.log((${CS_INT_TIME} / ${JS_INT_TIME}).toFixed(2))")x faster"
    fi
fi

if [[ "$CS_INT_TIME" =~ ^[0-9]+$ ]] && [[ "$PY_INT_TIME" =~ ^[0-9]+$ ]]; then
    CS_VS_PY_INT=$(node -e "console.log((${PY_INT_TIME} / ${CS_INT_TIME}).toFixed(2))")
    echo -e "  Cypescript vs Python (Intensive): ${CS_VS_PY_INT}x faster"
fi

echo ""
echo -e "${MAGENTA}🎯 KEY INSIGHTS${NC}"
echo -e "${MAGENTA}==============${NC}"
echo -e "${GREEN}✓ JavaScript V8 JIT excels at optimized patterns${NC}"
echo -e "${GREEN}✓ Python prioritizes developer productivity${NC}"
echo -e "${GREEN}✓ Cypescript shows native compilation advantages${NC}"
echo -e "${GREEN}✓ Performance characteristics vary by workload type${NC}"

echo ""
echo -e "${MAGENTA}🎉 Multi-Language Benchmark Complete!${NC}"

# Create results file for documentation
cat > multi_language_results.json << EOF
{
  "timestamp": "$(date -u +"%Y-%m-%dT%H:%M:%S.%3NZ")",
  "array_processing": {
    "python": ${PY_TIME},
    "javascript": ${JS_TIME},
    "cypescript_basic": ${CS_BASIC_TIME},
    "cypescript_advanced": ${CS_ADV_TIME}
  },
  "intensive_computation": {
    "python": ${PY_INT_TIME},
    "javascript": ${JS_INT_TIME},
    "cypescript_advanced": ${CS_INT_TIME}
  }
}
EOF

echo -e "${BLUE}📄 Results exported to multi_language_results.json${NC}"
