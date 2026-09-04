#include "box2d/box2d.h"
#include <math.h>
#include <stdint.h>

/* Milli-units (int): value / 1000.0f = meters / density / material scale.
 * Angular: millidegrees (deg×1000) and milli-deg/s (deg/s×1000).
 * Ori float→C FFI is unreliable in this package; keep ints at the boundary. */

#define MAX_WORLDS 16
#define MAX_BODIES 512
#define MAX_JOINTS 256
#define MAX_AABB_HITS 64
#define MAX_CONTACTS 256
#define PI_F 3.14159265f

static b2WorldId g_worlds[MAX_WORLDS];
static int g_world_live[MAX_WORLDS];
static b2BodyId g_bodies[MAX_BODIES];
static int g_body_live[MAX_BODIES];
static int g_body_world[MAX_BODIES];
static b2JointId g_joints[MAX_JOINTS];
static int g_joint_live[MAX_JOINTS];
static int g_joint_world[MAX_JOINTS];

/* Last raycast hit (milli-meters world coords). */
static int g_last_ray_hit = 0;
static int g_last_ray_x_m = 0;
static int g_last_ray_y_m = 0;

/* Last AABB query results (body slot ids). */
static int g_aabb_count = 0;
static int g_aabb_bodies[MAX_AABB_HITS];

/* Active contact pairs after last step (body slot ids). */
static int g_contact_count = 0;
static int g_contact_a[MAX_CONTACTS];
static int g_contact_b[MAX_CONTACTS];
static int g_contacts_world = -1;

static float milli(int v) { return (float)v / 1000.0f; }
static int to_milli(float v) { return (int)(v * 1000.0f + (v >= 0.0f ? 0.5f : -0.5f)); }

/* rad ↔ milli-deg (deg×1000) */
static int rad_to_mdeg(float rad) {
    return (int)(rad * 180000.0f / PI_F + (rad >= 0.0f ? 0.5f : -0.5f));
}
static float mdeg_to_rad(int mdeg) {
    return (float)mdeg * PI_F / 180000.0f;
}

static int alloc_world_slot(void) {
    for (int i = 0; i < MAX_WORLDS; ++i) {
        if (!g_world_live[i]) {
            g_world_live[i] = 1;
            return i;
        }
    }
    return -1;
}

static int alloc_body_slot(void) {
    for (int i = 0; i < MAX_BODIES; ++i) {
        if (!g_body_live[i]) {
            g_body_live[i] = 1;
            return i;
        }
    }
    return -1;
}

static int alloc_joint_slot(void) {
    for (int i = 0; i < MAX_JOINTS; ++i) {
        if (!g_joint_live[i]) {
            g_joint_live[i] = 1;
            return i;
        }
    }
    return -1;
}

static int body_ok(int body) {
    return body >= 0 && body < MAX_BODIES && g_body_live[body];
}

static int world_ok(int world) {
    return world >= 0 && world < MAX_WORLDS && g_world_live[world];
}

static int joint_ok(int joint) {
    return joint >= 0 && joint < MAX_JOINTS && g_joint_live[joint];
}

static int find_body_slot(b2BodyId id) {
    if (B2_IS_NULL(id)) return -1;
    for (int i = 0; i < MAX_BODIES; ++i) {
        if (g_body_live[i] && B2_ID_EQUALS(g_bodies[i], id)) return i;
    }
    return -1;
}

static b2ShapeDef default_shape_def(int density_m, b2BodyType type) {
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.enableContactEvents = true;
    if (type == b2_dynamicBody) {
        sd.density = milli(density_m);
        if (sd.density <= 0.0f) sd.density = 1.0f;
    }
    return sd;
}

static void apply_material_to_body(int body, float friction, float restitution) {
    if (!body_ok(body)) return;
    b2ShapeId shapes[8];
    int n = b2Body_GetShapes(g_bodies[body], shapes, 8);
    for (int i = 0; i < n; ++i) {
        if (friction >= 0.0f) b2Shape_SetFriction(shapes[i], friction);
        if (restitution >= 0.0f) b2Shape_SetRestitution(shapes[i], restitution);
    }
}

