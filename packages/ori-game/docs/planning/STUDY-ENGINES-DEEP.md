# Estudo aprofundado — engines em `study/` → Ori

> **Data:** 2026-07-15  
> **Clones:** `game-engine-full/study/{godot,trenchbroom,rbfx,fyrox}` (shallow)  
> **Filosofia Ori (não negociável):** funcionalidade ≫ polish; ECS-lite (não flecs);  
> G1 primeiro; Studio destino **ori-imgui** + raylib; IDE código **externa**;  
> packages ECO separados; `game.world` + systems ordenados.  
> **Companion:** [`STUDY-ENGINES.md`](STUDY-ENGINES.md) · [`PLANO-AMADURECIMENTO-ENGINE.md`](PLANO-AMADURECIMENTO-ENGINE.md) ·  
> [`PLANO-IMPLEMENTACAO-STUDIO.md`](PLANO-IMPLEMENTACAO-STUDIO.md) · [`ROADMAP-GAME-ECO.md`](ROADMAP-GAME-ECO.md)

Este documento **não** propõe portar código C++/Rust das engines.  
Extrai **filosofia, padrões de implementação e features** e classifica o que encaixa no stack Ori já portado.

**Journal append-only (obrigatório a cada leitura de source):**  
[`study/SOURCE-LEARNINGS.md`](../../../study/SOURCE-LEARNINGS.md) — catálogo `P-*` + entradas datadas.  
Este DEEP é **síntese**; o log é o **diário que cresce** a cada iteração.

---

## 0. Mapa rápido: Ori hoje vs as quatro

| Camada | Ori (hoje) | Godot | TrenchBroom | rbfx | Fyrox |
|--------|------------|-------|-------------|------|-------|
| Loop de frame | `app.run_window` + systems explícitos | `SceneTree::physics_process` / `process` | N/A (editor) | `Scene::Update` + events | `Scene::update` → graph |
| Mundo | `game.world` AoS + flags | Node tree | Brush/Entity map | Node + Component | Graph + **pool handles** (não ECS) |
| Script | Ori + ogs + ScriptHost | GDScript/C# | — | C#/AngelScript/plugins | scripts crate |
| Render L0 | **raylib** | RenderingServer | TbRender/GL | Graphics/RenderAPI | deferred+forward |
| Física | sphere–AABB level; `ori-box2d` / `ori-jolt` | PhysicsServer 2D/3D | colliders de brush | Physics / Physics2D | physics no graph update |
| Editor | Tauri (transição) → **ImGui** | editor/ monólito | **o produto** (mapa) | Source/Editor | `editor/` nativo |
| Serialização | scene/entity JSON | Resource + .tscn | .map / formatos Quake | XML/JSON scene | visitor + scene |
| Nav | `ori-recast` | NavigationServer | — | Navigation | pathfinding no core |
| UI tools | `ori-imgui` + extensões | docks custom | Qt/custom UI libs | SystemUI / RmlUI | **fyrox-ui** (draw cmds) |

### Packages ECO já alinhados a “servers”

| Papel “server-like” | Package / módulo Ori |
|---------------------|----------------------|
| Display / input / audio L0 | `ori-raylib` |
| Physics 2D / 3D | `ori-box2d`, `ori-jolt` |
| Navmesh | `ori-recast` |
| Assets pack / FS | `ori-rres`, `ori-physfs` |
| Mesh import | `ori-cgltf`, `ori-fast-obj` |
| Tasks | `ori-enkiTS` |
| Net | `ori-enet` |
| Text | `ori-freetype`, `ori-harfbuzz` |
| Studio shell | `ori-imgui`, imguizmo, imnodes, implot, nfd… |
| World / content | `ori-game` (`game.world`, `ogame_*`, tiled, ldtk…) |

**Conclusão de arquitetura:** Ori já tem a **separação ECO em packages** (estilo Godot *servers* + monorepo de games). O gap não é “mais libs” — é **colar World + systems + um caminho de conteúdo + Studio na mesma verdade**.

---

## 1. Godot (`study/godot`)

### 1.1 O que o source mostra

**Layout real (confirmado no clone):**

```text
core/       # Object, Variant, OS
servers/    # audio, display, navigation_*, physics_*, rendering, text, xr…
scene/main/ # Node, SceneTree, Viewport, Window, Timer…
scene/2d|3d/
editor/     # monólito do editor
modules/    # gdscript, etc.
main/       # entry / MainLoop
```

**Loop (código):**

