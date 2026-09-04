/* ori_rl_* scalar ABI → real raylib (struct-by-value pack/unpack + handle tables).
 * Link with libraylib.a. Do not export raylib names from this unit.
 *
 * ABI: Ori `float` is IEEE f64. All ori_rl_* floating parameters/returns use
 * `double` so System V register passing matches the compiler. Using C `float`
 * (f32) here mis-reads xmm args (often as 0) → black 3D / broken camera.
 */
#include "raylib.h"
#include <stdint.h>
#include <string.h>

#define MAX_TEX 256
#define MAX_SOUND 128
#define MAX_MUSIC 32
#define MAX_MODEL 64
#define MAX_FONT 16
#define MAX_MESH 64

static Texture2D g_tex[MAX_TEX];
static int g_tex_live[MAX_TEX];
static char g_tex_path[MAX_TEX][512];
static Sound g_sound[MAX_SOUND];
static int g_sound_live[MAX_SOUND];
static Music g_music[MAX_MUSIC];
static int g_music_live[MAX_MUSIC];
static Model g_model[MAX_MODEL];
static int g_model_live[MAX_MODEL];
/* Godot-style path cache: never LoadModel(path) twice (editor frame was reloading GLB). */
static char g_model_path[MAX_MODEL][512];
static Font g_font[MAX_FONT];
static int g_font_live[MAX_FONT];
static int g_default_font_id;
static Mesh g_mesh[MAX_MESH];
static int g_mesh_live[MAX_MESH];

static Color col(int r, int g, int b, int a) {
    Color c;
    c.r = (unsigned char)r;
    c.g = (unsigned char)g;
    c.b = (unsigned char)b;
    c.a = (unsigned char)a;
    return c;
}

static const char *cstr(int64_t ptr) {
    if (ptr == 0) return "";
    return (const char *)(intptr_t)ptr;
}

static int alloc_slot(int *live, int maxn) {
    for (int i = 1; i < maxn; ++i) {
        if (!live[i]) {
            live[i] = 1;
            return i;
        }
    }
    return 0;
}

static int tex_ok(int id) { return id > 0 && id < MAX_TEX && g_tex_live[id]; }
static int sound_ok(int id) { return id > 0 && id < MAX_SOUND && g_sound_live[id]; }
static int music_ok(int id) { return id > 0 && id < MAX_MUSIC && g_music_live[id]; }
static int model_ok(int id) { return id > 0 && id < MAX_MODEL && g_model_live[id]; }
static int font_ok(int id) { return id > 0 && id < MAX_FONT && g_font_live[id]; }
static int mesh_ok(int id) { return id > 0 && id < MAX_MESH && g_mesh_live[id]; }

void ori_rl_InitWindow(int width, int height, int64_t title) {
    InitWindow(width, height, cstr(title));
    if (!g_default_font_id) {
        int id = alloc_slot(g_font_live, MAX_FONT);
        if (id) {
            g_font[id] = GetFontDefault();
            g_default_font_id = id;
        }
    }
}

void ori_rl_CloseWindow(void) { CloseWindow(); }

int ori_rl_WindowShouldClose(void) { return WindowShouldClose() ? 1 : 0; }

void ori_rl_SetTargetFPS(int fps) { SetTargetFPS(fps); }

int ori_rl_GetFPS(void) { return GetFPS(); }

double ori_rl_GetFrameTime(void) { return (double)GetFrameTime(); }

double ori_rl_GetTime(void) { return GetTime(); }

void ori_rl_SetConfigFlags(unsigned int flags) { SetConfigFlags(flags); }

void ori_rl_ToggleFullscreen(void) { ToggleFullscreen(); }

int ori_rl_GetScreenWidth(void) { return GetScreenWidth(); }

int ori_rl_GetScreenHeight(void) { return GetScreenHeight(); }

void ori_rl_SetExitKey(int key) { SetExitKey(key); }

void ori_rl_BeginDrawing(void) { BeginDrawing(); }

void ori_rl_EndDrawing(void) { EndDrawing(); }

void ori_rl_ClearBackground(int r, int g, int b, int a) {
    ClearBackground(col(r, g, b, a));
}

void ori_rl_BeginMode2D(
    double ox, double oy, double tx, double ty, double rot, double zoom
) {
    Camera2D cam;
    cam.offset = (Vector2){ox, oy};
    cam.target = (Vector2){tx, ty};
    cam.rotation = rot;
    cam.zoom = zoom;
    BeginMode2D(cam);
}

void ori_rl_EndMode2D(void) { EndMode2D(); }

void ori_rl_BeginMode3D(
    double pos_x, double pos_y, double pos_z,
    double target_x, double target_y, double target_z,
    double up_x, double up_y, double up_z,
    double fovy, int projection
) {
    Camera3D cam;
    cam.position = (Vector3){pos_x, pos_y, pos_z};
    cam.target = (Vector3){target_x, target_y, target_z};
    cam.up = (Vector3){up_x, up_y, up_z};
    cam.fovy = fovy;
    cam.projection = projection;
    BeginMode3D(cam);
}

void ori_rl_EndMode3D(void) { EndMode3D(); }

void ori_rl_DrawLineV(double x1, double y1, double x2, double y2, int r, int g, int b, int a) {
    DrawLineV((Vector2){x1, y1}, (Vector2){x2, y2}, col(r, g, b, a));
}

void ori_rl_DrawLineEx(double x1, double y1, double x2, double y2, double thick, int r, int g, int b, int a) {
    DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, thick, col(r, g, b, a));
}

