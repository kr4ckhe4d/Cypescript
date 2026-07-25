// runtime/game/cyps_game.c — Cypescript game runtime
//
// A flat C shim over raylib. Everything here exists to keep the compiler
// ignorant of graphics: Cypescript has no by-value struct ABI, so raylib's
// Vector2/Color/Rectangle never cross the boundary. Every entry point takes and
// returns only scalars and opaque pointers, which is exactly what a
// `declare function` in Cypescript can express.
//
// Colors are packed into one i32 as 0xRRGGBBAA — build them with cyps_rgb/cyps_rgba.
// Handles (textures, sounds) are heap-allocated raylib structs returned as ptr.
//
// Swapping raylib for SDL means rewriting this file and nothing else.

#include "raylib.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// --- Headless mode -----------------------------------------------------------
// CYPS_HEADLESS=1 runs the whole game loop with no window and no drawing, so a
// game can be exercised in CI. CYPS_FRAMES caps the run length (default 120).

static int g_headless = 0;
static int g_headless_frames = 120;
static int g_frames_elapsed = 0;
static int g_audio_ready = 0;

static void cyps_read_env(void) {
    const char *headless = getenv("CYPS_HEADLESS");
    g_headless = (headless && headless[0] == '1');
    const char *frames = getenv("CYPS_FRAMES");
    if (frames) {
        int parsed = atoi(frames);
        if (parsed > 0) g_headless_frames = parsed;
    }
}

// --- Colors ------------------------------------------------------------------

static Color cyps_unpack(int packed) {
    unsigned int rgba = (unsigned int)packed;
    Color c;
    c.r = (unsigned char)((rgba >> 24) & 0xFF);
    c.g = (unsigned char)((rgba >> 16) & 0xFF);
    c.b = (unsigned char)((rgba >> 8) & 0xFF);
    c.a = (unsigned char)(rgba & 0xFF);
    return c;
}

// Cypescript has no bitwise operators yet, so colors are composed here.
int cyps_rgba(int r, int g, int b, int a) {
    unsigned int packed = ((unsigned int)(r & 0xFF) << 24) |
                          ((unsigned int)(g & 0xFF) << 16) |
                          ((unsigned int)(b & 0xFF) << 8) |
                          ((unsigned int)(a & 0xFF));
    return (int)packed;
}

int cyps_rgb(int r, int g, int b) { return cyps_rgba(r, g, b, 255); }

// --- Window and frame lifecycle ----------------------------------------------

void cyps_win_open(int width, int height, const char *title) {
    cyps_read_env();
    if (g_headless) return;
    InitWindow(width, height, title);
}

void cyps_win_close(void) {
    if (g_headless) return;
    CloseWindow();
}

int cyps_win_should_close(void) {
    if (g_headless) return (++g_frames_elapsed > g_headless_frames) ? 1 : 0;
    return WindowShouldClose() ? 1 : 0;
}

void cyps_set_target_fps(int fps) {
    if (g_headless) return;
    SetTargetFPS(fps);
}

void cyps_frame_begin(void) {
    if (g_headless) return;
    BeginDrawing();
}

void cyps_frame_end(void) {
    if (g_headless) return;
    EndDrawing();
}

void cyps_clear(int color) {
    if (g_headless) return;
    ClearBackground(cyps_unpack(color));
}

// Lets a game know it is running without a window, so it can drive itself in CI.
int cyps_headless(void) {
    cyps_read_env();
    return g_headless;
}

int cyps_win_width(void) { return g_headless ? 0 : GetScreenWidth(); }
int cyps_win_height(void) { return g_headless ? 0 : GetScreenHeight(); }

// Seconds since the previous frame. Headless runs report a fixed 60 Hz step so
// simulation behaves identically with and without a window.
double cyps_dt(void) {
    if (g_headless) return 1.0 / 60.0;
    return (double)GetFrameTime();
}

double cyps_time(void) {
    if (g_headless) return (double)g_frames_elapsed / 60.0;
    return GetTime();
}

int cyps_fps(void) { return g_headless ? 60 : GetFPS(); }

// --- Immediate-mode drawing --------------------------------------------------

void cyps_rect(double x, double y, double w, double h, int color) {
    if (g_headless) return;
    DrawRectangleV((Vector2){(float)x, (float)y}, (Vector2){(float)w, (float)h},
                   cyps_unpack(color));
}

void cyps_rect_lines(double x, double y, double w, double h, double thickness, int color) {
    if (g_headless) return;
    DrawRectangleLinesEx((Rectangle){(float)x, (float)y, (float)w, (float)h},
                         (float)thickness, cyps_unpack(color));
}

void cyps_circle(double x, double y, double radius, int color) {
    if (g_headless) return;
    DrawCircleV((Vector2){(float)x, (float)y}, (float)radius, cyps_unpack(color));
}

void cyps_line(double x1, double y1, double x2, double y2, double thickness, int color) {
    if (g_headless) return;
    DrawLineEx((Vector2){(float)x1, (float)y1}, (Vector2){(float)x2, (float)y2},
               (float)thickness, cyps_unpack(color));
}

void cyps_text(const char *text, double x, double y, int size, int color) {
    if (g_headless) return;
    DrawText(text, (int)x, (int)y, size, cyps_unpack(color));
}

int cyps_text_width(const char *text, int size) {
    if (g_headless) return (int)(strlen(text) * size / 2);
    return MeasureText(text, size);
}

