/* ori_jolt_* C API over Jolt Physics (C++).
 * Boundary: milli-units (meters×1000), µs for dt — same pattern as ori-box2d.
 */
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Body/BodyLock.h>

#include <cfloat>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <cstdio>

using namespace JPH;
using namespace JPH::literals;

static void TraceImpl(const char *inFMT, ...) {
    (void)inFMT;
}
#ifdef JPH_ENABLE_ASSERTS
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine) {
    (void)inExpression; (void)inMessage; (void)inFile; (void)inLine;
    return false;
}
#endif

#define MAX_BODIES 512
#define MAX_SYSTEMS 4

static float milli(int v) { return (float)v / 1000.0f; }
static int to_milli(float v) {
    return (int)(v * 1000.0f + (v >= 0.0f ? 0.5f : -0.5f));
}

/* Wave 4: 4 object layers. Defaults: 0 = static/non-moving, 1 = dynamic/moving,
 * 2–3 free for gameplay (player, debris, projectiles, …). Collision via per-system masks. */
namespace ObjLayers {
    static constexpr ObjectLayer LAYER0 = 0;
    static constexpr ObjectLayer LAYER1 = 1;
    static constexpr ObjectLayer LAYER2 = 2;
    static constexpr ObjectLayer LAYER3 = 3;
    static constexpr ObjectLayer NON_MOVING = LAYER0;
    static constexpr ObjectLayer MOVING = LAYER1;
    static constexpr uint NUM_LAYERS = 4;
}

namespace BPLayers {
    static constexpr BroadPhaseLayer NON_MOVING(0);
    static constexpr BroadPhaseLayer MOVING(1);
    static constexpr uint NUM_LAYERS(2);
}

class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter {
public:
    uint16_t masks[ObjLayers::NUM_LAYERS];

    ObjectLayerPairFilterImpl() {
        /* Every layer collides with every layer by default. */
        for (uint i = 0; i < ObjLayers::NUM_LAYERS; ++i)
            masks[i] = (uint16_t)((1u << ObjLayers::NUM_LAYERS) - 1u);
    }

    bool ShouldCollide(ObjectLayer a, ObjectLayer b) const override {
        if (a >= ObjLayers::NUM_LAYERS || b >= ObjLayers::NUM_LAYERS) return false;
        return (masks[a] & (uint16_t)(1u << b)) != 0 && (masks[b] & (uint16_t)(1u << a)) != 0;
    }
};