void ori_rl_DrawCircleV(double x, double y, double radius, int r, int g, int b, int a) {
    DrawCircleV((Vector2){x, y}, radius, col(r, g, b, a));
}

void ori_rl_DrawCircleLinesV(double x, double y, double radius, int r, int g, int b, int a) {
    DrawCircleLinesV((Vector2){x, y}, radius, col(r, g, b, a));
}

void ori_rl_DrawRectangleRec(double x, double y, double w, double h, int r, int g, int b, int a) {
    DrawRectangleRec((Rectangle){x, y, w, h}, col(r, g, b, a));
}

void ori_rl_DrawRectangleLinesEx(double x, double y, double w, double h, int thick, int r, int g, int b, int a) {
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, (float)thick, col(r, g, b, a));
}

void ori_rl_DrawTriangle(double x1, double y1, double x2, double y2, double x3, double y3, int r, int g, int b, int a) {
    DrawTriangle((Vector2){x1, y1}, (Vector2){x2, y2}, (Vector2){x3, y3}, col(r, g, b, a));
}

void ori_rl_DrawPoly(double x, double y, int sides, double radius, double rotation, int r, int g, int b, int a) {
    DrawPoly((Vector2){x, y}, sides, radius, rotation, col(r, g, b, a));
}

void ori_rl_DrawCube(double x, double y, double z, double w, double h, double l, int r, int g, int b, int a) {
    DrawCube((Vector3){x, y, z}, w, h, l, col(r, g, b, a));
}

void ori_rl_DrawCubeWires(double x, double y, double z, double w, double h, double l, int r, int g, int b, int a) {
    DrawCubeWires((Vector3){x, y, z}, w, h, l, col(r, g, b, a));
}

void ori_rl_DrawSphere(double x, double y, double z, double radius, int r, int g, int b, int a) {
    DrawSphere((Vector3){x, y, z}, radius, col(r, g, b, a));
}

void ori_rl_DrawSphereWires(double x, double y, double z, double radius, int rings, int slices, int r, int g, int b, int a) {
    DrawSphereWires((Vector3){x, y, z}, radius, rings, slices, col(r, g, b, a));
}

void ori_rl_DrawPlane(double x, double y, double z, double size_x, double size_y, int r, int g, int b, int a) {
    DrawPlane((Vector3){x, y, z}, (Vector2){size_x, size_y}, col(r, g, b, a));
}

void ori_rl_DrawGrid(int slices, double spacing) { DrawGrid(slices, spacing); }

void ori_rl_DrawRay(double ox, double oy, double oz, double dx, double dy, double dz, int r, int g, int b, int a) {
    Ray ray;
    ray.position = (Vector3){ox, oy, oz};
    ray.direction = (Vector3){dx, dy, dz};
    DrawRay(ray, col(r, g, b, a));
}

void ori_rl_DrawLine3D(
    double start_x, double start_y, double start_z,
    double end_x, double end_y, double end_z,
    int r, int g, int b, int a
) {
    DrawLine3D(
        (Vector3){(float)start_x, (float)start_y, (float)start_z},
        (Vector3){(float)end_x, (float)end_y, (float)end_z},
        col(r, g, b, a)
    );
}

void ori_rl_DrawTriangle3D(
    double v1x, double v1y, double v1z,
    double v2x, double v2y, double v2z,
    double v3x, double v3y, double v3z,
    int r, int g, int b, int a
) {
    DrawTriangle3D(
        (Vector3){(float)v1x, (float)v1y, (float)v1z},
        (Vector3){(float)v2x, (float)v2y, (float)v2z},
        (Vector3){(float)v3x, (float)v3y, (float)v3z},
        col(r, g, b, a)
    );
}

void ori_rl_DrawText(int64_t text, int x, int y, int size, int r, int g, int b, int a) {
    DrawText(cstr(text), x, y, size, col(r, g, b, a));
}

void ori_rl_DrawTextEx(int font, int64_t text, double x, double y, double size, double spacing, int r, int g, int b, int a) {
    Font f = GetFontDefault();
    if (font_ok(font)) f = g_font[font];
    DrawTextEx(f, cstr(text), (Vector2){x, y}, size, spacing, col(r, g, b, a));
}

int ori_rl_MeasureText(int64_t text, int fontSize) {
    return MeasureText(cstr(text), fontSize);
}

int ori_rl_GetFontDefault(void) {
    if (!g_default_font_id) {
        int id = alloc_slot(g_font_live, MAX_FONT);
        if (id) {
            g_font[id] = GetFontDefault();
            g_default_font_id = id;
        }
    }
    return g_default_font_id;
}

int ori_rl_LoadTexture(int64_t fileName) {
    const char *path = cstr(fileName);
    if (path && path[0]) {
        for (int i = 1; i < MAX_TEX; ++i) {
            if (g_tex_live[i] && g_tex_path[i][0] && strcmp(g_tex_path[i], path) == 0) {
                return i; /* cache hit — Godot ResourceCache pattern */
            }
        }
    }
    int id = alloc_slot(g_tex_live, MAX_TEX);
    if (!id) return 0;
    g_tex[id] = LoadTexture(path);
    if (path) {
        strncpy(g_tex_path[id], path, sizeof(g_tex_path[id]) - 1);
        g_tex_path[id][sizeof(g_tex_path[id]) - 1] = '\0';
    } else {
        g_tex_path[id][0] = '\0';
    }
    return id;
}

void ori_rl_UnloadTexture(int texture) {
    if (!tex_ok(texture)) return;
    UnloadTexture(g_tex[texture]);
    g_tex_live[texture] = 0;
    g_tex_path[texture][0] = '\0';
}