- `SceneTree::physics_process` (`scene/main/scene_tree.cpp`):  
  flush transforms → `MainLoop::physics_process` → signal `physics_frame` →  
  `_process(true)` (nós physics) → flush ugc/message queue → **timers/tweens** physics →  
  flush transforms → **delete queue** → idle callbacks.
- `SceneTree::process` (frame variável): FTI / interpolation, depois process normal dos nós.
- `Node`: `NOTIFICATION_PROCESS` / `NOTIFICATION_PHYSICS_PROCESS`, prioridades,  
  `_process` / `_physics_process` virtuais (GDVIRTUAL), process groups.

**Ideia-chave:** árvore de nós **não é** o low-level de física/render — isso passa por **servers** (RIDs). A árvore orquestra; o server executa.

**Groups:** `call_group` / `call_group_flags` — broadcast por tag de grupo (útil para “todos os enemies”, “pause menu”).

### 1.2 Filosofia

| Pegar | Evitar no G1 |
|-------|----------------|
| Separar **autor (scene)** de **serviços (servers)** | Reimplementar RenderingServer |
| Ordem de frame com fases (physics vs idle) | Process thread groups complexos |
| Resource serializável ≠ Node runtime | Variant/Object monólito em Ori |
| RID = handle opaco (`ModelId`, …) | Editor monólito C++ |
| Timers/tweens no tree (como alarms) | Signals genéricos cedo (pós-G1) |
| Editor e runtime falam a **mesma** scene | Viewport Web ≠ runtime (nosso gap atual) |

### 1.3 O que trazer para Ori

| Prioridade | Ideia | Onde encaixa | Quando |
|------------|-------|--------------|--------|
| **P0** | Documentar e **fixar** ordem de frame como Godot light (input → script → move → collision → camera → draw) | `world_systems` + sample | **B5** (já quase) |
| **P0** | Alarms/timers no tick (Godot `process_timers`) | `ogame_alarms` + system | pós-B3 polish |
| **P1** | Tags/groups → `call_group` light (iterar por tag no World) | `game.world` tags | pós-G1 |
| **P1** | Resource vs Instance: entity.json = Resource; World entity = runtime | já conceitual | B8 doc |
| **P2** | PhysicsServer façade: `game.physics_*` wrapping box2d/jolt com IDs | `ori-box2d`/`ori-jolt` | Fase F |
| **P2** | NavigationServer thin → recast já portado | `game.navmesh` | pós-G1 |
| **—** | Signals/bus | strings/tags | Fase F |
| **—** | MultiplayerAPI | `ori-enet` sample | pós-G1 |

### 1.4 Não copiar

- SceneTree com milhares de nós e process modes ricos.  
- `editor/` inteiro.  
- Physics interpolation (FTI) antes de colisão estável.  
- GDScript dual — **Ori é a linguagem**.

---

## 2. TrenchBroom (`study/trenchbroom`)

### 2.1 O que o source mostra

**Arquitetura em libs (não monólito único):**

| Lib | Papel |
|-----|--------|
| **TbMdlLib** | Modelo de mapa: `Brush`, `BrushFace`, `BrushNode`, builders, validators |
| **VmLib** | Math (vec, plane, bbox, mat) |
| **TbRenderLib / TbGlLib** | Visualização |
| **TbUiLib** | UI do editor |
| **TbFsLib / TbBaseLib** | FS, util |

**Brush (código):** `mdl/Brush.h` — faces (`BrushFace`) + `BrushGeometry` (poliedro), create a partir de bounds+faces, epsilon de vértice.

**CSG real:** `Polyhedron_CSG.h` (`Subtract` minuend/subtrahend → fragments), `Polyhedron_Clip.h` — **poliedro / half-space**, não AABB slab.

**Entidades de mapa:** brushes + point entities + validadores (empty brush entity, mixed contents, …).

**Formato:** pensado em pipelines Quake/id Tech (`.map`), não em “engine genérica”.

### 2.2 Filosofia

| Pegar | Evitar |
|-------|--------|
| **Mapa é o produto** — ferramentas de brush primeiro | Virar TB clone |
| Separar **modelo de mapa** (Mdl) de render/UI | CSG half-space no G1 |
| Operações com **Result** e geometria explícita | Compilar BSP completo cedo |
| Extrude / clip / vertex edit como **ops de domínio** | UI Qt |

### 2.3 O que trazer para Ori

