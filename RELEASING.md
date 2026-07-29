# Releasing Cypescript

The complete runbook for cutting a release: versioning, tagging, sha256
generation, Homebrew, Linux packages, and the VSCode extension.
Written after shipping v1.0.0 (2026-07-08).

## 0. Prerequisites (one-time)

- Push access to `kr4ckhe4d/Cypescript` and `kr4ckhe4d/homebrew-cypescript`
- `vsce` for the VSCode extension: `npm install -g @vscode/vsce`
- On Linux build machines: `cmake`, `llvm-dev` (or `llvm`), `clang`

## 1. Bump the version

The single source of truth is the `project(... VERSION x.y.z ...)` line in
`CMakeLists.txt`. It feeds `cscript --version` via a compile definition.

```bash
# edit CMakeLists.txt:  project(Cypescript VERSION 1.1.0 ...)
./build.sh
./build/cscript --version    # must print the new version
```

Also bump, if they changed this release:
- `vscode-extension/package.json` → `"version"`
- `packaging/cypescript.rb` → the `v<x.y.z>` in `url` (sha256 comes later)
- `packaging/arch/PKGBUILD` → `pkgver` (sha256 comes later)

## 2. Run the full gauntlet

```bash
./build.sh
bash tests/run_tests.sh                        # all tests, output-asserted
for f in example/*.csc example/14_modules/main.csc; do
    case "$f" in *math_utils*) continue;; esac
    ./build/cscript -r "$f" > /dev/null || echo "FAIL $f"
done
bash benchmarks/cross/run_cross_benchmarks.sh 1  # sanity, not regression
```

CI runs the same steps on every push — do not tag with a red CI.

## 3. Commit, tag, push

```bash
git add -A && git commit -m "release: v1.1.0"
git push origin main
# wait for CI to go green on this commit, then:
git tag -a v1.1.0 -m "Cypescript v1.1.0 — <one-line summary>"
git push origin v1.1.0
```

**Never move or re-tag a published tag** — the tarball sha256 below is derived
from it, and Homebrew/Arch will refuse mismatched checksums. If something is
wrong, cut a new patch version.

## 4. Generate the tarball sha256

GitHub serves a source tarball for every tag at a stable URL. All package
managers verify against its sha256:

```bash
VERSION=1.1.0
curl -sL "https://github.com/kr4ckhe4d/Cypescript/archive/refs/tags/v${VERSION}.tar.gz" -o "/tmp/cyps-${VERSION}.tar.gz"

# sanity: it must be a real archive, not an error page
tar -tzf "/tmp/cyps-${VERSION}.tar.gz" | head -1     # -> Cypescript-1.1.0/

shasum -a 256 "/tmp/cyps-${VERSION}.tar.gz"          # macOS
# sha256sum on Linux
```

Paste the hash into:
- `packaging/cypescript.rb` → `sha256 "..."`
- `packaging/arch/PKGBUILD` → `sha256sums=('...')`

Commit and push these (the tag does not need to contain its own checksum —
package files are distributed from `main` / the tap, not from the tarball).

## 5. GitHub release

On https://github.com/kr4ckhe4d/Cypescript/releases/new:
- choose the tag, title `v1.1.0`
- paste highlights (new features, breaking changes, benchmark deltas)
- optionally attach the CI artifacts (`cscript` + `libcypescript.a`) and the
  `.deb` produced in step 7

## 6. Homebrew (macOS)

The tap repo is `kr4ckhe4d/homebrew-cypescript`; the formula lives at
`Formula/cypescript.rb` there. `packaging/cypescript.rb` in this repo is the
master copy.

```bash
git clone git@github.com:kr4ckhe4d/homebrew-cypescript.git /tmp/tap
cp packaging/cypescript.rb /tmp/tap/Formula/cypescript.rb
cd /tmp/tap && git add -A && git commit -m "cypescript <x.y.z>" && git push
```

Verify as a user would:

```bash
brew update
brew upgrade cypescript        # or: brew install cypescript
brew test cypescript           # runs the formula's compile-and-run self-test
cscript --version
```

## 7. Linux packages

### Debian / Ubuntu (.deb)

`packaging/build-deb.sh` builds the compiler and assembles a binary `.deb`
(installs `/usr/bin/cscript` + `/usr/lib/libcypescript.a`, depends on `clang`):

```bash
sudo apt-get install -y cmake llvm-dev clang
bash packaging/build-deb.sh            # version read from cscript --version
sudo apt install ./cypescript_<x.y.z>_<arch>.deb
cscript --version
```

The Linux CI job also builds this `.deb` and uploads it as an artifact —
you can download it from the Actions run and attach it to the GitHub release
instead of building locally.

### Arch Linux (PKGBUILD)

`packaging/arch/PKGBUILD` builds from the tag tarball (update `pkgver` +
`sha256sums` per steps 1 and 4):

```bash
cd packaging/arch
makepkg -si          # builds, runs the test suite (check()), installs
```

To publish on the AUR: push the PKGBUILD (plus `.SRCINFO` from
`makepkg --printsrcinfo > .SRCINFO`) to an AUR git repo named `cypescript`.

### Why no prebuilt Linux binaries?

`cscript` shells out to `clang++` and links `libcypescript.a` from
`/usr/lib` — the packages carry both, but the compiler itself links against
the distro's LLVM, so source builds are the reliable path. Prebuilt binaries
would need static LLVM linking (future work).

## 8. VSCode extension

```bash
cd vscode-extension
# bump "version" in package.json first
vsce package --allow-missing-repository
```

Commit the new `.vsix`, remove the old one, and attach it to the GitHub
release. (Publishing to the VS Code Marketplace additionally needs a
publisher account + `vsce publish` — not set up yet.)

## 9. Post-release checklist

- [ ] CI green on the tagged commit
- [ ] `brew install cypescript` works on a clean machine
- [ ] `.deb` installs and `cscript -r` compiles a hello program
- [ ] README badges show the new version
- [ ] progress.md / ROADMAP.md updated

## Quick reference: published values

| Version | sha256 of the tag tarball |
|---|---|
| v1.0.0 | `d4279bbd9abf39c413e62bfbff660765603c6683e14eeedc94eaab9697f4fa6d` |
| v1.1.0 | `fe97686d62da0f1efac33b2b5a91620b2ceb13a6ffb1061c89342707f594faf4` |

Tarballs are at
`https://github.com/kr4ckhe4d/Cypescript/archive/refs/tags/v<x.y.z>.tar.gz`.

Two things worth doing that the steps above do not spell out. Download the
tarball **twice** and confirm the hash matches before publishing it — a truncated
download otherwise becomes a checksum no package manager can ever satisfy, and
the tag cannot be moved to fix it. And leave `sha256sums=('SKIP')` in the
PKGBUILD between step 1 and step 4 rather than an invented string: `SKIP` is a
value makepkg understands, so the file stays parseable in the window where the
real hash does not exist yet.