class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface {
public:
    BPLayerInterfaceImpl() {
        mMap[0] = BPLayers::NON_MOVING;
        mMap[1] = BPLayers::MOVING;
        mMap[2] = BPLayers::MOVING;
        mMap[3] = BPLayers::MOVING;
    }
    uint GetNumBroadPhaseLayers() const override { return BPLayers::NUM_LAYERS; }
    BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override {
        if ((uint)inLayer >= ObjLayers::NUM_LAYERS) return BPLayers::MOVING;
        return mMap[inLayer];
    }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char *GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override {
        switch ((BroadPhaseLayer::Type)inLayer) {
        case (BroadPhaseLayer::Type)BPLayers::NON_MOVING: return "NON_MOVING";
        case (BroadPhaseLayer::Type)BPLayers::MOVING: return "MOVING";
        default: return "INVALID";
        }
    }
#endif
private:
    BroadPhaseLayer mMap[ObjLayers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override {
        /* Layer 0 is static-ish (broadphase non-moving); others are moving BP layer. */
        if (inLayer1 == ObjLayers::NON_MOVING)
            return inLayer2 == BPLayers::MOVING;
        return true;
    }
};

struct SystemSlot {
    int live;
    TempAllocatorImpl *temp;
    JobSystemSingleThreaded *jobs;
    PhysicsSystem *physics;
    BPLayerInterfaceImpl *bp;
    ObjectVsBroadPhaseLayerFilterImpl *obj_vs_bp;
    ObjectLayerPairFilterImpl *obj_pair;
};

static SystemSlot g_sys[MAX_SYSTEMS];
static BodyID g_bodies[MAX_BODIES];
static int g_body_live[MAX_BODIES];
static int g_body_sys[MAX_BODIES];
static int g_factory_ready;

static void destroy_system_constraints(int system);
static void update_characters(int system, float dt);
static void update_vehicles(int system, float dt); /* defined with constraint tables */


static float g_hit_x, g_hit_y, g_hit_z, g_hit_nx, g_hit_ny, g_hit_nz, g_hit_frac;
static int g_hit_body;

static int clamp_layer(int layer) {
    if (layer < 0) return 0;
    if (layer >= (int)ObjLayers::NUM_LAYERS) return (int)ObjLayers::NUM_LAYERS - 1;
    return layer;
}

static int find_body_handle(BodyID bid) {
    if (bid.IsInvalid()) return 0;
    for (int i = 1; i < MAX_BODIES; ++i)
        if (g_body_live[i] && g_bodies[i] == bid) return i;
    return 0;
}

static void ensure_factory(void) {
    if (g_factory_ready) return;
    RegisterDefaultAllocator();
    Trace = TraceImpl;
#ifdef JPH_ENABLE_ASSERTS
    AssertFailed = AssertFailedImpl;
#endif
    Factory::sInstance = new Factory();
    RegisterTypes();
    g_factory_ready = 1;
}

static int alloc_sys(void) {
    for (int i = 0; i < MAX_SYSTEMS; ++i)
        if (!g_sys[i].live) return i;
    return -1;
}

static int alloc_body(void) {
    for (int i = 1; i < MAX_BODIES; ++i)
        if (!g_body_live[i]) return i;
    return 0;
}

static int sys_ok(int s) { return s >= 0 && s < MAX_SYSTEMS && g_sys[s].live; }
static int body_ok(int b) { return b > 0 && b < MAX_BODIES && g_body_live[b]; }

extern "C" int ori_jolt_create_system(int gravity_x_m, int gravity_y_m, int gravity_z_m) {
    ensure_factory();
    int s = alloc_sys();
    if (s < 0) return -1;

    SystemSlot &slot = g_sys[s];
    memset(&slot, 0, sizeof(slot));
    slot.temp = new TempAllocatorImpl(10 * 1024 * 1024);
    slot.jobs = new JobSystemSingleThreaded(cMaxPhysicsJobs);
    slot.bp = new BPLayerInterfaceImpl();
    slot.obj_vs_bp = new ObjectVsBroadPhaseLayerFilterImpl();
    slot.obj_pair = new ObjectLayerPairFilterImpl();
    slot.physics = new PhysicsSystem();
    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;
    slot.physics->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                       *slot.bp, *slot.obj_vs_bp, *slot.obj_pair);
    slot.physics->SetGravity(Vec3(milli(gravity_x_m), milli(gravity_y_m), milli(gravity_z_m)));
    slot.live = 1;
    return s;
}

extern "C" void ori_jolt_destroy_system(int system) {
    if (!sys_ok(system)) return;
    destroy_system_constraints(system);
    for (int i = 0; i < MAX_BODIES; ++i) {
        if (g_body_live[i] && g_body_sys[i] == system) {
            g_body_live[i] = 0;
        }
    }
    SystemSlot &slot = g_sys[system];
    delete slot.physics;
    delete slot.obj_pair;
    delete slot.obj_vs_bp;
    delete slot.bp;
    delete slot.jobs;
    delete slot.temp;
    slot.live = 0;
}

extern "C" int ori_jolt_system_is_valid(int system) { return sys_ok(system) ? 1 : 0; }

extern "C" void ori_jolt_set_gravity(int system, int gx_m, int gy_m, int gz_m) {
    if (!sys_ok(system)) return;
    g_sys[system].physics->SetGravity(Vec3(milli(gx_m), milli(gy_m), milli(gz_m)));
}

extern "C" void ori_jolt_step(int system, int dt_micros, int collision_steps) {
    if (!sys_ok(system)) return;
    if (collision_steps < 1) collision_steps = 1;
    if (collision_steps > 8) collision_steps = 8;
    float dt = (float)dt_micros / 1000000.0f;
    SystemSlot &slot = g_sys[system];
    update_characters(system, dt);
    update_vehicles(system, dt);
    slot.physics->Update(dt, collision_steps, slot.temp, slot.jobs);
}