static int contact_pair_exists(int a, int b) {
    for (int i = 0; i < g_contact_count; ++i) {
        if ((g_contact_a[i] == a && g_contact_b[i] == b) ||
            (g_contact_a[i] == b && g_contact_b[i] == a)) {
            return 1;
        }
    }
    return 0;
}

static void add_contact_pair(int a, int b) {
    if (a < 0 || b < 0 || a == b) return;
    if (contact_pair_exists(a, b)) return;
    if (g_contact_count >= MAX_CONTACTS) return;
    if (a > b) {
        int t = a;
        a = b;
        b = t;
    }
    g_contact_a[g_contact_count] = a;
    g_contact_b[g_contact_count] = b;
    g_contact_count++;
}

static void refresh_contacts(int world) {
    g_contact_count = 0;
    g_contacts_world = world;
    if (!world_ok(world)) return;
    for (int bi = 0; bi < MAX_BODIES; ++bi) {
        if (!g_body_live[bi] || g_body_world[bi] != world) continue;
        b2ContactData data[32];
        int n = b2Body_GetContactData(g_bodies[bi], data, 32);
        for (int i = 0; i < n; ++i) {
            b2BodyId ba = b2Shape_GetBody(data[i].shapeIdA);
            b2BodyId bb = b2Shape_GetBody(data[i].shapeIdB);
            add_contact_pair(find_body_slot(ba), find_body_slot(bb));
        }
    }
}

int ori_b2_create_world(int gravity_x_m, int gravity_y_m) {
    int slot = alloc_world_slot();
    if (slot < 0) return -1;
    b2WorldDef def = b2DefaultWorldDef();
    def.gravity = (b2Vec2){ milli(gravity_x_m), milli(gravity_y_m) };
    g_worlds[slot] = b2CreateWorld(&def);
    return slot;
}

void ori_b2_destroy_world(int world) {
    if (!world_ok(world)) return;
    b2DestroyWorld(g_worlds[world]);
    g_world_live[world] = 0;
    for (int i = 0; i < MAX_BODIES; ++i) {
        if (g_body_live[i] && g_body_world[i] == world) {
            g_body_live[i] = 0;
        }
    }
    for (int i = 0; i < MAX_JOINTS; ++i) {
        if (g_joint_live[i] && g_joint_world[i] == world) {
            g_joint_live[i] = 0;
        }
    }
    if (g_contacts_world == world) {
        g_contact_count = 0;
        g_contacts_world = -1;
    }
}

int ori_b2_world_is_valid(int world) {
    return world_ok(world) ? 1 : 0;
}

void ori_b2_set_gravity(int world, int gravity_x_m, int gravity_y_m) {
    if (!world_ok(world)) return;
    b2World_SetGravity(g_worlds[world], (b2Vec2){ milli(gravity_x_m), milli(gravity_y_m) });
}

void ori_b2_step(int world, int dt_micros, int substeps) {
    if (!world_ok(world)) return;
    if (substeps < 1) substeps = 1;
    if (substeps > 16) substeps = 16;
    float dt = (float)dt_micros / 1000000.0f;
    if (dt <= 0.0f) return;
    b2World_Step(g_worlds[world], dt, substeps);
    refresh_contacts(world);
}

static int create_box(int world, int x_m, int y_m, int half_w_m, int half_h_m, int density_m, b2BodyType type) {
    if (!world_ok(world)) return -1;
    int slot = alloc_body_slot();
    if (slot < 0) return -1;
    b2BodyDef bd = b2DefaultBodyDef();
    bd.position = (b2Vec2){ milli(x_m), milli(y_m) };
    bd.type = type;
    b2BodyId body = b2CreateBody(g_worlds[world], &bd);
    b2Polygon box = b2MakeBox(milli(half_w_m), milli(half_h_m));
    b2ShapeDef sd = default_shape_def(density_m, type);
    b2CreatePolygonShape(body, &sd, &box);
    g_bodies[slot] = body;
    g_body_world[slot] = world;
    return slot;
}

