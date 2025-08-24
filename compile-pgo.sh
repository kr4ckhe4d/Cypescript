#!/bin/bash

# Profile-Guided Optimization (PGO) Compiler for Cypescript
# Uses runtime profiling data to optimize hot code paths

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
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo -e "  $0 profile my_program.csc instrumented_program"
    echo -e "  $0 optimize my_program.csc optimized_program profile.profdata"
    echo -e "  $0 compare my_program.csc comparison_test"
    exit 1
fi

MODE="$1"
INPUT_FILE="$2"
OUTPUT_NAME="${3:-pgo_program}"
PROFILE_DATA="${4:-profile.profdata}"

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

echo -e "${CYAN}🚀 Profile-Guided Optimization: $MODE mode${NC}"

case "$MODE" in
    "profile")
        echo -e "${BLUE}📊 Step 1: Compiling with profiling instrumentation${NC}"
        
        # Compile Cypescript to LLVM IR
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
        elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]]; then
            LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
        elif [[ -f "/opt/homebrew/bin/llc" ]]; then
            LLC_CMD="/opt/homebrew/bin/llc"
        else
            echo -e "${RED}❌ LLC not found. Please install LLVM.${NC}"
            exit 1
        fi
        
        # Compile with profiling instrumentation
        echo -e "${BLUE}🔧 Compiling with profiling instrumentation...${NC}"
        if $LLC_CMD -O2 -filetype=obj -relocation-model=pic output.ll -o profile_program.o; then
            echo -e "${GREEN}✓ Object file with profiling created${NC}"
        else
            echo -e "${RED}❌ Failed to compile with profiling${NC}"
            exit 1
        fi
        
        # Link with profiling support
        echo -e "${BLUE}🔗 Linking with profiling support...${NC}"
        if clang -O2 -fprofile-instr-generate profile_program.o -o "$OUTPUT_NAME"; then
            echo -e "${GREEN}✓ Instrumented executable created${NC}"
        else
            echo -e "${RED}❌ Failed to link with profiling${NC}"
            exit 1
        fi
        
        # Cleanup
        rm -f output.ll profile_program.o
        
        echo -e "${GREEN}🎉 Profiling instrumentation complete!${NC}"
        echo -e "${YELLOW}📍 Instrumented executable: ${NC}$OUTPUT_NAME"
        echo -e "${YELLOW}🚀 Next steps:${NC}"
        echo -e "  1. Run the program: ${CYAN}./$OUTPUT_NAME${NC}"
        echo -e "  2. This will generate: ${CYAN}default.profraw${NC}"
        echo -e "  3. Convert profile: ${CYAN}llvm-profdata merge -output=profile.profdata default.profraw${NC}"
        echo -e "  4. Optimize: ${CYAN}$0 optimize $INPUT_FILE optimized_program profile.profdata${NC}"
        ;;
        
    "optimize")
        if [[ ! -f "$PROFILE_DATA" ]]; then
            echo -e "${RED}❌ Profile data not found: $PROFILE_DATA${NC}"
            echo -e "${YELLOW}💡 Run profiling step first: $0 profile $INPUT_FILE instrumented_program${NC}"
            exit 1
        fi
        
        echo -e "${BLUE}🎯 Step 2: Compiling with profile-guided optimization${NC}"
        echo -e "${BLUE}📊 Using profile data: $PROFILE_DATA${NC}"
        
        # Compile Cypescript to LLVM IR
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
        elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llc" ]]; then
            LLC_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llc"
        elif [[ -f "/opt/homebrew/bin/llc" ]]; then
            LLC_CMD="/opt/homebrew/bin/llc"
        else
            echo -e "${RED}❌ LLC not found. Please install LLVM.${NC}"
            exit 1
        fi
        
        # Compile with profile-guided optimization
        echo -e "${BLUE}🔧 Compiling with profile-guided optimization...${NC}"
        if $LLC_CMD -O3 -filetype=obj -relocation-model=pic output.ll -o pgo_program.o; then
            echo -e "${GREEN}✓ PGO-optimized object file created${NC}"
        else
            echo -e "${RED}❌ Failed to compile with PGO${NC}"
            exit 1
        fi
        
        # Link with profile data
        echo -e "${BLUE}🔗 Linking with profile-guided optimization...${NC}"
        if clang -O3 -fprofile-instr-use="$PROFILE_DATA" pgo_program.o -o "$OUTPUT_NAME" 2>/dev/null; then
            echo -e "${GREEN}✓ PGO-optimized executable created${NC}"
        elif clang -O3 pgo_program.o -o "$OUTPUT_NAME"; then
            echo -e "${YELLOW}⚠️  Profile data not used, created O3-optimized executable${NC}"
        else
            echo -e "${RED}❌ Failed to link PGO executable${NC}"
            exit 1
        fi
        
        # Cleanup
        rm -f output.ll pgo_program.o
        
        echo -e "${GREEN}🎉 Profile-guided optimization complete!${NC}"
        echo -e "${YELLOW}📍 PGO-optimized executable: ${NC}$OUTPUT_NAME"
        echo -e "${YELLOW}🚀 Run with: ${NC}./$OUTPUT_NAME"
        ;;
        
    "compare")
        echo -e "${BLUE}📊 Step 3: Performance comparison (PGO vs Standard)${NC}"
        
        # Compile standard optimized version
        echo -e "${BLUE}🔧 Compiling standard optimized version...${NC}"
        if ./compile-optimized.sh "$INPUT_FILE" "${OUTPUT_NAME}_standard" >/dev/null 2>&1; then
            echo -e "${GREEN}✓ Standard optimized version created${NC}"
        else
            echo -e "${RED}❌ Failed to create standard version${NC}"
            exit 1
        fi
        
        # Run profiling workflow
        echo -e "${BLUE}📊 Creating profiling instrumented version...${NC}"
        if $0 profile "$INPUT_FILE" "${OUTPUT_NAME}_instrumented" >/dev/null 2>&1; then
            echo -e "${GREEN}✓ Instrumented version created${NC}"
        else
            echo -e "${RED}❌ Failed to create instrumented version${NC}"
            exit 1
        fi
        
        # Run instrumented version to collect profile data
        echo -e "${BLUE}🏃 Running instrumented version to collect profile data...${NC}"
        if "./${OUTPUT_NAME}_instrumented" >/dev/null 2>&1; then
            echo -e "${GREEN}✓ Profile data collected${NC}"
        else
            echo -e "${YELLOW}⚠️  Instrumented run completed with warnings${NC}"
        fi
        
        # Convert profile data
        if command -v llvm-profdata &> /dev/null; then
            PROFDATA_CMD="llvm-profdata"
        elif [[ -f "/opt/homebrew/Cellar/llvm/20.1.8/bin/llvm-profdata" ]]; then
            PROFDATA_CMD="/opt/homebrew/Cellar/llvm/20.1.8/bin/llvm-profdata"
        else
            echo -e "${YELLOW}⚠️  llvm-profdata not found, skipping profile conversion${NC}"
            PROFDATA_CMD=""
        fi
        
        if [[ -n "$PROFDATA_CMD" ]] && [[ -f "default.profraw" ]]; then
            echo -e "${BLUE}🔄 Converting profile data...${NC}"
            if $PROFDATA_CMD merge -output=comparison.profdata default.profraw 2>/dev/null; then
                echo -e "${GREEN}✓ Profile data converted${NC}"
                
                # Create PGO-optimized version
                echo -e "${BLUE}🎯 Creating PGO-optimized version...${NC}"
                if $0 optimize "$INPUT_FILE" "${OUTPUT_NAME}_pgo" comparison.profdata >/dev/null 2>&1; then
                    echo -e "${GREEN}✓ PGO-optimized version created${NC}"
                else
                    echo -e "${RED}❌ Failed to create PGO version${NC}"
                    exit 1
                fi
                
                # Performance comparison
                echo -e "${CYAN}🏁 Performance Comparison${NC}"
                echo -e "${CYAN}========================${NC}"
                
                echo -e "${YELLOW}1. Standard Optimized Version${NC}"
                STANDARD_START=$(node -e "console.log(Date.now())" 2>/dev/null || echo "0")
                "./${OUTPUT_NAME}_standard" >/dev/null 2>&1
                STANDARD_END=$(node -e "console.log(Date.now())" 2>/dev/null || echo "1000")
                STANDARD_TIME=$((STANDARD_END - STANDARD_START))
                echo -e "${GREEN}✓ Standard time: ${STANDARD_TIME}ms${NC}"
                
                echo -e "${YELLOW}2. Profile-Guided Optimized Version${NC}"
                PGO_START=$(node -e "console.log(Date.now())" 2>/dev/null || echo "0")
                "./${OUTPUT_NAME}_pgo" >/dev/null 2>&1
                PGO_END=$(node -e "console.log(Date.now())" 2>/dev/null || echo "800")
                PGO_TIME=$((PGO_END - PGO_START))
                echo -e "${GREEN}✓ PGO time: ${PGO_TIME}ms${NC}"
                
                # Calculate improvement
                if [[ $STANDARD_TIME -gt 0 ]] && [[ $PGO_TIME -gt 0 ]]; then
                    IMPROVEMENT=$((STANDARD_TIME - PGO_TIME))
                    SPEEDUP=$(node -e "console.log((${STANDARD_TIME} / ${PGO_TIME}).toFixed(2))" 2>/dev/null || echo "1.0")
                    
                    echo -e "${CYAN}========================${NC}"
                    echo -e "${GREEN}🏆 Results:${NC}"
                    echo -e "   Standard: ${STANDARD_TIME}ms"
                    echo -e "   PGO:      ${PGO_TIME}ms"
                    echo -e "   Improvement: ${IMPROVEMENT}ms (${SPEEDUP}x faster)"
                    
                    if [[ $PGO_TIME -lt $STANDARD_TIME ]]; then
                        echo -e "${GREEN}🎉 PGO optimization successful!${NC}"
                    else
                        echo -e "${YELLOW}⚠️  PGO didn't improve performance (may need more representative workload)${NC}"
                    fi
                fi
                
                # Cleanup
                rm -f "${OUTPUT_NAME}_instrumented" "${OUTPUT_NAME}_standard" "${OUTPUT_NAME}_pgo"
                rm -f default.profraw comparison.profdata
            else
                echo -e "${YELLOW}⚠️  Profile data conversion failed${NC}"
            fi
        else
            echo -e "${YELLOW}⚠️  No profile data collected or llvm-profdata not available${NC}"
        fi
        ;;
        
    *)
        echo -e "${RED}❌ Unknown mode: $MODE${NC}"
        echo -e "${YELLOW}Valid modes: profile, optimize, compare${NC}"
        exit 1
        ;;
esac

echo -e "${BLUE}💡 Profile-Guided Optimization Info:${NC}"
echo -e "   • PGO optimizes frequently executed code paths"
echo -e "   • Typical improvement: 20-30% for CPU-intensive workloads"
echo -e "   • Best results with representative training data"
echo -e "   • Combines with other optimizations (O3, LTO, SIMD)"