void ori_rl_DrawTexture(int texture, int x, int y, int r, int g, int b, int a) {
    if (!tex_ok(texture)) return;
    DrawTexture(g_tex[texture], x, y, col(r, g, b, a));
}

void ori_rl_DrawTextureV(int texture, double x, double y, int r, int g, int b, int a) {
    if (!tex_ok(texture)) return;
    DrawTextureV(g_tex[texture], (Vector2){x, y}, col(r, g, b, a));
}

void ori_rl_DrawTextureRec(
    int texture, double sx, double sy, double sw, double sh, double x, double y, int r, int g, int b, int a
) {
    if (!tex_ok(texture)) return;
    DrawTextureRec(
        g_tex[texture],
        (Rectangle){sx, sy, sw, sh},
        (Vector2){x, y},
        col(r, g, b, a)
    );
}

int ori_rl_GetTextureWidth(int texture) {
    if (!tex_ok(texture)) return 0;
    return g_tex[texture].width;
}

int ori_rl_GetTextureHeight(int texture) {
    if (!tex_ok(texture)) return 0;
    return g_tex[texture].height;
}

/* OpenGL texture name for ImGui::Image (raylib Texture2D.id). */
int ori_rl_GetTextureGlId(int texture) {
    if (!tex_ok(texture)) return 0;
    return (int)g_tex[texture].id;
}

int ori_rl_LoadModel(int64_t fileName) {
    const char *path = cstr(fileName);
    if (path && path[0]) {
        for (int i = 1; i < MAX_MODEL; ++i) {
            if (g_model_live[i] && g_model_path[i][0] && strcmp(g_model_path[i], path) == 0) {
                return i; /* cache hit — never re-LoadModel same path per process */
            }
        }
    }
    int id = alloc_slot(g_model_live, MAX_MODEL);
    if (!id) return 0;
    g_model[id] = LoadModel(path);
    if (path) {
        strncpy(g_model_path[id], path, sizeof(g_model_path[id]) - 1);
        g_model_path[id][sizeof(g_model_path[id]) - 1] = '\0';
    } else {
        g_model_path[id][0] = '\0';
    }
    return id;
}

void ori_rl_UnloadModel(int model) {
    if (!model_ok(model)) return;
    UnloadModel(g_model[model]);
    g_model_live[model] = 0;
    g_model_path[model][0] = '\0';
}

void ori_rl_DrawModel(int model, double x, double y, double z, double scale, int r, int g, int b, int a) {
    if (!model_ok(model)) return;
    DrawModel(g_model[model], (Vector3){x, y, z}, scale, col(r, g, b, a));
}

void ori_rl_DrawModelEx(
    int model,
    double x, double y, double z,
    double axis_x, double axis_y, double axis_z,
    double angle,
    double scale_x, double scale_y, double scale_z,
    int r, int g, int b, int a
) {
    if (!model_ok(model)) return;
    DrawModelEx(
        g_model[model],
        (Vector3){x, y, z},
        (Vector3){axis_x, axis_y, axis_z},
        angle,
        (Vector3){scale_x, scale_y, scale_z},
        col(r, g, b, a)
    );
}

int ori_rl_IsKeyPressed(int key) { return IsKeyPressed(key) ? 1 : 0; }
int ori_rl_IsKeyDown(int key) { return IsKeyDown(key) ? 1 : 0; }
int ori_rl_IsKeyReleased(int key) { return IsKeyReleased(key) ? 1 : 0; }
int ori_rl_IsKeyUp(int key) { return IsKeyUp(key) ? 1 : 0; }
int ori_rl_GetKeyPressed(void) { return GetKeyPressed(); }

double ori_rl_GetMouseX(void) { return (float)GetMouseX(); }
double ori_rl_GetMouseY(void) { return (float)GetMouseY(); }
double ori_rl_GetMouseDeltaX(void) { return GetMouseDelta().x; }
double ori_rl_GetMouseDeltaY(void) { return GetMouseDelta().y; }

int ori_rl_IsMouseButtonPressed(int button) { return IsMouseButtonPressed(button) ? 1 : 0; }
int ori_rl_IsMouseButtonDown(int button) { return IsMouseButtonDown(button) ? 1 : 0; }
int ori_rl_IsMouseButtonReleased(int button) { return IsMouseButtonReleased(button) ? 1 : 0; }
double ori_rl_GetMouseWheelMove(void) { return GetMouseWheelMove(); }

void ori_rl_InitAudioDevice(void) { InitAudioDevice(); }
void ori_rl_CloseAudioDevice(void) { CloseAudioDevice(); }

int ori_rl_LoadSound(int64_t fileName) {
    int id = alloc_slot(g_sound_live, MAX_SOUND);
    if (!id) return 0;
    g_sound[id] = LoadSound(cstr(fileName));
    return id;
}

void ori_rl_UnloadSound(int sound) {
    if (!sound_ok(sound)) return;
    UnloadSound(g_sound[sound]);
    g_sound_live[sound] = 0;
}

void ori_rl_PlaySound(int sound) {
    if (!sound_ok(sound)) return;
    PlaySound(g_sound[sound]);
}

void ori_rl_StopSound(int sound) {
    if (!sound_ok(sound)) return;
    StopSound(g_sound[sound]);
}

void ori_rl_SetSoundVolume(int sound, double volume) {
    if (!sound_ok(sound)) return;
    SetSoundVolume(g_sound[sound], volume);
}