static int create_body(
    int system, EMotionType motion, RVec3 pos, RefConst<Shape> shape, float density
) {
    (void)density;
    if (!sys_ok(system)) return 0;
    int id = alloc_body();
    if (!id) return 0;
    BodyInterface &bi = g_sys[system].physics->GetBodyInterface();
    ObjectLayer layer = motion == EMotionType::Static ? ObjLayers::NON_MOVING : ObjLayers::MOVING;
    EActivation act = motion == EMotionType::Static ? EActivation::DontActivate : EActivation::Activate;
    BodyCreationSettings settings(shape, pos, Quat::sIdentity(), motion, layer);
    BodyID bid = bi.CreateAndAddBody(settings, act);
    if (bid.IsInvalid()) {
        return 0;
    }
    g_bodies[id] = bid;
    g_body_live[id] = 1;
    g_body_sys[id] = system;
    g_sys[system].physics->OptimizeBroadPhase();
    return id;
}

extern "C" int ori_jolt_create_static_box(
    int system, int x_m, int y_m, int z_m, int half_x_m, int half_y_m, int half_z_m
) {
    BoxShapeSettings shape_settings(Vec3(milli(half_x_m), milli(half_y_m), milli(half_z_m)));
    shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult result = shape_settings.Create();
    if (result.HasError()) return 0;
    return create_body(system, EMotionType::Static,
                       RVec3(milli(x_m), milli(y_m), milli(z_m)), result.Get(), 0.0f);
}

extern "C" int ori_jolt_create_dynamic_box(
    int system, int x_m, int y_m, int z_m, int half_x_m, int half_y_m, int half_z_m, int density_m
) {
    BoxShapeSettings shape_settings(Vec3(milli(half_x_m), milli(half_y_m), milli(half_z_m)));
    shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult result = shape_settings.Create();
    if (result.HasError()) return 0;
    return create_body(system, EMotionType::Dynamic,
                       RVec3(milli(x_m), milli(y_m), milli(z_m)), result.Get(), milli(density_m));
}

extern "C" int ori_jolt_create_static_sphere(int system, int x_m, int y_m, int z_m, int radius_m) {
    SphereShapeSettings shape_settings(milli(radius_m));
    shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult result = shape_settings.Create();
    if (result.HasError()) return 0;
    return create_body(system, EMotionType::Static,
                       RVec3(milli(x_m), milli(y_m), milli(z_m)), result.Get(), 0.0f);
}

extern "C" int ori_jolt_create_dynamic_sphere(
    int system, int x_m, int y_m, int z_m, int radius_m, int density_m
) {
    SphereShapeSettings shape_settings(milli(radius_m));
    shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult result = shape_settings.Create();
    if (result.HasError()) return 0;
    return create_body(system, EMotionType::Dynamic,
                       RVec3(milli(x_m), milli(y_m), milli(z_m)), result.Get(), milli(density_m));
}

extern "C" int ori_jolt_create_dynamic_capsule(
    int system, int x_m, int y_m, int z_m, int half_height_m, int radius_m, int density_m
) {
    CapsuleShapeSettings shape_settings(milli(half_height_m), milli(radius_m));
    shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult result = shape_settings.Create();
    if (result.HasError()) return 0;
    return create_body(system, EMotionType::Dynamic,
                       RVec3(milli(x_m), milli(y_m), milli(z_m)), result.Get(), milli(density_m));
}

extern "C" void ori_jolt_destroy_body(int body) {
    if (!body_ok(body)) return;
    int s = g_body_sys[body];
    if (!sys_ok(s)) {
        g_body_live[body] = 0;
        return;
    }
    BodyInterface &bi = g_sys[s].physics->GetBodyInterface();
    bi.RemoveBody(g_bodies[body]);
    bi.DestroyBody(g_bodies[body]);
    g_body_live[body] = 0;
}

extern "C" int ori_jolt_body_is_valid(int body) { return body_ok(body) ? 1 : 0; }

extern "C" int ori_jolt_body_x_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli((float)bi.GetCenterOfMassPosition(g_bodies[body]).GetX());
}
extern "C" int ori_jolt_body_y_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli((float)bi.GetCenterOfMassPosition(g_bodies[body]).GetY());
}
extern "C" int ori_jolt_body_z_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli((float)bi.GetCenterOfMassPosition(g_bodies[body]).GetZ());
}

extern "C" int ori_jolt_body_vx_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetLinearVelocity(g_bodies[body]).GetX());
}
extern "C" int ori_jolt_body_vy_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetLinearVelocity(g_bodies[body]).GetY());
}
extern "C" int ori_jolt_body_vz_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetLinearVelocity(g_bodies[body]).GetZ());
}

