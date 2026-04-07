#!/bin/bash
# Cypescript Benchmark Suite - Cypescript vs Node.js
set +e

CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
COMPILER="$ROOT_DIR/build/cscript"
RUNS=${1:-3}

if [[ ! -f "$COMPILER" ]]; then
    echo -e "\033[0;31m❌ Compiler not found. Run ./build.sh first\033[0m"
    exit 1
fi

if ! command -v node &> /dev/null; then
    echo -e "\033[0;31m❌ Node.js not found. Install it for comparison benchmarks.\033[0m"
    exit 1
fi

echo -e "${BOLD}============================================${NC}"
echo -e "${BOLD}  Cypescript vs Node.js Benchmark Suite${NC}"
echo -e "${BOLD}  Runs per benchmark: $RUNS${NC}"
echo -e "${BOLD}============================================${NC}"
echo ""

# Compile all Cypescript benchmarks
echo -e "${CYAN}Compiling Cypescript benchmarks...${NC}"
for csc_file in "$SCRIPT_DIR"/bench_*.csc "$SCRIPT_DIR"/benchmark_*.csc; do
    [[ -f "$csc_file" ]] || continue
    name=$(basename "$csc_file" .csc)
    ts_file="$SCRIPT_DIR/${name}.ts"
    [[ -f "$ts_file" ]] || continue  # skip if no matching .ts file
    rel_csc="benchmarks/$(basename "$csc_file")"
    rel_out="benchmarks/$name"
    (cd "$ROOT_DIR" && "$COMPILER" "$rel_csc" -o "$rel_out" 2>/dev/null) || echo "  ⚠ Skipping $name (compile failed)"
done
echo ""

# Run benchmarks
declare -a BENCHMARKS=(
    "Simple Loop (100M):benchmark_simple"
    "Fibonacci (10M calls):bench_fibonacci"
    "Matrix Mult (300^3):bench_matrix"
    "Prime Sieve (500K):bench_primes"
)

printf "${BOLD}%-28s %15s %15s %10s${NC}\n" "Benchmark" "Cypescript" "Node.js" "Speedup"
echo "----------------------------------------------------------------------"

for bench in "${BENCHMARKS[@]}"; do
    IFS=':' read -r label name <<< "$bench"
    
    csc_bin="$SCRIPT_DIR/$name"
    ts_file="$SCRIPT_DIR/${name}.ts"
    
    [[ -f "$csc_bin" && -f "$ts_file" ]] || continue
    
    # Cypescript: best of N runs (user time in seconds)
    best_csc=999
    for ((i=0; i<RUNS; i++)); do
        t=$( { time "$csc_bin" > /dev/null 2>&1; } 2>&1 | grep user | sed 's/.*0m//' | sed 's/s//')
        if (( $(echo "$t < $best_csc" | bc -l) )); then best_csc=$t; fi
    done
    
    # Node.js: best of N runs
    best_node=999
    for ((i=0; i<RUNS; i++)); do
        t=$( { time node "$ts_file" > /dev/null 2>&1; } 2>&1 | grep user | sed 's/.*0m//' | sed 's/s//')
        if (( $(echo "$t < $best_node" | bc -l) )); then best_node=$t; fi
    done
    
    # Calculate speedup
    if (( $(echo "$best_csc > 0" | bc -l) )); then
        speedup=$(echo "scale=1; $best_node / $best_csc" | bc -l)
        speedup_str="${speedup}x"
    else
        speedup_str="∞"
    fi
    
    printf "%-28s %13.3fs %13.3fs %10s\n" "$label" "$best_csc" "$best_node" "$speedup_str"
done

echo ""
echo -e "${GREEN}✅ Benchmark complete. Cypescript compiled with -O2 optimizations.${NC}"

# Cleanup binaries
rm -f "$SCRIPT_DIR"/benchmark_simple "$SCRIPT_DIR"/bench_fibonacci "$SCRIPT_DIR"/bench_matrix "$SCRIPT_DIR"/bench_primes
