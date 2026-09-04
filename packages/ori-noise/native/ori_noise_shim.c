#define FNL_IMPL
#include "../vendor/FastNoiseLite.h"

#include <stdint.h>

static fnl_state g_state;

void ori_fnl_init(void) {
    g_state = fnlCreateState();
}

void ori_fnl_set_seed(int64_t seed) {
    g_state.seed = (int)seed;
}

void ori_fnl_set_frequency_m(int64_t freq_m) {
    g_state.frequency = (float)freq_m / 1000.0f;
}

void ori_fnl_set_type_open_simplex(void) {
    g_state.noise_type = FNL_NOISE_OPENSIMPLEX2;
}

void ori_fnl_set_type_perlin(void) {
    g_state.noise_type = FNL_NOISE_PERLIN;
}

void ori_fnl_set_type_cellular(void) {
    g_state.noise_type = FNL_NOISE_CELLULAR;
}

void ori_fnl_set_type_value(void) {
    g_state.noise_type = FNL_NOISE_VALUE;
}

void ori_fnl_set_fractal_none(void) {
    g_state.fractal_type = FNL_FRACTAL_NONE;
}

void ori_fnl_set_fractal_fbm(void) {
    g_state.fractal_type = FNL_FRACTAL_FBM;
}

void ori_fnl_set_fractal_ridged(void) {
    g_state.fractal_type = FNL_FRACTAL_RIDGED;
}

void ori_fnl_set_octaves(int64_t octaves) {
    if (octaves < 1) {
        octaves = 1;
    }
    if (octaves > 32) {
        octaves = 32;
    }
    g_state.octaves = (int)octaves;
}

void ori_fnl_set_lacunarity_m(int64_t lac_m) {
    g_state.lacunarity = (float)lac_m / 1000.0f;
}

void ori_fnl_set_gain_m(int64_t gain_m) {
    g_state.gain = (float)gain_m / 1000.0f;
}

int64_t ori_fnl_get_2d_m(int64_t x_m, int64_t y_m) {
    float v = fnlGetNoise2D(&g_state, (float)x_m / 1000.0f, (float)y_m / 1000.0f);
    return (int64_t)(v * 1000.0f);
}

int64_t ori_fnl_get_3d_m(int64_t x_m, int64_t y_m, int64_t z_m) {
    float v = fnlGetNoise3D(
        &g_state,
        (float)x_m / 1000.0f,
        (float)y_m / 1000.0f,
        (float)z_m / 1000.0f
    );
    return (int64_t)(v * 1000.0f);
}
