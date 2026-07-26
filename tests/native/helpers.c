// Compiled as C — `void*` conversions and C99 rules apply, not C++ ones.
#include <stdlib.h>
#include <string.h>

int c_triple(int x) { return x * 3; }

void *c_make_buffer(int size) {
    char *buffer = malloc(size);   // implicit void* conversion: C, not C++
    memset(buffer, 0, size);
    return buffer;
}

int c_buffer_ok(void *handle) { return handle != NULL; }
void c_free_buffer(void *handle) { free(handle); }