int ori_rl_LoadMusicStream(int64_t fileName) {
    int id = alloc_slot(g_music_live, MAX_MUSIC);
    if (!id) return 0;
    g_music[id] = LoadMusicStream(cstr(fileName));
    return id;
}

void ori_rl_UnloadMusicStream(int music) {
    if (!music_ok(music)) return;
    UnloadMusicStream(g_music[music]);
    g_music_live[music] = 0;
}

void ori_rl_PlayMusicStream(int music) {
    if (!music_ok(music)) return;
    PlayMusicStream(g_music[music]);
}

void ori_rl_StopMusicStream(int music) {
    if (!music_ok(music)) return;
    StopMusicStream(g_music[music]);
}

void ori_rl_UpdateMusicStream(int music) {
    if (!music_ok(music)) return;
    UpdateMusicStream(g_music[music]);
}

void ori_rl_SetMusicVolume(int music, double volume) {
    if (!music_ok(music)) return;
    SetMusicVolume(g_music[music], volume);
}

int ori_rl_IsMusicStreamPlaying(int music) {
    if (!music_ok(music)) return 0;
    return IsMusicStreamPlaying(g_music[music]) ? 1 : 0;
}

/* --- R3 raycast / bounds --- */
static Ray g_last_ray;
static RayCollision g_last_hit;

void ori_rl_DrawBoundingBox(
    double min_x, double min_y, double min_z,
    double max_x, double max_y, double max_z,
    int r, int g, int b, int a
) {
    BoundingBox box;
    box.min = (Vector3){min_x, min_y, min_z};
    box.max = (Vector3){max_x, max_y, max_z};
    DrawBoundingBox(box, col(r, g, b, a));
}

void ori_rl_GetMouseRay(
    double mouse_x, double mouse_y,
    double pos_x, double pos_y, double pos_z,
    double target_x, double target_y, double target_z,
    double up_x, double up_y, double up_z,
    double fovy, int projection
) {
    Camera3D cam;
    cam.position = (Vector3){pos_x, pos_y, pos_z};
    cam.target = (Vector3){target_x, target_y, target_z};
    cam.up = (Vector3){up_x, up_y, up_z};
    cam.fovy = fovy;
    cam.projection = projection;
    g_last_ray = GetMouseRay((Vector2){mouse_x, mouse_y}, cam);
}

int ori_rl_RayHitSphere(
    double ox, double oy, double oz,
    double dx, double dy, double dz,
    double cx, double cy, double cz,
    double radius
) {
    Ray ray;
    ray.position = (Vector3){ox, oy, oz};
    ray.direction = (Vector3){dx, dy, dz};
    g_last_hit = GetRayCollisionSphere(ray, (Vector3){cx, cy, cz}, radius);
    return g_last_hit.hit ? 1 : 0;
}

int ori_rl_RayHitBox(
    double ox, double oy, double oz,
    double dx, double dy, double dz,
    double min_x, double min_y, double min_z,
    double max_x, double max_y, double max_z
) {
    Ray ray;
    ray.position = (Vector3){ox, oy, oz};
    ray.direction = (Vector3){dx, dy, dz};
    BoundingBox box;
    box.min = (Vector3){min_x, min_y, min_z};
    box.max = (Vector3){max_x, max_y, max_z};
    g_last_hit = GetRayCollisionBox(ray, box);
    return g_last_hit.hit ? 1 : 0;
}

int ori_rl_RayHitGround(
    double ox, double oy, double oz,
    double dx, double dy, double dz,
    double ground_y
) {
    Ray ray;
    ray.position = (Vector3){ox, oy, oz};
    ray.direction = (Vector3){dx, dy, dz};
    /* Infinite horizontal plane at y = ground_y via quad large enough */
    Vector3 a = (Vector3){-10000.0f, ground_y, -10000.0f};
    Vector3 b = (Vector3){ 10000.0f, ground_y, -10000.0f};
    Vector3 c = (Vector3){ 10000.0f, ground_y,  10000.0f};
    Vector3 d = (Vector3){-10000.0f, ground_y,  10000.0f};
    g_last_hit = GetRayCollisionQuad(ray, a, b, c, d);
    return g_last_hit.hit ? 1 : 0;
}

double ori_rl_LastRayOx(void) { return g_last_ray.position.x; }
double ori_rl_LastRayOy(void) { return g_last_ray.position.y; }
double ori_rl_LastRayOz(void) { return g_last_ray.position.z; }
double ori_rl_LastRayDx(void) { return g_last_ray.direction.x; }
double ori_rl_LastRayDy(void) { return g_last_ray.direction.y; }
double ori_rl_LastRayDz(void) { return g_last_ray.direction.z; }
double ori_rl_LastHitX(void) { return g_last_hit.point.x; }
double ori_rl_LastHitY(void) { return g_last_hit.point.y; }
double ori_rl_LastHitZ(void) { return g_last_hit.point.z; }
double ori_rl_LastHitDist(void) { return g_last_hit.distance; }
double ori_rl_LastHitNx(void) { return g_last_hit.normal.x; }
double ori_rl_LastHitNy(void) { return g_last_hit.normal.y; }
double ori_rl_LastHitNz(void) { return g_last_hit.normal.z; }

/* --- R3 model texture / animation --- */
#define MAX_ANIM_SETS 64
static ModelAnimation *g_anim_sets[MAX_ANIM_SETS];
static int g_anim_counts[MAX_ANIM_SETS];
static int g_anim_live[MAX_ANIM_SETS];