static int create_circle(int world, int x_m, int y_m, int radius_m, int density_m, b2BodyType type) {
    if (!world_ok(world)) return -1;
    int slot = alloc_body_slot();
    if (slot < 0) return -1;
    b2BodyDef bd = b2DefaultBodyDef();
    bd.position = (b2Vec2){ milli(x_m), milli(y_m) };
    bd.type = type;
    b2BodyId body = b2CreateBody(g_worlds[world], &bd);
    b2Circle circle = { 0 };
    circle.center = (b2Vec2){ 0.0f, 0.0f };
    circle.radius = milli(radius_m);
    if (circle.radius <= 0.0f) circle.radius = 0.1f;
    b2ShapeDef sd = default_shape_def(density_m, type);
    b2CreateCircleShape(body, &sd, &circle);
    g_bodies[slot] = body;
    g_body_world[slot] = world;
    return slot;
}

/* Convex poly4: body at (x,y); verts are local milli-meters relative to body origin. */
static int create_poly4(int world, int x_m, int y_m,
                        int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3,
                        int density_m, b2BodyType type) {
    if (!world_ok(world)) return -1;
    int slot = alloc_body_slot();
    if (slot < 0) return -1;
    b2Vec2 pts[4] = {
        { milli(x0), milli(y0) },
        { milli(x1), milli(y1) },
        { milli(x2), milli(y2) },
        { milli(x3), milli(y3) },
    };
    b2Hull hull = b2ComputeHull(pts, 4);
    if (hull.count < 3) {
        g_body_live[slot] = 0;
        return -1;
    }
    b2Polygon poly = b2MakePolygon(&hull, 0.0f);
    b2BodyDef bd = b2DefaultBodyDef();
    bd.position = (b2Vec2){ milli(x_m), milli(y_m) };
    bd.type = type;
    b2BodyId body = b2CreateBody(g_worlds[world], &bd);
    b2ShapeDef sd = default_shape_def(density_m, type);
    b2CreatePolygonShape(body, &sd, &poly);
    g_bodies[slot] = body;
    g_body_world[slot] = world;
    return slot;
}

int ori_b2_create_static_box(int world, int x_m, int y_m, int half_w_m, int half_h_m) {
    return create_box(world, x_m, y_m, half_w_m, half_h_m, 0, b2_staticBody);
}

int ori_b2_create_dynamic_box(int world, int x_m, int y_m, int half_w_m, int half_h_m, int density_m) {
    return create_box(world, x_m, y_m, half_w_m, half_h_m, density_m, b2_dynamicBody);
}

int ori_b2_create_kinematic_box(int world, int x_m, int y_m, int half_w_m, int half_h_m) {
    return create_box(world, x_m, y_m, half_w_m, half_h_m, 0, b2_kinematicBody);
}

int ori_b2_create_static_circle(int world, int x_m, int y_m, int radius_m) {
    return create_circle(world, x_m, y_m, radius_m, 0, b2_staticBody);
}

int ori_b2_create_dynamic_circle(int world, int x_m, int y_m, int radius_m, int density_m) {
    return create_circle(world, x_m, y_m, radius_m, density_m, b2_dynamicBody);
}

int ori_b2_create_static_poly4(int world, int x_m, int y_m,
                               int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
    return create_poly4(world, x_m, y_m, x0, y0, x1, y1, x2, y2, x3, y3, 0, b2_staticBody);
}

int ori_b2_create_dynamic_poly4(int world, int x_m, int y_m,
                                int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3,
                                int density_m) {
    return create_poly4(world, x_m, y_m, x0, y0, x1, y1, x2, y2, x3, y3, density_m, b2_dynamicBody);
}

void ori_b2_destroy_body(int body) {
    if (!body_ok(body)) return;
    b2DestroyBody(g_bodies[body]);
    g_body_live[body] = 0;
}

int ori_b2_body_is_valid(int body) {
    return body_ok(body) ? 1 : 0;
}

