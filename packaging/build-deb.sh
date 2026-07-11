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
# Runtime dependency: clang (cscript shells out to clang++ to link executables)
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

INSTALLED_SIZE="$(du -sk "$STAGE" | cut -f1)"
cat > "$STAGE/DEBIAN/control" << EOF
Package: cypescript
Version: ${VERSION}
Section: devel
Priority: optional
Architecture: ${ARCH}
Depends: clang
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
