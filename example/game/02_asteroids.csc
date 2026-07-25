// 02 — Asteroids: dynamic entities, written the way you'd actually write them.
//
// Where Breakout (01) had to keep entities in parallel arrays, this game uses
// real objects: classes with constructors and methods, stored in `Rock[]` and
// `Bullet[]`, spawned by functions that return them, and removed when they die.
// That is Phase 3 of GAME_ROADMAP.md — objects now live on the heap, so they
// outlive the function that created them.
//
//   Run:      cscript -r example/game/02_asteroids.csc
//   Headless: CYPS_HEADLESS=1 CYPS_FRAMES=600 cscript -r example/game/02_asteroids.csc
//
// Controls: LEFT/RIGHT rotate, UP thrusts, SPACE fires, R restarts, ESC quits.

import { } from "game";

const WIDTH: f64 = 900.0;
const HEIGHT: f64 = 650.0;

const SHIP_TURN: f64 = 3.4;      // radians/second
const SHIP_THRUST: f64 = 260.0;
const SHIP_DRAG: f64 = 0.72;
const BULLET_SPEED: f64 = 520.0;
const BULLET_LIFE: f64 = 1.1;
const FIRE_DELAY: f64 = 0.18;

// =============================================================================
// Entities
// =============================================================================

class Ship {
    x: f64;
    y: f64;
    vx: f64;
    vy: f64;
    angle: f64;
    alive: boolean;

    constructor(x: f64, y: f64) {
        this.x = x;
        this.y = y;
        this.vx = 0.0;
        this.vy = 0.0;
        this.angle = 0.0 - 1.5707963;   // pointing up
        this.alive = true;
    }

    thrust(dt: f64): void {
        this.vx += Math.cos(this.angle) * SHIP_THRUST * dt;
        this.vy += Math.sin(this.angle) * SHIP_THRUST * dt;
    }

    step(dt: f64): void {
        this.x += this.vx * dt;
        this.y += this.vy * dt;
        // Drag, so the ship is controllable rather than purely Newtonian
        this.vx -= this.vx * SHIP_DRAG * dt;
        this.vy -= this.vy * SHIP_DRAG * dt;
    }
}

class Rock {
    x: f64;
    y: f64;
    vx: f64;
    vy: f64;
    radius: f64;
    size: i32;          // 3 = large, 2 = medium, 1 = small

    constructor(x: f64, y: f64, vx: f64, vy: f64, size: i32) {
        this.x = x;
        this.y = y;
        this.vx = vx;
        this.vy = vy;
        this.size = size;
        this.radius = size * 15.0;
    }

    step(dt: f64): void {
        this.x += this.vx * dt;
        this.y += this.vy * dt;
    }
}

class Bullet {
    x: f64;
    y: f64;
    vx: f64;
    vy: f64;
    life: f64;

    constructor(x: f64, y: f64, vx: f64, vy: f64) {
        this.x = x;
        this.y = y;
        this.vx = vx;
        this.vy = vy;
        this.life = BULLET_LIFE;
    }

    step(dt: f64): void {
        this.x += this.vx * dt;
        this.y += this.vy * dt;
        this.life -= dt;
    }
}

// =============================================================================
// Helpers — these return objects, which only became possible in Phase 3
// =============================================================================

function wrap(value: f64, limit: f64): f64 {
    if (value < 0.0) { return value + limit; }
    if (value > limit) { return value - limit; }
    return value;
}

function distance(ax: f64, ay: f64, bx: f64, by: f64): f64 {
    let dx: f64 = ax - bx;
    let dy: f64 = ay - by;
    return Math.sqrt(dx * dx + dy * dy);
}

function spawnRock(size: i32): Rock {
    // Start near an edge so rocks drift inward rather than onto the ship
    let x: f64 = Math.random() * WIDTH;
    let y: f64 = 0.0;
    if (Math.random() < 0.5) { y = HEIGHT; }
    let speed: f64 = 40.0 + Math.random() * 60.0;
    let dir: f64 = Math.random() * 6.2831853;
    return new Rock(x, y, Math.cos(dir) * speed, Math.sin(dir) * speed, size);
}

function splitRock(parent: Rock, dir: f64): Rock {
    let speed: f64 = 70.0 + Math.random() * 70.0;
    return new Rock(parent.x, parent.y,
                    Math.cos(dir) * speed, Math.sin(dir) * speed,
                    parent.size - 1);
}