int ori_b2_body_x_m(int body) {
    if (!body_ok(body)) return 0;
    return to_milli(b2Body_GetPosition(g_bodies[body]).x);
}

int ori_b2_body_y_m(int body) {
    if (!body_ok(body)) return 0;
    return to_milli(b2Body_GetPosition(g_bodies[body]).y);
}

int ori_b2_body_angle_mdeg(int body) {
    if (!body_ok(body)) return 0;
    float rad = b2Rot_GetAngle(b2Body_GetRotation(g_bodies[body]));
    return rad_to_mdeg(rad);
}

int ori_b2_body_vx_m(int body) {
    if (!body_ok(body)) return 0;
    return to_milli(b2Body_GetLinearVelocity(g_bodies[body]).x);
}

int ori_b2_body_vy_m(int body) {
    if (!body_ok(body)) return 0;
    return to_milli(b2Body_GetLinearVelocity(g_bodies[body]).y);
}

/* Angular velocity: milli-deg/s (degrees per second × 1000). */
int ori_b2_body_omega_mdeg(int body) {
    if (!body_ok(body)) return 0;
    return rad_to_mdeg(b2Body_GetAngularVelocity(g_bodies[body]));
}

void ori_b2_set_angular_velocity(int body, int omega_mdeg) {
    if (!body_ok(body)) return;
    b2Body_SetAngularVelocity(g_bodies[body], mdeg_to_rad(omega_mdeg));
}

void ori_b2_set_position(int body, int x_m, int y_m) {
    if (!body_ok(body)) return;
    b2Rot rot = b2Body_GetRotation(g_bodies[body]);
    b2Body_SetTransform(g_bodies[body], (b2Vec2){ milli(x_m), milli(y_m) }, rot);
}

void ori_b2_set_linear_velocity(int body, int vx_m, int vy_m) {
    if (!body_ok(body)) return;
    b2Body_SetLinearVelocity(g_bodies[body], (b2Vec2){ milli(vx_m), milli(vy_m) });
}

void ori_b2_apply_force_to_center(int body, int fx_m, int fy_m) {
    if (!body_ok(body)) return;
    b2Body_ApplyForceToCenter(g_bodies[body], (b2Vec2){ milli(fx_m), milli(fy_m) }, true);
}

void ori_b2_apply_linear_impulse_to_center(int body, int ix_m, int iy_m) {
    if (!body_ok(body)) return;
    b2Body_ApplyLinearImpulseToCenter(g_bodies[body], (b2Vec2){ milli(ix_m), milli(iy_m) }, true);
}

void ori_b2_set_awake(int body, int awake) {
    if (!body_ok(body)) return;
    b2Body_SetAwake(g_bodies[body], awake != 0);
}

/* Material: friction/restitution as milli (×1000), e.g. 500 = 0.5. Applied to all shapes. */
void ori_b2_set_friction(int body, int friction_m) {
    apply_material_to_body(body, milli(friction_m), -1.0f);
}

void ori_b2_set_restitution(int body, int restitution_m) {
    apply_material_to_body(body, -1.0f, milli(restitution_m));
}

int ori_b2_body_friction_m(int body) {
    if (!body_ok(body)) return 0;
    b2ShapeId shapes[1];
    int n = b2Body_GetShapes(g_bodies[body], shapes, 1);
    if (n < 1) return 0;
    return to_milli(b2Shape_GetFriction(shapes[0]));
}

int ori_b2_body_restitution_m(int body) {
    if (!body_ok(body)) return 0;
    b2ShapeId shapes[1];
    int n = b2Body_GetShapes(g_bodies[body], shapes, 1);
    if (n < 1) return 0;
    return to_milli(b2Shape_GetRestitution(shapes[0]));
}

int ori_b2_body_count_live(void) {
    int n = 0;
    for (int i = 0; i < MAX_BODIES; ++i) {
        if (g_body_live[i]) n++;
    }
    return n;
}

/* --- Joints (handles = int slots) ---
 * Anchor args are world milli-meters; converted to local body space.
 * Distance length_m: if <= 0, use current distance between anchors. */

