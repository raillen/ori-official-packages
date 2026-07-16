/* Headless stub of ori_jolt_* — Euler dynamics for CI without Jolt C++.
 * Same ABI as ori_jolt_shim.cpp (milli-units). Wave 4: hit body, friction,
 * layers, angular velocity/torque, static floor helper.
 */
#include <stdint.h>
#include <string.h>

#define MAX_SYS 4
#define MAX_BODY 512
#define MAX_CONSTRAINT 256
#define MAX_CHARS 32
#define MAX_VEHS 16
#define NUM_LAYERS 4

typedef struct {
    int live;
    int gx, gy, gz;
    /* Bitmask of layers each object layer collides with (bits 0..3). */
    int layer_mask[NUM_LAYERS];
} Sys;

typedef struct {
    int live, sys;
    int dynamic;
    int x, y, z;
    int vx, vy, vz;
    int wx, wy, wz; /* angular velocity milli-rad/s (stub scale) */
    int hx, hy, hz;
    int kind;
    int friction_m;    /* milli: 0..1000+ → 0.0..1.0+ */
    int restitution_m;
    int layer; /* 0..3 */
} Body;

typedef struct {
    int live, sys, kind;
    int a, b;
    int min_d, max_d;
} Constraint;

typedef struct {
    int live, sys;
    int x, y, z, vx, vy, vz;
    int radius, half_h;
    int want_jump, jump_v;
} Char;

typedef struct {
    int live, sys, chassis;
    int throttle, steer, brake;
} Veh;

static Sys g_sys[MAX_SYS];
static Body g_body[MAX_BODY];
static Constraint g_c[MAX_CONSTRAINT];
static Char g_ch[MAX_CHARS];
static Veh g_vh[MAX_VEHS];
static int g_hit_x, g_hit_y, g_hit_z, g_hit_frac, g_hit_body;

static int clamp_layer(int layer) {
    if (layer < 0) return 0;
    if (layer >= NUM_LAYERS) return NUM_LAYERS - 1;
    return layer;
}

static int alloc_sys(void) {
    for (int i = 0; i < MAX_SYS; ++i)
        if (!g_sys[i].live) return i;
    return -1;
}
static int alloc_body(void) {
    for (int i = 1; i < MAX_BODY; ++i)
        if (!g_body[i].live) return i;
    return 0;
}
static int alloc_c(void) {
    for (int i = 1; i < MAX_CONSTRAINT; ++i)
        if (!g_c[i].live) return i;
    return 0;
}
static int alloc_ch(void) {
    for (int i = 1; i < MAX_CHARS; ++i)
        if (!g_ch[i].live) return i;
    return 0;
}
static int alloc_vh(void) {
    for (int i = 1; i < MAX_VEHS; ++i)
        if (!g_vh[i].live) return i;
    return 0;
}
static int sys_ok(int s) { return s >= 0 && s < MAX_SYS && g_sys[s].live; }
static int body_ok(int b) { return b > 0 && b < MAX_BODY && g_body[b].live; }

int ori_jolt_create_system(int gx, int gy, int gz) {
    int s = alloc_sys();
    if (s < 0) return -1;
    g_sys[s].live = 1;
    g_sys[s].gx = gx;
    g_sys[s].gy = gy;
    g_sys[s].gz = gz;
    /* Default: every layer collides with all layers. */
    for (int L = 0; L < NUM_LAYERS; ++L)
        g_sys[s].layer_mask[L] = (1 << NUM_LAYERS) - 1;
    return s;
}

void ori_jolt_destroy_system(int system) {
    if (!sys_ok(system)) return;
    for (int i = 0; i < MAX_BODY; ++i)
        if (g_body[i].live && g_body[i].sys == system) g_body[i].live = 0;
    for (int i = 0; i < MAX_CONSTRAINT; ++i)
        if (g_c[i].live && g_c[i].sys == system) g_c[i].live = 0;
    for (int i = 0; i < MAX_CHARS; ++i)
        if (g_ch[i].live && g_ch[i].sys == system) g_ch[i].live = 0;
    for (int i = 0; i < MAX_VEHS; ++i)
        if (g_vh[i].live && g_vh[i].sys == system) g_vh[i].live = 0;
    g_sys[system].live = 0;
}

int ori_jolt_system_is_valid(int system) { return sys_ok(system) ? 1 : 0; }

