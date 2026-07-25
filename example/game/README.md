# Game examples

Native games written in Cypescript, drawing through `lib/game.csc` — a module of
`declare function` bindings to the C shim in `runtime/game/cyps_game.c`, which wraps
raylib. **The compiler has no built-in knowledge of graphics**: adding an API means one
line in `lib/game.csc` and one function in the shim.

## Running them

No dependencies to install — raylib is vendored, so CMake builds it from source and links
it statically. From the repository root:

```bash
cmake -S . -B build && cmake --build build --parallel
```

```bash
./build/cscript -r example/game/01_breakout.csc
```

> ⚠️ **Use `./build/cscript`, not a bare `cscript`.**
> If you have installed Cypescript before (Homebrew, `.deb`, `make install`), a bare
> `cscript` runs *that* copy, which will not have the game runtime or the bundled `game`
> module and fails with:
>
> ```
> ✗ Error: Imported module not found: example/game/game.csc
> ```
>
> Check which one you are running with `cscript --version` — the game runtime needs
> **1.1.0 or newer**. To make the new build the one on your PATH:
>
> ```bash
> cmake --install build --prefix /opt/homebrew
> ```

Build with `-DCYPESCRIPT_VENDOR_RAYLIB=OFF` to link a system raylib instead, or
`-DCYPESCRIPT_BUILD_GAME_RUNTIME=OFF` to skip the game runtime entirely — the compiler
and the full language test suite work either way.

## Examples

| File | What it shows |
|---|---|
| `00_window.csc` | Opening a window, the frame loop, text, colors |
| `01_breakout.csc` | A complete game: paddle, ball physics, brick field, scoring, lives, sound |

```bash
./build/cscript -r example/game/00_window.csc
./build/cscript -r example/game/01_breakout.csc
```

**Breakout controls:** LEFT/RIGHT or A/D to move, SPACE to launch, R to restart, ESC to quit.

## Headless mode

`CYPS_HEADLESS=1` runs the whole game loop with no window and no drawing, and `CYPS_FRAMES`
caps the run length. The simulation still executes, so games are testable in CI:

```bash
CYPS_HEADLESS=1 CYPS_FRAMES=1200 cscript -r example/game/01_breakout.csc
# Breakout ended — score 290, 1200 frames, 51 bricks left
```

Breakout checks `isHeadless()` and plays itself when there is no keyboard, which is how
`tests/run_game_tests.sh` asserts that collision and scoring actually work.

## A note on style

Entities live in **parallel arrays** rather than an array of objects. That is not
preference — objects cannot yet be returned from a function or stored in an array. Fixing
that is Phase 3 in [GAME_ROADMAP.md](../../GAME_ROADMAP.md), after which these examples get
rewritten idiomatically.

## Writing your own

```ts
import { } from "game";        // ships with the compiler, imported by bare name

openWindow(640, 360, "my game");
setTargetFps(60);

let x: f64 = 100.0;

while (!windowShouldClose()) {
    let dt: f64 = deltaTime();
    if (isKeyDown(KEY_LEFT))  { x -= 200.0 * dt; }
    if (isKeyDown(KEY_RIGHT)) { x += 200.0 * dt; }

    beginFrame();
    clearScreen(rgb(20, 20, 30));
    drawRect(x, 100.0, 80.0, 40.0, rgb(255, 140, 60));
    drawText(`x = ${x}`, 20.0, 20.0, 20, rgb(200, 200, 220));
    endFrame();
}

closeWindow();
```

Colors are packed `i32` values — build them with `rgb(r, g, b)` or `rgba(r, g, b, a)`.
Sound effects can be synthesised with `makeTone(freq, ms, volume)`, so a game needs no
asset files at all.

### API summary

| Area | Functions |
|---|---|
| Window | `openWindow`, `closeWindow`, `windowShouldClose`, `windowWidth/Height`, `setTargetFps`, `setExitKey`, `quietLogs`, `isHeadless` |
| Frame | `beginFrame`, `endFrame`, `clearScreen`, `deltaTime`, `elapsedTime`, `currentFps` |
| Draw | `drawRect`, `drawRectOutline`, `drawCircle`, `drawLine`, `drawText`, `measureText` |
| Color | `rgb`, `rgba` |
| Textures | `loadTexture`, `unloadTexture`, `textureWidth/Height`, `drawTexture`, `drawSprite` |
| Input | `isKeyDown`, `isKeyPressed`, `isKeyReleased`, `mouseX/Y`, `isMouseDown`, `isMousePressed`, `hasGamepad`, `isGamepadDown`, `gamepadAxis` |
| Audio | `initAudio`, `closeAudio`, `loadSound`, `makeTone`, `playSound`, `setSoundVolume`, `unloadSound` |
