#!/bin/bash
# Game runtime tests.
#
# These need the optional game runtime (libcypescript_game.a, built when raylib
# is installed). They run the examples in headless mode — CYPS_HEADLESS=1 means
# no window is created and drawing is skipped, so the whole game loop, physics
# and scoring still execute and can be asserted in CI.
#
# Skips cleanly (exit 0) when the game runtime was not built.
#
# Deliberately NOT `set -e`: this script's job is to report the outcome of every
# check. Under `set -e` a single non-zero command — a pipeline whose grep found
# nothing, say — aborts the run with no output at all, which reads as a failing
# step with no explanation.

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
# Environment, so a CI failure is diagnosable from the log alone rather than
# needing someone to reproduce it locally on that platform.
echo "  platform:     $(uname -s) $(uname -m)"
echo "  compiler:     $COMPILER ($("$COMPILER" --version 2>/dev/null))"
echo "  game runtime: $GAME_RUNTIME"
if [[ -x /usr/bin/time ]]; then
    echo "  /usr/bin/time: present (RSS checks enabled)"
else
    echo "  /usr/bin/time: missing (RSS checks will skip)"
fi
echo "--------------------------------------------"

# Compiles a game, reporting the compiler's own output if it fails
compile_game() {
    local source="$1" output="$2" log
    if ! log=$("$COMPILER" -o "$output" "$source" 2>&1); then
        echo -e "  ${RED}❌ FAILED TO COMPILE${NC} $source"
        echo "$log" | sed 's/^/      /' | tail -15
        FAIL=$((FAIL + 1))
        return 1
    fi
    return 0
}

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
compile_game "$ROOT_DIR/example/game/00_window.csc" "$BIN_DIR/window" || exit 1
out=$(CYPS_HEADLESS=1 CYPS_FRAMES=45 "$BIN_DIR/window" 2>&1) || true
check "00_window runs 45 frames" "$out" "ran 45 frames"

# --- 01_breakout: self-play must actually break bricks and score ---
compile_game "$ROOT_DIR/example/game/01_breakout.csc" "$BIN_DIR/breakout" || exit 1
out=$(CYPS_HEADLESS=1 CYPS_FRAMES=1200 "$BIN_DIR/breakout" 2>&1) || true
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

# --- 02_asteroids: heap objects in arrays, spawned and despawned live ---
compile_game "$ROOT_DIR/example/game/02_asteroids.csc" "$BIN_DIR/asteroids" || exit 1
out=$(CYPS_HEADLESS=1 CYPS_FRAMES=12000 "$BIN_DIR/asteroids" 2>&1) || true
check "02_asteroids completes a run" "$out" "Asteroids ended"

# Rocks are shot, split into smaller rocks, and removed from the array; clearing
# a wave spawns the next one. Reaching a later wave proves the whole object
# lifecycle works — spawn, store in Rock[], mutate, remove.
wave=$(echo "$out" | sed -n 's/.*wave \([0-9]*\),.*/\1/p')
printf "  %-32s" "02_asteroids clears waves"
if [[ -n "$wave" && "$wave" -gt 1 ]]; then
    echo -e "${GREEN}✅ PASS${NC} (reached wave ${wave})"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (wave: '${wave}', expected > 1)"
    FAIL=$((FAIL + 1))
fi

score=$(echo "$out" | sed -n 's/.*score \([0-9]*\),.*/\1/p')
printf "  %-32s" "02_asteroids scores points"
if [[ -n "$score" && "$score" -gt 0 ]]; then
    echo -e "${GREEN}✅ PASS${NC} (score ${score})"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (score: '${score}', expected > 0)"
    FAIL=$((FAIL + 1))
fi

# --- 03_native_extension: a game bringing its own C via `link source` ---
compile_game "$ROOT_DIR/example/game/03_native_extension.csc" "$BIN_DIR/native_ext" || exit 1
out=$(CYPS_HEADLESS=1 CYPS_FRAMES=300 "$BIN_DIR/native_ext" 2>&1) || true
check "03_native_extension runs" "$out" "Native extension ended"

emitted=$(echo "$out" | sed -n 's/.*— \([0-9]*\) particles emitted.*/\1/p')
printf "  %-32s" "03 drives its C simulation"
if [[ -n "$emitted" && "$emitted" -gt 0 ]]; then
    echo -e "${GREEN}✅ PASS${NC} (${emitted} particles)"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (emitted: '${emitted}', expected > 0)"
    FAIL=$((FAIL + 1))
fi