void ori_jolt_set_gravity(int system, int gx, int gy, int gz) {
    if (!sys_ok(system)) return;
    g_sys[system].gx = gx;
    g_sys[system].gy = gy;
    g_sys[system].gz = gz;
}

void ori_jolt_step(int system, int dt_micros, int collision_steps) {
    (void)collision_steps;
    if (!sys_ok(system)) return;
    float dt = (float)dt_micros / 1000000.0f;
    int dgx = (int)((float)g_sys[system].gx * dt);
    int dgy = (int)((float)g_sys[system].gy * dt);
    int dgz = (int)((float)g_sys[system].gz * dt);
    for (int i = 0; i < MAX_BODY; ++i) {
        if (!g_body[i].live || g_body[i].sys != system || !g_body[i].dynamic) continue;
        g_body[i].vx += dgx;
        g_body[i].vy += dgy;
        g_body[i].vz += dgz;
        g_body[i].x += (int)((float)g_body[i].vx * dt);
        g_body[i].y += (int)((float)g_body[i].vy * dt);
        g_body[i].z += (int)((float)g_body[i].vz * dt);
        /* Crude angular integrate (milli-rad/s → unused pose). */
        (void)g_body[i].wx;
        int half_y = g_body[i].kind == 1 ? g_body[i].hx : g_body[i].hy;
        if (g_body[i].y - half_y < 0) {
            g_body[i].y = half_y;
            if (g_body[i].vy < 0) {
                /* Bounce if restitution set (simple). */
                if (g_body[i].restitution_m > 0) {
                    g_body[i].vy = -(int)((float)g_body[i].vy * (float)g_body[i].restitution_m / 1000.0f);
                } else {
                    g_body[i].vy = 0;
                }
            }
            /* Horizontal damping as crude friction. */
            if (g_body[i].friction_m > 0) {
                int f = g_body[i].friction_m;
                if (f > 1000) f = 1000;
                g_body[i].vx = g_body[i].vx * (1000 - f) / 1000;
                g_body[i].vz = g_body[i].vz * (1000 - f) / 1000;
            }
        }
    }
    for (int c = 1; c < MAX_CHARS; ++c) {
        if (!g_ch[c].live || g_ch[c].sys != system) continue;
        int feet = g_ch[c].half_h + g_ch[c].radius;
        int on_ground = (g_ch[c].y - feet) <= 50;
        if (g_ch[c].want_jump && on_ground) {
            g_ch[c].vy = g_ch[c].jump_v;
            g_ch[c].want_jump = 0;
        } else {
            g_ch[c].vy += dgy;
        }
        g_ch[c].x += (int)((float)g_ch[c].vx * dt);
        g_ch[c].y += (int)((float)g_ch[c].vy * dt);
        g_ch[c].z += (int)((float)g_ch[c].vz * dt);
        if (g_ch[c].y - feet < 0) {
            g_ch[c].y = feet;
            if (g_ch[c].vy < 0) g_ch[c].vy = 0;
        }
    }
    for (int v = 1; v < MAX_VEHS; ++v) {
        if (!g_vh[v].live || g_vh[v].sys != system) continue;
        int b = g_vh[v].chassis;
        if (!body_ok(b)) continue;
        g_body[b].vx += g_vh[v].throttle / 10;
        g_body[b].vz += g_vh[v].steer / 20;
        if (g_vh[v].brake > 0) {
            g_body[b].vx = g_body[b].vx * 9 / 10;
            g_body[b].vz = g_body[b].vz * 9 / 10;
        }
    }
}

static int make_body(int system, int dyn, int x, int y, int z, int hx, int hy, int hz, int kind) {
    if (!sys_ok(system)) return 0;
    int id = alloc_body();
    if (!id) return 0;
    g_body[id].live = 1;
    g_body[id].sys = system;
    g_body[id].dynamic = dyn;
    g_body[id].x = x;
    g_body[id].y = y;
    g_body[id].z = z;
    g_body[id].vx = g_body[id].vy = g_body[id].vz = 0;
    g_body[id].wx = g_body[id].wy = g_body[id].wz = 0;
    g_body[id].hx = hx;
    g_body[id].hy = hy;
    g_body[id].hz = hz;
    g_body[id].kind = kind;
    g_body[id].friction_m = 200; /* 0.2 default-ish */
    g_body[id].restitution_m = 0;
    g_body[id].layer = dyn ? 1 : 0;
    return id;
}