/* Angular velocity (rad/s × 1000) */
extern "C" int ori_jolt_body_wx_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetAngularVelocity(g_bodies[body]).GetX());
}
extern "C" int ori_jolt_body_wy_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetAngularVelocity(g_bodies[body]).GetY());
}
extern "C" int ori_jolt_body_wz_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetAngularVelocity(g_bodies[body]).GetZ());
}

/* Rotation as quaternion ×1000 */
extern "C" int ori_jolt_body_qw_m(int body) {
    if (!body_ok(body)) return 1000;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetRotation(g_bodies[body]).GetW());
}
extern "C" int ori_jolt_body_qx_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetRotation(g_bodies[body]).GetX());
}
extern "C" int ori_jolt_body_qy_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetRotation(g_bodies[body]).GetY());
}
extern "C" int ori_jolt_body_qz_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetRotation(g_bodies[body]).GetZ());
}

extern "C" void ori_jolt_set_linear_velocity(int body, int vx_m, int vy_m, int vz_m) {
    if (!body_ok(body)) return;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    bi.SetLinearVelocity(g_bodies[body], Vec3(milli(vx_m), milli(vy_m), milli(vz_m)));
}

extern "C" void ori_jolt_add_impulse(int body, int ix_m, int iy_m, int iz_m) {
    if (!body_ok(body)) return;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    bi.AddImpulse(g_bodies[body], Vec3(milli(ix_m), milli(iy_m), milli(iz_m)));
}

extern "C" void ori_jolt_add_torque(int body, int tx_m, int ty_m, int tz_m) {
    if (!body_ok(body)) return;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    bi.AddTorque(g_bodies[body], Vec3(milli(tx_m), milli(ty_m), milli(tz_m)));
}

extern "C" void ori_jolt_set_position(int body, int x_m, int y_m, int z_m) {
    if (!body_ok(body)) return;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    bi.SetPosition(g_bodies[body], RVec3(milli(x_m), milli(y_m), milli(z_m)), EActivation::Activate);
}

/* Friction / restitution as milli (1000 = 1.0). */
extern "C" void ori_jolt_set_friction(int body, int friction_m) {
    if (!body_ok(body)) return;
    if (friction_m < 0) friction_m = 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    bi.SetFriction(g_bodies[body], milli(friction_m));
}
extern "C" void ori_jolt_set_restitution(int body, int restitution_m) {
    if (!body_ok(body)) return;
    if (restitution_m < 0) restitution_m = 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    bi.SetRestitution(g_bodies[body], milli(restitution_m));
}
extern "C" int ori_jolt_get_friction_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetFriction(g_bodies[body]));
}
extern "C" int ori_jolt_get_restitution_m(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return to_milli(bi.GetRestitution(g_bodies[body]));
}

extern "C" void ori_jolt_set_body_layer(int body, int layer) {
    if (!body_ok(body)) return;
    layer = clamp_layer(layer);
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    bi.SetObjectLayer(g_bodies[body], (ObjectLayer)layer);
}
extern "C" int ori_jolt_get_body_layer(int body) {
    if (!body_ok(body)) return 0;
    BodyInterface &bi = g_sys[g_body_sys[body]].physics->GetBodyInterface();
    return (int)bi.GetObjectLayer(g_bodies[body]);
}
extern "C" void ori_jolt_set_layer_mask(int system, int layer, int mask) {
    if (!sys_ok(system) || !g_sys[system].obj_pair) return;
    layer = clamp_layer(layer);
    g_sys[system].obj_pair->masks[layer] = (uint16_t)(mask & ((1 << ObjLayers::NUM_LAYERS) - 1));
}
extern "C" int ori_jolt_get_layer_mask(int system, int layer) {
    if (!sys_ok(system) || !g_sys[system].obj_pair) return 0;
    return (int)g_sys[system].obj_pair->masks[clamp_layer(layer)];
}

/* Large static box with top surface at y_m (500 m half-extent XZ, 0.5 m half-height). */
extern "C" int ori_jolt_create_static_floor(int system, int y_m) {
    const int half_y = 500;
    const int half_xz = 500000;
    return ori_jolt_create_static_box(system, 0, y_m - half_y, 0, half_xz, half_y, half_xz);
}