# --- Memory: a long run must not grow -----------------------------------------
# The whole point of Phase 4. Peak RSS is sampled at a short and a long run; a
# leak shows up as the long run using materially more memory than the short one.
# Frame-scoped strings and entity pooling should make the two nearly identical.
peak_rss_mb() {
    # BSD time (-l) reports peak RSS in bytes; GNU time (-v) in kilobytes.
    # Try both, since neither flag is understood by the other implementation.
    local binary="$1" frames="$2" raw="" divisor=1048576 flag="-l"
    # GNU time wants -v and reports kilobytes; BSD time wants -l and reports bytes
    if [[ "$(uname)" != "Darwin" ]]; then
        flag="-v"
        divisor=1024
    fi
    if [[ ! -x /usr/bin/time ]]; then
        echo ""
        return 0
    fi
    raw=$(CYPS_HEADLESS=1 CYPS_FRAMES="$frames" /usr/bin/time "$flag" "$binary" 2>&1 \
          | grep -iE "maximum resident set size" \
          | grep -oE "[0-9]+" | head -1) || true
    if [[ -z "$raw" ]]; then
        echo ""
        return 0
    fi
    awk -v v="$raw" -v d="$divisor" 'BEGIN { printf "%.2f", v / d }'
    return 0
}

for game in breakout asteroids; do
    short=$(peak_rss_mb "$BIN_DIR/$game" 2000)
    long=$(peak_rss_mb "$BIN_DIR/$game" 60000)
    printf "  %-32s" "$game memory stays flat"
    if [[ -z "$short" || -z "$long" ]]; then
        echo -e "${YELLOW}⏭  SKIP${NC} (could not measure RSS on this platform)"
    elif awk -v s="$short" -v l="$long" 'BEGIN { exit !(l <= s + 1.5) }'; then
        echo -e "${GREEN}✅ PASS${NC} (${short} MB at 2k frames, ${long} MB at 60k)"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}❌ FAIL${NC} (grew ${short} MB -> ${long} MB; a leak is back)"
        FAIL=$((FAIL + 1))
    fi
done

# --- Assets and bundling ------------------------------------------------------
# A distributed game is launched from anywhere, so relative asset paths must
# resolve against the binary rather than the working directory.
ASSET_DIR="$BIN_DIR/assetgame"
mkdir -p "$ASSET_DIR/assets"
echo "sample asset" > "$ASSET_DIR/assets/sprite.txt"
cat > "$ASSET_DIR/probe.csc" <<'PROBE'
import { } from "game";
println(assetPath("sprite.txt"));
PROBE

compile_game "$ASSET_DIR/probe.csc" "$ASSET_DIR/probe" || exit 1
# Run from a directory that has no assets/, so a cwd-relative lookup would fail
out=$(cd / && CYPS_HEADLESS=1 "$ASSET_DIR/probe" 2>&1) || true
printf "  %-32s" "assets resolve next to binary"
if [[ "$out" == *"assetgame/assets/sprite.txt" ]]; then
    echo -e "${GREEN}✅ PASS${NC}"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (got '${out}')"
    FAIL=$((FAIL + 1))
fi

# --bundle packages the binary with its assets where the runtime looks for them
bundle_log=$("$COMPILER" --bundle -o "$ASSET_DIR/Probe" "$ASSET_DIR/probe.csc" 2>&1) || {
    echo -e "  ${RED}❌ --bundle failed to compile${NC}"
    echo "$bundle_log" | sed 's/^/      /' | tail -10
}
printf "  %-32s" "--bundle produces a package"
if [[ "$(uname)" == "Darwin" ]]; then
    BUNDLE_BIN="$ASSET_DIR/Probe.app/Contents/MacOS/Probe"
    BUNDLE_ASSET="$ASSET_DIR/Probe.app/Contents/Resources/sprite.txt"
else
    BUNDLE_BIN="$ASSET_DIR/Probe-bundle/Probe"
    BUNDLE_ASSET="$ASSET_DIR/Probe-bundle/assets/sprite.txt"
fi
if [[ -x "$BUNDLE_BIN" && -f "$BUNDLE_ASSET" ]]; then
    echo -e "${GREEN}✅ PASS${NC}"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (missing binary or asset in the bundle)"
    FAIL=$((FAIL + 1))
fi

# ...and the packaged game finds those assets when run from elsewhere
out=$(cd / && CYPS_HEADLESS=1 "$BUNDLE_BIN" 2>&1) || true
printf "  %-32s" "bundled game finds its assets"
if [[ -n "$out" && -f "$out" ]]; then
    echo -e "${GREEN}✅ PASS${NC}"
    PASS=$((PASS + 1))
else
    echo -e "${RED}❌ FAIL${NC} (resolved to '${out}', which does not exist)"
    FAIL=$((FAIL + 1))
fi

echo "============================================"
echo -e "  ${GREEN}Passed: $PASS${NC}  ${RED}Failed: $FAIL${NC}  Total: $((PASS + FAIL))"

if [[ $FAIL -gt 0 ]]; then
    exit 1
fi
echo -e "\n${GREEN}🎉 All game tests passed!${NC}"
