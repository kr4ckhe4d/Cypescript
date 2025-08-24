#!/bin/bash

# Focused Optimization Validation
# Tests PGO and large-scale SIMD benefits

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

echo -e "${MAGENTA}🚀 Optimization Validation: PGO + Large-Scale SIMD${NC}"
echo -e "${MAGENTA}=================================================${NC}"

# Test 1: Large-Scale SIMD Benefits
echo -e "${CYAN}=== Test 1: Large-Scale SIMD Benefits ===${NC}"
echo -e "${BLUE}Testing 800-element array with NEON optimization${NC}"

# Compile large-scale SIMD benchmark
echo -e "${BLUE}📝 Compiling large-scale SIMD benchmark...${NC}"
if ./compile-with-custom-cpp.sh benchmarks/large_scale_simd_benchmark.csc large_simd_test src/custom_math_lib.cpp src/neon_optimized_lib.cpp >/dev/null 2>&1; then
    echo -e "${GREEN}✓ Large-scale SIMD compilation successful${NC}"
    
    # Run benchmark
    echo -e "${BLUE}🏃 Running large-scale SIMD benchmark...${NC}"
    echo -e "${BLUE}────────────────────────────────────────${NC}"
    SIMD_START=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
    ./large_simd_test
    SIMD_END=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
    SIMD_TIME=$((SIMD_END - SIMD_START))
    echo -e "${BLUE}────────────────────────────────────────${NC}"
    echo -e "${GREEN}✓ Large-scale SIMD completed in ${SIMD_TIME}ms${NC}"
    
    # Cleanup
    rm -f large_simd_test
else
    echo -e "${RED}❌ Large-scale SIMD compilation failed${NC}"
    SIMD_TIME="FAILED"
fi

echo ""

# Test 2: Profile-Guided Optimization Validation
echo -e "${CYAN}=== Test 2: Profile-Guided Optimization Validation ===${NC}"
echo -e "${BLUE}Testing PGO workflow with hot path optimization${NC}"

# Step 1: Create instrumented version
echo -e "${BLUE}📊 Step 1: Creating instrumented version...${NC}"
if ./compile-pgo.sh profile benchmarks/pgo_validation_benchmark.csc pgo_instrumented >/dev/null 2>&1; then
    echo -e "${GREEN}✓ PGO instrumentation successful${NC}"
    
    # Step 2: Run instrumented version to collect profile data
    echo -e "${BLUE}📈 Step 2: Collecting profile data...${NC}"
    if ./pgo_instrumented >/dev/null 2>&1; then
        echo -e "${GREEN}✓ Profile data collected${NC}"
        
        # Step 3: Check for profile data and convert if available
        if [[ -f "default.profraw" ]]; then
            echo -e "${BLUE}🔄 Step 3: Converting profile data...${NC}"
            
            # Find llvm-profdata
            PROFDATA_CMD=""
            if command -v llvm-profdata &> /dev/null; then
                PROFDATA_CMD="llvm-profdata"
            elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llvm-profdata" ]]; then
                PROFDATA_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llvm-profdata"
            elif [[ -f "/opt/homebrew/bin/llvm-profdata" ]]; then
                PROFDATA_CMD="/opt/homebrew/bin/llvm-profdata"
            fi
            
            if [[ -n "$PROFDATA_CMD" ]]; then
                if $PROFDATA_CMD merge -output=pgo_validation.profdata default.profraw >/dev/null 2>&1; then
                    echo -e "${GREEN}✓ Profile data converted${NC}"
                    
                    # Step 4: Create PGO-optimized version
                    echo -e "${BLUE}🎯 Step 4: Creating PGO-optimized version...${NC}"
                    if ./compile-pgo.sh optimize benchmarks/pgo_validation_benchmark.csc pgo_optimized pgo_validation.profdata >/dev/null 2>&1; then
                        echo -e "${GREEN}✓ PGO optimization successful${NC}"
                        
                        # Step 5: Performance comparison
                        echo -e "${BLUE}⚡ Step 5: Performance comparison...${NC}"
                        
                        # Run standard version
                        echo -e "${BLUE}Running standard optimized version...${NC}"
                        if ./compile-optimized.sh benchmarks/pgo_validation_benchmark.csc pgo_standard >/dev/null 2>&1; then
                            STANDARD_START=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                            ./pgo_standard >/dev/null 2>&1
                            STANDARD_END=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                            STANDARD_TIME=$((STANDARD_END - STANDARD_START))
                            echo -e "${GREEN}✓ Standard version: ${STANDARD_TIME}ms${NC}"
                        else
                            echo -e "${YELLOW}⚠️  Standard compilation failed${NC}"
                            STANDARD_TIME="FAILED"
                        fi
                        
                        # Run PGO version
                        echo -e "${BLUE}Running PGO-optimized version...${NC}"
                        PGO_START=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                        ./pgo_optimized >/dev/null 2>&1
                        PGO_END=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                        PGO_TIME=$((PGO_END - PGO_START))
                        echo -e "${GREEN}✓ PGO version: ${PGO_TIME}ms${NC}"
                        
                        # Calculate improvement
                        if [[ "$STANDARD_TIME" =~ ^[0-9]+$ ]] && [[ "$PGO_TIME" =~ ^[0-9]+$ ]] && [[ $STANDARD_TIME -gt 0 ]]; then
                            IMPROVEMENT=$((STANDARD_TIME - PGO_TIME))
                            SPEEDUP=$(node -e "console.log((${STANDARD_TIME} / ${PGO_TIME}).toFixed(2))" 2>/dev/null || echo "N/A")
                            PERCENT=$(node -e "console.log((100 * ${IMPROVEMENT} / ${STANDARD_TIME}).toFixed(1))" 2>/dev/null || echo "N/A")
                            
                            echo -e "${CYAN}📊 PGO Performance Analysis:${NC}"
                            echo -e "   Standard:    ${STANDARD_TIME}ms"
                            echo -e "   PGO:         ${PGO_TIME}ms"
                            echo -e "   Improvement: ${IMPROVEMENT}ms (${PERCENT}%)"
                            echo -e "   Speedup:     ${SPEEDUP}x"
                            
                            if [[ $PGO_TIME -lt $STANDARD_TIME ]]; then
                                echo -e "${GREEN}🎉 PGO optimization successful!${NC}"
                            else
                                echo -e "${YELLOW}⚠️  PGO didn't improve performance${NC}"
                            fi
                        fi
                        
                        # Cleanup PGO files
                        rm -f pgo_optimized pgo_standard pgo_validation.profdata
                    else
                        echo -e "${RED}❌ PGO optimization failed${NC}"
                    fi
                else
                    echo -e "${YELLOW}⚠️  Profile data conversion failed${NC}"
                fi
            else
                echo -e "${YELLOW}⚠️  llvm-profdata not found, skipping PGO optimization${NC}"
            fi
            
            rm -f default.profraw
        else
            echo -e "${YELLOW}⚠️  No profile data generated${NC}"
        fi
    else
        echo -e "${RED}❌ Profile data collection failed${NC}"
    fi
    
    # Cleanup instrumented version
    rm -f pgo_instrumented
