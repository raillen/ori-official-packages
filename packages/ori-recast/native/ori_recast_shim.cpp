/* ori_recast_* C ABI over Recast + Detour.
 * Milli-units: meters×1000. Global navmesh slot for smoke/tests.
 * Triangle-soup builder + plane helper; path, nearest poly, raycast.
 */
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCommon.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

static float milli_to_m(int64_t v) { return (float)v / 1000.0f; }
static int64_t m_to_milli(float v) {
    return (int64_t)(v * 1000.0f + (v >= 0.0f ? 0.5f : -0.5f));
}

static const int MAX_PATH_POLYS = 256;
static const int MAX_WAYPOINTS = 256;
static const int MAX_MESH_VERTS = 65536;
static const int MAX_MESH_TRIS = 65536;
static const float DEFAULT_AGENT_RADIUS = 0.3f;
static const float DEFAULT_AGENT_HEIGHT = 2.0f;
static const float DEFAULT_AGENT_MAX_CLIMB = 0.4f;
static const float DEFAULT_AGENT_MAX_SLOPE = 45.0f;

static dtNavMesh *g_nav = nullptr;
static dtNavMeshQuery *g_query = nullptr;
static dtQueryFilter g_filter;

static float g_wp_x[MAX_WAYPOINTS];
static float g_wp_y[MAX_WAYPOINTS];
static float g_wp_z[MAX_WAYPOINTS];
static int g_wp_count = 0;

/* Staging triangle soup (meters) for build_mesh. */
static std::vector<float> g_mesh_verts;
static std::vector<int> g_mesh_tris;

/* Last find_nearest result (meters). */
static float g_near_x = 0.0f, g_near_y = 0.0f, g_near_z = 0.0f;
static int g_near_ok = 0;

/* Last raycast result (meters / unit t). */
static float g_ray_hx = 0.0f, g_ray_hy = 0.0f, g_ray_hz = 0.0f;
static float g_ray_t = 0.0f;
static int g_ray_hit = 0; /* 1 = wall hit, 0 = clear or fail */

static void destroy_nav() {
    if (g_query) {
        dtFreeNavMeshQuery(g_query);
        g_query = nullptr;
    }
    if (g_nav) {
        dtFreeNavMesh(g_nav);
        g_nav = nullptr;
    }
    g_wp_count = 0;
    g_near_ok = 0;
    g_ray_hit = 0;
}

static float resolve_agent_radius(int64_t agent_radius_m) {
    if (agent_radius_m <= 0) {
        return DEFAULT_AGENT_RADIUS;
    }
    float r = milli_to_m(agent_radius_m);
    if (r < 0.05f) {
        return 0.05f;
    }
    return r;
}

