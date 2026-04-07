#!/bin/bash
# Cypescript Test Suite
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
COMPILER="$ROOT_DIR/build/cscript"
PASS=0
FAIL=0
ERRORS=""

if [[ ! -f "$COMPILER" ]]; then
    echo -e "${RED}❌ Compiler not found. Run ./build.sh first${NC}"
    exit 1
fi

echo -e "${CYAN}🧪 Cypescript Test Suite${NC}"
echo "============================================"

for test_file in "$SCRIPT_DIR"/test_*.csc; do
    test_name=$(basename "$test_file" .csc)
    printf "  %-25s" "$test_name"
    
    # Compile and run, capture output and exit code
    output=$("$COMPILER" -r "$test_file" 2>/dev/null)
    exit_code=$?
    
    if [[ $exit_code -eq 0 ]]; then
        echo -e "${GREEN}✅ PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}❌ FAIL${NC}"
        FAIL=$((FAIL + 1))
        ERRORS="$ERRORS\n  - $test_name"
    fi
    
    # Clean up compiled binary
    rm -f "$ROOT_DIR/$test_name"
done

echo "============================================"
echo -e "  ${GREEN}Passed: $PASS${NC}  ${RED}Failed: $FAIL${NC}  Total: $((PASS + FAIL))"

if [[ $FAIL -gt 0 ]]; then
    echo -e "\n${RED}Failed tests:${ERRORS}${NC}"
    exit 1
else
    echo -e "\n${GREEN}🎉 All tests passed!${NC}"
fi