extern "C" int ori_jolt_raycast(
    int system,
    int ox_m, int oy_m, int oz_m,
    int dx_m, int dy_m, int dz_m,
    int max_dist_m
) {
    g_hit_body = 0;
    if (!sys_ok(system)) return 0;
    RRayCast ray;
    ray.mOrigin = RVec3(milli(ox_m), milli(oy_m), milli(oz_m));
    Vec3 dir = Vec3(milli(dx_m), milli(dy_m), milli(dz_m));
    float len = dir.Length();
    if (len < 1e-6f) return 0;
    dir = dir / len;
    float max_d = milli(max_dist_m);
    if (max_d <= 0.0f) max_d = 1000.0f;
    ray.mDirection = dir * max_d;

    RayCastResult hit;
    hit.mFraction = 1.0f + FLT_EPSILON;
    /* Default filters: hit any layer */
    bool found = g_sys[system].physics->GetNarrowPhaseQuery().CastRay(ray, hit);
    if (!found) return 0;
    RVec3 p = ray.GetPointOnRay(hit.mFraction);
    g_hit_x = (float)p.GetX();
    g_hit_y = (float)p.GetY();
    g_hit_z = (float)p.GetZ();
    g_hit_frac = hit.mFraction;
    g_hit_nx = 0.0f;
    g_hit_ny = 1.0f;
    g_hit_nz = 0.0f;
    g_hit_body = find_body_handle(hit.mBodyID);
    return 1;
}

extern "C" int ori_jolt_last_hit_x_m(void) { return to_milli(g_hit_x); }
extern "C" int ori_jolt_last_hit_y_m(void) { return to_milli(g_hit_y); }
extern "C" int ori_jolt_last_hit_z_m(void) { return to_milli(g_hit_z); }
extern "C" int ori_jolt_last_hit_frac_m(void) { return to_milli(g_hit_frac); }
extern "C" int ori_jolt_last_hit_body(void) { return g_hit_body; }

extern "C" int ori_jolt_body_count_live(void) {
    int n = 0;
    for (int i = 0; i < MAX_BODIES; ++i)
        if (g_body_live[i]) n++;
    return n;
}


/* --- Constraints residual --- */

#define MAX_CONSTRAINTS 256
static TwoBodyConstraint *g_constraints[MAX_CONSTRAINTS];
static int g_constraint_live[MAX_CONSTRAINTS];
static int g_constraint_sys[MAX_CONSTRAINTS];

static void destroy_system_constraints(int system) {
    for (int ci = 1; ci < MAX_CONSTRAINTS; ++ci) {
        if (g_constraint_live[ci] && g_constraint_sys[ci] == system) {
            if (g_constraints[ci]) g_sys[system].physics->RemoveConstraint(g_constraints[ci]);
            g_constraints[ci] = nullptr;
            g_constraint_live[ci] = 0;
        }
    }
}

static int alloc_constraint(void) {
    for (int i = 1; i < MAX_CONSTRAINTS; ++i)
        if (!g_constraint_live[i]) return i;
    return 0;
}

static int constraint_ok(int c) {
    return c > 0 && c < MAX_CONSTRAINTS && g_constraint_live[c];
}

extern "C" int ori_jolt_create_fixed_constraint(int system, int body_a, int body_b) {
    if (!sys_ok(system) || !body_ok(body_a) || !body_ok(body_b)) return 0;
    int id = alloc_constraint();
    if (!id) return 0;
    BodyInterface &bi = g_sys[system].physics->GetBodyInterface();
    BodyLockWrite lock_a(g_sys[system].physics->GetBodyLockInterface(), g_bodies[body_a]);
    BodyLockWrite lock_b(g_sys[system].physics->GetBodyLockInterface(), g_bodies[body_b]);
    if (!lock_a.Succeeded() || !lock_b.Succeeded()) return 0;
    FixedConstraintSettings settings;
    settings.mAutoDetectPoint = true;
    TwoBodyConstraint *c = settings.Create(lock_a.GetBody(), lock_b.GetBody());
    g_sys[system].physics->AddConstraint(c);
    g_constraints[id] = c;
    g_constraint_live[id] = 1;
    g_constraint_sys[id] = system;
    return id;
}

