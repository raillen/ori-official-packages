# ori-official-packages

Official **Ori** libraries and packages maintained by the language team.

| | |
|--|--|
| **Language** | [ori-lang](https://github.com/raillen/ori-lang) |
| **Public registry (OriLamp)** | [ori-lamp](https://github.com/raillen/ori-lamp) |
| **Web stack (Lantern)** | [ori-web-framework](https://github.com/raillen/ori-web-framework) |

## Layout

```text
packages/
  ori-web/              # name = "web"
  ori-templates/        # name = "templates"
  ori-raylib/           # name = "raylib"
  ori-sqlite/           # name = "sqlite"
  …
```

Each package has `ori.pkg.toml`. The **package name** in the manifest is without
the `ori-` prefix (import / registry id). Folder names keep the `ori-` prefix
for GitHub discoverability.

## Install (registry)

```bash
export ORI_REGISTRY=https://<your-orilamp-host>/registry
ori install raylib@0.1.0
ori install web@0.1.0
```

## Local path deps

```toml
# ori.proj
[dependencies]
raylib = { path = "../packages/ori-raylib", version = "0.1.0" }
web = { path = "../packages/ori-web", version = "0.1.0" }
```

## Packages by category

Registry **id** is the short name (without `ori-`). Folder = `packages/ori-*`  
(or `web_framework/`). **Interops with** lists other packages you typically
combine it with, and **what for**.

### Meta

| Package | What it does | Interops with → purpose |
|---------|--------------|-------------------------|
| **`web_framework`** | Symbolic meta-package: installs Lantern core (`web` + `templates` + `web_app`) in one step | Optional later: `web_auth` (2FA), `web_session_sqlite` + `sqlite` (persistent sessions) |

### Web — Lantern (HTML-first)

Product: **[Lantern](https://github.com/raillen/ori-web-framework)** — *Ori is my light. Lantern is how it shines on the web.*

| Package | What it does | Interops with → purpose |
|---------|--------------|-------------------------|
| **`templates`** | Server HTML templates (`@{ }`, layouts, escape, S8 trim) | `web` → SSR pages; `web_app` → scaffolded views |
| **`web`** | HTTP router, session, CSRF, static files, JSON, middleware, uploads | `templates` → HTML responses; `web_auth` → login/2FA; `web_session_sqlite` → session backend; `sqlite` → app data |
| **`web_app`** | App conventions, `standard_app`, generators/scaffolds | `web` + `templates` → full app skeleton; demos (notes/blog/landing) |
| **`web_auth`** | TOTP 2FA + recovery helpers | `web` → secure sessions/login; needs `ori.crypto` in language runtime |
| **`web_session_sqlite`** | SQLite-backed session store for `web` | `web` → `use_custom_sessions`; **`sqlite`** → native DB |

### Storage

| Package | What it does | Interops with → purpose |
|---------|--------------|-------------------------|
| **`sqlite`** | Embedded SQLite (open/exec/query, prepared stmts, transactions) | `web_session_sqlite` → HTTP sessions; any app → local-first data; games → saves |

### Graphics & game

| Package | What it does | Interops with → purpose |
|---------|--------------|-------------------------|
| **`raylib`** | Raylib L0 bindings (`ori_rl_*` scalar shim) | `ori_game` → higher-level game API; `raygui` → in-game UI; `imgui` → editor overlay; `rres`/`stb` → assets |
| **`ori_game`** | Game helpers L1 (draw, assets, fonts, scenes) | `raylib` → backend; `rres`/`physfs` → packs/VFS; `box2d`/`jolt` → physics; `enet` → multiplayer |
| **`raygui`** | Immediate-mode UI on raylib | `raylib` / `ori_game` → HUD and tools inside the game window |
| **`rres`** | Resource packs (ORPK + rres CRC) | `ori_game` / `raylib` → ship assets in one pack; `physfs` → mount archives |
| **`freetype`** | FreeType faces + glyph raster | `harfbuzz` → shaped text; `ori_game`/`raylib` → fonts in game |
| **`harfbuzz`** | Text shaping (complex scripts) | `freetype` → faces; `ori_game` → layout/render glyphs |
| **`stb`** | stb headers (image, perlin, rect_pack) | `raylib`/`ori_game` → load images; `noise` alternative for simple noise |
| **`cgltf`** | glTF 2.0 mesh/material loader | `raylib`/`ori_game` → 3D models; `jolt` → collision meshes |
| **`fast_obj`** | Wavefront OBJ loader | `raylib`/`ori_game` → simple 3D meshes |
| **`noise`** | FastNoiseLite procedural noise | `ori_game`/`raylib` → terrain, VFX; `stb` → image-side utilities |

### Physics & navigation

| Package | What it does | Interops with → purpose |
|---------|--------------|-------------------------|
| **`box2d`** | Box2D 3.x 2D physics | `raylib`/`ori_game` → 2D games; `enet` → sync bodies |
| **`jolt`** | Jolt 3D physics (milli-unit ABI) | `raylib`/`ori_game`/`cgltf` → 3D simulation |
| **`recast`** | Recast/Detour navmesh + path queries | `jolt`/`ori_game` → AI pathfinding on level geometry |

### Systems (I/O, net, jobs, tools)

| Package | What it does | Interops with → purpose |
|---------|--------------|-------------------------|
| **`enet`** | ENet UDP host/peer + jam helpers | `ori_game` → multiplayer; `box2d`/`jolt` → state sync |
| **`physfs`** | PhysicsFS virtual filesystem | `rres`/`miniz` → mount packs/zips; `ori_game` → asset paths |
| **`lz4`** | LZ4 block + OLZ1 stream compress | `enet` → payload shrink; `physfs`/`rres` → packed data |
| **`miniz`** | deflate/CRC/zip | `physfs` → zip archives; tools → zip export |
| **`nfd`** | Native open/save file dialogs | `imgui` / desktop tools → pick files (OS dialogs) |
| **`tracy`** | Tracy profiler instrumentation | any hot path (`ori_game`, physics, UI) → frame profiling |
| **`enkits`** | enkiTS job/task scheduler | `ori_game` / tools → parallel jobs off main thread |

### UI — Dear ImGui family (tools & editors)

| Package | What it does | Interops with → purpose |
|---------|--------------|-------------------------|
| **`imgui`** | Dear ImGui core (dock, tables, multi-context; optional raylib) | Base for all `im*` packages; `raylib` → game+editor; extras below |
| **`implot`** | 2D plots (ImPlot) | `imgui` → dashboards, metrics |
| **`implot3d`** | 3D plots (ImPlot3D) | `imgui` → 3D data viz |
| **`imnodes`** | Node graph editor | `imgui` → visual scripting / graphs |
| **`imguizmo`** | TRS gizmo + curves/gradient/zoom | `imgui` + `raylib`/`ori_game` → 3D transform tools |
| **`clay`** | Clay immediate layout (headless) | UI layout without full ImGui; can pair with custom draw |
| **`imgui_extras`** | Toast, search filter, hotkeys, command palette, metrics | `imgui` → app chrome UX |
| **`markdown`** | Markdown subset drawn in ImGui | `imgui` → docs/help panels |
| **`immemory`** | Hex memory editor | `imgui` → debug tools |
| **`texinspect`** | Texture inspect (zoom, channels) | `imgui` + `raylib`/`stb` → art pipeline |
| **`imtextedit`** | Multiline text buffer | `imgui` → code/text editors |
| **`widgets`** | Knobs, toggles, spinners, themes | `imgui` → richer controls |
| **`imguidialog`** | In-ImGui open/save dialog | `imgui` → file UI without OS `nfd` |

### Quick “stack recipes”

| Goal | Suggested combo |
|------|-----------------|
| Minimal website | `templates` + `web` |
| Scaffolded web app | `web_framework` (or `web` + `templates` + `web_app`) |
| Web + login 2FA + SQLite sessions | core + `web_auth` + `web_session_sqlite` + `sqlite` |
| 2D game | `raylib` + `ori_game` (+ `box2d`, `raygui`) |
| 3D game | `raylib` + `ori_game` + `cgltf` + `jolt` (+ `recast`) |
| Editor / tooling | `imgui` + extras (`implot`, `imnodes`, `imguizmo`, …) optional `raylib` |
| Multiplayer jam | `ori_game` + `enet` (+ `lz4`) |
| Asset pipeline | `physfs` + `rres` / `miniz` + `stb` |

## Notes

- Prebuilt native libs live under each package’s `lib/` when available.
- Upstream C/C++ trees (`vendor/`) and large demos are **not** shipped here;
  rebuild scripts remain under `tools/` where present.
- Source of truth for day-to-day web work: `ori-web-framework`.
- Source of truth for game/FFI bindings during development: `game-engine-full`
  (local monorepo). This repo is the **publishable / official** snapshot.

## License

Per-package (see each `ori.pkg.toml` / README). Most packages are MIT;
exceptions (e.g. Tracy) keep their upstream licenses.