int ori_jolt_create_static_box(int s, int x, int y, int z, int hx, int hy, int hz) {
    return make_body(s, 0, x, y, z, hx, hy, hz, 0);
}
int ori_jolt_create_dynamic_box(int s, int x, int y, int z, int hx, int hy, int hz, int density) {
    (void)density;
    return make_body(s, 1, x, y, z, hx, hy, hz, 0);
}
int ori_jolt_create_static_sphere(int s, int x, int y, int z, int r) {
    return make_body(s, 0, x, y, z, r, r, r, 1);
}
int ori_jolt_create_dynamic_sphere(int s, int x, int y, int z, int r, int density) {
    (void)density;
    return make_body(s, 1, x, y, z, r, r, r, 1);
}
int ori_jolt_create_dynamic_capsule(int s, int x, int y, int z, int hh, int r, int density) {
    (void)density;
    return make_body(s, 1, x, y, z, r, hh, r, 2);
}

/* Wave 4: large static box with top surface at y_m. */
int ori_jolt_create_static_floor(int system, int y_m) {
    const int half_y = 500;
    const int half_xz = 500000; /* 500 m */
    return ori_jolt_create_static_box(system, 0, y_m - half_y, 0, half_xz, half_y, half_xz);
}

void ori_jolt_destroy_body(int body) {
    if (body_ok(body)) g_body[body].live = 0;
}
int ori_jolt_body_is_valid(int body) { return body_ok(body) ? 1 : 0; }
int ori_jolt_body_x_m(int body) { return body_ok(body) ? g_body[body].x : 0; }
int ori_jolt_body_y_m(int body) { return body_ok(body) ? g_body[body].y : 0; }
int ori_jolt_body_z_m(int body) { return body_ok(body) ? g_body[body].z : 0; }
int ori_jolt_body_vx_m(int body) { return body_ok(body) ? g_body[body].vx : 0; }
int ori_jolt_body_vy_m(int body) { return body_ok(body) ? g_body[body].vy : 0; }
int ori_jolt_body_vz_m(int body) { return body_ok(body) ? g_body[body].vz : 0; }
int ori_jolt_body_wx_m(int body) { return body_ok(body) ? g_body[body].wx : 0; }
int ori_jolt_body_wy_m(int body) { return body_ok(body) ? g_body[body].wy : 0; }
int ori_jolt_body_wz_m(int body) { return body_ok(body) ? g_body[body].wz : 0; }
int ori_jolt_body_qw_m(int body) { (void)body; return 1000; }
int ori_jolt_body_qx_m(int body) { (void)body; return 0; }
int ori_jolt_body_qy_m(int body) { (void)body; return 0; }
int ori_jolt_body_qz_m(int body) { (void)body; return 0; }

void ori_jolt_set_linear_velocity(int body, int vx, int vy, int vz) {
    if (!body_ok(body)) return;
    g_body[body].vx = vx;
    g_body[body].vy = vy;
    g_body[body].vz = vz;
}
void ori_jolt_add_impulse(int body, int ix, int iy, int iz) {
    if (!body_ok(body)) return;
    g_body[body].vx += ix;
    g_body[body].vy += iy;
    g_body[body].vz += iz;
}
void ori_jolt_add_torque(int body, int tx_m, int ty_m, int tz_m) {
    if (!body_ok(body) || !g_body[body].dynamic) return;
    /* Stub: treat torque as instantaneous angular-velocity impulse. */
    g_body[body].wx += tx_m;
    g_body[body].wy += ty_m;
    g_body[body].wz += tz_m;
}
void ori_jolt_set_position(int body, int x, int y, int z) {
    if (!body_ok(body)) return;
    g_body[body].x = x;
    g_body[body].y = y;
    g_body[body].z = z;
}

void ori_jolt_set_friction(int body, int friction_m) {
    if (!body_ok(body)) return;
    if (friction_m < 0) friction_m = 0;
    g_body[body].friction_m = friction_m;
}
void ori_jolt_set_restitution(int body, int restitution_m) {
    if (!body_ok(body)) return;
    if (restitution_m < 0) restitution_m = 0;
    g_body[body].restitution_m = restitution_m;
}
int ori_jolt_get_friction_m(int body) {
    return body_ok(body) ? g_body[body].friction_m : 0;
}
int ori_jolt_get_restitution_m(int body) {
    return body_ok(body) ? g_body[body].restitution_m : 0;
}