void ori_rl_SetModelDiffuseTexture(int model, int texture) {
    if (!model_ok(model) || !tex_ok(texture)) return;
    if (g_model[model].materialCount < 1) return;
    SetMaterialTexture(&g_model[model].materials[0], MATERIAL_MAP_DIFFUSE, g_tex[texture]);
}

int ori_rl_LoadModelAnimations(int64_t fileName) {
    int id = 0;
    for (int i = 1; i < MAX_ANIM_SETS; ++i) {
        if (!g_anim_live[i]) { id = i; break; }
    }
    if (!id) return 0;
    int count = 0;
    ModelAnimation *anims = LoadModelAnimations(cstr(fileName), &count);
    if (!anims || count <= 0) return 0;
    g_anim_sets[id] = anims;
    g_anim_counts[id] = count;
    g_anim_live[id] = 1;
    return id;
}

int ori_rl_ModelAnimationCount(int anim_set) {
    if (anim_set <= 0 || anim_set >= MAX_ANIM_SETS || !g_anim_live[anim_set]) return 0;
    return g_anim_counts[anim_set];
}

void ori_rl_UpdateModelAnimation(int model, int anim_set, int anim_index, int frame) {
    if (!model_ok(model)) return;
    if (anim_set <= 0 || anim_set >= MAX_ANIM_SETS || !g_anim_live[anim_set]) return;
    if (anim_index < 0 || anim_index >= g_anim_counts[anim_set]) return;
    UpdateModelAnimation(g_model[model], g_anim_sets[anim_set][anim_index], frame);
}

/* Apply pose; when in_place != 0, zero root bone XZ translation for that frame
 * (keeps vertical root so feet stay grounded) then restore the clip data. */
void ori_rl_UpdateModelAnimationInPlace(int model, int anim_set, int anim_index, int frame) {
    if (!model_ok(model)) return;
    if (anim_set <= 0 || anim_set >= MAX_ANIM_SETS || !g_anim_live[anim_set]) return;
    if (anim_index < 0 || anim_index >= g_anim_counts[anim_set]) return;
    ModelAnimation anim = g_anim_sets[anim_set][anim_index];
    if (frame < 0 || frame >= anim.frameCount) return;
    if (anim.boneCount > 0 && anim.framePoses != NULL && anim.framePoses[frame] != NULL) {
        Vector3 saved = anim.framePoses[frame][0].translation;
        anim.framePoses[frame][0].translation.x = 0.0f;
        anim.framePoses[frame][0].translation.z = 0.0f;
        UpdateModelAnimation(g_model[model], anim, frame);
        anim.framePoses[frame][0].translation = saved;
    } else {
        UpdateModelAnimation(g_model[model], anim, frame);
    }
}

/* GPU skinning: fill mesh.boneMatrices only (vertex shader deforms). */
void ori_rl_UpdateModelAnimationBones(int model, int anim_set, int anim_index, int frame) {
    if (!model_ok(model)) return;
    if (anim_set <= 0 || anim_set >= MAX_ANIM_SETS || !g_anim_live[anim_set]) return;
    if (anim_index < 0 || anim_index >= g_anim_counts[anim_set]) return;
    UpdateModelAnimationBones(g_model[model], g_anim_sets[anim_set][anim_index], frame);
}

void ori_rl_UpdateModelAnimationBonesInPlace(int model, int anim_set, int anim_index, int frame) {
    if (!model_ok(model)) return;
    if (anim_set <= 0 || anim_set >= MAX_ANIM_SETS || !g_anim_live[anim_set]) return;
    if (anim_index < 0 || anim_index >= g_anim_counts[anim_set]) return;
    ModelAnimation anim = g_anim_sets[anim_set][anim_index];
    if (frame < 0 || frame >= anim.frameCount) return;
    if (anim.boneCount > 0 && anim.framePoses != NULL && anim.framePoses[frame] != NULL) {
        Vector3 saved = anim.framePoses[frame][0].translation;
        anim.framePoses[frame][0].translation.x = 0.0f;
        anim.framePoses[frame][0].translation.z = 0.0f;
        UpdateModelAnimationBones(g_model[model], anim, frame);
        anim.framePoses[frame][0].translation = saved;
    } else {
        UpdateModelAnimationBones(g_model[model], anim, frame);
    }
}

/* 1 if any mesh has boneMatrices allocated (GPU skinning ready). */
int ori_rl_ModelSupportsGpuSkinning(int model) {
    if (!model_ok(model)) return 0;
    for (int i = 0; i < g_model[model].meshCount; ++i) {
        if (g_model[model].meshes[i].boneMatrices != NULL &&
            g_model[model].meshes[i].boneCount > 0 &&
            g_model[model].meshes[i].boneIds != NULL) {
            return 1;
        }
    }
    return 0;
}

void ori_rl_UnloadModelAnimations(int anim_set) {
    if (anim_set <= 0 || anim_set >= MAX_ANIM_SETS || !g_anim_live[anim_set]) return;
    UnloadModelAnimations(g_anim_sets[anim_set], g_anim_counts[anim_set]);
    g_anim_sets[anim_set] = NULL;
    g_anim_counts[anim_set] = 0;
    g_anim_live[anim_set] = 0;
}

/* --- shaders --- */
#define MAX_SHADERS 64
static Shader g_shaders[MAX_SHADERS];
static int g_shader_live[MAX_SHADERS];

static int alloc_shader(void) {
    for (int i = 1; i < MAX_SHADERS; ++i)
        if (!g_shader_live[i]) return i;
    return 0;
}
static int shader_ok(int id) {
    return id > 0 && id < MAX_SHADERS && g_shader_live[id];
}

