#!/bin/bash
# Cypescript Test Suite
# Each tests/test_*.csc is compiled and run; if tests/expected/<name>.out
# exists, the program's stdout must match it exactly (not just exit 0).
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

    bin="$SCRIPT_DIR/.bin_$test_name"
    expected_file="$SCRIPT_DIR/expected/$test_name.out"
    status="ok"

    # Compile
    if ! "$COMPILER" -o "$bin" "$test_file" >/dev/null 2>&1; then
        status="compile failed"
    else
        # Run
        actual=$("$bin" 2>/dev/null)
        exit_code=$?
        if [[ $exit_code -ne 0 ]]; then
            status="exit code $exit_code"
        elif [[ -f "$expected_file" ]]; then
            expected=$(cat "$expected_file")
            if [[ "$actual" != "$expected" ]]; then
                status="output mismatch"
            fi
        fi
    fi
    rm -f "$bin"

    if [[ "$status" == "ok" ]]; then
        echo -e "${GREEN}✅ PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}❌ FAIL ($status)${NC}"
        FAIL=$((FAIL + 1))
        ERRORS="$ERRORS\n  - $test_name ($status)"
        if [[ "$status" == "output mismatch" ]]; then
            diff <(echo "$expected") <(echo "$actual") | head -10 | sed 's/^/      /'
        fi
    fi
done

echo "============================================"
echo -e "  ${GREEN}Passed: $PASS${NC}  ${RED}Failed: $FAIL${NC}  Total: $((PASS + FAIL))"

if [[ $FAIL -gt 0 ]]; then
    echo -e "\n${RED}Failed tests:${ERRORS}${NC}"
    exit 1
else
    echo -e "\n${GREEN}🎉 All tests passed!${NC}"
fi
