# Roadmap — Ori game ECO (runtime + tools)

> **Updated:** 2026-07-15 (PR15 Phase OS scaffolding — ImGui tools plan **complete**)  
> **Next work (active):** Phase OS **execution** on MSVC (optional / last / non-blocking)  
> **ImGui tools residual:** plan **PRs 1–15 complete** —  
>   [`pr-plan-imgui-tools-maturity-5.md`](../../../ori-lang/docs/planning/pr-plan-imgui-tools-maturity-5.md)  
>   (`a68f7529`)  
> **Engine packages maturity-5:** `ori-lang/docs/planning/pr-plan-eco-maturity-5.md` (**complete** · W10)  
> **Layout:** packages under `Documentos/Projetos/game-engine-full/` (model A)  
> **Source of maturity scores:** `ori-lang/docs/planning/game-ports-maturity-matrix.md`  
> Studio product discussion is **separate** (Tauri vs native ImGui); this file tracks **engine libs + shared roadmap items** that Studio may consume later.  
> **Policy:** Linux implement / mature / port **first**. **Phase OS (Win/mac) is last.**  
> **Maturity (P3-B surfaces):** counted by **implementation / API / smoke of libs**, **not** by polished examples (examples can land later).  
> **ImGui rank detail:** [`IMGUI-EXTENSIONS-RANKING.md`](IMGUI-EXTENSIONS-RANKING.md) (N/D/V scores).

---

## Next work

| Priority | Item | Pointer |
|----------|------|---------|
| done | ImGui tools residual (P0–P3) → **5 (Linux)** | [`pr-plan-imgui-tools-maturity-5.md`](../../../ori-lang/docs/planning/pr-plan-imgui-tools-maturity-5.md) · **PRs 1–15 complete** |
| done | Engine packages U1–U15 → 5 | [`pr-plan-eco-maturity-5.md`](../../../ori-lang/docs/planning/pr-plan-eco-maturity-5.md) · **complete** |
| **1 — last** | Phase OS multi-OS **execution** (Win/mac smoke) | non-blocking; scaffolding done |
| separate | Studio product | `ori-game-studio` / PLANO-AMADURECIMENTO |

**Do not re-queue** packages already **5 (Linux)** (nfd, implot, imnodes, imguizmo **0.3**, imgui **0.5.x**, ImGui tools A–D, stb…recast, core stack).

---

## Architecture direction (agreed)

| Decision | Choice |
|----------|--------|
| Packages | Keep **separate** `ori-*` packages (game, box2d, jolt, imgui, …) |
| Future layout | **Monorepo** (or multi-package workspace) for development — **not** one giant linked lib |
| Studio app | Depends on packages via path/git; does not swallow them |
| Content loaders | Prefer modules in `ori-game` when pure Ori (`game.tiled`, `game.aseprite`); native FFI stays sibling packages |
| In-engine helpers | Prefer **pure Ori modules** on top of existing ECO/stdlib before new FFI packages |
| ECS | **No flecs/EnTT port by default** — composition via structs + systems/funções (see § ECS) |

---

## Implementation queue (recommended order)

| # | ID | Item | Status |
|---|-----|------|--------|
| 1 | P1-A | Tiled JSON (`game.tiled`) | **done** |
| 2 | P1-C | **Aseprite** JSON + sheet anim (`game.aseprite`) | **done** |
| 3 | P2-A | ImGui **backend raylib** (embed, not merge packages) | **done** (`init_raylib` + `demos/imgui_game`) |
| 4 | P2-B | 3D + audio deepen on `ori-game` | **done** |
| 5 | P2-C/D | rres ↔ assets + physics debug draw | **done** |
| 6 | P1-B | ENet / multiplayer | **done** (`ori-enet` **0.3.0** / maturity **5 Linux**) |
| 7 | P1-C2 | Spine runtime (after Aseprite) | **done** (bones + timelines, milli-int) |
| 8 | P1-A2 | LDtk loader | **done** |
| 9 | P3-A | ImGui multi-context + Tier 2 content tools | **done (Linux)** |
| 10 | P3-B | Surface maturity 4→5 (3D/shaders, mechanics, audio) | **done (Linux)** |
| 11 | P3-C | New `ori-*` engine ports (nfd…recast) | **done** (W9/W10 — all **5 Linux**) |
| 11b | P3-D | ImGui tools residual (P0–P3) | **done** — plan **PRs 1–15 complete** · [`pr-plan-imgui-tools-maturity-5.md`](../../../ori-lang/docs/planning/pr-plan-imgui-tools-maturity-5.md) (`a68f7529`) |
| 12 | Phase OS | Windows/mac smoke validation | **last** (scaffolding done; execution deferred) |
| — | Studio | App architecture (Tauri vs native ImGui) | **see PLANO-AMADURECIMENTO §4** (spike pós-G1 ou se viewport bloquear) |
| — | Studio ECO hooks | Content/Nodes/Play surfaces ↔ `game.*` packages | partial (hooks doc; product reality) |
| — | **Maturation / first game** | Full engine backlog + G1 gate | **[`PLANO-AMADURECIMENTO-ENGINE.md`](PLANO-AMADURECIMENTO-ENGINE.md)** |

