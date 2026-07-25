// 01 — Breakout: a complete arcade game in Cypescript.
//
// Everything here is ordinary Cypescript. The graphics, input and audio calls
// are `declare function` bindings from lib/game.csc — the compiler has no
// built-in knowledge of any of them.
//
//   Run:      cscript -r example/game/01_breakout.csc
//   Headless: CYPS_HEADLESS=1 CYPS_FRAMES=300 cscript -r example/game/01_breakout.csc
//
// Controls: LEFT/RIGHT or A/D to move, SPACE to launch, R to restart, ESC to quit.
//
// NOTE ON STYLE: entities are held in parallel arrays (struct-of-arrays) rather
// than an array of objects, because objects cannot yet live in arrays or escape
// the function that created them. That is the Phase 3 work in GAME_ROADMAP.md;
// this file is deliberately written against what the language can do today.

import { } from "game";

const WIDTH: i32 = 800;
const HEIGHT: i32 = 600;

const PADDLE_W: f64 = 110.0;
const PADDLE_H: f64 = 16.0;
const PADDLE_Y: f64 = 545.0;
const PADDLE_SPEED: f64 = 620.0;

const BALL_R: f64 = 8.0;
const BALL_SPEED: f64 = 380.0;

const BRICK_COLS: i32 = 10;
const BRICK_ROWS: i32 = 6;
const BRICK_W: f64 = 72.0;
const BRICK_H: f64 = 26.0;
const BRICK_GAP: f64 = 6.0;
const BRICK_TOP: f64 = 70.0;
const BRICK_LEFT: f64 = 22.0;

// Game states
const STATE_READY: i32 = 0;
const STATE_PLAYING: i32 = 1;
const STATE_WON: i32 = 2;
const STATE_LOST: i32 = 3;

// HUD text is rebuilt every frame; frame-scoped strings make that free
// instead of leaking. See lib/game.csc on what this costs you.
enableFrameStrings();
quietLogs();
openWindow(WIDTH, HEIGHT, "Cypescript Breakout");
setTargetFps(60);
initAudio();
random_seed(20260725);

// Sounds are synthesised, so the game needs no asset files at all
let sndPaddle: ptr = makeTone(440.0, 60, 0.35);
let sndBrick: ptr = makeTone(880.0, 50, 0.30);
let sndWall: ptr = makeTone(330.0, 40, 0.25);
let sndLose: ptr = makeTone(140.0, 320, 0.40);

// --- Palette ---
let colBg: i32 = rgb(16, 18, 28);
let colPaddle: i32 = rgb(235, 235, 245);
let colBall: i32 = rgb(255, 196, 92);
let colText: i32 = rgb(220, 220, 235);
let colDim: i32 = rgb(120, 124, 145);
let colWall: i32 = rgb(40, 44, 62);

// Row colours, indexed by brick row
let rowColors: i32[] = [];
rowColors.push(rgb(239, 83, 80));
rowColors.push(rgb(255, 167, 38));
rowColors.push(rgb(255, 238, 88));
rowColors.push(rgb(102, 187, 106));
rowColors.push(rgb(66, 165, 245));
rowColors.push(rgb(171, 71, 188));

// --- Entity state (struct-of-arrays) ---
let brickX: f64[] = [];
let brickY: f64[] = [];
let brickRow: i32[] = [];
let brickAlive: i32[] = [];

let paddleX: f64 = 0.0;
let ballX: f64 = 0.0;
let ballY: f64 = 0.0;
let ballVX: f64 = 0.0;
let ballVY: f64 = 0.0;

let score: i32 = 0;
let lives: i32 = 3;
let bricksLeft: i32 = 0;
let state: i32 = STATE_READY;
let frames: i32 = 0;

// --- Build the brick field ---
let totalBricks: i32 = BRICK_COLS * BRICK_ROWS;
let b: i32 = 0;
while (b < totalBricks) {
    let row: i32 = b / BRICK_COLS;
    let col: i32 = b % BRICK_COLS;
    brickX.push(BRICK_LEFT + col * (BRICK_W + BRICK_GAP));
    brickY.push(BRICK_TOP + row * (BRICK_H + BRICK_GAP));
    brickRow.push(row);
    brickAlive.push(1);
    b += 1;
}
bricksLeft = totalBricks;

// Resets the paddle and parks the ball on top of it
paddleX = WIDTH / 2.0 - PADDLE_W / 2.0;
ballX = paddleX + PADDLE_W / 2.0;
ballY = PADDLE_Y - BALL_R - 2.0;

