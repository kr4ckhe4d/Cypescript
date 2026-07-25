// Math.* additions used by game code: clamping, rounding, angles, randomness.
println(Math.min(2.0, 3.0));
println(Math.max(2.0, 3.0));
println(Math.round(2.6));
println(Math.ceil(2.1));
println(Math.atan2(1.0, 1.0));
println(Math.PI);

// Clamp idiom, the single most common one in a game loop
let x: f64 = 42.0;
let clamped: f64 = Math.max(0.0, Math.min(x, 10.0));
println(clamped);

// Math.random() stays in [0, 1)
random_seed(7);
let r: f64 = Math.random();
if (r >= 0.0) {
    if (r < 1.0) { println("random in range"); }
}
