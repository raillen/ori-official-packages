# Plano de implementação — Ori Game Studio + runtime

> **Revisado:** 2026-07-15 — A–F **MVP done** + fases **G+** (faltantes) neste arquivo  
> **Status anterior (2026-07-10):** Tauri+React como host definitivo — **superseded**  
> **Runtime:** Ori + `ori-game` + raylib (+ ECO)  
> **Docs-mãe:** [`PRODUCT-REALITY.md`](PRODUCT-REALITY.md) · [`FEATURE-CATALOG.md`](FEATURE-CATALOG.md) · [`PLANO-AMADURECIMENTO-ENGINE.md`](PLANO-AMADURECIMENTO-ENGINE.md) · specs `docs/specs/`

---

## Documento-base da fila de implementação

**Este arquivo é a base canônica** da ordem **A→F (MVP)** e **G→M (pós-MVP)** + progresso por checkbox.

| Papel | Doc |
|-------|-----|
| **Fila + checkboxes (este)** | `PLANO-IMPLEMENTACAO-STUDIO.md` ← **marcar `[x]` ao fechar cada ID** |
| Detalhe / narrativa dos faltantes | [`PLANO-IMPLEMENTACAO-FALTANTES.md`](PLANO-IMPLEMENTACAO-FALTANTES.md) (espelho explicativo; checkboxes **aqui** têm precedência) |
| Continuidade da sessão | `DEV-HANDOFF.md` §3 |
| Como implementar | `docs/specs/<módulo>/` |
| Catálogo produto | `FEATURE-CATALOG.md` |

**Regra para agentes:** ao fechar ID (S1.1, V2.1, L3, …), marcar **`[x]` na §4 deste plano**, atualizar `DEV-HANDOFF.md` §3, `./scripts/sync_planning.sh`.
---

## 0. Norte (não negociável)

| # | Princípio |
|---|-----------|
| 1 | **Funcionalidade do jogo > polish visual** da IDE |
| 2 | **Gate G1** = dá para fazer o **primeiro jogo** (Play, câmera, colisão, mesh/sprite, loop estável) |
| 3 | **Modelo de mundo:** ECS-lite / híbrido — `EntityId` + **components** (dados) + **systems** (funções) + **script events** opcionais (estilo GM) |
| 4 | **Não** ECS hard (flecs/DOTS/Bevy) como default |
| 5 | **Studio canônico (destino):** `ori-imgui` + raylib (mesmo stack do jogo) |
| 6 | **Studio atual (transição):** Tauri 2 + React — manter até G1 runtime; não investir em polish Three |
| 7 | **IDE de código:** **externa** (VS Code / Zed + `ori-lsp`); Monaco in-process é opcional |
| 8 | **Uma verdade de render no destino:** raylib (não Three como preview “oficial”) |
| 9 | Nunca marcar **done** sem Play/smoke real |

---

## 1. Decisões de produto (revisadas 2026-07-15)

| Item | Valor (novo) | Nota |
|------|----------------|------|
| Nome | **Ori Game Studio** | — |
| Host Studio **destino** | **ori-imgui** + raylib (+ extensões ImGui) | Hierarchy/Inspector/Viewport/Console |
| Host Studio **agora** | Tauri 2 + React (legado útil) | Transicional; sem novas features cosméticas |
| Código / LSP | **IDE externa** + `ori-lsp` | “Open in editor” no Studio |
| Runtime world | **`game.world` ECS-lite** | Ver §2 |
| UI vocabulário | **Entity** (tipo) + **Instance/EntityId** (cena) | Sem “kind” na UI |
| Cena | JSON próprio (`scenes/*.scene.json`) | Evolve para components explícitos |
| Script | 1 Entity tipo ↔ 1 `.orl` (+ `.ogn` layout nós) | Dispatch no tick do World |
| Play | `ori check` + **`ORI_USE_AOT=1`** + `run_window` | Ver PLAY-PACKAGING |
| Nós visuais | ogs:* + `imgui.nodes` / futuro **imnodes** | `.orl` canônico |
| Level | Brushes/CSG no mesmo espaço 3D | Extrude depois de G1 loop |
| Lógica pesada | Runtime Ori; tools ImGui; sem reimplementar jogo em WebGL | — |

