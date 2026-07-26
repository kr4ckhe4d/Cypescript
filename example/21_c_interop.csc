// 21 — Calling your own C from Cypescript
//
// Two lines of ceremony: `link source` names the C file, `declare function`
// names the functions. No separate build step, no library, no header, no
// wrapper script — cscript compiles the C alongside your program.
//
//   cscript -r example/21_c_interop.csc

link source "native/stats.c";

// The C file's path is relative to THIS file, so a project keeps its native
// code beside its Cypescript.

declare function stats_sum(values: ptr, count: i32): i32;
declare function stats_mean(values: ptr, count: i32): f64;
declare function stats_describe(count: i32): string;

// An opaque handle. Cypescript never needs to know what an Accumulator is.
declare function acc_new(): ptr;
declare function acc_add(handle: ptr, value: i32): void;
declare function acc_total(handle: ptr): i32;
declare function acc_free(handle: ptr): void;

// Handles round-trip through Cypescript unchanged
let acc: ptr = acc_new();
acc_add(acc, 10);
acc_add(acc, 20);
acc_add(acc, 12);
println(acc_total(acc));            // 42
println(stats_describe(3));         // 3 sample(s)
acc_free(acc);

// A `declare` can also give a C symbol a friendlier Cypescript name
declare function mean(values: ptr, count: i32): f64 = "stats_mean";

// Anything from libc or libm is available the same way — those are already linked
declare function labs(x: i64): i64;
println(labs(0 - 7));               // 7

println("C interop needs no build system");