extern "C" int ori_jolt_create_distance_constraint(
    int system, int body_a, int body_b, int min_m, int max_m
) {
    if (!sys_ok(system) || !body_ok(body_a) || !body_ok(body_b)) return 0;
    int id = alloc_constraint();
    if (!id) return 0;
    BodyLockWrite lock_a(g_sys[system].physics->GetBodyLockInterface(), g_bodies[body_a]);
    BodyLockWrite lock_b(g_sys[system].physics->GetBodyLockInterface(), g_bodies[body_b]);
    if (!lock_a.Succeeded() || !lock_b.Succeeded()) return 0;
    DistanceConstraintSettings settings;
    settings.mSpace = EConstraintSpace::LocalToBodyCOM;
    settings.mPoint1 = RVec3::sZero();
    settings.mPoint2 = RVec3::sZero();
    float min_d = milli(min_m);
    float max_d = milli(max_m);
    if (min_d < 0.0f) min_d = 0.0f;
    if (max_d < min_d) max_d = min_d;
    settings.mMinDistance = min_d;
    settings.mMaxDistance = max_d;
    TwoBodyConstraint *c = settings.Create(lock_a.GetBody(), lock_b.GetBody());
    g_sys[system].physics->AddConstraint(c);
    g_constraints[id] = c;
    g_constraint_live[id] = 1;
    g_constraint_sys[id] = system;
    return id;
}

extern "C" int ori_jolt_create_hinge_constraint(
    int system, int body_a, int body_b,
    int ax_m, int ay_m, int az_m
) {
    if (!sys_ok(system) || !body_ok(body_a) || !body_ok(body_b)) return 0;
    int id = alloc_constraint();
    if (!id) return 0;
    BodyLockWrite lock_a(g_sys[system].physics->GetBodyLockInterface(), g_bodies[body_a]);
    BodyLockWrite lock_b(g_sys[system].physics->GetBodyLockInterface(), g_bodies[body_b]);
    if (!lock_a.Succeeded() || !lock_b.Succeeded()) return 0;
    HingeConstraintSettings settings;
    settings.mPoint1 = RVec3::sZero();
    settings.mPoint2 = RVec3::sZero();
    Vec3 axis(milli(ax_m), milli(ay_m), milli(az_m));
    if (axis.LengthSq() < 1e-8f) axis = Vec3::sAxisY();
    else axis = axis.Normalized();
    settings.mHingeAxis1 = axis;
    settings.mHingeAxis2 = axis;
    settings.mNormalAxis1 = axis.GetNormalizedPerpendicular();
    settings.mNormalAxis2 = settings.mHingeAxis2.GetNormalizedPerpendicular();
    TwoBodyConstraint *c = settings.Create(lock_a.GetBody(), lock_b.GetBody());
    g_sys[system].physics->AddConstraint(c);
    g_constraints[id] = c;
    g_constraint_live[id] = 1;
    g_constraint_sys[id] = system;
    return id;
}

extern "C" void ori_jolt_destroy_constraint(int constraint) {
    if (!constraint_ok(constraint)) return;
    int s = g_constraint_sys[constraint];
    if (sys_ok(s) && g_constraints[constraint]) {
        g_sys[s].physics->RemoveConstraint(g_constraints[constraint]);
    }
    g_constraints[constraint] = nullptr;
    g_constraint_live[constraint] = 0;
}

extern "C" int ori_jolt_constraint_is_valid(int constraint) {
    return constraint_ok(constraint) ? 1 : 0;
}

/* --- CharacterVirtual + simplified vehicle --- */
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#define MAX_CHARS 32
#define MAX_VEHICLES 16

struct CharSlot {
    int live;
    int system;
    CharacterVirtual *ch;
    float jump_v;
    int want_jump;
};

struct VehSlot {
    int live;
    int system;
    int chassis; /* body handle */
    int wheel[4];
    int hinge[4];
    int throttle_m; /* -1000..1000 */
    int steer_m;
    int brake_m;
};

static CharSlot g_chars[MAX_CHARS];
static VehSlot g_vehs[MAX_VEHICLES];

static int alloc_char(void) {
    for (int i = 1; i < MAX_CHARS; ++i)
        if (!g_chars[i].live) return i;
    return 0;
}
static int alloc_veh(void) {
    for (int i = 1; i < MAX_VEHICLES; ++i)
        if (!g_vehs[i].live) return i;
    return 0;
}
static int char_ok(int c) { return c > 0 && c < MAX_CHARS && g_chars[c].live && g_chars[c].ch; }
static int veh_ok(int v) { return v > 0 && v < MAX_VEHICLES && g_vehs[v].live; }