Documento de visão histórica: `ORI-GAME-STUDIO-VISAO.md` (atualizar mentalmente com este plano).

---

## 2. Arquitetura alvo — ECS-lite / híbrido

### 2.1 Modelo de dados

```text
EntityId
  ├── Transform          (x,y,z / rot / scale)
  ├── Mesh | Sprite      (path, scale)
  ├── Collider           (sphere/box, layer)
  ├── PhysicsBody?       (handle box2d/jolt — depois)
  ├── Script             (entity type id → .orl / ogs host)
  ├── Vars               (map de overrides da cena)
  └── Tags               (strings)

World
  ├── entities: ids vivos
  ├── tables/maps de components
  └── ordered system list
```

**Híbrido de propósito:**

- Entities **simples** (sem Script): só systems (`move`, `draw`).  
- Entities **ricas** (player/NPC): component Script → `on_create` / `on_step` / ogs / alarms (GM-like).

### 2.2 Player loop (ordem fixa — G1)

```text
1. input_system
2. alarm_system
3. script_system          (on_step / ogs.tick)
4. move_system            (se houver Velocity)
5. collision / level resolve
6. camera_system          (follow)
7. draw_system            (level + meshes/sprites + debug)
```

API única preferida: `game.app.run_window` + world tick, ou `game.loop` futuro.

### 2.3 Studio nativo (destino) vs Tauri (agora)

```text
DESTINO                          TRANSIÇÃO
┌─────────────────────┐          ┌─────────────────────┐
│ ori-imgui shell     │          │ Tauri + React       │
│ Hierarchy/Inspector │          │ docks + Three prev  │
│ Viewport = raylib   │          │ Play = ori run AOT  │
│ Play in-process or  │          │ IDE script embutido │
│   subprocess AOT    │          │   (opcional)        │
└──────────┬──────────┘          └──────────┬──────────┘
           │ scene.json · .orl · assets     │
           └────────────┬───────────────────┘
                        ▼
              ori-game + raylib + World
```

**IDE externa** em ambos os caminhos para editar `.orl`.

### 2.4 Repos

| Repo | Papel na visão nova |
|------|---------------------|
| `ori-game` | Runtime, World, sample G1, planos |
| `ori-imgui` | Shell Studio nativo + extensões |
| `ori-game-studio` | App Tauri **legacy/transição** |
| `ori-raylib`, `ori-jolt`, … | ECO L0/L1 |

---

## 3. Skills e agents (atualizado)

### 3.1 Skills por fatia

| Área | Skills |
|------|--------|
| Sempre | `clean-code`, `living-docs`, `ori-game-studio` (atualizar skill se stack mudar) |
| Runtime World / G1 | `ori-testing`, `rust` se FFI |
| Studio nativo ImGui | `ori-game-studio`, Ori + `c-secure` (shim), refs ImGui ranking |
| Studio Tauri (só manutenção) | `tauri-desktop`, `typescript`, `react-spa` |
| Ports ImGui C++ | `c-secure`, `rust` se host, ranking P0–P4 |

### 3.2 Agents (orientativos)

| Agent | Foco novo |
|-------|-----------|
| `ogs-runtime` | World, systems, sample Play, smoke |
| `ogs-studio-native` | ori-imgui Hierarchy/Inspector/demo |
| `ogs-studio-legacy` | Tauri só se bugblocker |
| `ogs-graph` | ogs lower/lift + imnodes depois |

---

## 4. Fases de implementação (checklist vivo)

> Marque **`[x]`** só com critério atendido (smoke/teste/visível).  
> **Próximo:** OS host Win/mac · polish opcional  
> **Deep Linux done:** Studio Bake+Play patrol · recast bake · world_net_sync · G1 · F6 — 2026-07-15

### Mapa A→M

