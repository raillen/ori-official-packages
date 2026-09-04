# Studio ↔ ECO surface hooks

> **Updated:** 2026-07-15  
> How Ori Game Studio consumes **ori-game** and sibling `ori-*` packages
> without swallowing them into the Tauri app.

Related: [`ROADMAP-GAME-ECO.md`](ROADMAP-GAME-ECO.md),
[`game-ports-maturity-matrix.md`](game-ports-maturity-matrix.md),
[`PLAY-PACKAGING.md`](PLAY-PACKAGING.md).

---

## Principle

| Layer | Lives in | Studio role |
|-------|----------|-------------|
| Runtime modules `game.*` | **ori-game** | Author via Monaco / Nodes / Play |
| Content loaders (Tiled, Aseprite, Spine, LDtk) | **ori-game** pure Ori | Content panel shows paths + load hints |
| Native FFI (raylib, box2d, jolt, imgui, …) | sibling `ori-*` | Linked via package path-deps / native_libs |
| Editor shell | **ori-game-studio** | Tauri + React; never embeds full engines |

Studio does **not** vendor raylib sources. It opens projects that depend on packages.

---

## Surfaces wired today

| ECO / module | Studio surface | Status |
|--------------|----------------|--------|
| `game.ogame` / scene JSON | Hierarchy, Place, Inspector, Play | done |
| `game.ogame_level` + CSG + PBR | Level tools, brushes, terrain, viewport | done (MVP) |
| `game.tiled` / `game.ldtk` | Content panel Maps filter + meta + `load_map` hint | done (browser) |
| `game.aseprite` / `game.spine` / `game.anim_player` | Content panel Anim filter + frame scrubber meta | done (browser) |
| `game.draw3d` mesh/GLB | Instance mesh path, Content Models, Play mesh | done |
| `game.ogame_alarms` | N1 `alarm_set` + `ogame_script` | **done** (runtime) |
| `game.ogame_script` | N1 host: parse/run `ogs:*`, vars, wait, if, call | **done** |
| `game.tiled.solid_tiles` | Content maps → solid boxes for level/collision | **done** API |
| `ori-imgui` tools | In-game optional (not Studio chrome) | ECO P3-A next |
| `ori-box2d` / `ori-jolt` | Package path-deps when needed | queued |
| `ori-enet` | Multiplayer samples outside Studio shell | done package |

---

## Path layout (dev)

```text
Documentos/Projetos/
  ori-game/                 -- runtime + sample_project
  ori-game-studio/          -- Tauri app
  ori-raylib/ ori-imgui/ …  -- sibling ports
```

Sample path-dep:

```toml
# assets/sample_project/ori.pkg.toml
[dependencies]
ori_game = { path = "../..", version = "0.3.0" }
```

---

## What “done” means for Studio ECO hooks

1. **Discoverable** — Content / docs point at the module.  
2. **Runnable** — Play/`ori check .` resolves imports with packaging rules.  
3. **Authorable** — path or field editable in Inspector / Content (not only raw JSON).  
4. **Not merged** — no monorepo squash of `ori-*` into Studio.

Full maturity scores stay in `game-ports-maturity-matrix.md` (Linux-first).

---

## Next ECO (outside this slice)

- P3-A ImGui multi-context + Tier 2 content tools (native tools track).  
- P3-B surface 4→5 (shaders/mechanics/audio edge).  
- Phase OS Win/mac smoke last.
