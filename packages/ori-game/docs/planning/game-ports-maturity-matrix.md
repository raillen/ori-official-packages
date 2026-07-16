# Game / ECO ports — maturity & backlog matrix

> **Status:** active consult doc (implementation reference)  
> **Updated:** 2026-07-14  
> **Program:** Linux-5 maturity wave **completed** for the seven packages below.  
> **Related:** [`eco-packages-status.md`](eco-packages-status.md)

### Maturity scale

| Score | Meaning |
|------:|---------|
| **1** | Skeleton / plan only |
| **2** | MVP links + minimal demo/smoke |
| **3** | Small real use / jam-viable on Linux |
| **4** | Broad surface + tests + several demos |
| **5** | Product engine-grade on **Linux** (this program’s target). Multi-OS = Phase OS |

---

## Wave progress (Linux-5 program)

| Wave | Focus | Status |
|------|--------|--------|
| **W1** | ori-game S3 demo hygiene + smoke expand | **done** |
| **W2** | ori-game gamepad + RenderTexture → 0.3.0 | **done** |
| **W3** | ori-box2d → 0.3.0 | **done** |
| **W4** | ori-jolt → 0.2.0 | **done** |
| **W5** | raygui 0.2.0 + imgui 0.3.0 | **done** |
| **W6** | rres 0.3.0 + sqlite 0.3.0 | **done** |
| **W7** | Matrix gate all **5 (Linux)** | **done** |
| **W8** | Integration demos + umbrella smoke | **done** (2026-07-14) |
| **Phase OS** | Win/mac stage + smoke | **Windows scripts ready** — run on MSVC host (`PHASE-OS.md`) |

---

## Table A — Already ported (Linux-5)

| Package | Repo | Ver. | Maturity | Status |
|---------|------|------|----------|--------|
| `raylib` (L0) | `ori-raylib` | **0.1.0** | **5 (Linux)** | split from ori-game |
| `ori_game` | `ori-game` | **0.3.0** | **5 (Linux)** | L1 `game.*`; path-dep raylib |
| `box2d` | `ori-box2d` | **0.3.0** | **5 (Linux)** | joints, poly4, queries, contacts, materials |
| `jolt` | `ori-jolt` | **0.2.0** | **5 (Linux)** | layers, friction, torque, floor, hit body |
| `imgui` | `ori-imgui` | **0.4.0** | **5 (Linux)** | Tier0+1 file/plot/nodes + dock |
| `raygui` | `ori-raygui` | **0.2.0** | **5 (Linux)** | textbox, toggle, dropdown, style, … |
| `rres` | `ori-rres` | **0.3.0** | **5 (Linux)** | validate, list_names, read_bytes |
| `sqlite` | `ori-sqlite` | **0.3.0** | **5 (Linux)** | prepared + multi-row JSON |
| `enet` | `ori-enet` | **0.3.0** | **5 (Linux)** | channels, broadcast, protocol, 5 demos |
| (content) | `ori-game` | — | — | `game.ldtk` + `game.spine` timelines |
| Studio | plan only | — | **0.5–1** | Not in this program |

### Detail surfaces (ori-game)

| Surface | Maturity |
|---------|----------|
| `game.app` | 5 |
| `game.input` (+ gamepad) | 5 |
| `game.draw` (+ RenderTexture) | 5 |
| `game.audio` | 4–5 |
| 2D systems (tilemap, particles, physics helpers, inventory, dialogue, scene, …) | 4–5 |
| 3D / shaders / light bank | 4 |
| Mechanics | 4 |

---

## Table B — Remaining (post Linux-5)

### B1 — Phase OS (multi-OS) — **last**

| ID | Item | Priority |
|----|------|----------|
| **B1.4** | Stage Win (+ mac) libs for all ECO packages | **last** |
| **B1.4b** | smoke_windows / CI multi-OS | **last** |

### B2 — Optional deepen

| ID | Item | Priority | Status |
|----|------|----------|--------|
| **B2.1** | Tiled JSON loader | P1 | **done** — `game.tiled` + `game.ldtk` |
| **B2.2** | enet / multiplayer | P1 | **done** — `ori-enet` **0.3.0** (maturity **5 Linux**) |
| **B2.3** | Aseprite (then Spine) | P1 | **done** — `game.aseprite` + `game.spine` timelines |
| **B2.4** | 3D/audio deepen | P2 | **done** — audio pitch/pan/master; 3D cylinder/capsule/billboard |
| **B2.5** | rres ↔ assets + physics debug draw | P2 | **done** — `game.rres_assets`; `box2d`/`jolt` `.debug_draw` |
| **B1.15** | ImGui inside raylib window (not merge packages) | P2 | **done** — `imgui.init_raylib` + demo |
| **B2.12** | Split standalone `ori-raylib` repo | P3 | **done** — package `raylib` 0.1.0 |
| **B2.13** | ImGui Tier 0 (dock/tables) | P2 | **done** — docking/tables |
| **B2.14** | ImGui Tier 1 (file/plot/nodes) | P2 | **done** — `ori-imgui` **0.4.0** |
| **B2.15** | ImGui multi-context (editor vs game) | P2 | **open** |
| **B2.16** | ImGui Tier 2 (gizmos, curves, timeline, style) | P2 | **open** |
| **B2.17** | Surface maturity 4→5 (3D/shaders, mechanics, audio edge) | P2 | **open** |
| **B2.18** | New sibling ports (product-driven `ori-*`) | P2 | **open** — catalog in ROADMAP Tier A/B/C |
| **B2.19** | In-`ori-game` exploration (camera, save, A\*, actions) | P2 | **open** — existing packages only |
| **B2.20** | ECS (flecs/EnTT) | — | **declined as default** |

### B3 — Studio

| ID | Item | Priority |
|----|------|----------|
| **B3.1** | Ori Game Studio Tauri app | separate product track |

---

## Acceptance notes (Linux-5 definition used)

- Product surface for jam/mid-size games on **Linux**, not 100% C 1:1 parity.
- Each package: S3 sources, smoke green, README/CHANGELOG, version bump.
- Multi-OS deferred by explicit user decision (2026-07-14).

## How to update

After Phase OS work: raise maturity to **5 (Linux+Win)** etc., clear B1 rows.
