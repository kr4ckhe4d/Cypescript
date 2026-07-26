// 03 — Extending a game with your own C
//
// The game runtime that ships with cscript is itself just `declare` bindings
// over a C shim. This example shows you can do exactly the same thing yourself,
// in your own project, without touching the compiler's build at all:
//
//     link source "native/particles.c";
//
// That one line compiles the C file alongside the program. No library to build
// first, no header to write, no CMake target, no wrapper script.
//
//   cscript -r example/game/03_native_extension.csc
//   CYPS_HEADLESS=1 CYPS_FRAMES=300 cscript -r example/game/03_native_extension.csc
//
// Click or press SPACE to emit particles.

import { } from "game";

link source "native/particles.c";

// The C side, declared exactly like any other foreign function. `ptr` holds the
// simulation handle — Cypescript never needs to know what a Field is.
declare function particles_new(seed: i32): ptr;
declare function particles_free(field: ptr): void;
declare function particles_emit(field: ptr, x: f64, y: f64, amount: i32): void;
declare function particles_step(field: ptr, dt: f64): void;
declare function particles_count(field: ptr): i32;
declare function particles_x(field: ptr, index: i32): f64;
declare function particles_y(field: ptr, index: i32): f64;
declare function particles_life(field: ptr, index: i32): f64;

const WIDTH: f64 = 800.0;
const HEIGHT: f64 = 520.0;

quietLogs();
enableFrameStrings();
openWindow(800, 520, "Cypescript — native extension");
setTargetFps(60);

let field: ptr = particles_new(20260725);

let colBg: i32 = rgb(12, 14, 22);
let colText: i32 = rgb(210, 215, 235);
let colDim: i32 = rgb(110, 116, 140);

let frames: i32 = 0;
let emitted: i32 = 0;

while (!windowShouldClose()) {
    let dt: f64 = deltaTime();
    frames += 1;

    // With no one at the keyboard, emit on a timer so CI and screenshots see
    // something happening.
    let firing: boolean = isMousePressed(MOUSE_LEFT) || isKeyDown(KEY_SPACE);
    let x: f64 = mouseX();
    let y: f64 = mouseY();

    if (isDemoMode()) {
        firing = frames % 12 == 0;
        x = WIDTH / 2.0 + Math.cos(frames * 0.05) * 220.0;
        y = HEIGHT / 2.0 + Math.sin(frames * 0.07) * 120.0;
    }

    if (firing) {
        particles_emit(field, x, y, 24);
        emitted += 24;
    }

    // The simulation runs entirely in C
    particles_step(field, dt);

    beginFrame();
    clearScreen(colBg);

    let count: i32 = particles_count(field);
    let i: i32 = 0;
    while (i < count) {
        let life: f64 = particles_life(field, i);
        let shade: i32 = 90 + life * 140.0;
        drawCircle(particles_x(field, i), particles_y(field, i), 2.0 + life * 2.0,
                   rgb(255, shade, 60));
        i += 1;
    }

    drawText(`PARTICLES ${count}`, 16.0, 12.0, 20, colText);
    drawText(`EMITTED ${emitted}`, 240.0, 12.0, 20, colDim);
    drawText("click or hold SPACE", 16.0, HEIGHT - 30.0, 18, colDim);

    endFrame();
}

particles_free(field);
closeWindow();

println(`Native extension ended — ${emitted} particles emitted over ${frames} frames`);