```text
A–F   MVP G1 + shell ECO + author mínimo          [x] done (Linux)
G     Studio usável (S*)                            [x] MVP
H     Viewport + assets (V*)                        [x] MVP (+ V3 probe)
I     Pipeline conteúdo                             [x] L1–L4 + T3
J     Runtime de jogo (R*)                          [x] MVP (+ R7 session)
K     Look / VFX / áudio (X*)                       [x] MVP Linux
L     Gameplay systems (GP*)                        [x] + nav_path API
M     Plataforma / ship (P*)                        [x] P1–P4 scaffold (Win/mac host-exec remaining)
```

**Fora (salvo pedido):** flecs; polish Three Tauri; IDE de código embutida.

### Fase A — Fundação honesta

- [x] **A1** — Play windowed + AOT + packaging (sample abre janela)
- [x] **A2** — Smoke check+compile+`run_window` (`smoke_sample_play.sh`)
- [x] **A3** — Docs realidade + este plano (PRODUCT-REALITY + planos)
- [x] **A4** — Não expandir Three “features” (só bugfixes críticos no Tauri)

### Fase B — **G1 runtime** (ECS-lite no sample)

- [x] **B1** — `game.world` (EntityId + components + spawn scene) — testes
- [x] **B2** — Systems ordenados no sample (`engine_test`) — um loop só
- [x] **B3** — script_system Create/Step (player) — ScriptHost + ogs
- [x] **B4** — camera follow — sample usa
- [x] **B5** — collision + debug estável — WASD / parede + tests
- [x] **B6** — mesh cowboy GLB no Play — estável ou fallback sphere
- [x] **B7** — level solids confiáveis — floor+wall, sem crash
- [x] **B8** — Guia “primeiro jogo / engine_test” — `ENGINE-TEST-GUIDE.md`

**Gate G1 (runtime sample):** B1–B8 fechados. Detalhe honesto: PRODUCT-REALITY / FEATURE-CATALOG.

### Fase C — Studio nativo mínimo (ori-imgui)

> Spec: [`docs/specs/studio-shell/`](../../../docs/specs/studio-shell/) · demo: `ori-imgui/demos/studio_shell/`

- [x] **C0** — Scaffold shell + dockspace (ImGui+raylib, menu)
- [x] **C1** — Hierarchy + select (instances JSON)
- [x] **C2** — Inspector x,y,z + save scene (instances round-trip)
- [x] **C3** — Viewport raylib = solids + mesh + seleção
- [x] **C4** — Play in-process (`world_systems`) + Console logs — `demos/studio_shell`
- [x] **C5** — Open script → IDE/OS (`app.open_path` + raylib OpenURL)
- [x] **C6** — **GO** ImGui shell principal — [`C6-IMGUI-SHELL-GO.md`](C6-IMGUI-SHELL-GO.md)

### Fase D — Ports ImGui P0 (ECO)  ← **completa**

Ordem canônica (`ROADMAP-GAME-ECO` + ranking). Packages portados **e** usados em `demos/studio_shell`:

- [x] **D1** — ImGuizmo (painel Gizmo + TRS selection; FULL + nudge shim)
- [x] **D2** — imnodes (grafo Create→Step ogs-like)
- [x] **D3** — ImPlot (stats dt_ms / frame)
- [x] **D4** — nfd / FileDialog (Open scene + project folder)

Smoke: `ori-imgui/demos/studio_shell/tools/smoke.sh` (check+compile+run exit 124).

### Fase E — Conteúdo autorável (pós-C mínimo)  ← **completa**

- [x] **E1** — Extrude / push-pull level (`game.ogame_level_tools` + painel shell)
- [x] **E2** — Timeline / curves MVP (`imgui.timeline` + `imgui.curves` no shell)
- [x] **E3** — Tilemap paint (`game.tilemap` paint/fill + painel; eixo 2D)
- [x] **E4** — notify / search / command palette (`ori-imgui-extras` no shell)

### Fase F — Ampliar engine  ← **completa (Linux first)**

