#!/bin/bash
# README snippet tests.
#
# Every Cypescript snippet in README.md is compiled. A snippet that does not
# compile is a snippet nobody has run, and this project has already shipped
# three of those — including one whose `super()` call exposed a real compiler
# bug when it was finally tried.
#
# A snippet that genuinely cannot stand alone — an FFI fragment naming a C
# symbol that does not exist, an `import` needing a sibling module — opts out
# with an HTML comment on the line before its fence:
#
#     <!-- snippet: illustrative — runnable version in example/21_c_interop.csc -->
#
# The comment is invisible in rendered Markdown, and it must name a path that
# exists. So opting out is not a way to avoid having an example; it is how a
# snippet declares which example backs it, and the path is checked.
#
# No `set -e`: a failing compile is data to report, not a reason to abort.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

COMPILER=""
for candidate in "$ROOT_DIR/build/cscript" "$ROOT_DIR/build/cscript.exe" \
                 "$ROOT_DIR/build/Release/cscript.exe"; do
    [[ -f "$candidate" ]] && COMPILER="$candidate" && break
done
if [[ -z "$COMPILER" ]]; then
    echo -e "${RED}❌ Compiler not found. Run ./build.sh first${NC}"
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo -e "${YELLOW}⏭  python3 not available — skipping README snippet tests${NC}"
    exit 0
fi

echo -e "${CYAN}📖 README Snippet Tests${NC}"
echo "============================================"

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

# Extract each snippet to its own file, plus an index of
#   <name> <TAB> <readme line> <TAB> <marker text or "-">
python3 - "$ROOT_DIR/README.md" "$WORK_DIR" <<'PYTHON'
import re, sys, os

readme, work = sys.argv[1], sys.argv[2]
lines = open(readme, encoding="utf-8").read().split("\n")

index = []
i = 0
while i < len(lines):
    fence = re.match(r"^```(typescript|ts)\s*$", lines[i])
    if not fence:
        i += 1
        continue

    # A marker sits on the line above the fence, or the one above a blank line
    marker = "-"
    for back in (1, 2):
        if i - back >= 0:
            m = re.match(r"^\s*<!--\s*snippet:\s*(.*?)\s*-->\s*$", lines[i - back])
            if m:
                marker = m.group(1)
                break

    start = i + 1
    body = []
    i += 1
    while i < len(lines) and lines[i].strip() != "```":
        body.append(lines[i])
        i += 1
    i += 1

    name = "L%04d" % start
    with open(os.path.join(work, name + ".csc"), "w", encoding="utf-8") as f:
        f.write("\n".join(body) + "\n")
    index.append("%s\t%d\t%s" % (name, start, marker))

with open(os.path.join(work, "index.tsv"), "w", encoding="utf-8") as f:
    f.write("\n".join(index) + "\n")
PYTHON

PASS=0
SKIP=0
FAIL=0
ERRORS=""

while IFS=$'\t' read -r name line marker; do
    [[ -z "$name" ]] && continue
    label="README:$line"
    printf "  %-22s" "$label"

    if [[ "$marker" != "-" ]]; then
        # Opting out is allowed, but the example it points at must exist.
        missing=""
        for token in $marker; do
            case "$token" in
                */*)
                    token="${token%%,}"; token="${token%%.}"; token="${token%%)}"
                    [[ -e "$ROOT_DIR/$token" ]] || missing="$token"
                    ;;
            esac
        done
        if [[ -n "$missing" ]]; then
            echo -e "${RED}❌ FAIL${NC} (marker points at missing path: $missing)"
            FAIL=$((FAIL + 1))
            ERRORS="$ERRORS\n  - $label: marker names '$missing', which does not exist"
        else
            echo -e "${YELLOW}⏭  SKIP${NC} (illustrative)"
            SKIP=$((SKIP + 1))
        fi
        continue
    fi

    if output=$("$COMPILER" -o "$WORK_DIR/.bin_$name" "$WORK_DIR/$name.csc" 2>&1); then
        echo -e "${GREEN}✅ PASS${NC}"
        PASS=$((PASS + 1))
    else
        reason=$(echo "$output" | grep -E "Error" | head -1 | sed 's/\x1b\[[0-9;]*m//g' | cut -c1-90)
        echo -e "${RED}❌ FAIL${NC}"
        FAIL=$((FAIL + 1))
        ERRORS="$ERRORS\n  - $label: ${reason:-did not compile}"
    fi
    rm -f "$WORK_DIR/.bin_$name"
done < "$WORK_DIR/index.tsv"

echo "============================================"
echo -e "  ${GREEN}Compiled: $PASS${NC}  ${YELLOW}Illustrative: $SKIP${NC}  ${RED}Failed: $FAIL${NC}"

if [[ $FAIL -gt 0 ]]; then
    echo -e "${RED}"
    echo "Failures:"
    echo -e "$ERRORS${NC}"
    echo ""
    echo "Either make the snippet self-contained, or mark it with"
    echo "  <!-- snippet: illustrative — runnable version in example/<file>.csc -->"
    echo "on the line above its fence, naming an example that exists."
    exit 1
fi

echo -e "${GREEN}🎉 Every README snippet compiles or names its example${NC}"
exit 0