| Prioridade | Ideia | Ori hoje | Quando |
|------------|-------|----------|--------|
| **P0** | Manter solids **confiáveis** no sample (AABB ok) | `ogame_level` + CSG multi-slab | **B5–B7** |
| **P1** | Modelo de brush autorável: faces/plane depois | só box/plane/ramp + subtract AABB | Fase **E1** |
| **P1** | Ops de nível como funções puras + testes (estilo Result TB) | `ogame_level_csg` | ao aprofundar |
| **P2** | Extrude / push-pull (ideia TB, geo simplificada) | — | E1 |
| **P2** | Grid snap + layers de contents | layer int nos solids | E |
| **—** | Import `.map` Quake | — | opcional / nunca se G1 3D AABB basta |
| **—** | Portar Polyhedron CSG C++ | — | só se 3D level for o eixo longo |

### 2.4 Relação com filosofia Ori

Ori **já escolheu** level 3D via scene brushes JSON + CSG AABB.  
TB ensina **o que um autor de mapa espera** (extrude, clip, feedback geométrico) — implementar **depois de G1**, sem abandonar o path AABB até o sample ser jogável.

---

## 3. rbfx (`study/rbfx` — Urho3D fork)

### 3.1 O que o source mostra

**Scene model:**

- `Node` + `Component` (Serializable, enable/disable, `DrawDebugGeometry`).  
- `LogicComponent`: lifecycle **Start / DelayedStart / Stop / Update / PostUpdate / FixedUpdate** + bitmasks `UpdateEvent` (USE_UPDATE, USE_FIXEDUPDATE, …).  
- `Scene::Update`: escala timeStep, dispara **eventos de update** configuráveis (`cookedUpdateEvents_`), não hardcode único de todos components no Scene — **subscribers** reagem.  
- Prefabs: `PrefabResource`, `PrefabReference`, `NodePrefab` — template reutilizável.  
- Subsystems: `GetSubsystem<ResourceCache>()`, WorkQueue, Physics, Navigation, Network (Replica), UI…

**Editor:** `Source/Editor` no mesmo repo — runtime e tools compartilham tipos.

### 3.2 Filosofia

| Pegar | Evitar |
|-------|--------|
| **Component + LogicComponent** = dados + hooks de frame | C++ component inheritance deep |
| Fixed vs variable update (physics vs frame) | Event bus com dezenas de event IDs cedo |
| Prefab = Resource reutilizável | Plugin C++ system |
| Debug draw por component | RmlUI paralelo ao ImGui |
| Scene update events = systems ordenáveis | Multiplayer Replica no G1 |

### 3.3 O que trazer para Ori

| Prioridade | Ideia | Mapeamento Ori | Quando |
|------------|-------|----------------|--------|
| **P0** | Lifecycle Create ≈ Start; Step ≈ Update; FixedUpdate ≈ collision/physics | ScriptHost + systems | **B3–B5** |
| **P0** | `DrawDebugGeometry` por collider/mesh | sample + `draw3d` wires | **B5–B6** |
| **P1** | Prefab = pasta `entities/<id>/` (já) + instance overrides | ogame | documentar B8 |
| **P1** | DelayedStart = “after all spawned” pass | `script_create` depois de spawn_scene | polish |
| **P2** | Subsystem registry mental = packages ECO | path-deps | já |
| **P2** | WorkQueue → `ori-enkiTS` para jobs de load | enkits | pós-G1 |
| **—** | AngelScript dual language | — | nunca (Ori only) |

### 3.4 Por que rbfx é o parente mais próximo do World Ori

`game.world` (EntityId + components opcionais + systems) ≈ **Node+Component light** sem árvore profunda.  
`LogicComponent::Update` ≈ `script_step` + move system.  
**Não** precisamos da hierarquia Urho inteira no G1 — só da **clareza de lifecycle**.

---

## 4. Fyrox (`study/fyrox`)

### 4.1 O que o source e `ARCHITECTURE.md` mostram

**Decisão explícita (ARCHITECTURE.md):**

- **Não usa ECS.**  
- Usa **generational arenas (pools)** + **handles** (`pool.rs` no fyrox-core).  
- Acoplamento monólito com deps **unidirecionais** (renderer depende de scene; scene não conhece renderer).  
- Crates semi-standalone: core, sound, ui; engine agrega.

**Scene (código):**

```rust
// fyrox-impl/src/scene/mod.rs
pub struct Scene {
    pub graph: Graph,           // nós + transforms + update
    pub rendering_options: ...,
    pub enabled: ...,           // multi-scene: menu vs level
}
// update → graph.update (physics, sound, hierarchy stats)
```

**PerformanceStatistics** no update: sync, sound, physics step, raycast, hierarchy — **instrumentação de frame** como cidadão de primeira classe.

**Editor:** crate `editor/` + plugins — nativo, **mesma engine** do jogo (alinha com destino ori-imgui).

