// A stand-in for a third-party header-only library.
//
// It lives under vendor/include/tinyclamp/, NOT next to the C file that uses
// it. That is the whole point: example/native/stats.c includes it as
//
//     #include <tinyclamp/tinyclamp.h>
//
// with angle brackets, so the compiler will not find it by looking beside the
// .c file. It is found only because 21_c_interop.csc says
//
//     link include "vendor/include";
//
// Drop a real vendored library in this shape and it works the same way.

#ifndef TINYCLAMP_H
#define TINYCLAMP_H

static inline int tinyclamp(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

#endif // TINYCLAMP_H