- [x] **Physics no World** — `game.world_physics` + `physics_system` (pure Ori gravity host; `body_handle` para ECO box2d/jolt)
- [x] **Multi-scene** — `game.multi_scene` + switch no shell (`engine_test` ↔ `room1`)
- [x] **Net sample** — `ori-enet` 5 demos (`tools/smoke_linux.sh`)
- [x] **Packaging multi-OS** — Linux AOT + `PHASE-OS.md` scripts Win (execução MSVC host-only)
- [x] **flecs** — **não adotado** sem medição; default **ECS-lite** (decisão explícita)

Smoke shell: `ori-imgui/demos/studio_shell/tools/smoke.sh`  
Tests: `test_level_tools`, `test_tilemap_paint`, `test_world_physics`, `test_multi_scene`

### Pós-F polish (já no produto base — não reabrir)

- [x] **PF1** — Save scene com brushes (`write_scene_with_level`)
- [x] **PF2** — Sample root auto-discover + mesh GLB no shell
- [x] **PF3** — `room1` jogável + win condition `inst_goal`
- [x] **PF4** — Box2D `world_sync` + jolt/box2d rename `*.dynamics`
- [x] **PF5** — Dock Godot-like (`dock_space_studio` + Reset layout)
- [x] **PF6** — Tema Spectrum dark/light (Adobe-inspired, sem fork)
- [x] **PF7** — Janela resizable + maximize; viewport free zone + RMB/MMB/wheel

---

### Fase G — Studio usável (author loop)  ← **MVP done**

> Specs: `studio-shell`, `studio-project-io`, `studio-play` · código: `demos/studio_shell/`

#### S1 — Place instance

- [x] **S1.1** — Hierarchy: “Add instance” (entity do catalog)
- [x] **S1.2** — Spawn posição default (origem / offset da seleção)
- [x] **S1.3** — Dirty + round-trip no save scene  
**Done:** criar player/enemy sem editar JSON.

#### S2 — Inspector completo

- [x] **S2.1** — x,y,z + **rot** + **scale** (overrides)
- [x] **S2.2** — `var_overrides` editáveis (float mesh/collider; upsert API)
- [x] **S2.3** — mesh_path / collider_radius / tags (mesh_path text + radius)
- [x] **S2.4** — bloqueio honesto em Play  
**Done:** editar instance → save → reabrir igual.

#### S3 — Project IO

- [x] **S3.1** — Open project (nfd) + `ori.pkg.toml` / `project.ogame.toml`
- [x] **S3.2** — Paths a partir do projeto aberto (não só cwd)
- [x] **S3.3** — Save scene no path da cena atual (não só `/tmp`)
- [x] **S3.4** — Dirty flag na UI (* toolbar / console)
- [x] **S3.5** — Recent projects (lista curta na sessão / Project panel)  
**Done:** open sample → edit → save → reopen.

#### S4 — Toolbar (Unity/Godot)

- [x] **S4.1** — Play / Stop (+ F5)
- [x] **S4.2** — Tool mode: Select · Place · Level
- [x] **S4.3** — Grid snap toggle + valor (tools)
- [x] **S4.4** — Debug colliders toggle  
**Done:** ações principais sem menus profundos.

#### S5 — App Studio (sair do demo)

- [x] **S5.1** — Package/app produto `ori-studio/` (`tools/run.sh` + `smoke.sh`) → shell
- [x] **S5.2** — Branding mínimo (Spectrum + título shell G–L)
- [x] **S5.3** — Docs “como abrir o Studio” (ENGINE-TEST-GUIDE / ori-studio README)  
**Done Linux:** entry product face; sources still under demos/studio_shell.

#### S6 — UX commands

- [x] Notify / search / palette wired (E4)
- [x] **S6.1** — Atalho teclado abre palette (Ctrl+P / Ctrl+K)
- [x] **S6.2** — Comandos cobrem place/save/play/reset layout/frame

---

### Fase H — Viewport + assets  ← **MVP done**

> Specs: `studio-viewport`, `runtime-assets`, `runtime-camera`

#### V1 — Viewport estável

- [x] **V1.0** — Free zone + orbit mouse (RMB/MMB/wheel) + F11
- [x] **V1.1** — Resize dock / reabrir sem crash (smoke OK; black screen residual rare)
- [x] **V1.2** — Grid + seleção sempre visíveis
- [x] **V1.3** — Banner se GLB/shader falhar (Inspector + log)
- [x] **V1.4** — Frame selection (F) + reset câmera editor

