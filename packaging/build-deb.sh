#!/bin/bash
# Builds a binary .deb package for Debian/Ubuntu on the current architecture.
#
# Usage:
#   sudo apt-get install -y cmake llvm-dev clang
#   bash packaging/build-deb.sh              # version read from the built compiler
#   sudo apt install ./cypescript_<ver>_<arch>.deb
#
# Layout installed by the package:
#   /usr/bin/cscript             the compiler (resolves the runtime via ../lib)
#   /usr/lib/libcypescript.a     precompiled runtime, linked into every program
# Runtime dependencies: clang (cscript shells out to clang++ to link executables),
# plus whatever shared libraries the compiler itself was linked against — computed
# from the binary, not hand-maintained. See the note above shlib_depends().
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

if ! command -v dpkg-deb >/dev/null; then
    echo "error: dpkg-deb not found — this script must run on a Debian-based system" >&2
    exit 1
fi

echo "==> Building compiler"
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

VERSION="${1:-$(./build/cscript --version | awk '{print $2}')}"
ARCH="$(dpkg --print-architecture)"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

echo "==> Staging cypescript ${VERSION} (${ARCH})"
mkdir -p "$STAGE/usr/bin" "$STAGE/usr/lib" \
         "$STAGE/usr/share/doc/cypescript" "$STAGE/DEBIAN"
install -m755 build/cscript          "$STAGE/usr/bin/cscript"
install -m644 build/libcypescript.a  "$STAGE/usr/lib/libcypescript.a"
install -m644 LICENSE                "$STAGE/usr/share/doc/cypescript/copyright"
install -m644 README.md              "$STAGE/usr/share/doc/cypescript/README.md"

# The game runtime is optional — include it only when it was actually built,
# otherwise a .deb-installed cscript could not compile the game examples.
if [[ -f build/libcypescript_game.a ]]; then
    echo "==> Including the game runtime"
    install -m644 build/libcypescript_game.a "$STAGE/usr/lib/libcypescript_game.a"
    mkdir -p "$STAGE/usr/lib/cypescript"
    install -m644 lib/game.csc "$STAGE/usr/lib/cypescript/game.csc"
    # The vendored raylib, so `link "raylib";` resolves without a system package
    if [[ -f build/libraylib.a ]]; then
        install -m644 build/libraylib.a "$STAGE/usr/lib/libraylib.a"
        install -m644 THIRD_PARTY.md \
                "$STAGE/usr/share/doc/cypescript/THIRD_PARTY.md"
    fi
fi

# --- Runtime dependencies ---
# Where LLVM is packaged with LLVM_LINK_LLVM_DYLIB=ON — Debian and Ubuntu both are
# — cscript carries a NEEDED entry for libLLVM.so.<major>. Declaring only `clang`
# left that implicit: `clang` is an unversioned metapackage, so on a release where
# it resolves to a different LLVM major the package installs cleanly and then fails
# to start on a missing .so. CI cannot catch that, because the machine that builds
# the package already has llvm-dev on it.
#
# So ask dpkg what the binary actually needs. dpkg-shlibdeps insists on a source
# tree, so hand it a throwaway one.
shlib_depends() {
    local tmp deps
    command -v dpkg-shlibdeps >/dev/null || return 0
    tmp="$(mktemp -d)"
    mkdir -p "$tmp/debian"
    cat > "$tmp/debian/control" << 'CTL'
Source: cypescript

Package: cypescript
Architecture: any
Depends: ${shlibs:Depends}
CTL
    deps="$(cd "$tmp" && dpkg-shlibdeps -O --ignore-missing-info \
                "$ROOT_DIR/build/cscript" 2>/dev/null \
            | sed -n 's/^shlibs:Depends=//p')"
    rm -rf "$tmp"
    printf '%s' "$deps"
}

DEPENDS="clang"
SHLIB_DEPENDS="$(shlib_depends || true)"
if [[ -n "$SHLIB_DEPENDS" ]]; then
    DEPENDS="${DEPENDS}, ${SHLIB_DEPENDS}"
else
    # dpkg-shlibdeps missing or silent: name the LLVM runtime package after the
    # soname the binary was actually linked against. Empty when LLVM was linked
    # statically, which is correct — then there is nothing extra to depend on.
    LLVM_MAJOR="$(readelf -d build/cscript 2>/dev/null \
                  | sed -n 's/.*libLLVM\.so\.\([0-9][0-9]*\).*/\1/p' | head -n1)"
    if [[ -n "${LLVM_MAJOR:-}" ]]; then
        DEPENDS="${DEPENDS}, libllvm${LLVM_MAJOR}"
    fi
fi
echo "==> Runtime dependencies: ${DEPENDS}"

INSTALLED_SIZE="$(du -sk "$STAGE" | cut -f1)"
cat > "$STAGE/DEBIAN/control" << EOF
Package: cypescript
Version: ${VERSION}
Section: devel
Priority: optional
Architecture: ${ARCH}
Depends: ${DEPENDS}
Installed-Size: ${INSTALLED_SIZE}
Maintainer: Nipuna H Herath <hash.crackhead@gmail.com>
Homepage: https://github.com/kr4ckhe4d/Cypescript
Description: TypeScript-style language that compiles to native code via LLVM
 Cypescript compiles a near-verbatim TypeScript subset (classes, interfaces,
 generics, closures, exceptions, modules) to native executables with
 Rust-class performance. This package installs the cscript compiler and its
 precompiled runtime library.
EOF

OUT="cypescript_${VERSION}_${ARCH}.deb"
echo "==> Building ${OUT}"
dpkg-deb --build --root-owner-group "$STAGE" "$OUT"

echo "==> Done. Install with:  sudo apt install ./${OUT}"
echo "==> Smoke test:          echo 'println(\"hi\");' > /tmp/hi.csc && cscript -r /tmp/hi.csc"