void ori_jolt_set_body_layer(int body, int layer) {
    if (!body_ok(body)) return;
    g_body[body].layer = clamp_layer(layer);
}
int ori_jolt_get_body_layer(int body) {
    return body_ok(body) ? g_body[body].layer : 0;
}
void ori_jolt_set_layer_mask(int system, int layer, int mask) {
    if (!sys_ok(system)) return;
    layer = clamp_layer(layer);
    g_sys[system].layer_mask[layer] = mask & ((1 << NUM_LAYERS) - 1);
}
int ori_jolt_get_layer_mask(int system, int layer) {
    if (!sys_ok(system)) return 0;
    return g_sys[system].layer_mask[clamp_layer(layer)];
}

/* Layers collide if both masks allow the pair (mirrors real filter). */
static int layers_collide(int system, int la, int lb) {
    if (!sys_ok(system)) return 0;
    la = clamp_layer(la);
    lb = clamp_layer(lb);
    int ma = g_sys[system].layer_mask[la];
    int mb = g_sys[system].layer_mask[lb];
    return ((ma & (1 << lb)) != 0 && (mb & (1 << la)) != 0) ? 1 : 0;
}

int ori_jolt_raycast(int system, int ox, int oy, int oz, int dx, int dy, int dz, int max_dist) {
    g_hit_body = 0;
    if (!sys_ok(system)) return 0;
    (void)max_dist;
    /* Prefer body tops (static floors / boxes), then infinite y=0 plane. */
    float best_t = 2.0f;
    int best_body = 0;
    int best_x = 0, best_y = 0, best_z = 0;
    if (dy != 0) {
        for (int i = 1; i < MAX_BODY; ++i) {
            if (!g_body[i].live || g_body[i].sys != system) continue;
            int half_y = g_body[i].kind == 1 ? g_body[i].hx : g_body[i].hy;
            int top = g_body[i].y + half_y;
            /* Ray in milli-space: P = O + D * t, t in [0,1] for segment of length |D|. */
            float t = (float)(top - oy) / (float)dy;
            if (t < 0.0f || t > 1.0f) continue;
            int hx = ox + (int)((float)dx * t);
            int hz = oz + (int)((float)dz * t);
            int half_x = g_body[i].hx;
            int half_z = g_body[i].kind == 1 ? g_body[i].hx : g_body[i].hz;
            if (hx < g_body[i].x - half_x || hx > g_body[i].x + half_x) continue;
            if (hz < g_body[i].z - half_z || hz > g_body[i].z + half_z) continue;
            if (t < best_t) {
                best_t = t;
                best_body = i;
                best_x = hx;
                best_y = top;
                best_z = hz;
            }
        }
    }
    if (best_body) {
        g_hit_x = best_x;
        g_hit_y = best_y;
        g_hit_z = best_z;
        g_hit_frac = (int)(best_t * 1000.0f);
        g_hit_body = best_body;
        (void)layers_collide; /* stored for API parity; Euler stub ignores filters */
        return 1;
    }
    /* Fallback: y = 0 plane. */
    if (dy == 0) return 0;
    float t = -(float)oy / (float)dy;
    if (t < 0.0f || t > 1.0f) return 0;
    g_hit_x = ox + (int)((float)dx * t);
    g_hit_y = 0;
    g_hit_z = oz + (int)((float)dz * t);
    g_hit_frac = (int)(t * 1000.0f);
    g_hit_body = 0;
    return 1;
}
int ori_jolt_last_hit_x_m(void) { return g_hit_x; }
int ori_jolt_last_hit_y_m(void) { return g_hit_y; }
int ori_jolt_last_hit_z_m(void) { return g_hit_z; }
int ori_jolt_last_hit_frac_m(void) { return g_hit_frac; }
int ori_jolt_last_hit_body(void) { return g_hit_body; }

int ori_jolt_body_count_live(void) {
    int n = 0;
    for (int i = 0; i < MAX_BODY; ++i)
        if (g_body[i].live) n++;
    return n;
}