extern "C" int ori_jolt_create_character(
    int system, int x_m, int y_m, int z_m, int radius_m, int half_height_m
) {
    if (!sys_ok(system)) return 0;
    int id = alloc_char();
    if (!id) return 0;
    float radius = milli(radius_m);
    float half_h = milli(half_height_m);
    if (radius < 0.05f) radius = 0.05f;
    if (half_h < 0.05f) half_h = 0.05f;

    RefConst<Shape> capsule = new CapsuleShape(half_h, radius);
    ShapeSettings::ShapeResult res = RotatedTranslatedShapeSettings(
        Vec3(0, half_h + radius, 0), Quat::sIdentity(), capsule).Create();
    if (res.HasError()) return 0;

    Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
    settings->mShape = res.Get();
    settings->mSupportingVolume = Plane(Vec3::sAxisY(), -radius);
    settings->mMaxSlopeAngle = DegreesToRadians(45.0f);

    CharacterVirtual *ch = new CharacterVirtual(
        settings,
        RVec3(milli(x_m), milli(y_m), milli(z_m)),
        Quat::sIdentity(),
        0,
        g_sys[system].physics
    );
    g_chars[id].live = 1;
    g_chars[id].system = system;
    g_chars[id].ch = ch;
    g_chars[id].jump_v = 0.0f;
    g_chars[id].want_jump = 0;
    return id;
}

extern "C" void ori_jolt_destroy_character(int character) {
    if (!char_ok(character)) return;
    delete g_chars[character].ch;
    g_chars[character].ch = nullptr;
    g_chars[character].live = 0;
}

extern "C" int ori_jolt_character_is_valid(int character) { return char_ok(character) ? 1 : 0; }

extern "C" void ori_jolt_character_set_velocity(int character, int vx_m, int vy_m, int vz_m) {
    if (!char_ok(character)) return;
    g_chars[character].ch->SetLinearVelocity(Vec3(milli(vx_m), milli(vy_m), milli(vz_m)));
}

extern "C" void ori_jolt_character_jump(int character, int speed_m) {
    if (!char_ok(character)) return;
    g_chars[character].want_jump = 1;
    g_chars[character].jump_v = milli(speed_m);
}

extern "C" int ori_jolt_character_x_m(int character) {
    if (!char_ok(character)) return 0;
    return to_milli((float)g_chars[character].ch->GetPosition().GetX());
}
extern "C" int ori_jolt_character_y_m(int character) {
    if (!char_ok(character)) return 0;
    return to_milli((float)g_chars[character].ch->GetPosition().GetY());
}
extern "C" int ori_jolt_character_z_m(int character) {
    if (!char_ok(character)) return 0;
    return to_milli((float)g_chars[character].ch->GetPosition().GetZ());
}

extern "C" int ori_jolt_character_on_ground(int character) {
    if (!char_ok(character)) return 0;
    return g_chars[character].ch->IsSupported() ? 1 : 0;
}

static void update_characters(int system, float dt) {
    CharacterVirtual::ExtendedUpdateSettings us;
    Vec3 gravity = g_sys[system].physics->GetGravity();
    for (int i = 1; i < MAX_CHARS; ++i) {
        if (!g_chars[i].live || g_chars[i].system != system || !g_chars[i].ch) continue;
        CharacterVirtual *ch = g_chars[i].ch;
        if (g_chars[i].want_jump && ch->IsSupported()) {
            Vec3 v = ch->GetLinearVelocity();
            v.SetY(g_chars[i].jump_v);
            ch->SetLinearVelocity(v);
            g_chars[i].want_jump = 0;
        }
        ch->ExtendedUpdate(
            dt,
            gravity,
            us,
            g_sys[system].physics->GetDefaultBroadPhaseLayerFilter(ObjLayers::MOVING),
            g_sys[system].physics->GetDefaultLayerFilter(ObjLayers::MOVING),
            {},
            {},
            *g_sys[system].temp
        );
    }
}