/* Assign skinning shader to every material (DrawMesh uploads boneMatrices). */
void ori_rl_ModelSetAllMaterialsShader(int model, int shader) {
    if (!model_ok(model) || !shader_ok(shader)) return;
    for (int i = 0; i < g_model[model].materialCount; ++i) {
        g_model[model].materials[i].shader = g_shaders[shader];
    }
}

int ori_rl_LoadShader(int64_t vsPath, int64_t fsPath) {
    int id = alloc_shader();
    if (!id) return 0;
    const char *vs = (vsPath == 0) ? 0 : cstr(vsPath);
    const char *fs = (fsPath == 0) ? 0 : cstr(fsPath);
    /* raylib accepts NULL for default vertex/fragment */
    g_shaders[id] = LoadShader(vs, fs);
    g_shader_live[id] = 1;
    return id;
}

void ori_rl_UnloadShader(int shader) {
    if (!shader_ok(shader)) return;
    UnloadShader(g_shaders[shader]);
    g_shader_live[shader] = 0;
}

void ori_rl_BeginShaderMode(int shader) {
    if (!shader_ok(shader)) return;
    BeginShaderMode(g_shaders[shader]);
}

void ori_rl_EndShaderMode(void) { EndShaderMode(); }

int ori_rl_GetShaderLocation(int shader, int64_t name) {
    if (!shader_ok(shader)) return -1;
    return GetShaderLocation(g_shaders[shader], cstr(name));
}

void ori_rl_SetShaderValueFloat(int shader, int loc, double value) {
    if (!shader_ok(shader) || loc < 0) return;
    SetShaderValue(g_shaders[shader], loc, &value, SHADER_UNIFORM_FLOAT);
}

void ori_rl_SetShaderValueVec3(int shader, int loc, double x, double y, double z) {
    if (!shader_ok(shader) || loc < 0) return;
    double v[3] = { x, y, z };
    SetShaderValue(g_shaders[shader], loc, v, SHADER_UNIFORM_VEC3);
}

void ori_rl_SetShaderValueVec4(int shader, int loc, double x, double y, double z, double w) {
    if (!shader_ok(shader) || loc < 0) return;
    double v[4] = { x, y, z, w };
    SetShaderValue(g_shaders[shader], loc, v, SHADER_UNIFORM_VEC4);
}

void ori_rl_SetShaderValueInt(int shader, int loc, int value) {
    if (!shader_ok(shader) || loc < 0) return;
    SetShaderValue(g_shaders[shader], loc, &value, SHADER_UNIFORM_INT);
}

int ori_rl_ModelAnimationFrameCount(int anim_set, int anim_index) {
    if (anim_set <= 0 || anim_set >= MAX_ANIM_SETS || !g_anim_live[anim_set]) return 0;
    if (anim_index < 0 || anim_index >= g_anim_counts[anim_set]) return 0;
    return g_anim_sets[anim_set][anim_index].frameCount;
}

/* --- gamepad --- */
int ori_rl_IsGamepadAvailable(int gamepad) {
    return IsGamepadAvailable(gamepad) ? 1 : 0;
}
int ori_rl_IsGamepadButtonPressed(int gamepad, int button) {
    return IsGamepadButtonPressed(gamepad, button) ? 1 : 0;
}
int ori_rl_IsGamepadButtonDown(int gamepad, int button) {
    return IsGamepadButtonDown(gamepad, button) ? 1 : 0;
}
int ori_rl_IsGamepadButtonReleased(int gamepad, int button) {
    return IsGamepadButtonReleased(gamepad, button) ? 1 : 0;
}
double ori_rl_GetGamepadAxisMovement(int gamepad, int axis) {
    return GetGamepadAxisMovement(gamepad, axis);
}
int ori_rl_GetGamepadButtonPressed(void) {
    return GetGamepadButtonPressed();
}

/* --- render texture --- */
#define MAX_RT 32
static RenderTexture2D g_rt[MAX_RT];
static int g_rt_live[MAX_RT];

static int alloc_rt(void) {
    for (int i = 1; i < MAX_RT; ++i)
        if (!g_rt_live[i]) return i;
    return 0;
}
static int rt_ok(int id) {
    return id > 0 && id < MAX_RT && g_rt_live[id];
}

int ori_rl_LoadRenderTexture(int width, int height) {
    int id = alloc_rt();
    if (!id) return 0;
    g_rt[id] = LoadRenderTexture(width, height);
    g_rt_live[id] = 1;
    return id;
}

void ori_rl_UnloadRenderTexture(int rt) {
    if (!rt_ok(rt)) return;
    UnloadRenderTexture(g_rt[rt]);
    g_rt_live[rt] = 0;
}

void ori_rl_BeginTextureMode(int rt) {
    if (!rt_ok(rt)) return;
    BeginTextureMode(g_rt[rt]);
}

void ori_rl_EndTextureMode(void) { EndTextureMode(); }

void ori_rl_DrawRenderTexture(int rt, int posX, int posY, int r, int g, int b, int a) {
    if (!rt_ok(rt)) return;
    DrawTexture(g_rt[rt].texture, posX, posY, col(r, g, b, a));
}

void ori_rl_DrawRenderTextureRec(
    int rt,
    double srcX, double srcY, double srcW, double srcH,
    double dstX, double dstY, double dstW, double dstH,
    int r, int g, int b, int a
) {
    if (!rt_ok(rt)) return;
    Rectangle src = { srcX, srcY, srcW, srcH };
    Rectangle dst = { dstX, dstY, dstW, dstH };
    Vector2 origin = { 0.0f, 0.0f };
    DrawTexturePro(g_rt[rt].texture, src, dst, origin, 0.0f, col(r, g, b, a));
}