function fire(ship: Ship): Bullet {
    let nose: f64 = 16.0;
    return new Bullet(ship.x + Math.cos(ship.angle) * nose,
                      ship.y + Math.sin(ship.angle) * nose,
                      ship.vx + Math.cos(ship.angle) * BULLET_SPEED,
                      ship.vy + Math.sin(ship.angle) * BULLET_SPEED);
}

// =============================================================================
// Setup
// =============================================================================

quietLogs();
openWindow(900, 650, "Cypescript Asteroids");
setTargetFps(60);
initAudio();
random_seed(20260725);

let sndFire: ptr = makeTone(720.0, 45, 0.20);
let sndHit: ptr = makeTone(220.0, 90, 0.30);
let sndDeath: ptr = makeTone(110.0, 400, 0.40);

let colBg: i32 = rgb(8, 10, 18);
let colShip: i32 = rgb(220, 235, 255);
let colRock: i32 = rgb(150, 160, 185);
let colBullet: i32 = rgb(255, 214, 120);
let colText: i32 = rgb(210, 215, 235);
let colDim: i32 = rgb(110, 116, 140);

let ship: Ship = new Ship(WIDTH / 2.0, HEIGHT / 2.0);
let rocks: Rock[] = [];
let bullets: Bullet[] = [];

let score: i32 = 0;
let lives: i32 = 3;
let wave: i32 = 1;
let fireCooldown: f64 = 0.0;
let frames: i32 = 0;

let startRocks: i32 = 4;
let r: i32 = 0;
while (r < startRocks) {
    rocks.push(spawnRock(3));
    r += 1;
}

