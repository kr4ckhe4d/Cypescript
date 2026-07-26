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
PASS=0
FAIL=0
ERRORS=""

# Locate the compiler across platforms and generator layouts
COMPILER=""
for candidate in "$ROOT_DIR/build/cscript" "$ROOT_DIR/build/cscript.exe" \
                 "$ROOT_DIR/build/Release/cscript.exe"; do
    [[ -f "$candidate" ]] && COMPILER="$candidate" && break
done
if [[ -z "$COMPILER" ]]; then
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

# --- Negative tests -----------------------------------------------------------
# Programs that MUST be rejected, with the message they must be rejected with.
# Without these nothing verifies that errors are actually reported — the whole
# semantic pass could stop working and every positive test would still pass.
if [[ -d "$SCRIPT_DIR/negative" ]]; then
    echo ""
    echo -e "${CYAN}Negative tests (must be rejected)${NC}"
    echo "--------------------------------------------"
    for neg_file in "$SCRIPT_DIR"/negative/*.csc; do
        [[ -e "$neg_file" ]] || continue
        neg_name=$(basename "$neg_file" .csc)
        printf "  %-25s" "$neg_name"

        expected=$(grep -m1 '^// EXPECT:' "$neg_file" | sed 's|^// EXPECT: *||')
        # A rejected program exits non-zero, which `set -e` would treat as a
        # script failure — the `if` makes the status observable instead.
        if output=$("$COMPILER" -o "$SCRIPT_DIR/.neg_bin" "$neg_file" 2>&1); then
            status=0
        else
            status=$?
        fi
        rm -f "$SCRIPT_DIR/.neg_bin"

        if [[ $status -eq 0 ]]; then
            echo -e "${RED}❌ FAIL (compiled, but should have been rejected)${NC}"
            FAIL=$((FAIL + 1))
            ERRORS="$ERRORS\n  - $neg_name (accepted an invalid program)"
        elif [[ -n "$expected" ]] && ! grep -qF "$expected" <<< "$output"; then
            echo -e "${RED}❌ FAIL (wrong message)${NC}"
            echo "      expected to contain: $expected"
            echo "      got: $(head -1 <<< "$output")"
            FAIL=$((FAIL + 1))
            ERRORS="$ERRORS\n  - $neg_name (wrong error message)"
        else
            echo -e "${GREEN}✅ PASS${NC}"
            PASS=$((PASS + 1))
        fi
    done
    echo ""
fi

echo "============================================"
echo -e "  ${GREEN}Passed: $PASS${NC}  ${RED}Failed: $FAIL${NC}  Total: $((PASS + FAIL))"

if [[ $FAIL -gt 0 ]]; then
    echo -e "\n${RED}Failed tests:${ERRORS}${NC}"
    exit 1
else
    echo -e "\n${GREEN}🎉 All tests passed!${NC}"
fi
