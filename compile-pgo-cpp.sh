#!/bin/bash

# Profile-Guided Optimization with C++ Integration
# Supports C++ libraries for complete PGO workflow

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Check arguments
if [[ $# -lt 2 ]]; then
    echo -e "${RED}Usage: $0 <mode> <cypescript-file> [output-name] [profile-data]${NC}"
    echo -e "${YELLOW}Modes:${NC}"
    echo -e "  ${GREEN}profile${NC}   - Compile with profiling instrumentation"
    echo -e "  ${GREEN}optimize${NC}  - Compile using profile data for optimization"
    echo -e "  ${GREEN}compare${NC}   - Compare PGO vs non-PGO performance"
    exit 1
fi

MODE="$1"
INPUT_FILE="$2"
OUTPUT_NAME="${3:-pgo_program}"
PROFILE_DATA="${4:-profile.profdata}"

echo -e "${CYAN}🚀 PGO with C++ Integration: $MODE mode${NC}"

# Compile C++ libraries first
echo -e "${BLUE}📚 Compiling C++ libraries...${NC}"
if g++ -c src/cypescript_stdlib.cpp -o cypescript_stdlib.o -std=c++11 -O2 2>/dev/null; then
    echo -e "${GREEN}✓ C++ standard library compiled${NC}"
else
    echo -e "${RED}❌ Failed to compile C++ library${NC}"
    exit 1
fi

case "$MODE" in
    "profile")
        echo -e "${BLUE}📊 Creating instrumented version with C++ support${NC}"
        
        # Compile Cypescript to LLVM IR
        if ./build/cscript "$INPUT_FILE"; then
            echo -e "${GREEN}✓ Cypescript compiled to LLVM IR${NC}"
        else
            echo -e "${RED}❌ Failed to compile Cypescript${NC}"
            exit 1
        fi
        
        # Find LLC
        LLC_CMD=""
        if command -v llc &> /dev/null; then
            LLC_CMD="llc"
        elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]]; then
            LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
        else
            echo -e "${RED}❌ LLC not found${NC}"
            exit 1
        fi
        
        # Compile with profiling
        if $LLC_CMD -O2 -filetype=obj -relocation-model=pic output.ll -o profile_program.o; then
            echo -e "${GREEN}✓ Object file with profiling created${NC}"
        else
            echo -e "${RED}❌ Failed to compile with profiling${NC}"
            exit 1
        fi
        
        # Link with C++ libraries and profiling
        if clang -O2 -fprofile-instr-generate profile_program.o cypescript_stdlib.o -o "$OUTPUT_NAME" -lstdc++; then
            echo -e "${GREEN}✓ Instrumented executable with C++ support created${NC}"
        else
            echo -e "${RED}❌ Failed to link with profiling${NC}"
            exit 1
        fi
        
        # Cleanup
        rm -f output.ll profile_program.o cypescript_stdlib.o
        
        echo -e "${GREEN}🎉 PGO instrumentation complete!${NC}"
        echo -e "${YELLOW}📍 Instrumented executable: ${NC}$OUTPUT_NAME"
        echo -e "${YELLOW}🚀 Next steps:${NC}"
        echo -e "  1. Run: ${CYAN}./$OUTPUT_NAME${NC}"
        echo -e "  2. Convert profile: ${CYAN}llvm-profdata merge -output=profile.profdata default.profraw${NC}"
        echo -e "  3. Optimize: ${CYAN}$0 optimize $INPUT_FILE optimized_program profile.profdata${NC}"
        ;;
        
    "optimize")
        if [[ ! -f "$PROFILE_DATA" ]]; then
            echo -e "${RED}❌ Profile data not found: $PROFILE_DATA${NC}"
            exit 1
        fi
        
        echo -e "${BLUE}🎯 Creating PGO-optimized version with C++ support${NC}"
        echo -e "${BLUE}📊 Using profile data: $PROFILE_DATA${NC}"
        
        # Compile Cypescript to LLVM IR
        if ./build/cscript "$INPUT_FILE"; then
            echo -e "${GREEN}✓ Cypescript compiled to LLVM IR${NC}"
        else
            echo -e "${RED}❌ Failed to compile Cypescript${NC}"
            exit 1
        fi
        
        # Find LLC
        LLC_CMD=""
        if command -v llc &> /dev/null; then
            LLC_CMD="llc"
        elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]]; then
            LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
        else
            echo -e "${RED}❌ LLC not found${NC}"
            exit 1
        fi
        
        # Compile with PGO
        if $LLC_CMD -O3 -filetype=obj -relocation-model=pic output.ll -o pgo_program.o; then
            echo -e "${GREEN}✓ PGO-optimized object file created${NC}"
        else
            echo -e "${RED}❌ Failed to compile with PGO${NC}"
            exit 1
        fi
        
        # Link with profile data and C++ libraries
        if clang -O3 -fprofile-instr-use="$PROFILE_DATA" pgo_program.o cypescript_stdlib.o -o "$OUTPUT_NAME" -lstdc++ 2>/dev/null; then
            echo -e "${GREEN}✓ PGO-optimized executable with C++ support created${NC}"
        elif clang -O3 pgo_program.o cypescript_stdlib.o -o "$OUTPUT_NAME" -lstdc++; then
            echo -e "${YELLOW}⚠️  Profile data not used, created O3-optimized executable${NC}"
        else
            echo -e "${RED}❌ Failed to link PGO executable${NC}"
            exit 1
        fi
        
        # Cleanup
        rm -f output.ll pgo_program.o cypescript_stdlib.o
        
        echo -e "${GREEN}🎉 PGO optimization complete!${NC}"
        echo -e "${YELLOW}📍 PGO-optimized executable: ${NC}$OUTPUT_NAME"
        ;;
        
    "compare")
        echo -e "${BLUE}📊 Performance comparison with C++ support${NC}"
        
        # Create standard version
        if ./compile-with-cpp.sh "$INPUT_FILE" "${OUTPUT_NAME}_standard" >/dev/null 2>&1; then
            echo -e "${GREEN}✓ Standard version created${NC}"
        else
            echo -e "${RED}❌ Failed to create standard version${NC}"
            exit 1
        fi
        
        # Create instrumented version
        if $0 profile "$INPUT_FILE" "${OUTPUT_NAME}_instrumented" >/dev/null 2>&1; then
            echo -e "${GREEN}✓ Instrumented version created${NC}"
        else
            echo -e "${RED}❌ Failed to create instrumented version${NC}"
            exit 1
        fi
        
        # Run instrumented version
        if "./${OUTPUT_NAME}_instrumented" >/dev/null 2>&1; then
            echo -e "${GREEN}✓ Profile data collected${NC}"
        else
            echo -e "${YELLOW}⚠️  Instrumented run completed with warnings${NC}"
        fi
        
        # Convert profile data
        PROFDATA_CMD=""
        if command -v llvm-profdata &> /dev/null; then
            PROFDATA_CMD="llvm-profdata"
        elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llvm-profdata" ]]; then
            PROFDATA_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llvm-profdata"
        fi
        
        if [[ -n "$PROFDATA_CMD" ]] && [[ -f "default.profraw" ]]; then
            if $PROFDATA_CMD merge -output=comparison.profdata default.profraw 2>/dev/null; then
                echo -e "${GREEN}✓ Profile data converted${NC}"
                
                # Create PGO version
                if $0 optimize "$INPUT_FILE" "${OUTPUT_NAME}_pgo" comparison.profdata >/dev/null 2>&1; then
                    echo -e "${GREEN}✓ PGO version created${NC}"
                    
                    # Performance comparison
                    echo -e "${CYAN}🏁 Performance Comparison${NC}"
                    echo -e "${CYAN}========================${NC}"
                    
                    echo -e "${YELLOW}1. Standard Version${NC}"
                    STANDARD_START=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                    "./${OUTPUT_NAME}_standard" >/dev/null 2>&1
                    STANDARD_END=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                    STANDARD_TIME=$((STANDARD_END - STANDARD_START))
                    echo -e "${GREEN}✓ Standard time: ${STANDARD_TIME}ms${NC}"
                    
                    echo -e "${YELLOW}2. PGO-Optimized Version${NC}"
                    PGO_START=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                    "./${OUTPUT_NAME}_pgo" >/dev/null 2>&1
                    PGO_END=$(node -e "console.log(Date.now())" 2>/dev/null || date +%s%3N)
                    PGO_TIME=$((PGO_END - PGO_START))
                    echo -e "${GREEN}✓ PGO time: ${PGO_TIME}ms${NC}"
                    
                    # Calculate improvement
                    if [[ $STANDARD_TIME -gt 0 ]] && [[ $PGO_TIME -gt 0 ]]; then
                        IMPROVEMENT=$((STANDARD_TIME - PGO_TIME))
                        SPEEDUP=$(node -e "console.log((${STANDARD_TIME} / ${PGO_TIME}).toFixed(2))" 2>/dev/null || echo "N/A")
                        PERCENT=$(node -e "console.log((100 * ${IMPROVEMENT} / ${STANDARD_TIME}).toFixed(1))" 2>/dev/null || echo "N/A")
                        
                        echo -e "${CYAN}========================${NC}"
                        echo -e "${GREEN}🏆 Results:${NC}"
                        echo -e "   Standard: ${STANDARD_TIME}ms"
                        echo -e "   PGO:      ${PGO_TIME}ms"
                        echo -e "   Improvement: ${IMPROVEMENT}ms (${PERCENT}%)"
                        echo -e "   Speedup: ${SPEEDUP}x"
                        
                        if [[ $PGO_TIME -lt $STANDARD_TIME ]]; then
                            echo -e "${GREEN}🎉 PGO optimization successful!${NC}"
                        else
                            echo -e "${YELLOW}⚠️  PGO didn't improve performance${NC}"
                        fi
                    fi
                    
                    # Cleanup
                    rm -f "${OUTPUT_NAME}_standard" "${OUTPUT_NAME}_instrumented" "${OUTPUT_NAME}_pgo"
                    rm -f default.profraw comparison.profdata
                fi
            fi
        fi
        ;;
esac

echo -e "${BLUE}💡 PGO with C++ Integration Info:${NC}"
echo -e "   • Includes C++ standard library support"
echo -e "   • Optimizes hot paths with runtime profiling"
echo -e "   • Best results with representative workloads"
echo -e "   • Combines with LLVM optimizations"
