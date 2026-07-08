#!/bin/bash
# Cross-language benchmark: Cypescript vs TypeScript (Node) vs Python vs Rust
# The exact same algorithm is implemented in all four languages.
set +e

CYAN='\033[0;36m'
GREEN='\033[0;32m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
COMPILER="$ROOT_DIR/build/cscript"
RUNS=${1:-3}

if [[ ! -f "$COMPILER" ]]; then
    echo -e "${RED}❌ Compiler not found. Run ./build.sh first${NC}"
    exit 1
fi

HAVE_NODE=1; command -v node   >/dev/null || HAVE_NODE=0
HAVE_PY=1;   command -v python3 >/dev/null || HAVE_PY=0
HAVE_RUST=1; command -v rustc  >/dev/null || HAVE_RUST=0

echo -e "${BOLD}=============================================================${NC}"
echo -e "${BOLD}  Cross-Language Benchmark: Cypescript vs TS vs Python vs Rust${NC}"
echo -e "${BOLD}  Best of $RUNS runs (wall-clock seconds, lower is better)${NC}"
echo -e "${BOLD}=============================================================${NC}"
echo ""

declare -a BENCHMARKS=(
    "Primes <1M (trial div):primes"
    "Fibonacci fib(35) recursive:fib"
)

# Compile ahead of time
echo -e "${CYAN}Compiling...${NC}"
for bench in "${BENCHMARKS[@]}"; do
    IFS=':' read -r _label name <<< "$bench"
    (cd "$ROOT_DIR" && "$COMPILER" "benchmarks/cross/$name.csc" -o "benchmarks/cross/${name}_csc" >/dev/null 2>&1) \
        || echo -e "${RED}  ⚠ Cypescript compile failed for $name${NC}"
    if [[ $HAVE_RUST -eq 1 ]]; then
        rustc -O "$SCRIPT_DIR/$name.rs" -o "$SCRIPT_DIR/${name}_rs" 2>/dev/null \
            || echo -e "${RED}  ⚠ Rust compile failed for $name${NC}"
    fi
done
echo ""

# best_time <command...> -> echoes best wall time over $RUNS runs
best_time() {
    local best="999999"
    local t
    for ((i = 0; i < RUNS; i++)); do
        t=$( { TIMEFORMAT='%R'; time "$@" > /dev/null 2>&1; } 2>&1 )
        if (( $(echo "$t < $best" | bc -l) )); then best=$t; fi
    done
    echo "$best"
}

printf "${BOLD}%-30s %12s %12s %12s %12s${NC}\n" "Benchmark" "Cypescript" "Node (TS)" "Python" "Rust"
echo "--------------------------------------------------------------------------------"

for bench in "${BENCHMARKS[@]}"; do
    IFS=':' read -r label name <<< "$bench"

    # Verify all implementations agree before timing
    expected=$("$SCRIPT_DIR/${name}_csc" 2>/dev/null)
    [[ $HAVE_NODE -eq 1 ]] && node_out=$(node "$SCRIPT_DIR/$name.ts" 2>/dev/null) || node_out="$expected"
    [[ $HAVE_PY -eq 1 ]] && py_out=$(python3 "$SCRIPT_DIR/$name.py" 2>/dev/null) || py_out="$expected"
    [[ $HAVE_RUST -eq 1 ]] && rs_out=$("$SCRIPT_DIR/${name}_rs" 2>/dev/null) || rs_out="$expected"
    if [[ "$node_out" != "$expected" || "$py_out" != "$expected" || "$rs_out" != "$expected" ]]; then
        echo -e "${RED}  ⚠ $name: outputs differ across languages (csc=$expected node=$node_out py=$py_out rs=$rs_out)${NC}"
    fi

    t_csc=$(best_time "$SCRIPT_DIR/${name}_csc")
    t_node="-"; t_py="-"; t_rs="-"
    [[ $HAVE_NODE -eq 1 ]] && t_node=$(best_time node "$SCRIPT_DIR/$name.ts")
    [[ $HAVE_PY -eq 1 ]] && t_py=$(best_time python3 "$SCRIPT_DIR/$name.py")
    [[ $HAVE_RUST -eq 1 ]] && t_rs=$(best_time "$SCRIPT_DIR/${name}_rs")

    printf "%-30s %11ss %11ss %11ss %11ss\n" "$label" "$t_csc" "$t_node" "$t_py" "$t_rs"
done

echo ""
echo -e "${GREEN}✅ Done. Cypescript compiled with -O2; Rust with rustc -O.${NC}"

# Cleanup binaries
for bench in "${BENCHMARKS[@]}"; do
    IFS=':' read -r _label name <<< "$bench"
    rm -f "$SCRIPT_DIR/${name}_csc" "$SCRIPT_DIR/${name}_rs"
done
