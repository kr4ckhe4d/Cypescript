# Screenshots

These are real frames, rendered by the games themselves — not screen grabs. The runtime
draws into an offscreen render texture and exports a PNG, so capture works unattended and
does not depend on the window being frontmost, composited, or even visible.

## Regenerating

From the repository root, after building:

```bash
cscript example/game/01_breakout.csc -o /tmp/shot_bo
cscript example/game/02_asteroids.csc -o /tmp/shot_ast

cd docs/images
CYPS_DEMO=1 CYPS_SCREENSHOT=breakout.png  CYPS_SCREENSHOT_FRAME=300 CYPS_FRAMES=320 /tmp/shot_bo
CYPS_DEMO=1 CYPS_SCREENSHOT=asteroids.png CYPS_SCREENSHOT_FRAME=500 CYPS_FRAMES=520 /tmp/shot_ast
```

| Variable | Meaning |
|---|---|
| `CYPS_DEMO=1` | The game plays itself, so the captured frame shows actual gameplay |
| `CYPS_SCREENSHOT` | Where to write the PNG |
| `CYPS_SCREENSHOT_FRAME` | Which frame to capture — pick one where the game looks interesting |
| `CYPS_FRAMES` | Frame cap, so the run ends on its own |

Any game can be captured this way; nothing in the game's source is involved. A game that
wants to trigger a capture itself can call `screenshot("path.png")` from `lib/game.csc`.