int ori_rl_GetRenderTextureWidth(int rt) {
    if (!rt_ok(rt)) return 0;
    return g_rt[rt].texture.width;
}

int ori_rl_GetRenderTextureHeight(int rt) {
    if (!rt_ok(rt)) return 0;
    return g_rt[rt].texture.height;
}

/* --- P2-B audio deepen --- */
void ori_rl_SetMasterVolume(double volume) { SetMasterVolume(volume); }
double ori_rl_GetMasterVolume(void) { return GetMasterVolume(); }

void ori_rl_PauseSound(int sound) {
    if (!sound_ok(sound)) return;
    PauseSound(g_sound[sound]);
}
void ori_rl_ResumeSound(int sound) {
    if (!sound_ok(sound)) return;
    ResumeSound(g_sound[sound]);
}
int ori_rl_IsSoundPlaying(int sound) {
    if (!sound_ok(sound)) return 0;
    return IsSoundPlaying(g_sound[sound]) ? 1 : 0;
}
void ori_rl_SetSoundPitch(int sound, double pitch) {
    if (!sound_ok(sound)) return;
    SetSoundPitch(g_sound[sound], pitch);
}
void ori_rl_SetSoundPan(int sound, double pan) {
    if (!sound_ok(sound)) return;
    SetSoundPan(g_sound[sound], pan);
}

void ori_rl_PauseMusicStream(int music) {
    if (!music_ok(music)) return;
    PauseMusicStream(g_music[music]);
}
void ori_rl_ResumeMusicStream(int music) {
    if (!music_ok(music)) return;
    ResumeMusicStream(g_music[music]);
}
void ori_rl_SetMusicPitch(int music, double pitch) {
    if (!music_ok(music)) return;
    SetMusicPitch(g_music[music], pitch);
}
double ori_rl_GetMusicTimeLength(int music) {
    if (!music_ok(music)) return 0.0f;
    return GetMusicTimeLength(g_music[music]);
}
double ori_rl_GetMusicTimePlayed(int music) {
    if (!music_ok(music)) return 0.0f;
    return GetMusicTimePlayed(g_music[music]);
}

/* --- P2-B 3D deepen --- */
void ori_rl_DrawCylinder(
    double x, double y, double z,
    double radiusTop, double radiusBottom, double height, int slices,
    int r, int g, int b, int a
) {
    DrawCylinder((Vector3){ x, y, z }, radiusTop, radiusBottom, height, slices, col(r, g, b, a));
}
void ori_rl_DrawCylinderWires(
    double x, double y, double z,
    double radiusTop, double radiusBottom, double height, int slices,
    int r, int g, int b, int a
) {
    DrawCylinderWires((Vector3){ x, y, z }, radiusTop, radiusBottom, height, slices, col(r, g, b, a));
}
void ori_rl_DrawCapsule(
    double ax, double ay, double az, double bx, double by, double bz, double radius,
    int slices, int rings, int r, int g, int b, int a
) {
    DrawCapsule((Vector3){ ax, ay, az }, (Vector3){ bx, by, bz }, radius, slices, rings, col(r, g, b, a));
}
void ori_rl_DrawCapsuleWires(
    double ax, double ay, double az, double bx, double by, double bz, double radius,
    int slices, int rings, int r, int g, int b, int a
) {
    DrawCapsuleWires((Vector3){ ax, ay, az }, (Vector3){ bx, by, bz }, radius, slices, rings, col(r, g, b, a));
}
void ori_rl_DrawModelWires(int model, double x, double y, double z, double scale, int r, int g, int b, int a) {
    if (!model_ok(model)) return;
    DrawModelWires(g_model[model], (Vector3){ x, y, z }, scale, col(r, g, b, a));
}
void ori_rl_DrawBillboard(
    double cpx, double cpy, double cpz,
    double ctx, double cty, double ctz,
    double cux, double cuy, double cuz,
    double fovy, int projection,
    int texture,
    double px, double py, double pz, double scale,
    int r, int g, int b, int a
) {
    if (!tex_ok(texture)) return;
    Camera3D cam = { 0 };
    cam.position = (Vector3){ cpx, cpy, cpz };
    cam.target = (Vector3){ ctx, cty, ctz };
    cam.up = (Vector3){ cux, cuy, cuz };
    cam.fovy = fovy;
    cam.projection = projection;
    DrawBillboard(cam, g_tex[texture], (Vector3){ px, py, pz }, scale, col(r, g, b, a));
}

void ori_rl_TakeScreenshot(int64_t fileName) {
    TakeScreenshot(cstr(fileName));
}

/* Gray8 (or pitch row) → RGBA texture. ptr is int64 for Ori ABI. */
int ori_rl_LoadTextureFromGray8(int64_t data_ptr, int width, int height, int pitch) {
    const unsigned char *src = (const unsigned char *)(intptr_t)data_ptr;
    if (!src || width <= 0 || height <= 0) return 0;
    int row = pitch > 0 ? pitch : width;
    size_t n = (size_t)width * (size_t)height * 4u;
    unsigned char *rgba = (unsigned char *)MemAlloc((unsigned int)n);
    if (!rgba) return 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char g = src[y * row + x];
            size_t o = ((size_t)y * (size_t)width + (size_t)x) * 4u;
            rgba[o + 0] = g;
            rgba[o + 1] = g;
            rgba[o + 2] = g;
            rgba[o + 3] = g; /* alpha = coverage */
        }
    }
    Image img = {0};
    img.data = rgba;
    img.width = width;
    img.height = height;
    img.mipmaps = 1;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    int id = alloc_slot(g_tex_live, MAX_TEX);
    if (!id) {
        MemFree(rgba);
        return 0;
    }
    g_tex[id] = LoadTextureFromImage(img);
    UnloadImage(img); /* frees rgba */
    return id;
}