#### V2 — Pick e gizmo 3D

- [x] **V2.1** — Click no viewport seleciona instance (raycast `game.ray3d`)
- [x] **V2.2** — ImGuizmo com view/proj da câmera editor (`game.camera_matrix` milli)
- [x] **V2.3** — Orbit/pan/zoom sem conflitar UI (base + free zone)

#### V3 — Preview de assets

- [x] **V3.1** — GLB/OBJ probe no Inspector (load status + Probe mesh)
- [x] **V3.2** — PNG/JPG GPU preview (`texture_gl_id` + ImGui `image`)
- [x] **V3.3** — Tree Project: `assets/` · `scenes/` · `entities/`

#### V4 — Script open

- [x] Open path / IDE externa (C5)
- [x] **V4.1** — Feedback se falta arquivo + criar stub `.orl`

---

### Fase I — Pipeline de conteúdo profundo  ← **L1–L3 done**

> Escolher **um** eixo (recomendação: **3D level**). Specs: `level-*`, `content-tilemap`

#### Trilha L — Level 3D (recomendado)

- [x] Extrude face + snap (E1)
- [x] Save brushes com scene
- [x] **L1** — Select brush na Hierarchy
- [x] **L2** — Extrude + highlight brush no viewport
- [x] **L3** — Subtract autorável = mesmo `collect_solids` runtime
- [x] **L4** — Clip plane AABB half (`clip_brush_at` Tools)
- [x] **L5** — Terrain column displacements (create/raise/row → solids)
- [x] **L6** — CSG boolean/merge/axis-BSP clip (`ogame_level_csg` + tools; not polyhedron Hammer)

**Done trilha L (MVP):** editar brush/subtract no Studio → solids iguais no Play.

#### Trilha T — Tilemap 2D (alternativa)

- [x] Paint/fill/erase shell (E3)
- [x] **T1** — Save compact tilemap (`serialize_compact` / write)
- [x] **T2** — `collect_solid_rects` (API; wiring Play 2D sample opcional)
- [x] **T3** — Sample 2D mínimo (`examples/sample_2d_walk.orl`)
- [x] **T4** — Viewport 2D / dual mode no Studio (Toolbar + View menu; tilemap + xz)

**Regra:** não metade L e metade T no mesmo sprint.

---

### Fase J — Runtime de jogo  ← **MVP done**

> Specs: `runtime-*` · testes: `tests/test_phase_g_l.orl`

#### R1 — Character controller

- [x] **R1.1** — Sphere controller (`game.character_controller`)
- [x] **R1.2** — Sample + Studio Play usam controller (toggle)
- [x] **R1.3** — Debug draw controller (sample key `1`)

#### R2 — Script / events

- [x] ScriptHost ogs Create/Step (B3)
- [x] **R2.1** — Dispatch create/step estável (tick_scripts)
- [x] **R2.2** — Alarms API (`game.ogame_alarms` / ogame facade)
- [x] **R2.3** — Play freshness stamp (`game.play_freshness` + shell warn / write stamp)

#### R3 — Collision / queries

- [x] Sphere–AABB + layer int
- [x] **R3.1** — Layers/masks nomeados (`game.collision_layers`)
- [x] **R3.2** — Raycast gameplay (`world.raycast_spheres` + pick ray3d)
- [x] **R3.3** — Triggers enter/exit (`game.world_triggers`)
- [x] **R3.4** — Box/capsule colliders (half-extent → resolve radius)

#### R4 — Physics ECO no jogo

- [x] PhysicsHost + box2d.world_sync smoke + jolt boxes_fall
- [x] **R4.1** — PhysicsHost sample Linux (`examples/physics_host_sample.orl`)
- [x] **R4.2** — body_handle / gravity no Play do Studio (toggle)
- [x] **R4.3** — Character sobre Jolt (`jolt.world_character` + `character_world_test`)

#### R5 — Câmera