---

## ImGui extensions roadmap (native Studio / tools)

> Goal: Studio **native** (raylib + ImGui) and in-game tools.  
> Requires **P2-A** (ImGui in raylib) — **done**.  
> Catalog ofício: [Useful Extensions](https://github.com/ocornut/imgui/wiki/Useful-Extensions)  
> Ranking N/D/V: [`IMGUI-EXTENSIONS-RANKING.md`](IMGUI-EXTENSIONS-RANKING.md)  
> **Policy:** prefer **one** upstream per category; pure-Ori MVP ok until C port pays off.  
> **IDE de código:** externa (VS Code + ori-lsp) por default — ColorTextEdit/Zep opcionais.

### Status legend

| Tag | Meaning |
|-----|---------|
| **done** | In `ori-imgui` (or equivalent) |
| **mvp** | Pure-Ori / thin MVP in `ori-imgui` — not full upstream yet |
| **P0–P4** | Port priority (P0 first) |
| **defer** | Catalogued; do not start until product gap |

### Tier 0 — Foundation (core ImGui)

| Item | Upstream | What it does | Status / Pri |
|------|----------|--------------|--------------|
| Dear ImGui | [ocornut/imgui](https://github.com/ocornut/imgui) | Immediate-mode UI core (windows, widgets, draw lists) | **done** (`ori-imgui` / cimgui) |
| Docking + multi-viewport | core ImGui docking branch | Dockable panels over game viewport | **done** (`dock_space_over_viewport`) |
| Tables / multi-select | core ImGui | Tables, selectable rows | **done** |
| Multi-context compositor | [Multi-Context Compositor](https://github.com/ocornut/imgui/wiki/Useful-Extensions) (wiki) | Multiple ImGui contexts (editor vs game) | **done** (`ori-imgui` **0.5.1** — create/set/destroy/current) |

### Tier 1 — Editor essentials

| Item | Upstream | What it does | Status / Pri |
|------|----------|--------------|--------------|
| File browser (in-UI) | own `imgui.file_browser` | List dirs/files inside ImGui | **done** |
| Native file dialog | [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended) → **ori-nfd** | OS open/save dialogs | **done** (`ori-nfd` **0.2.0** · 5 Linux) |
| ImGuiFileDialog | [aiekick/ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) | Full open/save with preview, bookmarks | **done** (`ori-imguidialog` **0.1.0** · pure Ori · 5 Linux) |
| ImGuiFD | wiki / minimal FD | Zero-STL minimal file dialog | **defer** (file_browser/nfd cover) |
| Flix01 ImGui-Addons | [Flix01/imgui wiki](https://github.com/Flix01/imgui/wiki/ImGui-Addons-Branch-Home) | Bundle of dialogs/toolbar/lists | **defer** — cherry-pick only |
| Inspector / property grid | own `imgui.inspector` | Labelled int/bool/milli fields | **done** MVP |
| Node canvas MVP | own `imgui.nodes` | Simple card graph + links | **mvp** |
| imnodes | [Nelarius/imnodes](https://github.com/Nelarius/imnodes) | Lightweight node editor, zero deps | **done** (`ori-imnodes` **0.2.0** · 5 Linux) |
| imgui-node-editor | [thedmd/imgui-node-editor](https://github.com/thedmd/imgui-node-editor) | Full blueprint-style node graph | **P1** if imnodes too limited |
| ImNodeFlow | [Fattorino/ImNodeFlow](https://github.com/Fattorino/ImNodeFlow) | Alternate flow node editor | **defer** (redundant) |
| Plot MVP | own `imgui.plot` | Ring buffer + PlotLines (milli) | **mvp** |
| ImPlot | [epezent/implot](https://github.com/epezent/implot) | GPU 2D charts (debug/profiling) | **done** (`ori-implot` **0.2.0** · 5 Linux) |
| ImPlot3D | [brenocq/implot3d](https://github.com/brenocq/implot3d) | 3D plots / point clouds | **done** (`ori-implot3d` **0.1.0** · 5 Linux) |
| Text multiline slots | core + `imgui.ui` | Basic multiline input | **partial** |
| ImGuiColorTextEdit (pthom) | [pthom/ImGuiColorTextEdit](https://github.com/pthom/ImGuiColorTextEdit/tree/imgui_bundle) | In-UI code editor + syntax highlight | **done** (`ori-imgui-textedit` **0.1.0** · 5 Linux) |
| ImGuiColorTextEdit (Balazs) | [BalazsJako/ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit) | Original ColorTextEdit | **defer** — prefer pthom fork |
| ColorTextEdit (goossens / forks) | community forks | Same role, alternate maintainers | **defer** — pick one fork only |
| Zep | [Rezonality/zep](https://github.com/Rezonality/zep) | Robust text editor (optional Vim) | **P3** / **defer** (heavy; external IDE preferred) |

### Tier 2 — Content tools (scene / anim / assets)

| Item | Upstream | What it does | Status / Pri |
|------|----------|--------------|--------------|
| ImGuizmo | [CedricGuillemet/ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) | 3D move/rotate/scale gizmos | **done** (`ori-imguizmo` **0.3.0** · TRS + suite · 5 Linux) |
| ImCurveEdit | same ImGuizmo repo | Bezier / curve editor (easing, anim) | **done** (`ori-imguizmo` **0.3.0**) |
| ImGradient | same ImGuizmo repo | Visual color gradient editor | **done** (`ori-imguizmo` **0.3.0**) |
| ImZoomSlider | same ImGuizmo repo | Zoom slider helper | **done** (`ori-imguizmo` **0.3.0**) |
| ImSequencer | CedricGuillemet ecosystem / wiki | Animation track timeline | **done** (pure Ori `imgui.timeline` in `ori-imgui` **0.5.1**) |
| Curves MVP | own `imgui.curves` | Piecewise milli curve | **mvp** |
| Timeline MVP | own `imgui.timeline` | Tracks + playhead (milli) | **done** product surface (G1–G7) |
| ImAnim | wiki Useful Extensions | UI widget animation engine | **defer** (cosmetic) |
| imGuIZMO.quat | wiki / mature quat gizmo | Quaternion-based gizmo alt | **defer** unless ImGuizmo fails |
| ImViewGuizmo | wiki | View cube / camera nav (Blender-like) | **P2** |
| Image / texture helpers | raylib + ImGui Image | Draw textures in panels | **done** (`ori-imgui` **0.5.1** image/image_button) |
| ImGuiTexInspect | wiki Useful Extensions | Advanced texture inspect (channels, zoom) | **done** (`ori-imgui-texinspect` **0.1.0** · 5 Linux) |
| imgInspect | wiki | Simpler image inspect | **P2** alt to TexInspect |
| imgui-knobs | [wiki / imgui-knobs](https://github.com/ocornut/imgui/wiki/Useful-Extensions) | Rotary dials for fine params | **done** (`ori-imgui-widgets` **0.1.0**) |
| imspinner | wiki | Loading spinners | **done** (`ori-imgui-widgets` **0.1.0**) |
| imgui_toggle | wiki | Switch-style toggles | **done** (`ori-imgui-widgets` **0.1.0**) |
| Style editor + themes | core ImGui | Built-in style editor | **partial** (ShowStyleEditor) + spectrum in widgets |
| imgui-spectrum | wiki / Adobe-like theme | Professional theme pack | **done** (`ori-imgui-widgets` spectrum helper) |

### Tier 2b — UX power tools (often pure Ori + ui)

| Item | Upstream | What it does | Status / Pri |
|------|----------|--------------|--------------|
| imgui-notify | [patrickcjk/imgui-notify](https://github.com/patrickcjk/imgui-notify) | Toast notifications | **done** (`ori-imgui-extras` **0.1.0**) |
| ImSearch | [GuusKemperman/imsearch](https://github.com/GuusKemperman/imsearch) | Immediate-mode search for large lists | **done** (`ori-imgui-extras` **0.1.0**) |
| ImHotKey | [CedricGuillemet/ImHotKey](https://github.com/CedricGuillemet/ImHotKey) | Hotkey capture/binding UI | **done** (`ori-imgui-extras` **0.1.0**) |
| ImGui Command Palette | wiki Useful Extensions | Ctrl+Shift+P style command search | **done** (`ori-imgui-extras` **0.1.0**) |
| ImGuiTextSelect | [AidanSun05/ImGuiTextSelect](https://github.com/AidanSun05/ImGuiTextSelect) | Selectable/copyable text in UI | **P2** |
| MetricsGui | [GameTechDev/MetricsGui](https://github.com/GameTechDev/MetricsGui) | FPS / frame-time widgets (industry) | **done** (`ori-imgui-extras` **0.1.0**) |
| InAppGpuProfiler | wiki Useful Extensions | In-UI GPU profiler | **P3** (or Tracy first) |
| Stack Layout | wiki | Flex-like layout | **P2** (docking covers much) |
| Splitters | wiki / ImGui demos | Draggable panel splitters | **P2** |

### Tier 3 — Debug / hex / reflection / tests

| Item | Upstream | What it does | Status / Pri |
|------|----------|--------------|--------------|
| imgui_memory_editor | [ocornut/imgui_club](https://github.com/ocornut/imgui_club) | Hex memory view | **done** (`ori-imgui-memory` **0.1.0** · 5 Linux) |
| imgui_hex_editor | [Teselka/imgui_hex_editor](https://github.com/Teselka/imgui_hex_editor) | Alternate hex editor | **defer** — prefer imgui_club |
| ImRefl | [fullptr/imrefl](https://github.com/fullptr/imrefl) | Reflection → auto UI | **P3** (C++ heavy; Ori may generate UI) |
| ImGui::Auto() | wiki Useful Extensions | Auto UI from types | **P3** same note |
| ImReflect | wiki Useful Extensions | Modern reflection UI wrapper | **P3** same note |
| imgui_test_engine | official ImGui test engine | Automated UI click tests | **done** pure Ori `imgui.test_harness` (`ori-imgui` **0.5.1**) — not full C++ engine |
| imgui_markdown | wiki Useful Extensions | Render Markdown in UI (help/docs) | **done** pure Ori (`ori-imgui-markdown` **0.1.0**) |
| DearImGui-with-IMM32 | wiki Useful Extensions | Windows IME (CJK input) | **done** Linux stubs; real Win IMM32 → Phase OS |
| ImGuiDatePicker | wiki Useful Extensions | Date picker widget | **defer** (low engine value) |

### Tier 3b — App shell / remoting / exotic

| Item | Upstream | What it does | Status / Pri |
|------|----------|--------------|--------------|
| Hello ImGui | [pthom/hello_imgui](https://github.com/pthom/hello_imgui) | App framework around ImGui | **defer** — we own raylib loop |
| ImRAD | wiki Useful Extensions | WYSIWYG UI designer → C++ | **defer** (study only) |
| netImGui | wiki Useful Extensions | Remote ImGui on another machine | **P3** / **defer** |
| imgui-ws | wiki Useful Extensions | ImGui over WebSockets (browser) | **P3** / **defer** |
| ImTui | wiki Useful Extensions | Terminal/text-mode ImGui | **defer** (headless niche) |
| Software Renderer (emilk) | wiki Useful Extensions | CPU ImGui render (no GPU) | **defer** (only if no GL) |

### ImGui port order (native Studio / tools)

```text
DONE (W9/W10):  ImGuizmo TRS · imnodes · ImPlot · nfd · engine ports 5 Linux

DONE (tools A–D, a68f7529 PRs 2–13):
  ori-imguidialog 0.1.0
  ori-imguizmo 0.3.0 (CurveEdit + Gradient + ZoomSlider)
  ori-imgui 0.5.1 (timeline + multi-ctx/image + test_harness)
  ori-imgui-extras 0.1.0
  ori-imgui-texinspect 0.1.0
  ori-imgui-textedit 0.1.0
  ori-imgui-widgets 0.1.0
  ori-imgui-memory 0.1.0
  ori-implot3d 0.1.0
  ori-imgui-markdown 0.1.0 (IME Linux stubs; Win real → Phase OS)

LAST: Phase OS (plan PR15 scaffolding + multi-OS execution)

defer  ImNodeFlow · imGuIZMO.quat · Hello ImGui · ImRAD · ImTui
       software renderer · DatePicker · ImAnim · Zep · reflection
       remoting · GPU profiler (use Tracy)
```

### Packaging suggestion

```
# DONE (5 Linux) — game-engine-full/
ori-raylib  ori-game  ori-imgui (0.5.1)  ori-raygui  ori-box2d  ori-jolt
ori-rres  ori-sqlite  ori-enet  ori-freetype  ori-harfbuzz
ori-nfd  ori-implot  ori-implot3d  ori-imnodes  ori-imguizmo (0.3.0)
ori-stb  ori-noise  ori-miniz  ori-lz4  ori-tracy  ori-enkiTS
ori-cgltf  ori-fast-obj  ori-physfs  ori-clay  ori-recast
ori-imguidialog  ori-imgui-extras  ori-imgui-texinspect
ori-imgui-textedit  ori-imgui-widgets  ori-imgui-memory
ori-imgui-markdown
# (no ori-imsequencer — pure Ori timeline in ori-imgui)
```

Do **not** merge into `ori-game`. Studio / tools depend via path-deps.  
**Layout:** `Documentos/Projetos/game-engine-full/` (not flat `Projetos/ori-*`).

### Quick catalog (all named libs → one line)

| Library | Link | One-line role |
|---------|------|----------------|
| Dear ImGui | https://github.com/ocornut/imgui | Core immediate-mode UI |
| imgui-node-editor | https://github.com/thedmd/imgui-node-editor | Full blueprint node graph |
| imnodes | https://github.com/Nelarius/imnodes | Lightweight node graph |
| ImNodeFlow | https://github.com/Fattorino/ImNodeFlow | Alternate node flow editor |
| ImGuizmo | https://github.com/CedricGuillemet/ImGuizmo | 3D transform gizmos |
| ImCurveEdit | same ImGuizmo repo | Curve/easing editor |
| ImGradient | same ImGuizmo repo | Color gradient editor |
| ImZoomSlider | same ImGuizmo repo | Zoom slider |
| ImHotKey | https://github.com/CedricGuillemet/ImHotKey | Hotkey binding UI |
| ImPlot | https://github.com/epezent/implot | 2D real-time plots |
| ImPlot3D | https://github.com/brenocq/implot3d | 3D plots |
| ImGuiColorTextEdit (Balazs) | https://github.com/BalazsJako/ImGuiColorTextEdit | Code editor (original) |
| ImGuiColorTextEdit (pthom) | https://github.com/pthom/ImGuiColorTextEdit/tree/imgui_bundle | Code editor (maintained fork) |
| Zep | https://github.com/Rezonality/zep | Heavyweight text editor (±Vim) |
| imgui_memory_editor | https://github.com/ocornut/imgui_club | Hex memory editor |
| imgui_hex_editor | https://github.com/Teselka/imgui_hex_editor | Alternate hex editor |
| ImGuiFileDialog | https://github.com/aiekick/ImGuiFileDialog | Rich file open/save dialog |
| Flix01 Addons | https://github.com/Flix01/imgui/wiki/ImGui-Addons-Branch-Home | Addon pack (dialogs, lists, …) |
| imgui-notify | https://github.com/patrickcjk/imgui-notify | Toast notifications |
| ImSearch | https://github.com/GuusKemperman/imsearch | List search widget |
| ImGuiTextSelect | https://github.com/AidanSun05/ImGuiTextSelect | Selectable UI text |
| MetricsGui | https://github.com/GameTechDev/MetricsGui | FPS / perf metric widgets |
| ImRefl | https://github.com/fullptr/imrefl | Reflection → auto inspector |
| Hello ImGui | https://github.com/pthom/hello_imgui | ImGui application framework |
| Useful Extensions (wiki) | https://github.com/ocornut/imgui/wiki/Useful-Extensions | Official extension index |
| imgui_test_engine | ImGui official test engine | Automated UI tests |
| ImSequencer | wiki / Cedric ecosystem | Animation timeline sequencer |
| ImAnim | wiki Useful Extensions | UI animation engine |
| ImGuiFD | wiki Useful Extensions | Minimal zero-STL file dialog |
| imgui_markdown | wiki Useful Extensions | Markdown rendering in UI |
| DearImGui-with-IMM32 | wiki Useful Extensions | Windows IME (CJK) |
| imgui-knobs | wiki Useful Extensions | Rotary knobs |
| imspinner | wiki Useful Extensions | Loading spinners |
| imgui_toggle | wiki Useful Extensions | Toggle switches |
| Stack Layout | wiki Useful Extensions | Flex-like layout |
| Splitters | wiki Useful Extensions | Draggable split panes |
| ImRAD | wiki Useful Extensions | WYSIWYG UI designer → C++ |
| imgui-spectrum | wiki Useful Extensions | Adobe Spectrum-like theme |
| Software Renderer (emilk) | wiki Useful Extensions | CPU-only ImGui renderer |
| netImGui | wiki Useful Extensions | Remote ImGui display |
| imgui-ws | wiki Useful Extensions | ImGui over WebSockets |
| ImTui | wiki Useful Extensions | Terminal ImGui |
| ImGuiTexInspect | wiki Useful Extensions | Advanced texture inspector |
| imgInspect | wiki Useful Extensions | Simple image inspector |
| ImGuiDatePicker | wiki Useful Extensions | Date picker |
| imGuIZMO.quat | wiki Useful Extensions | Quaternion gizmo alt |
| ImViewGuizmo | wiki Useful Extensions | Camera view-cube gizmo |
| ImGui::Auto() | wiki Useful Extensions | Auto UI from types |
| ImReflect | wiki Useful Extensions | Modern reflection UI |
| Multi-Context Compositor | wiki Useful Extensions | Multiple ImGui contexts |
| Command Palette | wiki Useful Extensions | VS Code-style command search |
| InAppGpuProfiler | wiki Useful Extensions | In-UI GPU profiler |

---

## Exploration inside `ori-game` (no new package)

> **Status:** actions / save / camera / pathfind / cutscene / net_predict — **done** (B2.19).  
These are **modules/helpers** on top of **existing** stack (stdlib + raylib/ori_game + optional sqlite/enet/raygui/imgui).  
They do **not** require new sibling repos.

| Área | Depends on (already in Ori / ECO) | New package? |
|------|-----------------------------------|--------------|
| Input rebinding / actions | `game.input` + L0 `raylib` | **No** — pure Ori |
| Save slots | `game.save` + `ori.fs`; optional **`sqlite`** | **No** |
| Camera 2D (follow, shake, bounds) | `game.camera` + raylib | **No** |
| UI layout 2D (nine-patch, anchors) | `game.draw`; optional **`raygui`** / **`imgui`** | **No** |
| Navmesh / A\* 2D (grid) | pure Ori + tilemap/LDtk grids | **No** |
| Cutscene / coroutine runner | pure Ori + `game.dialogue` / `game.scene` | **No** |
| Net prediction helpers | pure Ori + **`enet`** | **No** |

Stdlib that already plugs in: `ori.fs`, `ori.json`, `ori.list`, `ori.map`, `ori.math`, `ori.mem`, `ori.time`, `ori.net` (when needed).

---

## Candidate new `ori-*` ports (library choices)

> **Canonical catalog (priorities + upstream):**  
> [`ori-lang/docs/planning/eco-library-ports-catalog.md`](../../../ori-lang/docs/planning/eco-library-ports-catalog.md)  
> (path from sibling checkout: `../ori-lang/docs/planning/eco-library-ports-catalog.md`)  
>
> **Rule of three for ports:** (1) clear product need, (2) C-friendly ABI or thin C++ wrapper, (3) does not duplicate an already-5 surface without a hard gap.  
> Tables below are a **summary**; edit the catalog when priorities change.

### Tier A — recommend porting when needed (high value)

| Package idea | Recommended upstream | Notes |
|--------------|----------------------|--------|
| **ori-nfd** | nfd / pfd | **done** 0.2.0 · 5 Linux |
| **ori-implot** | implot | **done** 0.2.0 · 5 Linux |
| **ori-imnodes** | imnodes | **done** 0.2.0 · 5 Linux |
| **ori-imguizmo** | ImGuizmo TRS + suite | **done** 0.3.0 · 5 Linux |
| **ori-stb** | stb image/perlin/rect_pack | **done** 0.2.0 · 5 Linux |
| **ori-cgltf** | cgltf | **done** 0.2.0 · 5 Linux |
| **ori-fast-obj** | fast_obj | **done** 0.2.0 · 5 Linux |
| **ori-noise** | FastNoiseLite | **done** 0.2.0 · 5 Linux |
| **ori-physfs** | PhysFS | **done** 0.2.0 · 5 Linux |
| **ori-miniz** | miniz | **done** 0.2.0 · 5 Linux |
| **ori-lz4** | lz4 | **done** 0.2.0 · 5 Linux |
| **ori-tracy** | Tracy | **done** 0.2.0 · 5 Linux |
| **ori-enkiTS** | enkiTS | **done** 0.2.0 · 5 Linux |
| **ori-imgui-textedit** | ColorTextEdit pthom | **done** 0.1.0 · 5 Linux |
| **ori-miniaudio** | miniaudio | **skipped** — `game.audio` covers gap |
| **ori-imguidialog** | ImGuiFileDialog (pure Ori) | **done** 0.1.0 · 5 Linux |
| **ori-imgui-extras** | notify/search/hotkey/palette/metrics | **done** 0.1.0 · 5 Linux |
| **ori-imgui-texinspect** | texture inspect | **done** 0.1.0 · 5 Linux |
| **ori-imgui-widgets** | knobs/toggle/spinner/theme | **done** 0.1.0 · 5 Linux |
| **ori-imgui-memory** | hex memory editor | **done** 0.1.0 · 5 Linux |
| **ori-implot3d** | ImPlot3D | **done** 0.1.0 · 5 Linux |
| **ori-imgui-markdown** | pure Ori markdown + IME stubs | **done** 0.1.0 · 5 Linux |

### Tier B — good later / conditional

| Package idea | Recommended upstream | When |
|--------------|----------------------|------|
| **ori-clay** | Clay | **done** 0.2.0 · 5 Linux |
| **ori-recast** | Recast Navigation | **done** 0.2.0 · 5 Linux |
| **OpenAL Soft** | openal-soft | **open** conditional (3D spatial only) |
| **ori-ozz** | ozz-animation | **open** conditional |
| **cute_c2** / cute_* | cute_headers | **open** conditional |
| **cJSON / parson** | — | Prefer **`ori.json`** — do not port by default |

### Tier C — usually **do not** port (or deprioritize)

| Idea | Verdict |
|------|---------|
| **Yoga** | Prefer **Clay** for our stack; Yoga is React-flex mental model, heavier C++ |
| **cglm / HandmadeMath** | Prefer **`ori.math` + small pure-Ori vec modules** (or thin raylib math); avoid second math ABI unless FFI forces it |
| **bgfx** | **No** as lighting lib — it is a **full rendering backend**. Conflicts with raylib GL path; only if abandoning raylib as primary renderer |
| **ejson** | Unclear need vs `ori.json` + PhysFS/rres; skip unless a concrete format requires it |
| **Chipmunk / Bullet** | Do not dual-stack physics while box2d/jolt are Linux-5 |
| **flecs / EnTT** | See **§ ECS** — not aligned as default engine core |

### Suggested port order (current)

1. ~~ImGui P0 + engine ports~~ **done** (W9/W10 — all **5 Linux**)  
2. ~~ImGui tools residual A–D → maturity 5~~ **done** (`a68f7529` PRs 1–14)  
3. miniaudio **skipped** unless new audio gap  
4. ozz / OpenAL Soft / cute_c2 — only with measured product gap  
5. Phase OS Win/mac execution + plan **PR15** scaffolding — **last**  
6. Studio product — separate track (`ori-game-studio`) 

---

## ECS (flecs / EnTT) — philosophy

**Recommendation: do not port flecs or EnTT as the default game object model.**

| Concern | Ori ECO position |
|---------|------------------|
| Language philosophy | Reading-first, **explicit composition** (structs + free functions + traits `apply`), not hidden archetype magic |
| Studio / editor | Entities as **data** (JSON/`ogame`) + **systems as named Ori modules** — already closer to “ECS lite” without a C++ runtime |
| Cost | flecs/EnTT = large C++ surface, codegen, versioning, and a second “truth” next to Ori types |
| When it *might* make sense | Only as an **optional advanced package** if a product needs millions of entities and measured bottlenecks — not for jam/mid-size Linux-5 |

**Preferred model (keep):**

- Components = **structs**  
- Systems = **functions** in `game.mechanics.*` / user modules  
- Scenes = **data** (`game.scene`, `game.ogame`, LDtk/Tiled)  
- Optional: a thin pure-Ori **archetype/query helper** later (no flecs) if ergonomics demand it  

---

## Surface maturity 4→5 (implementation checklist)

Examples **do not** gate maturity. Focus: API completeness, edge cases, smoke of the **library**.

| Surface | Gaps to close for 5 |
|---------|---------------------|
| `game.audio` | Buses/groups, SFX pool, seek/loop range, spatial helper, load from pack/memory |
| 2D systems | Tilemap draw/collide; particles sprites; inventory remove/use/serialize; dialogue branches; scene lifecycle |
| Content | LDtk external/world; spine skins or documented bones-only 5; tiled render/tileset; rres batch/errors |
| 3D / shaders / lights | Material MVP, shadow or honest limits, fog/env preset, shader presets, light bank limits documented |
| Mechanics | Integrate with input/physics; combat i-frames/hitbox; AI patrol/aggro; reference platformer *as optional later* |

---

## Monorepo (future)

When ready to consolidate development:

```
ori-ecosystem/   (or keep separate remotes + workspace)
  packages/
    ori-game/
    ori-box2d/
    ori-jolt/
    ori-imgui/
    …
  apps/
    ori-game-studio/
  docs/
```

Until then: sibling repos under `Documentos/Projetos/game-engine-full/ori-*` remain canonical (model A folder; each package keeps its git remote).

---

## Done recently (context)

- Linux-5 on seven ECO packages  
- Integration demos + `smoke_eco_linux.sh`  
- Windows PS1 scaffolding (unvalidated)  
- `game.tiled` P1-A · `game.aseprite` P1-C · ImGui raylib P2-A · audio/3D P2-B  
- **P2-C** `game.rres_assets` + `demos/rres_assets`  
- **P2-D** `box2d.debug_draw` / `jolt.debug_draw` (visual demos updated)  
- **P1-B** `ori-enet` **0.3.0** — channels, broadcast, protocol, 5 demos (maturity 5 Linux)  
- **P1-A2** `game.ldtk` · **P1-C2** `game.spine` (bones + rotate/translate timelines)  
- **Phase OS** scripts complete for 8 packages + `smoke_eco_windows.ps1` includes enet

- **W9/W10** engine ECO packages → **5 (Linux)** @ 0.2.0 (stb…recast + high ports)
- **game-engine-full/** layout (model A)
- **ImGui tools residual A–D** → **5 (Linux)** (`a68f7529` PRs 2–13) + catalog/smoke PR14
  - imguidialog · imguizmo 0.3 · imgui 0.5.1 · extras · texinspect · textedit · widgets · memory · implot3d · markdown