void ori_rl_SeekMusicStream(int music, double position_sec) {
    if (!music_ok(music)) return;
    SeekMusicStream(g_music[music], (float)position_sec);
}

/* Upload triangle soup: verts_xyz_m = [x,y,z,...] milli-int; indices length multiple of 3.
 * data_ptr is int64; arrays are int64_t for Ori list storage compatibility when pinned —
 * here we take contiguous int32_t milli via int64_t elements written by caller as C array. */
int ori_rl_UploadMeshMilli(
    int64_t xyz_m_ptr,
    int vert_count,
    int64_t indices_ptr,
    int index_count
) {
    const int64_t *xyz = (const int64_t *)(intptr_t)xyz_m_ptr;
    const int64_t *idx = (const int64_t *)(intptr_t)indices_ptr;
    if (!xyz || !idx || vert_count <= 0 || index_count < 3) return 0;
    if ((index_count % 3) != 0) return 0;

    int id = alloc_slot(g_mesh_live, MAX_MESH);
    if (!id) return 0;

    Mesh mesh = {0};
    mesh.vertexCount = vert_count;
    mesh.triangleCount = index_count / 3;
    mesh.vertices = (float *)MemAlloc((unsigned int)(vert_count * 3 * (int)sizeof(float)));
    mesh.normals = (float *)MemAlloc((unsigned int)(vert_count * 3 * (int)sizeof(float)));
    mesh.texcoords = (float *)MemAlloc((unsigned int)(vert_count * 2 * (int)sizeof(float)));
    mesh.indices = (unsigned short *)MemAlloc((unsigned int)(index_count * (int)sizeof(unsigned short)));
    if (!mesh.vertices || !mesh.normals || !mesh.texcoords || !mesh.indices) {
        if (mesh.vertices) MemFree(mesh.vertices);
        if (mesh.normals) MemFree(mesh.normals);
        if (mesh.texcoords) MemFree(mesh.texcoords);
        if (mesh.indices) MemFree(mesh.indices);
        g_mesh_live[id] = 0;
        return 0;
    }
    for (int i = 0; i < vert_count; ++i) {
        mesh.vertices[i * 3 + 0] = (float)xyz[i * 3 + 0] / 1000.0f;
        mesh.vertices[i * 3 + 1] = (float)xyz[i * 3 + 1] / 1000.0f;
        mesh.vertices[i * 3 + 2] = (float)xyz[i * 3 + 2] / 1000.0f;
        mesh.normals[i * 3 + 0] = 0.0f;
        mesh.normals[i * 3 + 1] = 1.0f;
        mesh.normals[i * 3 + 2] = 0.0f;
        mesh.texcoords[i * 2 + 0] = 0.0f;
        mesh.texcoords[i * 2 + 1] = 0.0f;
    }
    for (int i = 0; i < index_count; ++i) {
        int v = (int)idx[i];
        if (v < 0) v = 0;
        if (v >= vert_count) v = vert_count - 1;
        mesh.indices[i] = (unsigned short)v;
    }
    UploadMesh(&mesh, false);
    g_mesh[id] = mesh;
    return id;
}

void ori_rl_UnloadMesh(int mesh) {
    if (!mesh_ok(mesh)) return;
    UnloadMesh(g_mesh[mesh]);
    g_mesh_live[mesh] = 0;
}

void ori_rl_DrawMesh(int mesh, double x, double y, double z, int r, int g, int b, int a) {
    if (!mesh_ok(mesh)) return;
    /* Identity + translation without raymath (not always linked). */
    Matrix m = {
        1.0f, 0.0f, 0.0f, (float)x,
        0.0f, 1.0f, 0.0f, (float)y,
        0.0f, 0.0f, 1.0f, (float)z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    Material mat = LoadMaterialDefault();
    mat.maps[MATERIAL_MAP_DIFFUSE].color = col(r, g, b, a);
    DrawMesh(g_mesh[mesh], mat, m);
}

int ori_rl_LoadModelFromMeshId(int mesh) {
    if (!mesh_ok(mesh)) return 0;
    int id = alloc_slot(g_model_live, MAX_MODEL);
    if (!id) return 0;
    g_model[id] = LoadModelFromMesh(g_mesh[mesh]);
    /* Mesh is now owned by model; clear mesh slot without UnloadMesh */
    g_mesh_live[mesh] = 0;
    memset(&g_mesh[mesh], 0, sizeof(Mesh));
    return id;
}

/* --- OS helpers (Studio C5: open script in external editor) --- */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void ori_rl_OpenURL(int64_t url) {
    OpenURL(cstr(url));
}

void ori_rl_OpenPath(int64_t path) {
    const char *p = cstr(path);
    if (!p || !p[0]) return;
    char resolved[PATH_MAX];
    char url[PATH_MAX + 16];
    if (realpath(p, resolved) != NULL) {
        snprintf(url, sizeof(url), "file://%s", resolved);
    } else {
        snprintf(url, sizeof(url), "file://%s", p);
    }
    OpenURL(url);
}
