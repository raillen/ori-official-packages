# ori-recast

[Recast Navigation](https://github.com/recastnavigation/recastnavigation) (Recast + Detour) for Ori (S3).

**Package:** `recast` · **Module:** `recast.nav` · **0.2.0** · Linux first

3D navmesh build from a **triangle soup** (or flat plane helper) + Detour path / nearest / raycast. Positions use **milli** (meters × 1000). Complements pure-Ori 2D `game.pathfind` (grid A*) — this is **3D navmesh**.

```bash
./tools/smoke_linux.sh
```

## API (`recast.nav`)

| API | Role |
|-----|------|
| `mesh_clear()` | Clear staging triangle soup |
| `mesh_add_vertex(x_m, y_m, z_m)` | Append vertex (milli); returns 0-based index |
| `mesh_add_triangle(i0, i1, i2)` | Append triangle by indices |
| `mesh_vertex_count` / `mesh_triangle_count` | Staging counts |
| `build_mesh(cell_size_m, agent_radius_m)` | Recast+Detour build from soup; `agent_radius_m <= 0` → default 300 |
| `build_plane(w, d, cell)` | Flat [0,w]×[0,d] plane at y=0 (default agent r=0.3 m) |
| `build_plane_ex(w, d, cell, agent_radius_m)` | Plane with explicit agent radius |
| `find_path(sx,sy,sz, ex,ey,ez)` | Detour straight path → waypoint count |
| `waypoint_count` / `waypoint_x/y/z(i)` | Last path result (milli) |
| `find_nearest(x,y,z)` | Nearest walkable poly point |
| `nearest_x/y/z` | Last nearest result (milli) |
| `raycast(sx,sy,sz, ex,ey,ez)` | Surface raycast: **-1** fail, **0** clear, **1** wall hit |
| `raycast_t_m` / `raycast_hit_x/y/z` | Hit parameter (0..1000) and hit point |
| `is_ready` / `destroy` | Lifetime |

**Units:** all distances/sizes are **milli-meters** (value × 1000). Example: `300` = 0.3 m cell size.

**Agent radius:** optional on `build_mesh` / `build_plane_ex`. Values `<= 0` select the default **0.3 m** (300 milli).

## Native link (Linux)

C++ Recast+Detour objects are archived as `libori_recast_objs.a`. Package name `ori_recast_shim` resolves via a **GNU ld script**:

```text
lib/x86_64-unknown-linux-gnu/libori_recast_shim.a  → INPUT(.../libori_recast_objs.a -lstdc++ -lm)
lib/x86_64-unknown-linux-gnu/libori_recast_shim.so  → shared for JIT (`ori run`)
```

Rebuild:

```bash
./tools/build_linux.sh
```

## Vendor

```text
vendor/recastnavigation/   # Recast + Detour sources (Zlib)
native/ori_recast_shim.cpp # C ABI (int64_t) for Ori
```

## License

Package shim: same spirit as ecosystem ports. Upstream Recast Navigation is **Zlib** — see `vendor/recastnavigation/License.txt`.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See [`ori-lang` `docs/planning/PHASE-OS.md`](https://github.com/raillen/ori-lang/blob/master/docs/planning/PHASE-OS.md) for the multi-OS staging policy
