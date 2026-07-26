// `Buffer<T>` is a flat, fixed-size block: an i64 length followed by the
// elements. Indexing compiles to an address and a load or store, with no call
// into the runtime — which also lets LLVM vectorise loops over it.

let ints = new Buffer<i32>(8);
println(ints.length);

ints[0] = 42;
ints[7] = 7;
println(ints[0]);
println(ints[7]);
println(ints[3]);        // calloc'd, so untouched elements read as zero

// Compound assignment goes through the same inline path
ints[3] += 5;
ints[3] *= 3;
println(ints[3]);

// f64 elements
let reals = new Buffer<f64>(4);
reals[1] = 1.5;
reals[1] *= 2.0;
println(reals[1]);
println(reals[0]);

// A computed index, and a loop
let n: i32 = ints.length;
let i: i32 = 0;
let total: i32 = 0;
while (i < n) {
    ints[i] = i * 2;
    i += 1;
}
i = 0;
while (i < n) {
    total += ints[i];
    i += 1;
}
println(total);          // 2*(0+1+..+7) = 56

// A buffer sized at runtime
declare function strlen(s: string): i64;
let sized = new Buffer<i32>(4 + strlen("ab"));
println(sized.length);
