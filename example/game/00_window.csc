// Milestone 1 — a native window, driven entirely from Cypescript.
// Nothing here is built into the compiler: every call below is a
// `declare function` in lib/game.csc bound to the C shim.

import { } from "game";

const WIDTH: i32 = 640;
const HEIGHT: i32 = 360;

quietLogs();
openWindow(WIDTH, HEIGHT, "Cypescript — hello, window");
setTargetFps(60);

let frames: i32 = 0;
let background: i32 = rgb(18, 18, 28);
let accent: i32 = rgb(120, 200, 255);

while (!windowShouldClose()) {
    frames += 1;

    beginFrame();
    clearScreen(background);
    drawText("Hello from Cypescript", 40.0, 140.0, 32, accent);
    drawText(`frame ${frames}`, 40.0, 190.0, 20, rgb(140, 140, 160));
    endFrame();
}

closeWindow();
println(`ran ${frames} frames`);