**fyrox-ui:** node-based UI que **não renderiza** — emite draw commands (qualquer backend).  
Analogia: ImGui já é o nosso “UI imediata”; fyrox-ui é outro paradigma (retained).

### 4.2 Filosofia

| Pegar | Evitar |
|-------|--------|
| **Handles + pool** (EntityId já é int) | Pool genérico Rust em Ori (não necessário) |
| Scene **enabled** flag multi-scene | Multi-scene no G1 |
| Editor = mesmo runtime | fyrox-ui segundo toolkit |
| Stats de update por fase | Deferred+forward custom |
| Graph update switches (ligar/desligar physics) | Reflect/type_uuid system |
| Visitor serialization unificada | Visitor complexo cedo (JSON já basta) |

### 4.3 O que trazer para Ori

| Prioridade | Ideia | Ori | Quando |
|------------|-------|-----|--------|
| **P0** | Studio nativo = **mesmo** World/draw do jogo | Fase **C** ImGui | pós-G1 |
| **P1** | `Scene.enabled` / múltiplas cenas carregadas | scene load API | F |
| **P1** | Contadores de tempo por system no sample (Tracy: `ori-tracy`) | debug HUD | B7–B8 / polish |
| **P1** | GraphUpdateSwitches ≈ flags “skip collision / skip scripts” | tick switches | útil debug |
| **P2** | Handles generation (stale EntityId) | `EntityId` + gen counter | se despawn reuso doer |
| **—** | ECS Bevy-style | contraria decisão ECO | não |
| **—** | Substituir ImGui por fyrox-ui | — | não |

### 4.4 Alinhamento forte com decisão Ori

Fyrox valida por escrito o que o ROADMAP ECO já fechou: **“composition + systems, not flecs”**.  
O pool/handle de Fyrox é o primo elegante do `EntityId` + lista densa de `game.world`.

---

## 5. Síntese cruzada — o que Ori deve “roubar”

### 5.1 Filosofia consolidada (já quase a nossa)

```text
GM:     autor pensa em Create/Step/Draw/Alarm (simples)
Unity:  ordem de frame previsível + composição
Godot:  scene/resource vs servers (packages ECO)
Urho:   LogicComponent lifecycle + prefab + debug draw
Fyrox:  handles/pools, sem ECS, editor = runtime
TB:     level tools sérios = domínio de mapa separado
```

**Ori G1:**

```text
Entity (tipo) + Instance (cena) + World (runtime)
Systems ordenados (funções Ori, sem function pointers em structs)
ogs/ScriptHost opcional (GM events light)
raylib L0 + ECO packages = servers
Studio → mesma verdade (ImGui destino)
```

### 5.2 Matriz “trazer / adiar / nunca”

| Item | Fonte | Ação | Fase Ori |
|------|-------|------|----------|
| Ordem de frame fixa + doc | Unity/Godot/Urho | **manter e documentar** | B5–B8 |
| Dispatch Create/Step | GM/Urho | **já ScriptHost**; completar entity scripts reais | B3+ |
| Debug draw colliders | Urho/Godot | **B5** | B5 |
| Camera follow | Unity/Godot | **já B4** | — |
| Mesh estável Play | — | **B6** | B6 |
| Solids sem crash | TB light | **B7** | B7 |
| Guia primeiro jogo | — | **B8** | B8 |
| Groups por tag | Godot | depois | pós-G1 |
| Physics façade IDs | Godot servers | box2d/jolt | F |
| Navmesh uso no World | Godot/Urho | recast | pós-G1 |
| Prefab doc | Urho/Unity | entity folder | B8 |
| Multi-scene enabled | Fyrox | | F |
| Extrude/clip brush | TB | | E1 |
| CSG half-space | TB | só se level 3D for eixo longo | E+ |
| Signals/event bus | Godot | | F |
| ImGui = Studio principal | Fyrox editor lesson | | **C** |
| ImGuizmo/imnodes/ImPlot | ranking ECO | packages **já portados** — **usar** | **D** |
| flecs/DOTS | — | **nunca default** | — |
| Segundo UI kit | Fyrox-ui | **não** | — |
| Three = verdade de jogo | — | **não** | — |

### 5.3 Onde o ECO já “ganhou o estudo”

Não precisamos portar:

| Engine tem | Ori já tem |
|------------|------------|
| Physics engine | box2d, jolt |
| Nav | recast |
| Net | enet |
| Job system | enkiTS |
| Pack assets | rres, physfs, miniz, lz4 |
| Gizmo/nodes/plots | imguizmo, imnodes, implot |
| File dialog | nfd |
| Profiling | tracy |
| Mesh load | cgltf, fast_obj |

