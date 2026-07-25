#!/bin/bash
# Game runtime tests.
#
# These need the optional game runtime (libcypescript_game.a, built when raylib
# is installed). They run the examples in headless mode — CYPS_HEADLESS=1 means
# no window is created and drawing is skipped, so the whole game loop, physics
# and scoring still execute and can be asserted in CI.
#
# Skips cleanly (exit 0) when the game runtime was not built.
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

# The game runtime is optional; skip cleanly when it wasn't built
GAME_RUNTIME=""
for candidate in "$ROOT_DIR/build/libcypescript_game.a" \
                 "$ROOT_DIR/build/cypescript_game.lib" \
                 "$ROOT_DIR/build/Release/cypescript_game.lib"; do
    [[ -f "$candidate" ]] && GAME_RUNTIME="$candidate" && break
done
if [[ -z "$GAME_RUNTIME" ]]; then
    echo -e "${YELLOW}⏭  Game runtime not built — skipping game tests${NC}"
    echo -e "${YELLOW}   Re-run: cmake -S . -B build -DCYPESCRIPT_VENDOR_RAYLIB=ON${NC}"
    exit 0
fi

echo -e "${CYAN}🕹  Cypescript Game Tests${NC}"
echo "============================================"

check() {
    local name="$1"
    local actual="$2"
    local pattern="$3"
    printf "  %-32s" "$name"
    if [[ "$actual" =~ $pattern ]]; then
        echo -e "${GREEN}✅ PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}❌ FAIL${NC}"
        echo "      expected to match: $pattern"
        echo "      got: $actual"
        FAIL=$((FAIL + 1))
    fi
}

BIN_DIR="$(mktemp -d)"
trap 'rm -rf "$BIN_DIR"' EXIT

# --- 00_window: the loop runs and the frame count is exact ---
"$COMPILER" -o "$BIN_DIR/window" "$ROOT_DIR/example/game/00_window.csc" >/dev/null 2>&1
out=$(CYPS_HEADLESS=1 CYPS_FRAMES=45 "$BIN_DIR/window" 2>/dev/null)
check "00_window runs 45 frames" "$out" "ran 45 frames"

# --- 01_breakout: self-play must actually break bricks and score ---
"$COMPILER" -o "$BIN_DIR/breakout" "$ROOT_DIR/example/game/01_breakout.csc" >/dev/null 2>&1
out=$(CYPS_HEADLESS=1 CYPS_FRAMES=1200 "$BIN_DIR/breakout" 2>/dev/null)
check "01_breakout completes a run" "$out" "Breakout ended"

# Physics and collision actually work: the autopilot must destroy bricks.
bricks_left=$(echo "$out" | sed -n 's/.*, \([0-9]*\) bricks left/\1/p')
printf "  %-32s" "01_breakout destroys bricks"
if [[ -n "$bricks_left" && "$bricks_left" -lt 60 ]]; then
    echo -e "${GREEN}✅ PASS${NC} (${bricks_left}/60 remaining)"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (bricks left: '${bricks_left}', expected < 60)"
    FAIL=$((FAIL + 1))
fi

# Scoring is wired to brick destruction
score=$(echo "$out" | sed -n 's/.*score \([0-9]*\),.*/\1/p')
printf "  %-32s" "01_breakout scores points"
if [[ -n "$score" && "$score" -gt 0 ]]; then
    echo -e "${GREEN}✅ PASS${NC} (score ${score})"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (score: '${score}', expected > 0)"
    FAIL=$((FAIL + 1))
fi

echo "============================================"
echo -e "  ${GREEN}Passed: $PASS${NC}  ${RED}Failed: $FAIL${NC}  Total: $((PASS + FAIL))"

if [[ $FAIL -gt 0 ]]; then
    exit 1
fi
echo -e "\n${GREEN}🎉 All game tests passed!${NC}"
