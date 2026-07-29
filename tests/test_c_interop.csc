// `link source` compiles a C or C++ file alongside the program — no separate
// build step, no library, no wrapper script. The path is relative to this file.
//
// Each source is compiled with the driver its language needs, so a .c file gets
// C rules (implicit void* conversion) and a .cpp file gets C++17.

link source "native/helpers.c";
link source "native/helpers.cpp";

// A header search path for those sources. helpers.c includes <cyps_scale.h>,
// which lives in native/include/ rather than beside it, so this line is load
// bearing — remove it and the C stops compiling.
link include "native/include";

declare function c_triple(x: i32): i32;
declare function c_scaled(x: i32): i32;
declare function c_make_buffer(size: i32): ptr;
declare function c_buffer_ok(handle: ptr): boolean;
declare function c_free_buffer(handle: ptr): void;

declare function cpp_sum_to(n: i32): i32;
declare function cpp_greet(name: string): string;

println(c_triple(14));
println(c_scaled(3));      // through the header found via `link include`

let buffer: ptr = c_make_buffer(128);
if (c_buffer_ok(buffer)) { println("buffer allocated"); }
if (buffer != null) { println("handle is not null"); }
c_free_buffer(buffer);

println(cpp_sum_to(10));
println(cpp_greet("cypescript"));

// A friendlier name for a C symbol
declare function triple(x: i32): i32 = "c_triple";
println(triple(5));
