# Third-party components

Cypescript itself is MIT licensed (see [LICENSE](LICENSE)). It builds against and
redistributes the components below.

---

## raylib

**Used for:** the optional game runtime (`runtime/game/cyps_game.c` wraps it; games
reach it through `lib/game.csc`).

**How it is included:** by default CMake downloads raylib 5.5 from source and links it
statically, so a `cscript` install is self-contained and needs no system package.
Build with `-DCYPESCRIPT_VENDOR_RAYLIB=OFF` to link a system raylib instead, or with
`-DCYPESCRIPT_BUILD_GAME_RUNTIME=OFF` to leave raylib out entirely.

**Upstream:** https://github.com/raysan5/raylib

**License:** zlib/libpng — permissive and compatible with MIT. It requires that the
notice below travels with binary redistributions, which is why this file exists and is
installed alongside the compiler.

```
Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)

This software is provided "as-is", without any express or implied warranty. In no event
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to the
following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you
     wrote the original software. If you use this software in a product, an
     acknowledgment in the product documentation would be appreciated but is not
     required.

  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
```

raylib bundles its own dependencies (GLFW, miniaudio, stb libraries), each under
permissive licenses — see `raylib/LICENSE` in the fetched source tree.

---

## LLVM

**Used for:** code generation. Linked against, not redistributed — building Cypescript
requires LLVM to be installed separately.

**License:** Apache 2.0 with LLVM Exceptions.

---

## Obligations when you distribute a Cypescript binary

If you ship `cscript` with the game runtime included, ship this file too. The Cypescript
install target does that automatically (`<prefix>/share/cypescript/THIRD_PARTY.md`).