static int create_distance_joint_impl(int body_a, int body_b,
                                      int ax_m, int ay_m, int bx_m, int by_m, int length_m) {
    if (!body_ok(body_a) || !body_ok(body_b)) return -1;
    if (g_body_world[body_a] != g_body_world[body_b]) return -1;
    int world = g_body_world[body_a];
    if (!world_ok(world)) return -1;
    int slot = alloc_joint_slot();
    if (slot < 0) return -1;

    b2Vec2 worldA = { milli(ax_m), milli(ay_m) };
    b2Vec2 worldB = { milli(bx_m), milli(by_m) };
    b2DistanceJointDef def = b2DefaultDistanceJointDef();
    def.bodyIdA = g_bodies[body_a];
    def.bodyIdB = g_bodies[body_b];
    def.localAnchorA = b2Body_GetLocalPoint(g_bodies[body_a], worldA);
    def.localAnchorB = b2Body_GetLocalPoint(g_bodies[body_b], worldB);
    if (length_m > 0) {
        def.length = milli(length_m);
    } else {
        float dx = worldB.x - worldA.x;
        float dy = worldB.y - worldA.y;
        def.length = sqrtf(dx * dx + dy * dy);
    }
    if (def.length < 0.01f) def.length = 0.01f;
    g_joints[slot] = b2CreateDistanceJoint(g_worlds[world], &def);
    g_joint_world[slot] = world;
    return slot;
}

static int create_revolute_joint_impl(int body_a, int body_b,
                                      int ax_m, int ay_m, int bx_m, int by_m) {
    if (!body_ok(body_a) || !body_ok(body_b)) return -1;
    if (g_body_world[body_a] != g_body_world[body_b]) return -1;
    int world = g_body_world[body_a];
    if (!world_ok(world)) return -1;
    int slot = alloc_joint_slot();
    if (slot < 0) return -1;

    b2Vec2 worldA = { milli(ax_m), milli(ay_m) };
    b2Vec2 worldB = { milli(bx_m), milli(by_m) };
    b2RevoluteJointDef def = b2DefaultRevoluteJointDef();
    def.bodyIdA = g_bodies[body_a];
    def.bodyIdB = g_bodies[body_b];
    def.localAnchorA = b2Body_GetLocalPoint(g_bodies[body_a], worldA);
    def.localAnchorB = b2Body_GetLocalPoint(g_bodies[body_b], worldB);
    g_joints[slot] = b2CreateRevoluteJoint(g_worlds[world], &def);
    g_joint_world[slot] = world;
    return slot;
}

static int create_weld_joint_impl(int body_a, int body_b,
                                  int ax_m, int ay_m, int bx_m, int by_m) {
    if (!body_ok(body_a) || !body_ok(body_b)) return -1;
    if (g_body_world[body_a] != g_body_world[body_b]) return -1;
    int world = g_body_world[body_a];
    if (!world_ok(world)) return -1;
    int slot = alloc_joint_slot();
    if (slot < 0) return -1;

    b2Vec2 worldA = { milli(ax_m), milli(ay_m) };
    b2Vec2 worldB = { milli(bx_m), milli(by_m) };
    b2WeldJointDef def = b2DefaultWeldJointDef();
    def.bodyIdA = g_bodies[body_a];
    def.bodyIdB = g_bodies[body_b];
    def.localAnchorA = b2Body_GetLocalPoint(g_bodies[body_a], worldA);
    def.localAnchorB = b2Body_GetLocalPoint(g_bodies[body_b], worldB);
    g_joints[slot] = b2CreateWeldJoint(g_worlds[world], &def);
    g_joint_world[slot] = world;
    return slot;
}

int ori_b2_create_distance_joint(int body_a, int body_b,
                                 int ax_m, int ay_m, int bx_m, int by_m, int length_m) {
    return create_distance_joint_impl(body_a, body_b, ax_m, ay_m, bx_m, by_m, length_m);
}

