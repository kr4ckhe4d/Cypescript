// A plain C file. No headers to write, no build step, no library to produce —
// example/21_c_interop.csc pulls it in with one `link source` line.
//
// The only rule is the C ABI: take and return scalars, strings (char*) or
// opaque pointers, because that is what a Cypescript `declare` can describe.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Angle brackets, and the header is not in this directory — it is under
// example/vendor/include/. The only reason this compiles is the
// `link include "vendor/include";` line in 21_c_interop.csc.
#include <tinyclamp/tinyclamp.h>

int stats_sum(const int *values, int count) {
    int total = 0;
    for (int i = 0; i < count; i++) total += values[i];
    return total;
}

double stats_mean(const int *values, int count) {
    if (count == 0) return 0.0;
    return (double)stats_sum(values, count) / (double)count;
}

// Returning a string: allocate it, and the caller treats it as a Cypescript string
const char *stats_describe(int count) {
    char *out = malloc(64);
    snprintf(out, 64, "%d sample(s)", count);
    return out;
}

// Uses the vendored header. Nothing about this is special to Cypescript — it is
// an ordinary C call into an ordinary third-party inline function.
int stats_clamped(int value) { return tinyclamp(value, 0, 100); }

// An opaque handle — Cypescript holds it as `ptr` and never looks inside
typedef struct { int total; int count; } Accumulator;

void *acc_new(void)                { return calloc(1, sizeof(Accumulator)); }
void  acc_add(void *h, int value)  { Accumulator *a = h; a->total += value; a->count++; }
int   acc_total(void *h)           { return ((Accumulator *)h)->total; }
void  acc_free(void *h)            { free(h); }