- [x] follow_offset sample
- [x] Editor orbit mouse base (PF7)
- [x] **R5.1** — Multi-cam / stack (`game.camera_stack` · sample + shell V/1–4)
- [x] **R5.2** — Timeline drive camera (`game.camera_drive` · sample/shell **T**)
- [x] **R5.3** — Editor camera documentada (console hints + F frame)

#### R6 — World model

- [x] EntityId + components densos
- [x] **R6.1** — Parent/child transform (1 nível)
- [x] **R6.2** — Despawn (`world.despawn` + gameplay dead)
- [x] **R6.3** — Multi-scene **runtime** stack push/pop
- [x] **R6.4** — flecs **não** adotado (decisão F)

#### R7 — Net

- [x] ori-enet demos smoke
- [x] **R7.1** — Sample host + 1 peer (`enet.session` + `game_tf_sync`)
- [x] **R7.2** — `enet.tf_codec` encode/decode + session exchange
- [x] **R7.2+** — World state replicate + **enet wire** (`world_replicate` · `examples/world_net_sync.orl`)

---

### Fase K — Look, VFX, áudio  ← **MVP Linux**

> Specs: `render-*`, `vfx-*`, `audio-runtime`

- [x] **X1** — Paths estáveis (`game.asset_paths` + shader/mesh resolve sample/shell)
- [x] **X2** — PostFX presets toggable (`game.postfx` · sample F · shell Project)
- [x] **X3** — Particles no sample (win FX) + API `game.particles`
- [x] **X4** — Decals pool (`game.decals` · sample win + shell spawn)
- [x] **X5** — Audio buses + `assets/audio/blip.wav` no sample (win SFX)
- [x] **X6** — Timeline shell (E2) + mesh_scale overrides (author); skeletal clips already runtime

---

### Fase L — Gameplay systems  ← **MVP Prot A**

> Specs: `gameplay-*` · host: `game.world_gameplay` · shell Play + sample

- [x] **GP1** — Combat contact damage (MVP hit radius)
- [x] **GP2** — Health + death + i-frames
- [x] **GP3** — AI chase (aggro radius)
- [x] **GP4** — Path follow pure + `nav_path` + **bake solids** (`game.nav_bake` · smoke)
- [x] **GP5** — Waves counters (2 waves)
- [x] **GP6** — Inventory coins on win + save/load slot (`/tmp/ori_engine_test_g1`)
- [x] **GP7** — HUD in-game (sample + Studio Play)
- [ ] **GP8** — Stealth (protótipo B opcional)

Protótipos: **A** waves+combat+HUD · **B** stealth+AI.

---

### Fase M — Plataforma e ship  ← **scripts done · host Win/mac = checkbox execução**

> Specs: `platform-packaging`, `platform-os` · `PHASE-OS.md` · `tools/export_game_package.sh` · `tools/smoke_ci_linux.sh`  
> Validate sem host: `./tools/validate_os_scripts.sh`

- [x] Linux AOT / smoke sample + shell
- [x] Scripts Windows ECO (build/smoke por package + umbrella)
- [x] **P1** — `smoke_eco_windows.ps1` polido (ok/fail/skip, `-ValidateOnly`, core 8 + studio probes; **execução MSVC = host**)
- [x] **P2** — Export/package jogo (`export_game_package.sh` · triple detect Linux/Darwin)
- [x] **P3** — macOS smoke scaffold (`smoke_macos.sh` + `smoke_eco_macos.sh`; **execução Mac = host**)
- [x] **P4** — CI smoke Linux shell + sample (`smoke_ci_linux.sh` + `.github/workflows/smoke-linux.yml`)
- [x] Guards rename dynamics (jolt/box2d smoke_windows) + `validate_os_scripts.sh`

---

### Dívidas técnicas (checklist)

- [x] **DT1** — Shaders/assets path via `game.asset_paths` (cwd-independent roots)
- [x] **DT2** — Clash `*.world` → `box2d/jolt.dynamics`
- [x] **DT3** — Split return World+Host AOT (política)
- [x] **DT4** — `begin_window` sempre com `end_window` (shell)
- [x] **DT5** — Refresh PRODUCT-REALITY vs tilemap/timeline/dock

