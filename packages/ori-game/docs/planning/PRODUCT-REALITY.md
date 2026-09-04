# Product reality — Ori Game Studio / ori-game (honest)

> **Date:** 2026-07-15 (refresh DT5 — pós G–L+K Linux MVP)  
> **Status:** pre-product · **not** shippable as a serious game engine  
> Audience: author + agents. This document **overrides** optimistic “done” labels
> in feature matrices when they conflict with user-visible usefulness.

---

## 1. Direct answer

**You are right.** Compared to GameMaker, Godot, Unity, or even Hammer/TrenchBroom
map workflows, this is still a **prototype shell** with partial modules, not a
finished engine you can ship games with.

Marking slices “done (MVP)” in planning docs did **not** mean:

- scenes actually run in a window,
- assets preview reliably in the editor,
- colliders/camera/animation pipelines are production-grade,
- or the viewport is stable enough for daily work.

It meant: *some code path exists, often behind flags, stubs, or headless smoke.*

That gap is a product failure mode (false confidence). This file corrects it.

---

## 2. What was actually broken (verified 2026-07-15)

| Symptom | Root cause |
|---------|------------|
| Play / “scene doesn’t start” | Sample `main()` was **empty** — never called `game.app.run_window` → process exits with no window. |
| `ori run` fails loading `.so` | JIT path needs `libori_raylib_shim.so` / `libraylib.so`; only **static** `.a` is staged. |
| AOT link looks for missing `./lib/...` | User/sample `ori.pkg.toml` listed `native_libs` on a package **without** staged `lib/<triple>/`. |
| Viewport fragile | Large Three.js editor surface (~3.7k LOC split poorly); mesh/GLB/async load + gizmo/CSG preview under continuous churn. |
| “Can’t see models/code/text/sprites” | Editor preview ≠ game runtime; Monaco/script IDE and Content browser are partial; no unified asset pipeline. |
| No real Hammer/Quake CSG | AABB multi-slab + L6 boolean/merge/axis half-space (still not polyhedron BSP). |
| No real animation / timeline | Runtime modules exist (`aseprite`, `anim_player`, `spine`); **no** Studio timeline editor. |
| No entity script loop in Play | `on_create`/`on_step` on entity files are not auto-dispatched by the engine tick. |

**Fixes applied in the same pass:** sample `main()` calls `run_window`; sample/new-project `ori.pkg.toml` drop local `native_libs`; Studio Play forces `ORI_USE_AOT=1`.

That makes the sample **able to open a window** when libs are staged — it does **not** make a full engine.

---

## 3. Honest maturity (not marketing)

| Area | Claimed earlier | Reality (2026-07-15 Linux) |
|------|-----------------|---------|
| Shell IDE (docks, tree, open project) | done | **Usable prototype** — place/save/project/toolbar/palette |
| Scene JSON + Hierarchy | done | **Author loop** — add instance, brushes, dirty save |
| Viewport 3D | done | **Better** — free zone, pick ray, frame F; gizmo cam matrices |
| Play | done | **In-process** shell + sample AOT; gameplay HUD |
| Level CSG | done L6 | **Boolean + merge + axis clip** on AABB — not Hammer poly |
| Nodes / ogs | done | **Markers + small interpreter** — not GM D&D |
| Tilemap / anim | done browser | **Paint in shell** + `sample_2d_walk`; timeline UI milli only |
| Camera system | partial | Editor orbit + **R5.1 stack** (chase/high/side/overview); cutscene still missing |
| Colliders | partial | Sphere (+ box half-extent approx) vs level solids |
| Look/VFX/audio | missing | **MVP** asset_paths, postfx toggles, decals, buses + blip.wav |
| Packaging / ship game | early | **Linux export** + **P4 CI smoke** |

---

## 4. What a “serious enough to make a game” bar looks like

Minimum **vertical slice** (one playable 3D or 2D game end-to-end):

1. **Play always works** from Studio: window, FPS, quit, logs, AOT link stable.  
2. **Viewport stable**: no crash on dock resize/reopen; always show grid + selected asset.  
3. **Asset visibility**: GLB/OBJ preview, PNG sprite preview, text mesh or billboard optional, Monaco always open for selected script.  
4. **Author loop**: place instance → edit script → Play sees change (even if recompile).  
5. **Collision usable**: debug draw + player walk on level solids (or 2D rects) without hand-rolling every sample.  
6. **Camera usable**: orbit editor + one runtime follow camera API documented and used by sample.  
7. **One content pipeline deep** (pick **either** 3D level **or** 2D tilemap first — finish it, don’t half both).  
8. Stop labeling unfinished work as “done”.

Hammer-class brushes, full boolean CSG, animation timeline, multiplayer, etc. are **after** that bar.

---

## 5. Priority order (recovery plan)

### P0 — Make Play + sample real (this week)

- [x] `main()` → `app.run_window`  
- [x] Drop false `native_libs` on empty package roots  
- [x] Studio Play `ORI_USE_AOT=1`  
- [ ] Smoke **run** (not only `ori check`) in `smoke_sample_play.sh`  
- [ ] New Project scaffold always generates a windowed `main` (started)  
- [ ] Document “windowed main” as invariant  

### P1 — Editor must not gaslight (viewport + assets)

- Hardening: single viewport lifecycle, resize-safe, dispose meshes, error banners when GLB fails  
- Instance mesh preview path tested with brazilian-cowboy  
- Script IDE open reliability  
- Content: image preview for png  

### P2 — One deep content path

**Recommendation:** finish **3D level walk** first (sample already points there):

- Extrude / push-pull box faces (even without full BSP)  
- Subtract that matches runtime (same solid list)  
- Collider debug always on toggle  
- Runtime camera follow player  

**Then** 2D tilemap paint **or** anim timeline — not both in parallel.

### P3 — Engine systems (only when P0–P1 hold)

- Entity event dispatch table (create/step/destroy/alarm)  
- Real layers / masks  
- Animation clip list + simple timeline  
- Optional true CSG (third-party or constrained BSP)  

### Explicitly **not** the bar yet

- Matching Unity/Godot feature lists  
- Multi-OS packaging polish  
- Marketplace  

---

## 6. Policy for agents / future work

1. Prefer **broken → works for a user** over new panels.  
2. Never mark a slice **done** unless:  
   - Studio or sample **shows it**,  
   - `ori run`/Play **exercises it**,  
   - smoke script covers it.  
3. Docs: use **prototype / partial / usable** — avoid “done” for thin MVPs.  
4. If the user says “nothing works”, **debug Play + viewport first**, not new features.

---

## 7. Relation to other docs

| Doc | Role after this |
|-----|-----------------|
| **`PLANO-AMADURECIMENTO-ENGINE.md`** | **Canonical maturation plan + full backlog + phases to first game** |
| **`STUDY-ENGINES.md`** | Godot/Unity/GM study notes + local Godot clone path |
| `STUDIO-PRODUCT-DECISIONS.md` | Decisions + history; **status must match this file** |
| `PLAY-PACKAGING.md` | Technical packaging; AOT-first for games |
| `STUDIO-ECO-HOOKS.md` | Module map — not maturity claims |
| `ROADMAP-GAME-ECO.md` | Sibling packages; Linux-first |

---

**Bottom line:** the product is an early Studio + runtime **lab**, not a finished engine.  
Recovery is: **honest status → Play always opens a game → stabilize viewport/assets → one deep content path.**  
Execute via **`PLANO-AMADURECIMENTO-ENGINE.md`** (gate G1).
