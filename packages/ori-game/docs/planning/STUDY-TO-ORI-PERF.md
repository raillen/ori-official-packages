# Study → Ori: frame performance (editor usability)

> **2026-07-15** · Gate: Studio must stay interactive (~60 FPS edit lite).  
> Sources: `study/godot`, `study/trenchbroom`, `study/fyrox`.  
> **Log canônico (sempre alimentar):** [`study/SOURCE-LEARNINGS.md`](../../../study/SOURCE-LEARNINGS.md)  
> (padrões P-LOOP / P-RES / P-EDIT desta fatia já no journal 2026-07-15).

## TL;DR

| Engine pattern | Godot / TB / Fyrox | Ori (was) | Ori (now) |
|----------------|-------------------|-----------|-----------|
| Resource load | `ResourceCache` by path | `LoadModel` every call → new handle | **path cache** in `ori_raylib_shim` |
| Geometry CSG | compile/cache solids | `collect_solids` **every draw** | **cache** `solids` + `draw_level_pieces*` |
| Editor draw quality | cheap preview vs game materials | PBR + GLB always | **editor lite**: unlit cubes, spheres, optional GLB |
| Frame loop | process then draw; no disk in draw | I/O + CSG in draw | draw = only GPU of cached data |

## Godot (`study/godot`)

- `ResourceLoader` + `ResourceCache::get_ref(path)` — load once, reuse RID/handle.
- `SceneTree::process` / `physics_process`: transform flush → node process → **then** render servers.
- Mesh lives in **RenderingServer** storage, not re-parsed from GLB per frame.

**Ori mapping:** `ori_rl_LoadModel` / `LoadTexture` now dedupe by path string (same idea as ResourceCache).

## TrenchBroom (`study/trenchbroom`)

- Brushes/CSG live in the **map model**; render uses **built solid mesh**, not subtract-on-draw.
- Polyhedron CSG is offline relative to the paint loop.

**Ori mapping:** AABB CSG stays; `refresh_solids()` only on edit; draw uses cached `list[SolidPiece]`.

## Fyrox

- `ResourceManager` for assets; scene `update` separate from render graph.

**Ori mapping:** same split — `on_update` gameplay, `on_draw` only presents cached state.

## Studio defaults (editor lite)

| Flag | Default | Why |
|------|---------|-----|
| `editor_lit` | false | No BeginShaderMode per cube |
| `draw_entity_meshes` | false | No 0.5MB GLB cowboy every frame until toggled |
| `show_debug` | false | Fewer wire draws |
| `show_nodes` / timeline / output | false | Less ImGui work until opened |

User can enable **PBR** / **Entity meshes** in Toolbar → Perf.

## F6 A/B (Studio shell)

| Mode | What runs | Measure |
|------|-----------|---------|
| **UI** (default) | viewport + ImGui dock/panels | toolbar `fps≈` + rolling `avg≈` |
| **MINIMAL** (F6) | viewport draw only, no ImGui | yellow overlay `fps≈`/`avg≈`/`dt_ms` |

On each **F6** toggle the shell logs  
`PERF A/B: <prev> avg fps≈N (n=samples) → <next>`  
and keeps `last <label> avg≈` for side-by-side compare.  
**How to use:** stay ~2–3s in UI → F6 → stay in MINIMAL → F6; compare avgs in console/log.

## Checklist before claiming “feature done”

1. Study note (this file or STUDY-ENGINES-DEEP) for the domain.  
2. Hot path: **no** `collect_solids`, **no** disk load, **no** unbounded alloc per frame.  
3. Smoke + FPS visible in toolbar (`fps≈` / F6 A/B).  
4. CHANGELOG if user-facing perf/behavior changed.