// =============================================================================
// Main loop
// =============================================================================
while (!windowShouldClose()) {
    let dt: f64 = deltaTime();
    frames += 1;
    let autoplay: boolean = isHeadless();

    // --- Input -------------------------------------------------------------
    let turnLeft: boolean = isKeyDown(KEY_LEFT);
    let turnRight: boolean = isKeyDown(KEY_RIGHT);
    let thrusting: boolean = isKeyDown(KEY_UP);
    let shooting: boolean = isKeyDown(KEY_SPACE);

    if (autoplay) {
        // With no keyboard, aim at the nearest rock and keep firing — this
        // exercises spawning, collision and despawning in CI.
        turnLeft = false;
        turnRight = false;
        thrusting = false;
        shooting = true;

        if (rocks.length > 0) {
            let target: Rock = rocks[0];
            let want: f64 = Math.atan2(target.y - ship.y, target.x - ship.x);
            let diff: f64 = want - ship.angle;
            while (diff > 3.1415927) { diff -= 6.2831853; }
            while (diff < 0.0 - 3.1415927) { diff += 6.2831853; }
            if (diff > 0.05) { turnRight = true; }
            if (diff < 0.0 - 0.05) { turnLeft = true; }
        }
    }

    if (ship.alive) {
        if (turnLeft)  { ship.angle -= SHIP_TURN * dt; }
        if (turnRight) { ship.angle += SHIP_TURN * dt; }
        if (thrusting) { ship.thrust(dt); }

        ship.step(dt);
        ship.x = wrap(ship.x, WIDTH);
        ship.y = wrap(ship.y, HEIGHT);
    }

    // --- Firing ------------------------------------------------------------
    fireCooldown -= dt;
    if (shooting) {
        if (ship.alive) {
            if (fireCooldown <= 0.0) {
                bullets.push(fire(ship));
                fireCooldown = FIRE_DELAY;
                playSound(sndFire);
            }
        }
    }

    // --- Advance bullets, dropping the expired ones -------------------------
    let b: i32 = bullets.length - 1;
    while (b >= 0) {
        let bullet: Bullet = bullets[b];
        bullet.step(dt);
        bullet.x = wrap(bullet.x, WIDTH);
        bullet.y = wrap(bullet.y, HEIGHT);
        if (bullet.life <= 0.0) {
            bullets.removeAt(b);
        }
        b -= 1;
    }

    // --- Advance rocks ------------------------------------------------------
    let k: i32 = 0;
    while (k < rocks.length) {
        let rock: Rock = rocks[k];
        rock.step(dt);
        rock.x = wrap(rock.x, WIDTH);
        rock.y = wrap(rock.y, HEIGHT);
        k += 1;
    }

    // --- Bullet / rock collisions ------------------------------------------
    // Walk backwards so removing an entity doesn't disturb the indices ahead.
    let bi: i32 = bullets.length - 1;
    while (bi >= 0) {
        let bullet: Bullet = bullets[bi];
        let ri: i32 = rocks.length - 1;
        let consumed: boolean = false;

        while (ri >= 0) {
            if (!consumed) {
                let rock: Rock = rocks[ri];
                if (distance(bullet.x, bullet.y, rock.x, rock.y) < rock.radius) {
                    score += rock.size * 20;
                    playSound(sndHit);

                    // Large and medium rocks split into two smaller ones
                    if (rock.size > 1) {
                        let spin: f64 = Math.random() * 6.2831853;
                        rocks.push(splitRock(rock, spin));
                        rocks.push(splitRock(rock, spin + 3.1415927));
                    }

                    rocks.removeAt(ri);
                    bullets.removeAt(bi);
                    consumed = true;
                }
            }
            ri -= 1;
        }
        bi -= 1;
    }

    // --- Ship / rock collisions --------------------------------------------
    // The headless autopilot doesn't dodge, so it would die in seconds and stop
    // exercising anything. Leave it invulnerable: a long CI run should keep
    // spawning, splitting and despawning entities for the whole run.
    if (ship.alive && !autoplay) {
        let ci: i32 = 0;
        while (ci < rocks.length) {
            let rock: Rock = rocks[ci];
            if (distance(ship.x, ship.y, rock.x, rock.y) < rock.radius + 9.0) {
                lives -= 1;
                playSound(sndDeath);
                ship.x = WIDTH / 2.0;
                ship.y = HEIGHT / 2.0;
                ship.vx = 0.0;
                ship.vy = 0.0;
                if (lives <= 0) { ship.alive = false; }
                ci = rocks.length;
            } else {
                ci += 1;
            }
        }
    }

    // --- Next wave ----------------------------------------------------------
    if (rocks.length == 0) {
        wave += 1;
        let spawnCount: i32 = 3 + wave;
        let s: i32 = 0;
        while (s < spawnCount) {
            rocks.push(spawnRock(3));
            s += 1;
        }
    }

    // --- Restart ------------------------------------------------------------
    if (!ship.alive) {
        if (isKeyPressed(KEY_R)) {
            ship.alive = true;
            ship.x = WIDTH / 2.0;
            ship.y = HEIGHT / 2.0;
            score = 0;
            lives = 3;
        }
    }

    // =========================================================================
    // Draw
    // =========================================================================
    beginFrame();
    clearScreen(colBg);

    let d: i32 = 0;
    while (d < rocks.length) {
        let rock: Rock = rocks[d];
        drawCircleOutline(rock.x, rock.y, rock.radius, colRock);
        d += 1;
    }

    let e: i32 = 0;
    while (e < bullets.length) {
        let bullet: Bullet = bullets[e];
        drawCircle(bullet.x, bullet.y, 2.5, colBullet);
        e += 1;
    }

    if (ship.alive) {
        // A triangle drawn from three points around the heading
        let nx: f64 = ship.x + Math.cos(ship.angle) * 16.0;
        let ny: f64 = ship.y + Math.sin(ship.angle) * 16.0;
        let lx: f64 = ship.x + Math.cos(ship.angle + 2.5) * 12.0;
        let ly: f64 = ship.y + Math.sin(ship.angle + 2.5) * 12.0;
        let rx: f64 = ship.x + Math.cos(ship.angle - 2.5) * 12.0;
        let ry: f64 = ship.y + Math.sin(ship.angle - 2.5) * 12.0;
        drawLine(nx, ny, lx, ly, 2.0, colShip);
        drawLine(nx, ny, rx, ry, 2.0, colShip);
        drawLine(lx, ly, rx, ry, 2.0, colShip);
    }

    drawText(`SCORE ${score}`, 16.0, 12.0, 20, colText);
    drawText(`LIVES ${lives}`, 220.0, 12.0, 20, colText);
    drawText(`WAVE ${wave}`, 400.0, 12.0, 20, colDim);
    drawText(`ROCKS ${rocks.length}`, 560.0, 12.0, 20, colDim);

    if (!ship.alive) {
        drawText("GAME OVER", 330.0, 300.0, 40, rgb(239, 83, 80));
        drawText("press R to play again", 320.0, 350.0, 20, colDim);
    }

    endFrame();
}

unloadSound(sndFire);
unloadSound(sndHit);
unloadSound(sndDeath);
closeAudio();
closeWindow();

println(`Asteroids ended — score ${score}, wave ${wave}, ${frames} frames`);