---

## 5. O que fazer com o trabalho Tauri já existente

| Manter | Não investir |
|--------|----------------|
| Open project, entity wizard, scene JSON save | Novos painéis cosméticos |
| Play AOT + console logs | Viewport Three “paridade raylib” |
| Hierarchy/Inspector como **referência de UX** | Monaco como único caminho de script |
| Schema scene/entity | Dock polish / temas premium |

**Regra:** bug no Tauri que **bloqueia** editar cena → corrige. Feature nova de editor → prefere **C\*** ImGui + World.

---

## 6. Mapeamento antigo → novo

| Fase antiga | Destino |
|-------------|---------|
| R0–R3, S0–S6, V3D* “done MVP” | Histórico; ver PRODUCT-REALITY (não = G1) |
| R-vis nodes xyflow | Mantém no Tauri se útil; destino imnodes + ogs |
| R4–R7 (timeline, dialogue editors…) | Fase E/F — **depois** de B+C |
| “Monaco + Tiptap P0” | Código = IDE externa; Notes = baixa prioridade vs G1 |

---

## 7. Critérios de “fase done”

1. Código com `clean-code` + AGENTS do repo  
2. Teste ou smoke **executável** (não só `ori check` quando for Play)  
3. CHANGELOG + este plano (checkbox)  
4. Amostra no sample ou demo ImGui  
5. Sem marcar done se o usuário não consegue **ver** a feature no Play/tools  

---

## 8. Riscos (visão nova)

| Risco | Mitigação |
|-------|-----------|
| Dois Studios para sempre | Fase C6 decide; Tauri vira archive |
| ECS-lite vira flecs cedo | Proibido sem profiling |
| ImGui sem editor de código | IDE externa desde o dia 1 |
| Three e raylib divergem | Destino = só raylib no viewport |
| Escopo “Unity completo” | G1 rígido; lista ImGui P3+ só sob demanda |

---

## 9. Próximas ações imediatas (ordem)

**Studio UI / G1 (agora):**  
1. **G1 / F6 / R5.2** — **done**  
2. **R7.2+** + enet wire — **done**  
3. **Recast bake + Studio Bake / Play patrol** — **done**  
4. OS host Win/mac

**Host-only (quando tiver máquina):** executar checklist Win/mac em `PHASE-OS.md`.

Narrativa: [`PLANO-IMPLEMENTACAO-FALTANTES.md`](PLANO-IMPLEMENTACAO-FALTANTES.md).

---

## 10. Ligações

| Doc | Papel |
|-----|--------|
| **Este plano** | **Base canônica A→M + checkboxes** |
| [PLANO-IMPLEMENTACAO-FALTANTES.md](PLANO-IMPLEMENTACAO-FALTANTES.md) | Narrativa / sprints dos faltantes |
| [PLANO-AMADURECIMENTO-ENGINE.md](PLANO-AMADURECIMENTO-ENGINE.md) | Backlog motor + estudo engines |
| [FEATURE-CATALOG.md](FEATURE-CATALOG.md) | Features + pipeline retro |
| [PRODUCT-REALITY.md](PRODUCT-REALITY.md) | Honestidade de maturidade |
| [ROADMAP-GAME-ECO.md](ROADMAP-GAME-ECO.md) | Ports + catálogo ImGui |
| [IMGUI-EXTENSIONS-RANKING.md](IMGUI-EXTENSIONS-RANKING.md) | Rank extensões |
| [PLAY-PACKAGING.md](PLAY-PACKAGING.md) | AOT / native_libs |
| [PHASE-OS.md](PHASE-OS.md) | Multi-OS scripts |
| [STUDY-ENGINES-DEEP.md](STUDY-ENGINES-DEEP.md) | Estudo 4 engines |
| [`docs/specs/`](../../../docs/specs/) | Specs por módulo |

---

*Plano vivo. A–F = MVP · G–M = caminho a produto.  
**Sempre** atualizar checkboxes da §4 ao fechar um ID.*
