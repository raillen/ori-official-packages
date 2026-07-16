# sample_project (engine_test)

Default **Studio / Play** project for Ori Game + raylib smoke.

## Package dependency

`ori.pkg.toml` path-depends on **`ori_game`** (`../../` = repo root of ori-game).
Without that, Play fails with `import game.* not found`.

Studio Play uses `ori check .` / `ori run .` (package root), not a bare `main.orl`.

## Active scene

`scenes/engine_test.scene.json` (also set in `project.ogame.toml` → `active_scene`)

| Content | Detail |
|---------|--------|
| **Floor** | Box brush `brush_floor` 200×2×200, layer 1, collider |
| **Wall** | `brush_wall_n` at +Z (colisão WASD) |
| **Player** | Entity `player` · instance `inst_player_cowboy` |
| **Enemy** | Entity `enemy` · `inst_enemy_01` (patrol + chase) |
| **Goal** | Solid `inst_goal` (esfera amarela, win radius) |
| **Mesh** | `assets/models/brazilian-cowboy.glb` (fallback: esfera se load falhar) |
| **Entry** | `main.orl` — World + systems + gameplay + save |

Player vars: instance `var_overrides` via catalog spawn.

**Guia G1:** [`docs/ENGINE-TEST-GUIDE.md`](../../docs/ENGINE-TEST-GUIDE.md)  
Packaging: `docs/planning/PLAY-PACKAGING.md`. Smoke: `tools/smoke_sample_play.sh`.

## Play requirement

`main()` **must** call `game.app.run_window(..., on_update, on_draw)`.  
An empty `main()` compiles but **never shows a window**.

Studio Play uses **`ORI_USE_AOT=1`** (static raylib). Do not list `native_libs`
on this package unless you stage `lib/<triple>/*.a` here — rely on path-dep.

Honest product status: `docs/planning/PRODUCT-REALITY.md`.

## Controls (Play)

| Key | Action |
|-----|--------|
| **WASD** | move player (X/Z) |
| **Space / Shift** | up / down |
| **Arrows** | pan camera offset |
| **V / 2–4** | cycle / select camera stack |
| **T** | timeline camera drive (R5.2) |
| **C** | collider / aggro debug wires |
| **F** | toggle fog (postfx) |
| **1** | controller debug draw |
| **R** | soft restart (reload scene + bag from save) |

## Loop

1. **Patrol** — enemy walks GP4 `path_follow` waypoints (red sphere).  
2. **Chase** — within aggro, AI overrides velocity toward player.  
3. **Combat** — touch enemy to damage (contact hit).  
4. **Goal** — reach yellow `inst_goal` → coin + SFX + **save slot** (`/tmp/ori_engine_test_g1.json`).  
5. **R** — restart; coins reload from save.

HUD lines: help · gameplay HP/score/wave · cam/patrol · **fps≈** · postfx/coins/save.

## Legacy

`scenes/room1.scene.json` remains for older Hierarchy / multi-scene samples.