/* Vehicle: chassis box + 4 sphere wheels hinged; drive via chassis forces. */
extern "C" int ori_jolt_create_vehicle(
    int system, int x_m, int y_m, int z_m,
    int half_x_m, int half_y_m, int half_z_m
) {
    if (!sys_ok(system)) return 0;
    int id = alloc_veh();
    if (!id) return 0;
    int chassis = ori_jolt_create_dynamic_box(
        system, x_m, y_m, z_m, half_x_m, half_y_m, half_z_m, 1000
    );
    if (!chassis) return 0;
    int hx = half_x_m;
    int hz = half_z_m;
    int wy = y_m - half_y_m;
    int wr = half_y_m / 2;
    if (wr < 200) wr = 200;
    int offsets[4][2] = {
        { -hx + wr, -hz + wr },
        {  hx - wr, -hz + wr },
        { -hx + wr,  hz - wr },
        {  hx - wr,  hz - wr },
    };
    g_vehs[id].live = 1;
    g_vehs[id].system = system;
    g_vehs[id].chassis = chassis;
    g_vehs[id].throttle_m = 0;
    g_vehs[id].steer_m = 0;
    g_vehs[id].brake_m = 0;
    for (int w = 0; w < 4; ++w) {
        int wx = x_m + offsets[w][0];
        int wz = z_m + offsets[w][1];
        int wheel = ori_jolt_create_dynamic_sphere(system, wx, wy, wz, wr, 500);
        g_vehs[id].wheel[w] = wheel;
        int hinge = ori_jolt_create_hinge_constraint(system, chassis, wheel, 1000, 0, 0);
        g_vehs[id].hinge[w] = hinge;
    }
    return id;
}

extern "C" void ori_jolt_destroy_vehicle(int vehicle) {
    if (!veh_ok(vehicle)) return;
    for (int w = 0; w < 4; ++w) {
        if (g_vehs[vehicle].hinge[w]) ori_jolt_destroy_constraint(g_vehs[vehicle].hinge[w]);
        if (g_vehs[vehicle].wheel[w]) ori_jolt_destroy_body(g_vehs[vehicle].wheel[w]);
    }
    if (g_vehs[vehicle].chassis) ori_jolt_destroy_body(g_vehs[vehicle].chassis);
    g_vehs[vehicle].live = 0;
}

extern "C" int ori_jolt_vehicle_is_valid(int vehicle) { return veh_ok(vehicle) ? 1 : 0; }

extern "C" void ori_jolt_vehicle_set_input(int vehicle, int throttle_m, int steer_m, int brake_m) {
    if (!veh_ok(vehicle)) return;
    g_vehs[vehicle].throttle_m = throttle_m;
    g_vehs[vehicle].steer_m = steer_m;
    g_vehs[vehicle].brake_m = brake_m;
}

extern "C" int ori_jolt_vehicle_chassis(int vehicle) {
    if (!veh_ok(vehicle)) return 0;
    return g_vehs[vehicle].chassis;
}

static void update_vehicles(int system, float dt) {
    (void)dt;
    for (int i = 1; i < MAX_VEHICLES; ++i) {
        if (!g_vehs[i].live || g_vehs[i].system != system) continue;
        int chassis = g_vehs[i].chassis;
        if (!body_ok(chassis)) continue;
        BodyInterface &bi = g_sys[system].physics->GetBodyInterface();
        BodyID bid = g_bodies[chassis];
        /* Forward is local -Z roughly world if no rotation tracking — use linear force in XZ from steer */
        float throttle = milli(g_vehs[i].throttle_m) * 50.0f; /* N force scale */
        float steer = milli(g_vehs[i].steer_m) * 20.0f;
        float brake = milli(g_vehs[i].brake_m);
        Quat rot = bi.GetRotation(bid);
        Vec3 forward = rot * Vec3(0, 0, -1);
        Vec3 right = rot * Vec3(1, 0, 0);
        bi.AddForce(bid, forward * throttle);
        bi.AddTorque(bid, Vec3(0, steer, 0));
        if (brake > 0.01f) {
            Vec3 v = bi.GetLinearVelocity(bid);
            bi.SetLinearVelocity(bid, v * (1.0f - (brake < 1.0f ? brake : 1.0f) * 0.1f));
        }
    }
}

/* Hook into step: patch by wrapping — call from a new exported step is cleaner.
 * We redefine step behavior by providing post-step helper called from Ori? 
 * Instead, modify ori_jolt_step body — search and inject. */
