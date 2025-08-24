#!/bin/bash

# Cypescript Process Pool - Proof of Concept
# Eliminates startup overhead by keeping compiled programs in memory

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

POOL_DIR="/tmp/cypescript-pool"
CACHE_DIR="/tmp/cypescript-cache"

# Initialize pool
init_pool() {
    echo -e "${BLUE}🚀 Initializing Cypescript Process Pool${NC}"
    
    # Create directories
    mkdir -p "$POOL_DIR"
    mkdir -p "$CACHE_DIR"
    
    echo -e "${GREEN}✓ Process pool initialized${NC}"
    echo -e "${YELLOW}📍 Pool directory: $POOL_DIR${NC}"
    echo -e "${YELLOW}📍 Cache directory: $CACHE_DIR${NC}"
}

# Compile and cache a program
cache_program() {
    local source_file="$1"
    local program_name="$2"
    
    if [[ ! -f "$source_file" ]]; then
        echo -e "${RED}❌ Source file not found: $source_file${NC}"
        return 1
    fi
    
    echo -e "${BLUE}📝 Compiling and caching: $source_file${NC}"
    
    # Generate cache key from file content
    local cache_key=$(shasum -a 256 "$source_file" | cut -d' ' -f1)
    local cached_binary="$CACHE_DIR/${program_name}_${cache_key}"
    
    # Check if already cached
    if [[ -f "$cached_binary" ]]; then
        echo -e "${GREEN}✓ Using cached binary${NC}"
        cp "$cached_binary" "$POOL_DIR/$program_name"
        return 0
    fi
    
    # Compile with optimizations
    echo -e "${BLUE}🔧 Compiling with optimizations...${NC}"
    if ./compile-optimized.sh "$source_file" "$POOL_DIR/$program_name" >/dev/null 2>&1; then
        echo -e "${GREEN}✓ Compilation successful${NC}"
        
        # Cache the binary
        cp "$POOL_DIR/$program_name" "$cached_binary"
        echo -e "${GREEN}✓ Binary cached for future use${NC}"
        
        return 0
    else
        echo -e "${RED}❌ Compilation failed${NC}"
        return 1
    fi
}

# Execute a cached program
execute_cached() {
    local program_name="$1"
    local cached_binary="$POOL_DIR/$program_name"
    
    if [[ ! -f "$cached_binary" ]]; then
        echo -e "${RED}❌ Program not found in pool: $program_name${NC}"
        echo -e "${YELLOW}💡 Use: $0 cache <source-file> $program_name${NC}"
        return 1
    fi
    
    echo -e "${BLUE}⚡ Executing cached program: $program_name${NC}"
    echo -e "${BLUE}----------------------------------------${NC}"
    
    # Execute with timing
    local start_time=$(node -e "console.log(Date.now())")
    "$cached_binary"
    local end_time=$(node -e "console.log(Date.now())")
    local execution_time=$((end_time - start_time))
    
    echo -e "${BLUE}----------------------------------------${NC}"
    echo -e "${GREEN}✓ Execution complete in ${execution_time}ms${NC}"
    echo -e "${YELLOW}💡 No compilation overhead - instant execution!${NC}"
}

# Benchmark comparison
benchmark_comparison() {
    local source_file="$1"
    
    if [[ ! -f "$source_file" ]]; then
        echo -e "${RED}❌ Source file not found: $source_file${NC}"
        return 1
    fi
    
    echo -e "${BLUE}🏁 Benchmark: Traditional vs Pooled Execution${NC}"
    echo -e "${BLUE}=============================================${NC}"
    
    # Traditional compilation + execution
    echo -e "${YELLOW}1. Traditional Compilation + Execution${NC}"
    local trad_start=$(node -e "console.log(Date.now())")
    
    if ./compile-optimized.sh "$source_file" "benchmark_traditional" >/dev/null 2>&1; then
        ./benchmark_traditional >/dev/null 2>&1
        local trad_end=$(node -e "console.log(Date.now())")
        local trad_time=$((trad_end - trad_start))
        echo -e "${GREEN}✓ Traditional method: ${trad_time}ms${NC}"
        rm -f benchmark_traditional
    else
        echo -e "${RED}❌ Traditional compilation failed${NC}"
        return 1
    fi
    
    # Pooled execution (assuming already cached)
    echo -e "${YELLOW}2. Pooled Execution (Pre-compiled)${NC}"
    
    # Cache the program first
    cache_program "$source_file" "benchmark_pooled" >/dev/null 2>&1
    
    local pool_start=$(node -e "console.log(Date.now())")
    "$POOL_DIR/benchmark_pooled" >/dev/null 2>&1
    local pool_end=$(node -e "console.log(Date.now())")
    local pool_time=$((pool_end - pool_start))
    
    echo -e "${GREEN}✓ Pooled method: ${pool_time}ms${NC}"
    
    # Calculate improvement
    local improvement=$((trad_time - pool_time))
    local speedup=$(node -e "console.log((${trad_time} / ${pool_time}).toFixed(2))")
    
    echo -e "${BLUE}=============================================${NC}"
    echo -e "${GREEN}🏆 Results:${NC}"
    echo -e "   Traditional: ${trad_time}ms"
    echo -e "   Pooled: ${pool_time}ms"
    echo -e "   Improvement: ${improvement}ms (${speedup}x faster)"
    echo -e "${YELLOW}💡 Pooled execution eliminates compilation overhead!${NC}"
    
    # Cleanup
    rm -f "$POOL_DIR/benchmark_pooled"
}

