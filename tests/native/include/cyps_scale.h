// Deliberately NOT next to helpers.c. It is included as <cyps_scale.h> with
// angle brackets, so it is reachable only through the `link include` path in
// tests/test_c_interop.csc. If that directive stops working, this stops
// compiling — which is the point of asserting it here.

#ifndef CYPS_SCALE_H
#define CYPS_SCALE_H

static inline int cyps_scale(int value, int factor) { return value * factor; }

#endif // CYPS_SCALE_H
