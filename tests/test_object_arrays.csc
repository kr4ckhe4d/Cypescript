class Entity {
    x: f64;
    y: f64;
    vx: f64;
    alive: boolean;
    name: string;
    constructor(x: f64, y: f64, vx: f64, name: string) {
        this.x = x; this.y = y; this.vx = vx;
        this.alive = true; this.name = name;
    }
    step(dt: f64): void { this.x += this.vx * dt; }
}

function spawn(i: i32): Entity {
    return new Entity(i * 10.0, 0.0, 1.0 + i, `e${i}`);
}

let entities: Entity[] = [];
let i: i32 = 0;
while (i < 4) { entities.push(spawn(i)); i += 1; }
println(entities.length);

// mutate through the array, the core game-loop pattern
let n: i32 = 0;
while (n < entities.length) {
    let e = entities[n];
    e.step(1.0);
    e.x += 100.0;
    n += 1;
}
println(entities[0].x);
println(entities[3].x);

// identity: array holds references, not copies
let ref = entities[1];
ref.alive = false;
println(entities[1].alive);
println(entities[1].name);

// for...of over objects
let total: f64 = 0.0;
for (const e of entities) { total += e.x; }
println(total);

// despawn
entities.removeAt(0);
println(entities.length);
println(entities[0].name);