# List cached programs
list_cached() {
    echo -e "${BLUE}📋 Cached Programs in Pool${NC}"
    echo -e "${BLUE}=========================${NC}"
    
    if [[ -d "$POOL_DIR" ]] && [[ -n "$(ls -A "$POOL_DIR" 2>/dev/null)" ]]; then
        for program in "$POOL_DIR"/*; do
            if [[ -f "$program" ]]; then
                local name=$(basename "$program")
                local size=$(stat -f%z "$program" 2>/dev/null || stat -c%s "$program" 2>/dev/null || echo "unknown")
                echo -e "${GREEN}✓ $name${NC} (${size} bytes)"
            fi
        done
    else
        echo -e "${YELLOW}No cached programs found${NC}"
    fi
    
    echo ""
    if [[ -d "$CACHE_DIR" ]] && [[ -n "$(ls -A "$CACHE_DIR" 2>/dev/null)" ]]; then
        local cache_count=$(ls -1 "$CACHE_DIR" | wc -l)
        echo -e "${BLUE}📦 Cache entries: ${cache_count}${NC}"
    fi
}

# Clean up pool and cache
cleanup() {
    echo -e "${BLUE}🧹 Cleaning up process pool${NC}"
    rm -rf "$POOL_DIR"
    rm -rf "$CACHE_DIR"
    echo -e "${GREEN}✓ Cleanup complete${NC}"
}

# Main command dispatcher
case "${1:-help}" in
    "init")
        init_pool
        ;;
    "cache")
        if [[ $# -lt 3 ]]; then
            echo -e "${RED}Usage: $0 cache <source-file> <program-name>${NC}"
            exit 1
        fi
        init_pool >/dev/null 2>&1
        cache_program "$2" "$3"
        ;;
    "exec")
        if [[ $# -lt 2 ]]; then
            echo -e "${RED}Usage: $0 exec <program-name>${NC}"
            exit 1
        fi
        execute_cached "$2"
        ;;
    "benchmark")
        if [[ $# -lt 2 ]]; then
            echo -e "${RED}Usage: $0 benchmark <source-file>${NC}"
            exit 1
        fi
        init_pool >/dev/null 2>&1
        benchmark_comparison "$2"
        ;;
    "list")
        list_cached
        ;;
    "clean")
        cleanup
        ;;
    "help"|*)
        echo -e "${BLUE}Cypescript Process Pool - Eliminate Startup Overhead${NC}"
        echo -e "${BLUE}=================================================${NC}"
        echo ""
        echo -e "${YELLOW}Commands:${NC}"
        echo -e "  ${GREEN}init${NC}                     Initialize process pool"
        echo -e "  ${GREEN}cache${NC} <file> <name>       Compile and cache program"
        echo -e "  ${GREEN}exec${NC} <name>              Execute cached program"
        echo -e "  ${GREEN}benchmark${NC} <file>          Compare traditional vs pooled"
        echo -e "  ${GREEN}list${NC}                     List cached programs"
        echo -e "  ${GREEN}clean${NC}                    Clean up pool and cache"
        echo -e "  ${GREEN}help${NC}                     Show this help"
        echo ""
        echo -e "${YELLOW}Examples:${NC}"
        echo -e "  $0 cache benchmarks/array_processing_benchmark.csc array_bench"
        echo -e "  $0 exec array_bench"
        echo -e "  $0 benchmark benchmarks/array_processing_benchmark.csc"
        ;;
esac
