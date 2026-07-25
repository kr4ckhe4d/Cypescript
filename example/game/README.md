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
| `02_asteroids.csc` | Entity objects — classes in arrays, spawned and despawned at runtime |

```bash
./build/cscript -r example/game/00_window.csc
./build/cscript -r example/game/01_breakout.csc
./build/cscript -r example/game/02_asteroids.csc
```

**Breakout controls:** LEFT/RIGHT or A/D to move, SPACE to launch, R to restart, ESC to quit.

**Asteroids controls:** LEFT/RIGHT rotate, UP thrusts, SPACE fires, R restarts, ESC quits.

## Headless mode

`CYPS_HEADLESS=1` runs the whole game loop with no window and no drawing, and `CYPS_FRAMES`
caps the run length. The simulation still executes, so games are testable in CI:

```bash
CYPS_HEADLESS=1 CYPS_FRAMES=1200 cscript -r example/game/01_breakout.csc
# Breakout ended — score 290, 1200 frames, 51 bricks left
```

Breakout checks `isHeadless()` and plays itself when there is no keyboard, which is how
`tests/run_game_tests.sh` asserts that collision and scoring actually work.

## Two styles, on purpose

**Breakout** keeps its entities in parallel arrays (`brickX[]`, `brickY[]`, `brickAlive[]`).
That was the only option when it was written — objects could not be returned from a
function or stored in an array. It is kept as-is because a fixed entity count genuinely
suits struct-of-arrays, and it shows what the language could do before Phase 3.

**Asteroids** uses real objects, which is what you'd reach for now:

```ts
class Rock {
    x: f64; y: f64; vx: f64; vy: f64; radius: f64; size: i32;
    constructor(x: f64, y: f64, vx: f64, vy: f64, size: i32) { ... }
    step(dt: f64): void { this.x += this.vx * dt; this.y += this.vy * dt; }
}

let rocks: Rock[] = [];
rocks.push(spawnRock(3));          // a function returning an object

let rock: Rock = rocks[i];
rock.step(dt);                     // mutation through the array
rocks.removeAt(i);                 // despawning
```

Objects are heap-allocated, so they outlive the function that built them.

## Memory

Both games hold **flat memory** — steady RSS from 2,000 to 300,000 headless frames, which
is 83 minutes of play at 60 fps. Two things make that true, and a game you write needs both:

**1. Frame-scoped strings.** A HUD rebuilt every frame used to leak ~100 bytes per frame.
Opt in once, before the loop:

```ts
enableFrameStrings();
```

Strings built by template literals and `+` then live only until the next `beginFrame()`.
The catch: anything that must outlive its frame — a string stored in an object field, or
pushed to an array you keep — must be copied out with `persist()`. Strings built *before*
the first `beginFrame()` are kept automatically, so start-up labels are fine as they are.

**2. Pool your entities.** Nothing frees a heap object when it leaves an array, and nothing
safely can — other references may still point at it. Asteroids allocates every rock and
bullet once at start-up and reuses them:

```ts
class Bullet {
    x: f64 = 0.0; y: f64 = 0.0; active: boolean = false;
    spawn(x: f64, y: f64): void { this.x = x; this.y = y; this.active = true; }
}

// Allocate once...
let bullets: Bullet[] = [];
let i: i32 = 0;
while (i < MAX_BULLETS) { bullets.push(new Bullet()); i += 1; }

// ...then reuse. Despawning is a flag, not a deallocation.
function takeBullet(pool: Bullet[]): Bullet {
    let i: i32 = 0;
    while (i < pool.length) {
        let candidate: Bullet = pool[i];
        if (!candidate.active) { return candidate; }
        i += 1;
    }
    return null;
}
```

`removeAt()` and `clear()` still exist and are the right tool when entity churn is not in
the hot path. See [GAME_ROADMAP.md](../../GAME_ROADMAP.md) Phase 4 for the measurements.

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