// =============================================================================
// Main loop
// =============================================================================
while (!windowShouldClose()) {
    let dt: f64 = deltaTime();
    frames += 1;

    // --- Input ---
    // With no window there is no keyboard, so the game plays itself: the paddle
    // chases the ball and launches on its own. This turns a headless run into a
    // real self-test of the physics, collisions and scoring.
    let autoplay: boolean = isDemoMode();

    let moveLeft: i32 = 0;
    let moveRight: i32 = 0;
    if (autoplay) {
        let target: f64 = ballX - PADDLE_W / 2.0;
        if (target < paddleX - 4.0) { moveLeft = 1; }
        if (target > paddleX + 4.0) { moveRight = 1; }
    } else {
        moveLeft = isKeyDown(KEY_LEFT) + isKeyDown(KEY_A);
        moveRight = isKeyDown(KEY_RIGHT) + isKeyDown(KEY_D);
    }

    if (moveLeft > 0) { paddleX -= PADDLE_SPEED * dt; }
    if (moveRight > 0) { paddleX += PADDLE_SPEED * dt; }

    // Keep the paddle on screen
    paddleX = Math.max(0.0, Math.min(paddleX, WIDTH - PADDLE_W));

    if (state == STATE_READY) {
        // Ball rides the paddle until launch
        ballX = paddleX + PADDLE_W / 2.0;
        ballY = PADDLE_Y - BALL_R - 2.0;

        let launch: boolean = isKeyPressed(KEY_SPACE);
        if (autoplay) { launch = true; }

        if (launch) {
            // Launch at a slight random angle, always upward
            let spread: f64 = (Math.random() - 0.5) * 0.8;
            ballVX = BALL_SPEED * spread;
            ballVY = 0.0 - BALL_SPEED;
            state = STATE_PLAYING;
        }
    }

    if (state == STATE_PLAYING) {
        ballX += ballVX * dt;
        ballY += ballVY * dt;

        // --- Walls ---
        if (ballX - BALL_R < 0.0) {
            ballX = BALL_R;
            ballVX = 0.0 - ballVX;
            playSound(sndWall);
        }
        if (ballX + BALL_R > WIDTH) {
            ballX = WIDTH - BALL_R;
            ballVX = 0.0 - ballVX;
            playSound(sndWall);
        }
        if (ballY - BALL_R < 40.0) {
            ballY = 40.0 + BALL_R;
            ballVY = 0.0 - ballVY;
            playSound(sndWall);
        }

        // --- Paddle ---
        // Only bounce while travelling downward, so the ball can't get stuck
        if (ballVY > 0.0) {
            if (ballY + BALL_R >= PADDLE_Y) {
                if (ballY - BALL_R <= PADDLE_Y + PADDLE_H) {
                    if (ballX >= paddleX) {
                        if (ballX <= paddleX + PADDLE_W) {
                            // Deflection angle depends on where it hit the paddle
                            let hit: f64 = (ballX - paddleX) / PADDLE_W - 0.5;
                            ballVX = BALL_SPEED * hit * 1.8;
                            ballVY = 0.0 - Math.sqrt(BALL_SPEED * BALL_SPEED - ballVX * ballVX);
                            ballY = PADDLE_Y - BALL_R - 1.0;
                            playSound(sndPaddle);
                        }
                    }
                }
            }
        }

        // --- Bricks ---
        let i: i32 = 0;
        while (i < totalBricks) {
            if (brickAlive[i] == 1) {
                let bx: f64 = brickX[i];
                let by: f64 = brickY[i];
                if (ballX + BALL_R >= bx) {
                    if (ballX - BALL_R <= bx + BRICK_W) {
                        if (ballY + BALL_R >= by) {
                            if (ballY - BALL_R <= by + BRICK_H) {
                                brickAlive[i] = 0;
                                bricksLeft -= 1;
                                score += 10 * (BRICK_ROWS - brickRow[i]);

                                // Bounce off whichever face was closer
                                let overlapX: f64 = Math.min(ballX + BALL_R - bx, bx + BRICK_W - ballX + BALL_R);
                                let overlapY: f64 = Math.min(ballY + BALL_R - by, by + BRICK_H - ballY + BALL_R);
                                if (overlapX < overlapY) {
                                    ballVX = 0.0 - ballVX;
                                } else {
                                    ballVY = 0.0 - ballVY;
                                }
                                playSound(sndBrick);
                                i = totalBricks; // one brick per frame keeps it fair
                            }
                        }
                    }
                }
            }
            i += 1;
        }

        // --- Ball lost ---
        if (ballY - BALL_R > HEIGHT) {
            lives -= 1;
            playSound(sndLose);
            if (lives <= 0) {
                state = STATE_LOST;
            } else {
                state = STATE_READY;
            }
        }

        if (bricksLeft <= 0) { state = STATE_WON; }
    }

    // --- Restart ---
    if (state == STATE_WON || state == STATE_LOST) {
        if (isKeyPressed(KEY_R)) {
            let j: i32 = 0;
            while (j < totalBricks) {
                brickAlive[j] = 1;
                j += 1;
            }
            bricksLeft = totalBricks;
            score = 0;
            lives = 3;
            state = STATE_READY;
        }
    }

    // =========================================================================
    // Draw
    // =========================================================================
    beginFrame();
    clearScreen(colBg);

    // Playfield border
    drawLine(0.0, 40.0, WIDTH, 40.0, 2.0, colWall);

    // Bricks
    let d: i32 = 0;
    while (d < totalBricks) {
        if (brickAlive[d] == 1) {
            drawRect(brickX[d], brickY[d], BRICK_W, BRICK_H, rowColors[brickRow[d]]);
        }
        d += 1;
    }

    // Paddle and ball
    drawRect(paddleX, PADDLE_Y, PADDLE_W, PADDLE_H, colPaddle);
    drawCircle(ballX, ballY, BALL_R, colBall);

    // HUD
    drawText(`SCORE ${score}`, 16.0, 12.0, 20, colText);
    drawText(`LIVES ${lives}`, 340.0, 12.0, 20, colText);
    drawText(`FPS ${currentFps()}`, 700.0, 12.0, 20, colDim);

    if (state == STATE_READY) {
        drawText("PRESS SPACE TO LAUNCH", 230.0, 300.0, 24, colText);
    }
    if (state == STATE_WON) {
        drawText("YOU WIN!", 320.0, 280.0, 40, colBall);
        drawText("press R to play again", 285.0, 330.0, 20, colDim);
    }
    if (state == STATE_LOST) {
        drawText("GAME OVER", 300.0, 280.0, 40, rgb(239, 83, 80));
        drawText("press R to play again", 285.0, 330.0, 20, colDim);
    }

    endFrame();
}

unloadSound(sndPaddle);
unloadSound(sndBrick);
unloadSound(sndWall);
unloadSound(sndLose);
closeAudio();
closeWindow();

println(`Breakout ended — score ${score}, ${frames} frames, ${bricksLeft} bricks left`);
