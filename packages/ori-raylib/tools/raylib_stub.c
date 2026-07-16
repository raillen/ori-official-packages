/* Headless ori_rl_* stub (scalar ABI) for link/smoke without X11.
 * Replace with a real libraylib.a for actual windowing (see setup_raylib_linux.sh).
 */
#include <stdint.h>
#include <stddef.h>

static int g_frames;
static int g_max_frames = 3;
static double g_mouse_x;
static double g_mouse_y;

void ori_rl_InitWindow(int width, int height, int title) {
    (void)width;
    (void)height;
    (void)title;
    g_frames = 0;
}

void ori_rl_CloseWindow(void) {}

int ori_rl_WindowShouldClose(void) {
    g_frames += 1;
    return g_frames >= g_max_frames ? 1 : 0;
}

void ori_rl_SetTargetFPS(int fps) { (void)fps; }
int ori_rl_GetFPS(void) { return 60; }
double ori_rl_GetFrameTime(void) { return 1.0f / 60.0f; }
double ori_rl_GetTime(void) { return (float)g_frames / 60.0; }
void ori_rl_SetConfigFlags(unsigned int flags) { (void)flags; }
void ori_rl_ToggleFullscreen(void) {}
int ori_rl_GetScreenWidth(void) { return 800; }
int ori_rl_GetScreenHeight(void) { return 600; }
void ori_rl_SetExitKey(int key) { (void)key; }