int ori_b2_create_revolute_joint(int body_a, int body_b,
                                 int ax_m, int ay_m, int bx_m, int by_m) {
    return create_revolute_joint_impl(body_a, body_b, ax_m, ay_m, bx_m, by_m);
}

int ori_b2_create_weld_joint(int body_a, int body_b,
                             int ax_m, int ay_m, int bx_m, int by_m) {
    return create_weld_joint_impl(body_a, body_b, ax_m, ay_m, bx_m, by_m);
}

void ori_b2_destroy_joint(int joint) {
    if (!joint_ok(joint)) return;
    b2DestroyJoint(g_joints[joint]);
    g_joint_live[joint] = 0;
}

int ori_b2_joint_is_valid(int joint) {
    if (!joint_ok(joint)) return 0;
    return b2Joint_IsValid(g_joints[joint]) ? 1 : 0;
}

/* --- Queries ---
 * Ray: origin + translation in milli-meters. Returns hit body slot or -1.
 * Stores last hit point in g_last_ray_*. */

int ori_b2_raycast(int world, int ox_m, int oy_m, int tx_m, int ty_m) {
    g_last_ray_hit = 0;
    g_last_ray_x_m = 0;
    g_last_ray_y_m = 0;
    if (!world_ok(world)) return -1;
    b2Vec2 origin = { milli(ox_m), milli(oy_m) };
    b2Vec2 translation = { milli(tx_m), milli(ty_m) };
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2RayResult result = b2World_CastRayClosest(g_worlds[world], origin, translation, filter);
    if (!result.hit) return -1;
    g_last_ray_hit = 1;
    g_last_ray_x_m = to_milli(result.point.x);
    g_last_ray_y_m = to_milli(result.point.y);
    b2BodyId bid = b2Shape_GetBody(result.shapeId);
    return find_body_slot(bid);
}

int ori_b2_last_ray_hit(void) {
    return g_last_ray_hit;
}

int ori_b2_last_ray_x_m(void) {
    return g_last_ray_x_m;
}

int ori_b2_last_ray_y_m(void) {
    return g_last_ray_y_m;
}

typedef struct {
    int count;
    int *bodies;
    int capacity;
} AabbQueryCtx;

static bool aabb_overlap_cb(b2ShapeId shapeId, void *context) {
    AabbQueryCtx *ctx = (AabbQueryCtx *)context;
    if (ctx->count >= ctx->capacity) return false;
    int slot = find_body_slot(b2Shape_GetBody(shapeId));
    if (slot < 0) return true;
    for (int i = 0; i < ctx->count; ++i) {
        if (ctx->bodies[i] == slot) return true;
    }
    ctx->bodies[ctx->count++] = slot;
    return true;
}

int ori_b2_query_aabb(int world, int min_x_m, int min_y_m, int max_x_m, int max_y_m) {
    g_aabb_count = 0;
    if (!world_ok(world)) return 0;
    b2AABB aabb;
    aabb.lowerBound = (b2Vec2){ milli(min_x_m), milli(min_y_m) };
    aabb.upperBound = (b2Vec2){ milli(max_x_m), milli(max_y_m) };
    AabbQueryCtx ctx;
    ctx.count = 0;
    ctx.bodies = g_aabb_bodies;
    ctx.capacity = MAX_AABB_HITS;
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(g_worlds[world], aabb, filter, aabb_overlap_cb, &ctx);
    g_aabb_count = ctx.count;
    return g_aabb_count;
}

int ori_b2_aabb_result_count(void) {
    return g_aabb_count;
}

int ori_b2_aabb_result_body(int index) {
    if (index < 0 || index >= g_aabb_count) return -1;
    return g_aabb_bodies[index];
}

/* --- Contacts (refreshed after each step) --- */

int ori_b2_contact_count(void) {
    return g_contact_count;
}

int ori_b2_contact_body_a(int index) {
    if (index < 0 || index >= g_contact_count) return -1;
    return g_contact_a[index];
}

int ori_b2_contact_body_b(int index) {
    if (index < 0 || index >= g_contact_count) return -1;
    return g_contact_b[index];
}
