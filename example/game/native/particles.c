// A particle simulation in plain C, driven from a Cypescript game.
//
// Nothing here knows about Cypescript, and Cypescript's build knows nothing
// about this file — `link source "native/particles.c";` in the .csc is the
// entire integration. No library to build, no header to write, no CMake target.
//
// The C ABI is the only contract: scalars in, scalars out, and one opaque
// handle that Cypescript holds as `ptr` and never looks inside.

#include <stdlib.h>
#include <math.h>

#define MAX_PARTICLES 512

typedef struct {
    float x, y, vx, vy, life;
} Particle;

typedef struct {
    Particle items[MAX_PARTICLES];
    int count;
    unsigned int seed;
} Field;

static float next_unit(Field *field) {
    field->seed = field->seed * 1664525u + 1013904223u;
    return (float)((field->seed >> 8) & 0xFFFF) / 65535.0f;
}

void *particles_new(int seed) {
    Field *field = calloc(1, sizeof(Field));   // implicit void* conversion: C, not C++
    field->seed = (unsigned int)seed;
    return field;
}

void particles_free(void *handle) { free(handle); }

void particles_emit(void *handle, double x, double y, int amount) {
    Field *field = handle;
    for (int i = 0; i < amount && field->count < MAX_PARTICLES; i++) {
        Particle *p = &field->items[field->count++];
        float angle = next_unit(field) * 6.2831853f;
        float speed = 40.0f + next_unit(field) * 90.0f;
        p->x = (float)x;
        p->y = (float)y;
        p->vx = cosf(angle) * speed;
        p->vy = sinf(angle) * speed;
        p->life = 0.6f + next_unit(field) * 0.6f;
    }
}

// The hot loop stays in C; Cypescript reads back only what it needs to draw.
void particles_step(void *handle, double dt) {
    Field *field = handle;
    for (int i = 0; i < field->count; i++) {
        Particle *p = &field->items[i];
        p->x += p->vx * (float)dt;
        p->y += p->vy * (float)dt;
        p->vy += 220.0f * (float)dt;      // gravity
        p->life -= (float)dt;
        if (p->life <= 0.0f) {
            field->items[i] = field->items[--field->count];
            i--;
        }
    }
}

int    particles_count(void *handle)       { return ((Field *)handle)->count; }
double particles_x(void *handle, int i)    { return ((Field *)handle)->items[i].x; }
double particles_y(void *handle, int i)    { return ((Field *)handle)->items[i].y; }
double particles_life(void *handle, int i) { return ((Field *)handle)->items[i].life; }