void ori_rl_BeginDrawing(void) {}
void ori_rl_EndDrawing(void) {}
void ori_rl_ClearBackground(int r, int g, int b, int a) {
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_BeginMode2D(double ox, double oy, double tx, double ty, double rot, double zoom) {
    (void)ox;
    (void)oy;
    (void)tx;
    (void)ty;
    (void)rot;
    (void)zoom;
}
void ori_rl_EndMode2D(void) {}

void ori_rl_DrawLineV(double x1, double y1, double x2, double y2, int r, int g, int b, int a) {
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawLineEx(double x1, double y1, double x2, double y2, double thick, int r, int g, int b, int a) {
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)thick;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawCircleV(double x, double y, double radius, int r, int g, int b, int a) {
    (void)x;
    (void)y;
    (void)radius;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawCircleLinesV(double x, double y, double radius, int r, int g, int b, int a) {
    (void)x;
    (void)y;
    (void)radius;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawRectangleRec(double x, double y, double w, double h, int r, int g, int b, int a) {
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawRectangleLinesEx(double x, double y, double w, double h, int thick, int r, int g, int b, int a) {
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)thick;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawTriangle(double x1, double y1, double x2, double y2, double x3, double y3, int r, int g, int b, int a) {
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)x3;
    (void)y3;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawPoly(double x, double y, int sides, double radius, double rotation, int r, int g, int b, int a) {
    (void)x;
    (void)y;
    (void)sides;
    (void)radius;
    (void)rotation;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawText(int text, int x, int y, int size, int r, int g, int b, int a) {
    (void)text;
    (void)x;
    (void)y;
    (void)size;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawTextEx(int font, int text, double x, double y, double size, double spacing, int r, int g, int b, int a) {
    (void)font;
    (void)text;
    (void)x;
    (void)y;
    (void)size;
    (void)spacing;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
int ori_rl_MeasureText(int text, int fontSize) {
    (void)text;
    (void)fontSize;
    return 10;
}
int ori_rl_GetFontDefault(void) { return 0; }

int ori_rl_LoadTexture(int fileName) {
    (void)fileName;
    return 1;
}
void ori_rl_UnloadTexture(int texture) { (void)texture; }
void ori_rl_DrawTexture(int texture, int x, int y, int r, int g, int b, int a) {
    (void)texture;
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawTextureV(int texture, double x, double y, int r, int g, int b, int a) {
    (void)texture;
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
void ori_rl_DrawTextureRec(int texture, double sx, double sy, double sw, double sh, double x, double y, int r, int g, int b, int a) {
    (void)texture;
    (void)sx;
    (void)sy;
    (void)sw;
    (void)sh;
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}
int ori_rl_GetTextureWidth(int texture) {
    (void)texture;
    return 1;
}
int ori_rl_GetTextureHeight(int texture) {
    (void)texture;
    return 1;
}

int ori_rl_GetTextureGlId(int texture) {
    (void)texture;
    return 0;
}

int ori_rl_IsKeyPressed(int key) {
    (void)key;
    return 0;
}
int ori_rl_IsKeyDown(int key) {
    (void)key;
    return 0;
}
int ori_rl_IsKeyReleased(int key) {
    (void)key;
    return 0;
}
int ori_rl_IsKeyUp(int key) {
    (void)key;
    return 1;
}
int ori_rl_GetKeyPressed(void) { return 0; }
double ori_rl_GetMouseX(void) { return g_mouse_x; }
double ori_rl_GetMouseY(void) { return g_mouse_y; }
double ori_rl_GetMouseDeltaX(void) { return 0.0f; }
double ori_rl_GetMouseDeltaY(void) { return 0.0f; }
int ori_rl_IsMouseButtonPressed(int button) {
    (void)button;
    return 0;
}
int ori_rl_IsMouseButtonDown(int button) {
    (void)button;
    return 0;
}
int ori_rl_IsMouseButtonReleased(int button) {
    (void)button;
    return 0;
}
double ori_rl_GetMouseWheelMove(void) { return 0.0f; }

void ori_rl_InitAudioDevice(void) {}
void ori_rl_CloseAudioDevice(void) {}
int ori_rl_LoadSound(int fileName) {
    (void)fileName;
    return 1;
}
void ori_rl_UnloadSound(int sound) { (void)sound; }
void ori_rl_PlaySound(int sound) { (void)sound; }
void ori_rl_StopSound(int sound) { (void)sound; }
void ori_rl_SetSoundVolume(int sound, double volume) {
    (void)sound;
    (void)volume;
}
int ori_rl_LoadMusicStream(int fileName) {
    (void)fileName;
    return 1;
}
void ori_rl_UnloadMusicStream(int music) { (void)music; }
void ori_rl_PlayMusicStream(int music) { (void)music; }
void ori_rl_StopMusicStream(int music) { (void)music; }
void ori_rl_UpdateMusicStream(int music) { (void)music; }
void ori_rl_SetMusicVolume(int music, double volume) {
    (void)music;
    (void)volume;
}
int ori_rl_IsMusicStreamPlaying(int music) {
    (void)music;
    return 0;
}

/* --- 3D (scalarized ABI matching raylib.orl) --- */
void ori_rl_BeginMode3D(
    double pos_x, double pos_y, double pos_z,
    double target_x, double target_y, double target_z,
    double up_x, double up_y, double up_z,
    double fovy, int projection
) {
    (void)pos_x; (void)pos_y; (void)pos_z;
    (void)target_x; (void)target_y; (void)target_z;
    (void)up_x; (void)up_y; (void)up_z;
    (void)fovy; (void)projection;
}
void ori_rl_EndMode3D(void) {}

void ori_rl_DrawCube(double x, double y, double z, double w, double h, double l, int r, int g, int b, int a) {
    (void)x; (void)y; (void)z; (void)w; (void)h; (void)l;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawCubeWires(double x, double y, double z, double w, double h, double l, int r, int g, int b, int a) {
    (void)x; (void)y; (void)z; (void)w; (void)h; (void)l;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawSphere(double x, double y, double z, double radius, int r, int g, int b, int a) {
    (void)x; (void)y; (void)z; (void)radius;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawSphereWires(double x, double y, double z, double radius, int rings, int slices, int r, int g, int b, int a) {
    (void)x; (void)y; (void)z; (void)radius; (void)rings; (void)slices;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawPlane(double x, double y, double z, double size_x, double size_y, int r, int g, int b, int a) {
    (void)x; (void)y; (void)z; (void)size_x; (void)size_y;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawGrid(int slices, double spacing) {
    (void)slices; (void)spacing;
}
void ori_rl_DrawRay(double ox, double oy, double oz, double dx, double dy, double dz, int r, int g, int b, int a) {
    (void)ox; (void)oy; (void)oz; (void)dx; (void)dy; (void)dz;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawLine3D(
    double start_x, double start_y, double start_z,
    double end_x, double end_y, double end_z,
    int r, int g, int b, int a
) {
    (void)start_x; (void)start_y; (void)start_z;
    (void)end_x; (void)end_y; (void)end_z;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawTriangle3D(
    double v1x, double v1y, double v1z,
    double v2x, double v2y, double v2z,
    double v3x, double v3y, double v3z,
    int r, int g, int b, int a
) {
    (void)v1x; (void)v1y; (void)v1z;
    (void)v2x; (void)v2y; (void)v2z;
    (void)v3x; (void)v3y; (void)v3z;
    (void)r; (void)g; (void)b; (void)a;
}

int ori_rl_LoadModel(int fileName) {
    (void)fileName;
    return 1;
}
void ori_rl_UnloadModel(int model) { (void)model; }
void ori_rl_DrawModel(int model, double x, double y, double z, double scale, int r, int g, int b, int a) {
    (void)model; (void)x; (void)y; (void)z; (void)scale;
    (void)r; (void)g; (void)b; (void)a;
}
void ori_rl_DrawModelEx(
    int model,
    double x, double y, double z,
    double axis_x, double axis_y, double axis_z,
    double angle,
    double scale_x, double scale_y, double scale_z,
    int r, int g, int b, int a
) {
    (void)model;
    (void)x; (void)y; (void)z;
    (void)axis_x; (void)axis_y; (void)axis_z;
    (void)angle;
    (void)scale_x; (void)scale_y; (void)scale_z;
    (void)r; (void)g; (void)b; (void)a;
}

/* --- R3 raycast / bounds --- */
static double g_ray_o[3], g_ray_d[3];
static double g_hit_p[3], g_hit_n[3], g_hit_dist;

void ori_rl_DrawBoundingBox(double min_x, double min_y, double min_z, double max_x, double max_y, double max_z, int r, int g, int b, int a) {
    (void)min_x;(void)min_y;(void)min_z;(void)max_x;(void)max_y;(void)max_z;(void)r;(void)g;(void)b;(void)a;
}

void ori_rl_GetMouseRay(
    double mouse_x, double mouse_y,
    double pos_x, double pos_y, double pos_z,
    double target_x, double target_y, double target_z,
    double up_x, double up_y, double up_z,
    double fovy, int projection
) {
    (void)mouse_x;(void)mouse_y;(void)pos_x;(void)pos_y;(void)pos_z;
    (void)target_x;(void)target_y;(void)target_z;(void)up_x;(void)up_y;(void)up_z;(void)fovy;(void)projection;
    g_ray_o[0]=pos_x; g_ray_o[1]=pos_y; g_ray_o[2]=pos_z;
    g_ray_d[0]=target_x-pos_x; g_ray_d[1]=target_y-pos_y; g_ray_d[2]=target_z-pos_z;
}

int ori_rl_RayHitSphere(double ox, double oy, double oz, double dx, double dy, double dz, double cx, double cy, double cz, double radius) {
    (void)ox;(void)oy;(void)oz;(void)dx;(void)dy;(void)dz;(void)cx;(void)cy;(void)cz;(void)radius;
    g_hit_p[0]=cx; g_hit_p[1]=cy; g_hit_p[2]=cz; g_hit_n[1]=1.0f; g_hit_dist=1.0f;
    return 1;
}
int ori_rl_RayHitBox(double ox, double oy, double oz, double dx, double dy, double dz,
    double min_x, double min_y, double min_z, double max_x, double max_y, double max_z) {
    (void)ox;(void)oy;(void)oz;(void)dx;(void)dy;(void)dz;
    (void)min_x;(void)min_y;(void)min_z;(void)max_x;(void)max_y;(void)max_z;
    g_hit_p[0]=(min_x+max_x)*0.5f; g_hit_p[1]=(min_y+max_y)*0.5f; g_hit_p[2]=(min_z+max_z)*0.5f;
    g_hit_n[1]=1.0f; g_hit_dist=1.0f;
    return 1;
}
int ori_rl_RayHitGround(double ox, double oy, double oz, double dx, double dy, double dz, double ground_y) {
    (void)ox;(void)oy;(void)oz;(void)dx;(void)dy;(void)dz;(void)ground_y;
    if (dy == 0.0f) return 0;
    double t = (ground_y - oy) / dy;
    if (t < 0.0f) return 0;
    g_hit_p[0]=ox+dx*t; g_hit_p[1]=ground_y; g_hit_p[2]=oz+dz*t;
    g_hit_n[0]=0; g_hit_n[1]=1; g_hit_n[2]=0; g_hit_dist=t;
    return 1;
}
double ori_rl_LastRayOx(void){return g_ray_o[0];}
double ori_rl_LastRayOy(void){return g_ray_o[1];}
double ori_rl_LastRayOz(void){return g_ray_o[2];}
double ori_rl_LastRayDx(void){return g_ray_d[0];}
double ori_rl_LastRayDy(void){return g_ray_d[1];}
double ori_rl_LastRayDz(void){return g_ray_d[2];}
double ori_rl_LastHitX(void){return g_hit_p[0];}
double ori_rl_LastHitY(void){return g_hit_p[1];}
double ori_rl_LastHitZ(void){return g_hit_p[2];}
double ori_rl_LastHitDist(void){return g_hit_dist;}
double ori_rl_LastHitNx(void){return g_hit_n[0];}
double ori_rl_LastHitNy(void){return g_hit_n[1];}
double ori_rl_LastHitNz(void){return g_hit_n[2];}

/* --- R3 model texture / animation --- */
static int g_anim_count[64];
static int g_anim_live[64];

void ori_rl_SetModelDiffuseTexture(int model, int texture) {
    (void)model; (void)texture;
}
int ori_rl_LoadModelAnimations(int fileName) {
    (void)fileName;
    for (int i = 1; i < 64; ++i) {
        if (!g_anim_live[i]) {
            g_anim_live[i] = 1;
            g_anim_count[i] = 1;
            return i;
        }
    }
    return 0;
}
int ori_rl_ModelAnimationCount(int anim_set) {
    if (anim_set <= 0 || anim_set >= 64 || !g_anim_live[anim_set]) return 0;
    return g_anim_count[anim_set];
}
void ori_rl_UpdateModelAnimation(int model, int anim_set, int anim_index, int frame) {
    (void)model; (void)anim_set; (void)anim_index; (void)frame;
}
void ori_rl_UnloadModelAnimations(int anim_set) {
    if (anim_set > 0 && anim_set < 64) g_anim_live[anim_set] = 0;
}

/* --- shaders / lights / anim frame count --- */
static int g_shader_live[64];
static int g_shader_next = 1;

int ori_rl_LoadShader(int vs, int fs) {
    (void)vs; (void)fs;
    if (g_shader_next >= 64) return 0;
    int id = g_shader_next++;
    g_shader_live[id] = 1;
    return id;
}
void ori_rl_UnloadShader(int shader) {
    if (shader > 0 && shader < 64) g_shader_live[shader] = 0;
}
void ori_rl_BeginShaderMode(int shader) { (void)shader; }
void ori_rl_EndShaderMode(void) {}
int ori_rl_GetShaderLocation(int shader, int name) { (void)shader; (void)name; return 0; }
void ori_rl_SetShaderValueFloat(int shader, int loc, double value) { (void)shader;(void)loc;(void)value; }
void ori_rl_SetShaderValueVec3(int shader, int loc, double x, double y, double z) {
    (void)shader;(void)loc;(void)x;(void)y;(void)z;
}
void ori_rl_SetShaderValueVec4(int shader, int loc, double x, double y, double z, double w) {
    (void)shader;(void)loc;(void)x;(void)y;(void)z;(void)w;
}
void ori_rl_SetShaderValueInt(int shader, int loc, int value) { (void)shader;(void)loc;(void)value; }
int ori_rl_ModelAnimationFrameCount(int anim_set, int anim_index) {
    (void)anim_set; (void)anim_index;
    return 30;
}

/* --- gamepad (stub) --- */
int ori_rl_IsGamepadAvailable(int gamepad) { (void)gamepad; return 0; }
int ori_rl_IsGamepadButtonPressed(int gamepad, int button) { (void)gamepad;(void)button; return 0; }
int ori_rl_IsGamepadButtonDown(int gamepad, int button) { (void)gamepad;(void)button; return 0; }
int ori_rl_IsGamepadButtonReleased(int gamepad, int button) { (void)gamepad;(void)button; return 0; }
double ori_rl_GetGamepadAxisMovement(int gamepad, int axis) { (void)gamepad;(void)axis; return 0.0f; }
int ori_rl_GetGamepadButtonPressed(void) { return 0; }

/* --- render texture (stub) --- */
static int g_rt_live[32];
static int g_rt_w[32];
static int g_rt_h[32];
static int g_rt_next = 1;

int ori_rl_LoadRenderTexture(int width, int height) {
    if (g_rt_next >= 32) return 0;
    int id = g_rt_next++;
    g_rt_live[id] = 1;
    g_rt_w[id] = width;
    g_rt_h[id] = height;
    return id;
}
void ori_rl_UnloadRenderTexture(int rt) {
    if (rt > 0 && rt < 32) g_rt_live[rt] = 0;
}
void ori_rl_BeginTextureMode(int rt) { (void)rt; }
void ori_rl_EndTextureMode(void) {}
void ori_rl_DrawRenderTexture(int rt, int posX, int posY, int r, int g, int b, int a) {
    (void)rt;(void)posX;(void)posY;(void)r;(void)g;(void)b;(void)a;
}
void ori_rl_DrawRenderTextureRec(
    int rt, double srcX, double srcY, double srcW, double srcH,
    double dstX, double dstY, double dstW, double dstH,
    int r, int g, int b, int a
) {
    (void)rt;(void)srcX;(void)srcY;(void)srcW;(void)srcH;
    (void)dstX;(void)dstY;(void)dstW;(void)dstH;(void)r;(void)g;(void)b;(void)a;
}
int ori_rl_GetRenderTextureWidth(int rt) {
    if (rt > 0 && rt < 32 && g_rt_live[rt]) return g_rt_w[rt];
    return 0;
}
int ori_rl_GetRenderTextureHeight(int rt) {
    if (rt > 0 && rt < 32 && g_rt_live[rt]) return g_rt_h[rt];
    return 0;
}

/* --- P2-B audio deepen (stub) --- */
void ori_rl_SetMasterVolume(double volume) { (void)volume; }
double ori_rl_GetMasterVolume(void) { return 1.0f; }
void ori_rl_PauseSound(int sound) { (void)sound; }
void ori_rl_ResumeSound(int sound) { (void)sound; }
int ori_rl_IsSoundPlaying(int sound) { (void)sound; return 0; }
void ori_rl_SetSoundPitch(int sound, double pitch) { (void)sound; (void)pitch; }
void ori_rl_SetSoundPan(int sound, double pan) { (void)sound; (void)pan; }
void ori_rl_PauseMusicStream(int music) { (void)music; }
void ori_rl_ResumeMusicStream(int music) { (void)music; }
void ori_rl_SetMusicPitch(int music, double pitch) { (void)music; (void)pitch; }
double ori_rl_GetMusicTimeLength(int music) { (void)music; return 0.0f; }
double ori_rl_GetMusicTimePlayed(int music) { (void)music; return 0.0f; }

/* --- P2-B 3D deepen (stub) --- */
void ori_rl_DrawCylinder(double x, double y, double z, double rt, double rb, double h, int slices, int r, int g, int b, int a) {
    (void)x;(void)y;(void)z;(void)rt;(void)rb;(void)h;(void)slices;(void)r;(void)g;(void)b;(void)a;
}
void ori_rl_DrawCylinderWires(double x, double y, double z, double rt, double rb, double h, int slices, int r, int g, int b, int a) {
    (void)x;(void)y;(void)z;(void)rt;(void)rb;(void)h;(void)slices;(void)r;(void)g;(void)b;(void)a;
}
void ori_rl_DrawCapsule(double ax, double ay, double az, double bx, double by, double bz, double radius, int slices, int rings, int r, int g, int b, int a) {
    (void)ax;(void)ay;(void)az;(void)bx;(void)by;(void)bz;(void)radius;(void)slices;(void)rings;(void)r;(void)g;(void)b;(void)a;
}
void ori_rl_DrawCapsuleWires(double ax, double ay, double az, double bx, double by, double bz, double radius, int slices, int rings, int r, int g, int b, int a) {
    (void)ax;(void)ay;(void)az;(void)bx;(void)by;(void)bz;(void)radius;(void)slices;(void)rings;(void)r;(void)g;(void)b;(void)a;
}
void ori_rl_DrawModelWires(int model, double x, double y, double z, double scale, int r, int g, int b, int a) {
    (void)model;(void)x;(void)y;(void)z;(void)scale;(void)r;(void)g;(void)b;(void)a;
}
void ori_rl_DrawBillboard(
    double cpx, double cpy, double cpz, double ctx, double cty, double ctz, double cux, double cuy, double cuz,
    double fovy, int projection, int texture, double px, double py, double pz, double scale,
    int r, int g, int b, int a
) {
    (void)cpx;(void)cpy;(void)cpz;(void)ctx;(void)cty;(void)ctz;(void)cux;(void)cuy;(void)cuz;
    (void)fovy;(void)projection;(void)texture;(void)px;(void)py;(void)pz;(void)scale;(void)r;(void)g;(void)b;(void)a;
}

#include <stdint.h>
int ori_rl_LoadTextureFromGray8(int64_t data_ptr, int width, int height, int pitch) {
    (void)data_ptr; (void)width; (void)height; (void)pitch;
    return 1;
}
void ori_rl_SeekMusicStream(int music, double position_sec) {
    (void)music; (void)position_sec;
}
int ori_rl_UploadMeshMilli(int64_t xyz_m_ptr, int vert_count, int64_t indices_ptr, int index_count) {
    (void)xyz_m_ptr; (void)vert_count; (void)indices_ptr; (void)index_count;
    return 1;
}
void ori_rl_UnloadMesh(int mesh) { (void)mesh; }
void ori_rl_DrawMesh(int mesh, double x, double y, double z, int r, int g, int b, int a) {
    (void)mesh; (void)x; (void)y; (void)z; (void)r; (void)g; (void)b; (void)a;
}
int ori_rl_LoadModelFromMeshId(int mesh) { (void)mesh; return 1; }