**Trabalho real:** integração no **World/sample/Studio**, não mais ports.

---

## 6. Recomendações por fase (ação concreta)

### Agora (fechar G1) — influências

1. **B5** — Urho `DrawDebugGeometry` + Godot “physics then draw”:  
   validar sphere–AABB no chão; debug ON por default no sample até estável.  
2. **B6** — Resource load path Godot-like: falha de GLB → fallback sphere + log (não crash).  
3. **B7** — TB: solids como “mapa mínimo”; smoke sem crash.  
4. **B8** — Doc no estilo GM: Entity / Instance / Scene / loop (1 página).

### Logo após G1 — Fase C (Fyrox + Godot editor lesson)

- Hierarchy = lista de instances do **mesmo** scene JSON do World.  
- Viewport = raylib (não Three).  
- Play = tick World in-process **ou** AOT (já estável).  
- **Não** copiar fyrox-ui; usar **ori-imgui** (+ docks simples).

### Fase D — packages já no disco

Ordem de **uso** no Studio nativo (não re-portar):

1. ImGuizmo (transform instance)  
2. imnodes (ogs graph)  
3. ImPlot (stats/frame)  
4. nfd (open project/assets)

### Fase E — TrenchBroom

- Extrude/push-pull em brushes box (sem half-space completo).  
- Só aprofundar CSG poliedral se o **primeiro jogo** exigir mapas complexos.

### Fase F

- PhysicsServer thin (jolt/box2d) no World.  
- Multi-scene (Fyrox `enabled`).  
- Groups/signals leves.  
- enet sample.  
- flecs **só se medido** (default continua systems).

---

## 7. Anti-padrões detectados no estudo (não importar)

| Anti-padrão | Fonte | Por quê Ori recusa |
|-------------|--------|---------------------|
| Editor Web reimplementa o jogo | nosso passado Three | uma verdade: raylib |
| ECS hard default | Bevy/DOTS community | ECO + plano fecham **não** |
| Monólito que engole packages | engines “one repo one lib” | packages `ori-*` irmãos |
| Dual language (C# + script) | Urho/Godot | Ori only |
| CSG industrial no G1 | TB full | bloqueia G1 |
| Process priorities / thread groups | Godot 4 | complexidade sem autor |
| UI retained + ImGui juntos | Fyrox+ImGui mix | um shell |

---

## 8. Como continuar o estudo (roteiro de leitura)

```bash
# Godot — loop e nó
study/godot/scene/main/scene_tree.cpp   # physics_process / process
study/godot/scene/main/node.h           # notifications, process flags
study/godot/servers/                    # lista de servers

# TrenchBroom — domínio de mapa
study/trenchbroom/lib/TbMdlLib/include/mdl/Brush.h
study/trenchbroom/lib/TbMdlLib/include/mdl/Polyhedron_CSG.h
study/trenchbroom/lib/TbMdlLib/include/mdl/BrushBuilder.h

# rbfx — components e update
study/rbfx/Source/Urho3D/Scene/LogicComponent.h
study/rbfx/Source/Urho3D/Scene/Component.h
study/rbfx/Source/Urho3D/Scene/Scene.cpp  # Update

# Fyrox — pool e scene
study/fyrox/ARCHITECTURE.md
study/fyrox/fyrox-core/src/pool.rs
study/fyrox/fyrox-impl/src/scene/mod.rs
study/fyrox/editor/src/                 # estrutura do editor nativo
```

Perguntas ao abrir cada arquivo:

1. Isso é **autor**, **runtime** ou **tool**?  
2. Já temos package ECO?  
3. Entra em G1, pós-G1, ou nunca?  
4. Cabe em **função Ori + struct**, sem herança OOP?

---

## 9. Conclusão executiva

| Engine | Presente principal para Ori |
|--------|----------------------------|
| **Godot** | Servers (= packages ECO) + SceneTree fases + groups |
| **TrenchBroom** | Domínio de mapa e ops de brush (pós-G1 / E1) |
| **rbfx** | LogicComponent lifecycle + prefab + debug geometry (G1) |
| **Fyrox** | Handles/pool sem ECS + editor = mesmo runtime (C) |

**Foco imediato do produto:** não estudar mais para adiar código — **fechar B5–B8** com as lições de Urho/Godot (debug, loop, fallbacks) e só então Studio ImGui (Fyrox lesson) usando **ImGuizmo/imnodes já portados**.

---

*Atualizar este arquivo quando um clone shallow mudar major ou quando uma fase B/C absorver uma ideia daqui.*