// --- Textures ----------------------------------------------------------------
// raylib returns Texture2D by value; we heap-allocate one and hand back a ptr.

void *cyps_tex_load(const char *path) {
    if (g_headless) return NULL;
    Texture2D *tex = (Texture2D *)malloc(sizeof(Texture2D));
    if (!tex) return NULL;
    *tex = LoadTexture(path);
    return tex;
}

void cyps_tex_unload(void *handle) {
    if (g_headless || !handle) return;
    UnloadTexture(*(Texture2D *)handle);
    free(handle);
}

int cyps_tex_width(void *handle) { return handle ? ((Texture2D *)handle)->width : 0; }
int cyps_tex_height(void *handle) { return handle ? ((Texture2D *)handle)->height : 0; }

void cyps_tex_draw(void *handle, double x, double y, double rotation, double scale, int tint) {
    if (g_headless || !handle) return;
    DrawTextureEx(*(Texture2D *)handle, (Vector2){(float)x, (float)y},
                  (float)rotation, (float)scale, cyps_unpack(tint));
}

// Sprite-sheet blit: source rect out of the texture into a destination rect.
void cyps_tex_draw_src(void *handle,
                       double sx, double sy, double sw, double sh,
                       double dx, double dy, double dw, double dh,
                       double rotation, int tint) {
    if (g_headless || !handle) return;
    DrawTexturePro(*(Texture2D *)handle,
                   (Rectangle){(float)sx, (float)sy, (float)sw, (float)sh},
                   (Rectangle){(float)dx, (float)dy, (float)dw, (float)dh},
                   (Vector2){0.0f, 0.0f}, (float)rotation, cyps_unpack(tint));
}

// --- Input -------------------------------------------------------------------

int cyps_key_down(int key)     { return (!g_headless && IsKeyDown(key)) ? 1 : 0; }
int cyps_key_pressed(int key)  { return (!g_headless && IsKeyPressed(key)) ? 1 : 0; }
int cyps_key_released(int key) { return (!g_headless && IsKeyReleased(key)) ? 1 : 0; }

double cyps_mouse_x(void) { return g_headless ? 0.0 : (double)GetMouseX(); }
double cyps_mouse_y(void) { return g_headless ? 0.0 : (double)GetMouseY(); }
int cyps_mouse_down(int button) { return (!g_headless && IsMouseButtonDown(button)) ? 1 : 0; }
int cyps_mouse_pressed(int button) { return (!g_headless && IsMouseButtonPressed(button)) ? 1 : 0; }

int cyps_gamepad_available(int pad) { return (!g_headless && IsGamepadAvailable(pad)) ? 1 : 0; }
int cyps_gamepad_down(int pad, int button) {
    return (!g_headless && IsGamepadButtonDown(pad, button)) ? 1 : 0;
}
double cyps_gamepad_axis(int pad, int axis) {
    return g_headless ? 0.0 : (double)GetGamepadAxisMovement(pad, axis);
}

// --- Audio -------------------------------------------------------------------

void cyps_audio_init(void) {
    if (g_headless) return;
    InitAudioDevice();
    g_audio_ready = IsAudioDeviceReady() ? 1 : 0;
}

void cyps_audio_close(void) {
    if (g_headless || !g_audio_ready) return;
    CloseAudioDevice();
    g_audio_ready = 0;
}

void *cyps_sound_load(const char *path) {
    if (g_headless || !g_audio_ready) return NULL;
    Sound *sound = (Sound *)malloc(sizeof(Sound));
    if (!sound) return NULL;
    *sound = LoadSound(path);
    return sound;
}

// Synthesises a square-wave blip so a game can have sound with no asset files.
// This is what makes an arcade game self-contained in a single .csc.
void *cyps_tone(double frequency, int milliseconds, double volume) {
    if (g_headless || !g_audio_ready) return NULL;

    const unsigned int sampleRate = 44100;
    unsigned int sampleCount = (unsigned int)(sampleRate * milliseconds / 1000);
    if (sampleCount == 0) return NULL;

    short *samples = (short *)malloc(sizeof(short) * sampleCount);
    if (!samples) return NULL;

    for (unsigned int i = 0; i < sampleCount; i++) {
        double phase = fmod((double)i * frequency / (double)sampleRate, 1.0);
        double envelope = 1.0 - ((double)i / (double)sampleCount); // fade out
        double amplitude = (phase < 0.5 ? 1.0 : -1.0) * volume * envelope;
        samples[i] = (short)(amplitude * 32000.0);
    }

    Wave wave;
    wave.frameCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;

    Sound *sound = (Sound *)malloc(sizeof(Sound));
    if (!sound) { free(samples); return NULL; }
    *sound = LoadSoundFromWave(wave);
    free(samples);
    return sound;
}

void cyps_sound_play(void *handle) {
    if (g_headless || !handle) return;
    PlaySound(*(Sound *)handle);
}

void cyps_sound_volume(void *handle, double volume) {
    if (g_headless || !handle) return;
    SetSoundVolume(*(Sound *)handle, (float)volume);
}

void cyps_sound_unload(void *handle) {
    if (g_headless || !handle) return;
    UnloadSound(*(Sound *)handle);
    free(handle);
}

// --- Misc --------------------------------------------------------------------

void cyps_set_exit_key(int key) {
    if (g_headless) return;
    SetExitKey(key);
}

// Quiets raylib's startup banner so a game's own output is readable.
void cyps_log_quiet(void) {
    SetTraceLogLevel(LOG_WARNING);
}