int ori_jolt_create_fixed_constraint(int system, int body_a, int body_b) {
    if (!sys_ok(system) || !body_ok(body_a) || !body_ok(body_b)) return 0;
    int id = alloc_c();
    if (!id) return 0;
    g_c[id].live = 1;
    g_c[id].sys = system;
    g_c[id].kind = 0;
    g_c[id].a = body_a;
    g_c[id].b = body_b;
    return id;
}
int ori_jolt_create_distance_constraint(int system, int body_a, int body_b, int min_m, int max_m) {
    if (!sys_ok(system) || !body_ok(body_a) || !body_ok(body_b)) return 0;
    int id = alloc_c();
    if (!id) return 0;
    g_c[id].live = 1;
    g_c[id].sys = system;
    g_c[id].kind = 1;
    g_c[id].a = body_a;
    g_c[id].b = body_b;
    g_c[id].min_d = min_m;
    g_c[id].max_d = max_m;
    return id;
}
int ori_jolt_create_hinge_constraint(int system, int body_a, int body_b, int ax, int ay, int az) {
    (void)ax;
    (void)ay;
    (void)az;
    return ori_jolt_create_fixed_constraint(system, body_a, body_b);
}
void ori_jolt_destroy_constraint(int c) {
    if (c > 0 && c < MAX_CONSTRAINT) g_c[c].live = 0;
}
int ori_jolt_constraint_is_valid(int c) {
    return (c > 0 && c < MAX_CONSTRAINT && g_c[c].live) ? 1 : 0;
}

int ori_jolt_create_character(int system, int x, int y, int z, int radius, int half_h) {
    if (!sys_ok(system)) return 0;
    int id = alloc_ch();
    if (!id) return 0;
    g_ch[id].live = 1;
    g_ch[id].sys = system;
    g_ch[id].x = x;
    g_ch[id].y = y;
    g_ch[id].z = z;
    g_ch[id].vx = g_ch[id].vy = g_ch[id].vz = 0;
    g_ch[id].radius = radius;
    g_ch[id].half_h = half_h;
    g_ch[id].want_jump = 0;
    g_ch[id].jump_v = 0;
    return id;
}
void ori_jolt_destroy_character(int c) {
    if (c > 0 && c < MAX_CHARS) g_ch[c].live = 0;
}
int ori_jolt_character_is_valid(int c) {
    return (c > 0 && c < MAX_CHARS && g_ch[c].live) ? 1 : 0;
}
void ori_jolt_character_set_velocity(int c, int vx, int vy, int vz) {
    if (!ori_jolt_character_is_valid(c)) return;
    g_ch[c].vx = vx;
    g_ch[c].vy = vy;
    g_ch[c].vz = vz;
}
void ori_jolt_character_jump(int c, int speed) {
    if (!ori_jolt_character_is_valid(c)) return;
    g_ch[c].want_jump = 1;
    g_ch[c].jump_v = speed;
}
int ori_jolt_character_x_m(int c) { return ori_jolt_character_is_valid(c) ? g_ch[c].x : 0; }
int ori_jolt_character_y_m(int c) { return ori_jolt_character_is_valid(c) ? g_ch[c].y : 0; }
int ori_jolt_character_z_m(int c) { return ori_jolt_character_is_valid(c) ? g_ch[c].z : 0; }
int ori_jolt_character_on_ground(int c) {
    if (!ori_jolt_character_is_valid(c)) return 0;
    int feet = g_ch[c].y - g_ch[c].half_h - g_ch[c].radius;
    return feet <= 50 ? 1 : 0;
}

int ori_jolt_create_vehicle(int system, int x, int y, int z, int hx, int hy, int hz) {
    if (!sys_ok(system)) return 0;
    int id = alloc_vh();
    if (!id) return 0;
    int chassis = ori_jolt_create_dynamic_box(system, x, y, z, hx, hy, hz, 1000);
    g_vh[id].live = 1;
    g_vh[id].sys = system;
    g_vh[id].chassis = chassis;
    g_vh[id].throttle = g_vh[id].steer = g_vh[id].brake = 0;
    return id;
}
void ori_jolt_destroy_vehicle(int v) {
    if (v > 0 && v < MAX_VEHS && g_vh[v].live) {
        if (g_vh[v].chassis) ori_jolt_destroy_body(g_vh[v].chassis);
        g_vh[v].live = 0;
    }
}
int ori_jolt_vehicle_is_valid(int v) {
    return (v > 0 && v < MAX_VEHS && g_vh[v].live) ? 1 : 0;
}
void ori_jolt_vehicle_set_input(int v, int throttle, int steer, int brake) {
    if (!ori_jolt_vehicle_is_valid(v)) return;
    g_vh[v].throttle = throttle;
    g_vh[v].steer = steer;
    g_vh[v].brake = brake;
}
int ori_jolt_vehicle_chassis(int v) {
    return ori_jolt_vehicle_is_valid(v) ? g_vh[v].chassis : 0;
}