/* Build Detour navmesh from a triangle soup (world units meters). */
static int build_from_tris_m(
    const float *verts,
    int nverts,
    const int *tris,
    int ntris,
    float cell_size,
    float agent_radius
) {
    destroy_nav();
    if (!verts || nverts < 3 || !tris || ntris < 1 || cell_size <= 0.0f) {
        return 1;
    }
    if (agent_radius < 0.05f) {
        agent_radius = 0.05f;
    }

    rcContext ctx(false);

    float bmin[3], bmax[3];
    rcCalcBounds(verts, nverts, bmin, bmax);
    /* Pad Y so agent height fits in the heightfield. */
    bmin[1] -= 1.0f;
    bmax[1] += 3.0f;
    /* Small XZ pad so boundary cells exist for erosion. */
    const float pad = cell_size * 2.0f + agent_radius * 2.0f;
    bmin[0] -= pad;
    bmin[2] -= pad;
    bmax[0] += pad;
    bmax[2] += pad;

    const float agent_height = DEFAULT_AGENT_HEIGHT;
    const float agent_max_climb = DEFAULT_AGENT_MAX_CLIMB;
    const float agent_max_slope = DEFAULT_AGENT_MAX_SLOPE;
    const float cell_height = cell_size;

    rcConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.cs = cell_size;
    cfg.ch = cell_height;
    cfg.walkableSlopeAngle = agent_max_slope;
    cfg.walkableHeight = (int)ceilf(agent_height / cfg.ch);
    cfg.walkableClimb = (int)floorf(agent_max_climb / cfg.ch);
    cfg.walkableRadius = (int)ceilf(agent_radius / cfg.cs);
    cfg.maxEdgeLen = (int)(12.0f / cell_size);
    cfg.maxSimplificationError = 1.3f;
    cfg.minRegionArea = (int)rcSqr(8);
    cfg.mergeRegionArea = (int)rcSqr(20);
    cfg.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
    cfg.detailSampleDist = cell_size * 6.0f;
    cfg.detailSampleMaxError = cell_height * 1.0f;
    rcVcopy(cfg.bmin, bmin);
    rcVcopy(cfg.bmax, bmax);
    rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

    if (cfg.width < 1 || cfg.height < 1) {
        return 1;
    }

    rcHeightfield *hf = rcAllocHeightfield();
    if (!hf || !rcCreateHeightfield(&ctx, *hf, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
        rcFreeHeightField(hf);
        return 1;
    }

    std::vector<unsigned char> tri_areas((size_t)ntris, 0);
    rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, verts, nverts, tris, ntris, tri_areas.data());
    if (!rcRasterizeTriangles(&ctx, verts, nverts, tris, tri_areas.data(), ntris, *hf, cfg.walkableClimb)) {
        rcFreeHeightField(hf);
        return 1;
    }

    rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *hf);
    rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *hf);
    rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *hf);

    rcCompactHeightfield *chf = rcAllocCompactHeightfield();
    if (!chf || !rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *hf, *chf)) {
        rcFreeCompactHeightfield(chf);
        rcFreeHeightField(hf);
        return 1;
    }
    rcFreeHeightField(hf);
    hf = nullptr;

    if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf)) {
        rcFreeCompactHeightfield(chf);
        return 1;
    }

    /* Monotone: fast and reliable for open plane / simple soup. */
    if (!rcBuildRegionsMonotone(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea)) {
        rcFreeCompactHeightfield(chf);
        return 1;
    }

    rcContourSet *cset = rcAllocContourSet();
    if (!cset || !rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset)) {
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return 1;
    }

    rcPolyMesh *pmesh = rcAllocPolyMesh();
    if (!pmesh || !rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh)) {
        rcFreePolyMesh(pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return 1;
    }

    rcPolyMeshDetail *dmesh = rcAllocPolyMeshDetail();
    if (!dmesh || !rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh)) {
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return 1;
    }
    rcFreeCompactHeightfield(chf);
    chf = nullptr;
    rcFreeContourSet(cset);
    cset = nullptr;

    for (int i = 0; i < pmesh->npolys; ++i) {
        if (pmesh->areas[i] == RC_WALKABLE_AREA) {
            pmesh->areas[i] = 0; /* ground */
        }
        pmesh->flags[i] = 0x01; /* walk */
    }

    if (pmesh->npolys == 0) {
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(pmesh);
        return 1;
    }

    dtNavMeshCreateParams params;
    memset(&params, 0, sizeof(params));
    params.verts = pmesh->verts;
    params.vertCount = pmesh->nverts;
    params.polys = pmesh->polys;
    params.polyAreas = pmesh->areas;
    params.polyFlags = pmesh->flags;
    params.polyCount = pmesh->npolys;
    params.nvp = pmesh->nvp;
    params.detailMeshes = dmesh->meshes;
    params.detailVerts = dmesh->verts;
    params.detailVertsCount = dmesh->nverts;
    params.detailTris = dmesh->tris;
    params.detailTriCount = dmesh->ntris;
    params.walkableHeight = agent_height;
    params.walkableRadius = agent_radius;
    params.walkableClimb = agent_max_climb;
    rcVcopy(params.bmin, pmesh->bmin);
    rcVcopy(params.bmax, pmesh->bmax);
    params.cs = cfg.cs;
    params.ch = cfg.ch;
    params.buildBvTree = true;

    unsigned char *nav_data = nullptr;
    int nav_data_size = 0;
    if (!dtCreateNavMeshData(&params, &nav_data, &nav_data_size)) {
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(pmesh);
        return 1;
    }
    rcFreePolyMeshDetail(dmesh);
    rcFreePolyMesh(pmesh);

    g_nav = dtAllocNavMesh();
    if (!g_nav) {
        dtFree(nav_data);
        return 1;
    }
    dtStatus st = g_nav->init(nav_data, nav_data_size, DT_TILE_FREE_DATA);
    if (dtStatusFailed(st)) {
        dtFree(nav_data);
        dtFreeNavMesh(g_nav);
        g_nav = nullptr;
        return 1;
    }

    g_query = dtAllocNavMeshQuery();
    if (!g_query) {
        destroy_nav();
        return 1;
    }
    st = g_query->init(g_nav, 2048);
    if (dtStatusFailed(st)) {
        destroy_nav();
        return 1;
    }

    g_filter.setIncludeFlags(0xffff);
    g_filter.setExcludeFlags(0);
    return 0;
}