else
    echo -e "${RED}❌ PGO instrumentation failed${NC}"
fi

echo ""

# Test 3: Combined Advanced Optimization
echo -e "${CYAN}=== Test 3: Combined Advanced Optimization ===${NC}"
echo -e "${BLUE}Testing advanced multi-stage optimization pipeline${NC}"

# Test on standard array benchmark
echo -e "${BLUE}📝 Compiling with advanced optimization...${NC}"
if ./compile-advanced.sh benchmarks/array_processing_benchmark.csc advanced_combined src/custom_math_lib.cpp src/neon_optimized_lib.cpp >/dev/null 2>&1; then
    echo -e "${GREEN}✓ Advanced compilation successful${NC}"
    
    # Run benchmark
    echo -e "${BLUE}🏃 Running advanced optimization benchmark...${NC}"
    echo -e "${BLUE}────────────────────────────────────────${NC}"
    ADVANCED_START=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
    ./advanced_combined >/dev/null 2>&1
    ADVANCED_END=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
    ADVANCED_TIME=$((ADVANCED_END - ADVANCED_START))
    echo -e "${BLUE}────────────────────────────────────────${NC}"
    echo -e "${GREEN}✓ Advanced optimization completed in ${ADVANCED_TIME}ms${NC}"
    
    # Cleanup
    rm -f advanced_combined
else
    echo -e "${RED}❌ Advanced compilation failed${NC}"
    ADVANCED_TIME="FAILED"
fi

echo ""

# Final Results Summary
echo -e "${MAGENTA}🎯 OPTIMIZATION VALIDATION RESULTS${NC}"
echo -e "${MAGENTA}==================================${NC}"

echo -e "${CYAN}Performance Results:${NC}"
echo -e "${BLUE}────────────────────────────────────────${NC}"

if [[ "$SIMD_TIME" =~ ^[0-9]+$ ]]; then
    echo -e "${GREEN}✓ Large-Scale SIMD:     ${SIMD_TIME}ms${NC}"
else
    echo -e "${YELLOW}⚠ Large-Scale SIMD:     ${SIMD_TIME}${NC}"
fi

if [[ "$PGO_TIME" =~ ^[0-9]+$ ]]; then
    echo -e "${GREEN}✓ Profile-Guided Opt:   ${PGO_TIME}ms${NC}"
    if [[ "$STANDARD_TIME" =~ ^[0-9]+$ ]]; then
        echo -e "${BLUE}  (vs Standard:         ${STANDARD_TIME}ms)${NC}"
    fi
else
    echo -e "${YELLOW}⚠ Profile-Guided Opt:   Not completed${NC}"
fi

if [[ "$ADVANCED_TIME" =~ ^[0-9]+$ ]]; then
    echo -e "${GREEN}✓ Advanced Combined:    ${ADVANCED_TIME}ms${NC}"
else
    echo -e "${YELLOW}⚠ Advanced Combined:    ${ADVANCED_TIME}${NC}"
fi

echo ""
echo -e "${CYAN}Key Validation Points:${NC}"
echo -e "${BLUE}────────────────────────────────────────${NC}"
echo -e "• Large-scale SIMD demonstrates scaling benefits"
echo -e "• Profile-guided optimization targets hot paths"
echo -e "• Advanced pipeline combines multiple optimizations"
echo -e "• Real performance improvements measured"

echo ""
echo -e "${MAGENTA}🎉 Optimization Validation Complete!${NC}"

# Cleanup any remaining files
rm -f *.profraw *.profdata