static int build_plane_m(float w, float d, float cs, float agent_radius) {
    if (w < 1.0f || d < 1.0f || cs < 0.05f) {
        return 1;
    }
    /* Four corners. Winding CCW from +Y so normals face up (walkable). */
    float verts[12] = {
        0.0f, 0.0f, 0.0f,
        w,    0.0f, 0.0f,
        w,    0.0f, d,
        0.0f, 0.0f, d,
    };
    int tris[6] = {
        0, 2, 1,
        0, 3, 2,
    };
    return build_from_tris_m(verts, 4, tris, 2, cs, agent_radius);
}

extern "C" {

/* Clear staging triangle soup (does not destroy current navmesh). */
int64_t ori_recast_mesh_clear(void) {
    g_mesh_verts.clear();
    g_mesh_tris.clear();
    return 0;
}

/* Append vertex in milli. Returns 0-based index, or -1 on overflow. */
int64_t ori_recast_mesh_add_vertex(int64_t x_m, int64_t y_m, int64_t z_m) {
    int n = (int)(g_mesh_verts.size() / 3);
    if (n >= MAX_MESH_VERTS) {
        return -1;
    }
    g_mesh_verts.push_back(milli_to_m(x_m));
    g_mesh_verts.push_back(milli_to_m(y_m));
    g_mesh_verts.push_back(milli_to_m(z_m));
    return (int64_t)n;
}

/* Append triangle by 0-based vertex indices. Returns 0 on success. */
int64_t ori_recast_mesh_add_triangle(int64_t i0, int64_t i1, int64_t i2) {
    int nverts = (int)(g_mesh_verts.size() / 3);
    if (i0 < 0 || i1 < 0 || i2 < 0 || i0 >= nverts || i1 >= nverts || i2 >= nverts) {
        return 1;
    }
    if (i0 == i1 || i1 == i2 || i0 == i2) {
        return 1;
    }
    if ((int)(g_mesh_tris.size() / 3) >= MAX_MESH_TRIS) {
        return 1;
    }
    g_mesh_tris.push_back((int)i0);
    g_mesh_tris.push_back((int)i1);
    g_mesh_tris.push_back((int)i2);
    return 0;
}

int64_t ori_recast_mesh_vertex_count(void) {
    return (int64_t)(g_mesh_verts.size() / 3);
}

int64_t ori_recast_mesh_triangle_count(void) {
    return (int64_t)(g_mesh_tris.size() / 3);
}

/* Build navmesh from staged triangle soup.
 * cell_size_m and agent_radius_m in milli; agent_radius_m <= 0 → default 0.3m.
 * Returns 0 on success. */
int64_t ori_recast_build_mesh(int64_t cell_size_m, int64_t agent_radius_m) {
    float cs = milli_to_m(cell_size_m);
    if (cs < 0.05f) {
        return 1;
    }
    int nverts = (int)(g_mesh_verts.size() / 3);
    int ntris = (int)(g_mesh_tris.size() / 3);
    if (nverts < 3 || ntris < 1) {
        return 1;
    }
    float ar = resolve_agent_radius(agent_radius_m);
    return (int64_t)build_from_tris_m(
        g_mesh_verts.data(), nverts,
        g_mesh_tris.data(), ntris,
        cs, ar
    );
}

/* Build a flat XZ plane [0,width] x [0,depth] at y=0 (two triangles).
 * Args are milli-meters. Default agent radius 0.3m. Returns 0 on success. */
int64_t ori_recast_build_plane(int64_t width_m, int64_t depth_m, int64_t cell_size_m) {
    return (int64_t)build_plane_m(
        milli_to_m(width_m), milli_to_m(depth_m), milli_to_m(cell_size_m),
        DEFAULT_AGENT_RADIUS
    );
}

/* Same as build_plane with explicit agent radius (milli; <=0 → default). */
int64_t ori_recast_build_plane_ex(
    int64_t width_m, int64_t depth_m, int64_t cell_size_m, int64_t agent_radius_m
) {
    return (int64_t)build_plane_m(
        milli_to_m(width_m), milli_to_m(depth_m), milli_to_m(cell_size_m),
        resolve_agent_radius(agent_radius_m)
    );
}

/* Path query in milli. Returns waypoint count (>=2 on success), 0 on failure. */
int64_t ori_recast_find_path(
    int64_t sx, int64_t sy, int64_t sz,
    int64_t ex, int64_t ey, int64_t ez
) {
    g_wp_count = 0;
    if (!g_nav || !g_query) {
        return 0;
    }

    float spos[3] = { milli_to_m(sx), milli_to_m(sy), milli_to_m(sz) };
    float epos[3] = { milli_to_m(ex), milli_to_m(ey), milli_to_m(ez) };
    float ext[3] = { 2.0f, 4.0f, 2.0f };

    dtPolyRef start_ref = 0, end_ref = 0;
    float nearest_s[3], nearest_e[3];
    dtStatus st = g_query->findNearestPoly(spos, ext, &g_filter, &start_ref, nearest_s);
    if (dtStatusFailed(st) || start_ref == 0) {
        return 0;
    }
    st = g_query->findNearestPoly(epos, ext, &g_filter, &end_ref, nearest_e);
    if (dtStatusFailed(st) || end_ref == 0) {
        return 0;
    }

    dtPolyRef polys[MAX_PATH_POLYS];
    int npolys = 0;
    st = g_query->findPath(start_ref, end_ref, nearest_s, nearest_e, &g_filter, polys, &npolys, MAX_PATH_POLYS);
    if (dtStatusFailed(st) || npolys == 0) {
        return 0;
    }

    float straight[MAX_WAYPOINTS * 3];
    unsigned char flags[MAX_WAYPOINTS];
    dtPolyRef refs[MAX_WAYPOINTS];
    int nstraight = 0;
    st = g_query->findStraightPath(
        nearest_s, nearest_e, polys, npolys,
        straight, flags, refs, &nstraight, MAX_WAYPOINTS, 0
    );
    if (dtStatusFailed(st) || nstraight < 1) {
        return 0;
    }

    for (int i = 0; i < nstraight && i < MAX_WAYPOINTS; ++i) {
        g_wp_x[i] = straight[i * 3 + 0];
        g_wp_y[i] = straight[i * 3 + 1];
        g_wp_z[i] = straight[i * 3 + 2];
    }
    g_wp_count = nstraight;
    return (int64_t)g_wp_count;
}

int64_t ori_recast_waypoint_count(void) {
    return (int64_t)g_wp_count;
}

int64_t ori_recast_waypoint_x(int64_t i) {
    if (i < 0 || i >= g_wp_count) return 0;
    return m_to_milli(g_wp_x[(int)i]);
}

int64_t ori_recast_waypoint_y(int64_t i) {
    if (i < 0 || i >= g_wp_count) return 0;
    return m_to_milli(g_wp_y[(int)i]);
}

int64_t ori_recast_waypoint_z(int64_t i) {
    if (i < 0 || i >= g_wp_count) return 0;
    return m_to_milli(g_wp_z[(int)i]);
}

/* Nearest walkable poly point to (x,y,z) milli. Returns 1 if found. */
int64_t ori_recast_find_nearest(int64_t x, int64_t y, int64_t z) {
    g_near_ok = 0;
    g_near_x = g_near_y = g_near_z = 0.0f;
    if (!g_nav || !g_query) {
        return 0;
    }
    float pos[3] = { milli_to_m(x), milli_to_m(y), milli_to_m(z) };
    float ext[3] = { 4.0f, 6.0f, 4.0f };
    dtPolyRef ref = 0;
    float nearest[3];
    dtStatus st = g_query->findNearestPoly(pos, ext, &g_filter, &ref, nearest);
    if (dtStatusFailed(st) || ref == 0) {
        return 0;
    }
    g_near_x = nearest[0];
    g_near_y = nearest[1];
    g_near_z = nearest[2];
    g_near_ok = 1;
    return 1;
}

int64_t ori_recast_nearest_x(void) {
    return g_near_ok ? m_to_milli(g_near_x) : 0;
}

int64_t ori_recast_nearest_y(void) {
    return g_near_ok ? m_to_milli(g_near_y) : 0;
}

int64_t ori_recast_nearest_z(void) {
    return g_near_ok ? m_to_milli(g_near_z) : 0;
}

/* Navmesh surface raycast start→end (milli).
 * Returns:
 *   -1 fail (no mesh / no start poly)
 *    0 clear (reached end without wall; hit pos = end)
 *    1 wall hit (t in [0,1); hit pos stored)
 * hit position via raycast_hit_*; t_milli via raycast_t_m (0..1000, 1000 = clear). */
int64_t ori_recast_raycast(
    int64_t sx, int64_t sy, int64_t sz,
    int64_t ex, int64_t ey, int64_t ez
) {
    g_ray_hit = 0;
    g_ray_t = 0.0f;
    g_ray_hx = g_ray_hy = g_ray_hz = 0.0f;
    if (!g_nav || !g_query) {
        return -1;
    }

    float spos[3] = { milli_to_m(sx), milli_to_m(sy), milli_to_m(sz) };
    float epos[3] = { milli_to_m(ex), milli_to_m(ey), milli_to_m(ez) };
    float ext[3] = { 2.0f, 4.0f, 2.0f };

    dtPolyRef start_ref = 0;
    float nearest_s[3];
    dtStatus st = g_query->findNearestPoly(spos, ext, &g_filter, &start_ref, nearest_s);
    if (dtStatusFailed(st) || start_ref == 0) {
        return -1;
    }

    float t = 0.0f;
    float hit_normal[3] = { 0.0f, 0.0f, 0.0f };
    st = g_query->raycast(start_ref, nearest_s, epos, &g_filter, &t, hit_normal, nullptr, nullptr, 0);
    if (dtStatusFailed(st)) {
        return -1;
    }

    /* Detour: t == FLT_MAX means no wall (reached end). */
    if (t > 1.0f) {
        g_ray_t = 1.0f;
        g_ray_hx = epos[0];
        g_ray_hy = epos[1];
        g_ray_hz = epos[2];
        g_ray_hit = 0;
        return 0;
    }

    g_ray_t = t;
    g_ray_hx = nearest_s[0] + (epos[0] - nearest_s[0]) * t;
    g_ray_hy = nearest_s[1] + (epos[1] - nearest_s[1]) * t;
    g_ray_hz = nearest_s[2] + (epos[2] - nearest_s[2]) * t;
    g_ray_hit = 1;
    return 1;
}

int64_t ori_recast_raycast_t_m(void) {
    return m_to_milli(g_ray_t);
}

int64_t ori_recast_raycast_hit_x(void) {
    return m_to_milli(g_ray_hx);
}

int64_t ori_recast_raycast_hit_y(void) {
    return m_to_milli(g_ray_hy);
}

int64_t ori_recast_raycast_hit_z(void) {
    return m_to_milli(g_ray_hz);
}

int64_t ori_recast_is_ready(void) {
    return (g_nav && g_query) ? 1 : 0;
}

void ori_recast_destroy(void) {
    destroy_nav();
}

} /* extern "C" */
