# Plano de amadurecimento — Ori Game Engine + Studio

> **Data:** 2026-07-15  
> **Objetivo de produto:** engine **extremamente funcional** para o **primeiro jogo completo**  
> **Prioridade:** funcionalidade real ≫ beleza visual / polish de UI  
> **Inspirações:** GameMaker (loop de autor simples), Godot (cena + servidores + open source), Unity (GameObject/Component + player loop)  
> **Status de partida:** protótipo (ver [`PRODUCT-REALITY.md`](PRODUCT-REALITY.md))  
> **Estudo de código:** clone shallow Godot em `Documentos/Projetos/study-godot` (quando concluído)

Este documento é a **lista canônica do que ainda falta** + **filosofia/arquitetura**  
extraída das engines de referência + **fases mensuráveis** até “consigo fazer meu primeiro jogo”.

---

## 0. Norte (não negociável)

| # | Princípio |
|---|-----------|
| 1 | **Um jogo vertical real** (ex.: top-down ou walk 3D curto) define o bar, não a feature list da Unity. |
| 2 | **Play sempre abre janela** e o sample é o contrato de qualidade. |
| 3 | **Um caminho de conteúdo profundo** por vez (level 3D **ou** 2D tilemap — não metade dos dois). |
| 4 | **Não marcar “done”** sem: aparece no editor/sample · `ori run`/Play · smoke. |
| 5 | **ECS pesado (DOTS/flecs) adiado** — composição com structs + sistemas explícitos (decisão ECO já fechada). |
| 6 | **UI Studio: Tauri OK no curto prazo**; migrar para **ImGui nativo** se o webview impedir estabilidade de ferramentas. |
| 7 | Linguagem de produto: **Ori** + runtime `game.*` + raylib L0; Studio edita, não reimplementa o jogo em WebGL. |

**Definição de “primeiro jogo possível” (gate G1):**

- Abrir projeto no Studio sem crash.  
- Colocar player + solids (level ou tilemap).  
- Editar script, Play, ver movimento + colisão + câmera.  
- Ver mesh **ou** sprite no Play (um dos dois bem).  
- Build/run AOT Linux estável.  
- Fechar e reabrir sem corromper cena.

Tudo o que não for necessário para G1 fica **depois de G1**.

---

## 1. Estudo comparativo — filosofia e arquitetura

### 1.1 GameMaker (inspiração de **loop de autor**)

| Conceito | Ideia | Lição para Ori |
|----------|--------|----------------|
| Object | Tipo reutilizável | **Entity** (`entities/<id>/`) |
| Instance | Colocação na room | Instance na `scene.json` |
| Events | Create / Step / Draw / Alarm / Collision | Handlers canônicos + **dispatch no tick** |
| Room | Nível / cena | Scene JSON |
| GML | Script 1:1 com Object | `.orl` 1:1; nós = lower/lift |
| Alarms | Timers fixos por instance | `game.ogame_alarms` + host |
| DnD / visual | Ações lineares | N1 ogs:* (subset; não copiar DnD inteiro) |

**Filosofia:** o autor não pensa em ECS — pensa em **“o que esse bicho faz no Create/Step”**.  
**Arquitetura:** loop global → para cada instance viva → dispara events na ordem fixa.  
**O que Ori ainda não tem:** dispatch automático Create/Step/Draw/Collision no Play.

### 1.2 Unity (inspiração de **composição + player loop**)

| Conceito | Ideia | Lição para Ori |
|----------|--------|----------------|
| GameObject | Nó de cena com transform | Instance 3D com `x,y,z` + rot/scale |
| Component | Dados + comportamento anexado | Hoje: vars + 1 script; **depois** tags/components leves |
| MonoBehaviour lifecycle | Awake → OnEnable → Start → FixedUpdate → Update → LateUpdate → render | **Player loop documentado** (ordem fixa) |
| Scene | Hierarquia + load/unload | Scene JSON + (futuro) multi-scene |
| Prefab | Template de GO | Entity pasta = prefab |
| Physics | Collider + Rigidbody + layers | Sphere/AABB agora; Box2D/Jolt depois |
| Camera | Component de view | Runtime camera3d + follow; editor Orbit/Top |
| Assets | Import pipeline | Content browser + path estável no Play |
| DOTS/ECS | Dados massivos | **Fora do G1** |

**Filosofia:** “tudo é GO + componentes”; ordem de execução **global e previsível**.  
**Arquitetura:** PlayerLoop (sistemas enfileirados) + scripts nos GOs.  
**O que Ori deve copiar cedo:** ordem de frame **explícita** (input → alarms → entity step → physics resolve → camera → draw).  
**O que Ori não deve copiar cedo:** DOTS, SRP custom completo, multi-pipeline de render.

Ordem de referência (Unity, simplificada para Ori):

```text
1. process input
2. tick alarms / timers
3. entity on_step (script / ogs host)
4. physics / resolve colliders
5. camera follow / late camera
6. begin draw → world → UI → end draw
```

### 1.3 Godot (inspiração de **cena + servidores + open source**)

| Conceito | Ideia | Lição para Ori |
|----------|--------|----------------|
| Node tree | Hierarquia de nós | Hierarchy + parent/children (já MVP) |
| Scene (.tscn) | Grafo instanciável | Scene JSON (schema estável) |
| Resource | Dados serializáveis reutilizáveis | entity.json, meshes paths, maps |
| Servers | RenderingServer, PhysicsServer, … (API low-level) | raylib + módulos `game.*` como “serviços” |
| RID | Handle opaco de recursos no server | `ModelId`, `TextureId` no runtime |
| Signals | Eventos desacoplados | Futuro: eventos string/tag (pós-G1) |
| Editor dock | Viewport + Inspector + FileSystem | Studio Unity-like (ou ImGui docks) |
| GDScript | Linguagem de jogo | **Ori** (não recriar GDScript) |

**Filosofia:** separar **cena (autor)** de **servers (motor)**; o editor fala a mesma linguagem conceitual do runtime.  
**Arquitetura (pastas típicas do source):**

```text
godot/
  core/       # tipos, OS, templates
  servers/    # rendering, physics, audio, text, navigation…
  scene/      # Node, Resource, Viewport, UI, 3D/2D nodes
  modules/    # gdscript, gridmap, …
  editor/     # todo o editor (ImGui não; custom + docks)
  platform/   # backends OS
  drivers/    # GL/Vulkan/etc.
```

**O que Ori deve copiar:** mental model **Resource + Node-ish hierarchy + servers finos**.  
**O que Ori não deve copiar no G1:** reimplementar RenderingServer completo; Godot editor inteiro.

### 1.4 Síntese filosófica para Ori

```text
                    ┌─────────────────────────────┐
                    │  Studio (Tauri ou ImGui)    │
                    │  edita Resource + Scene     │
                    └─────────────┬───────────────┘
                                  │ JSON / .orl / assets
                    ┌─────────────▼───────────────┐
                    │  Runtime Ori (AOT)          │
                    │  player loop + game.*       │
                    │  raylib L0 = “display/audio │
                    │  server” fino               │
                    └─────────────────────────────┘
```

| Camada | Papel | Analogia |
|--------|--------|----------|
| **Resource** | entity.json, scene, mesh path, map | Godot Resource / Unity asset |
| **Entity type** | pasta + script | GM Object / Unity Prefab |
| **Instance** | na cena | GM Instance / GO instance |
| **Player loop** | ordem de frame | Unity PlayerLoop (simplificado) |
| **Events** | create/step/draw/alarm/collision | GM events |
| **Servers** | draw3d, audio, physics modules | Godot servers (mínimos) |
| **Editor** | só edita dados; Play = processo nativo | Godot/Unity (não web-game) |

**Anti-padrões (proibidos no plano):**

- Duplicar o jogo em Three.js “completo”.  
- Feature list Unity sem loop de autor.  
- ECS por default.  
- “Done” em docs sem Play real.  
- Dois motores (2D e 3D) paralelos sem unificar Y-up.

---

## 2. Inventário do que ainda precisa (backlog completo)

> **Catálogo de produto completo** (features Auk adaptadas + pipeline retro +  
> protótipos Megabonk/Tenchu, com fase Ori e status):  
> **[`FEATURE-CATALOG.md`](FEATURE-CATALOG.md)**  
> Fonte bruta: `game-engine-full/docs/AUK GAME ENGINE.md`.

Legenda de status:  
`missing` · `partial` · `prototype` · `ok-for-g1` · `post-g1`

### 2.1 Runtime / game loop

| ID | Item | Status | Inspiração | Critério de pronto |
|----|------|--------|------------|--------------------|
| R-LOOP | Player loop documentado + API única (`game.loop` ou app) | partial | Unity | sample só usa essa API |
| R-WIN | Windowed main invariant | **ok-for-g1** | todos | smoke exige `run_window` |
| R-EVT | Dispatch Create/Step/Destroy por instance | missing | GM | entity scripts rodam no Play sem main manual |
| R-ALM | Alarms tick no loop | partial | GM | alarm dispara callback |
| R-COL | Collision events (enter/stay) | missing | GM/Unity | on_collision no script |
| R-FIX | Fixed step opcional (physics) | missing | Unity FixedUpdate | post-g1 se 2D physics |
| R-DT | dt estável + pause | partial | todos | pause flag no loop |
| R-SCE | Load/reload scene API | partial | Godot/Unity | `load_scene` + clear |

### 2.2 Câmera

| ID | Item | Status | Critério |
|----|------|--------|----------|
| CAM-ED | Editor Orbit/Top estável | prototype | sem crash resize |
| CAM-RT | Runtime perspective + look-at | partial | API estável |
| CAM-FLW | Follow player (3rd person / top) | missing | sample usa follow |
| CAM-ZON | Bounds / clamp | partial (módulos) | post-g1 |
| CAM-SHK | Shake | partial | post-g1 |

### 2.3 Colisão / física

| ID | Item | Status | Critério |
|----|------|--------|----------|
| COL-DBG | Debug draw sempre toggle | partial | C no sample |
| COL-SP | Sphere vs AABB solids | partial | player anda no chão |
| COL-LAY | Layers/masks usáveis | partial | filter no resolve |
| COL-BOX2 | Integração box2d no sample 2D | partial package | se G1 for 2D |
| COL-JOLT | Jolt 3D | partial package | post-g1 |
| COL-TRIG | Triggers | missing | post-g1 |

### 2.4 Level / mapas (eixo 3D)

| ID | Item | Status | Critério |
|----|------|--------|----------|
| LVL-BRU | Brushes place/edit | prototype | |
| LVL-SUB | Subtract AABB multi-piece | prototype | match runtime solids |
| LVL-EXT | **Extrude** / push-pull face | missing | Hammer-lite |
| LVL-CSG | Boolean real / clip plane | missing | post-g1 (ou mid se level-first) |
| LVL-TER | Terrain paint | prototype | |
| LVL-MAT | Albedo/textura em cubos | partial | |
| LVL-MESH | Mesh brush + scale/rot runtime | partial | cowboy ok no Play |
| LVL-CHU | Chunks streaming | stub | post-g1 |

### 2.5 2D / tilemap / sprite

| ID | Item | Status | Critério |
|----|------|--------|----------|
| TM-LOAD | Tiled/LDtk load | ok runtime | |
| TM-SOL | solid_tiles → colliders | partial | |
| TM-PAI | **Paint tilemap no editor** | missing | se G1 for 2D |
| SPR-DRW | Draw sprite/sheet | partial | |
| SPR-ANI | Aseprite player no loop | partial | |
| SPR-PRV | Preview PNG no Studio | missing | |

### 2.6 Animação / timeline

| ID | Item | Status | Critério |
|----|------|--------|----------|
| AN-SKEL | Skeletal / anim_player | partial | post-g1 se não for core do 1º jogo |
| AN-TL | Timeline editor Studio | missing | post-g1 |
| AN-CLIP | Clip list + switch | partial | |

### 2.7 Assets / conteúdo

| ID | Item | Status | Critério |
|----|------|--------|----------|
| AS-GLB | Preview GLB estável no viewport | prototype | cowboy sempre visível |
| AS-OBJ | OBJ preview | partial | |
| AS-PNG | Image preview Content | missing | |
| AS-PATH | Paths relativos estáveis no Play | partial | cwd package root |
| AS-IMP | Import wizard (copy to assets/) | missing | |
| AS-SHD | Shaders sample paths | partial | |

### 2.8 Scripting / nodes / IDE

| ID | Item | Status | Critério |
|----|------|--------|----------|
| SC-MON | Monaco + save | prototype | sempre abre script da entity |
| SC-LSP | ori-lsp hover/complete | partial | |
| SC-OGS | ogs interpreter | partial | wired no dispatch |
| SC-NOD | Nodes UI | prototype | |
| SC-HOT | Hot reload script (recompile Play) | missing | nice-to-have G1 |

### 2.9 Studio shell / estabilidade

| ID | Item | Status | Critério |
|----|------|--------|----------|
| ST-VP | Viewport lifecycle robusto | prototype | resize/reopen ok |
| ST-DOC | Docks sem perder painéis | partial | |
| ST-PLY | Play AOT + logs | partial | **ok-for-g1** path |
| ST-PKG | Package path-deps | partial | |
| ST-IMGUI | **Migração ImGui nativo** | decision | ver §4 |
| ST-HIE | Hierarchy parent/rename | partial | |
| ST-UND | Undo cena | partial | |

### 2.10 Áudio / UI in-game / save

| ID | Item | Status | Critério |
|----|------|--------|----------|
| AU-PLAY | SFX/music one-shot no sample | partial | 1 sfx G1 |
| UI-MENU | Menu pause simples | missing | texto + tecla |
| SV-SLOT | Save/load | partial modules | post-g1 |

### 2.11 Empacote / DX

| ID | Item | Status | Critério |
|----|------|--------|----------|
| DX-SMOKE | smoke check+compile+run_window | partial | expandir |
| DX-DOC | guides “primeiro jogo” | missing | |
| DX-AOT | AOT default documentado | ok | |
| DX-SO | .so para JIT nativo | missing | opcional |

---

## 3. Fases de amadurecimento (roadmap executável)

### Fase A — **G0 Destravar** (fundação) · meta: “não mente”

| Slice | Entrega | Done quando |
|-------|---------|-------------|
| A1 | Play windowed + AOT + packaging | sample abre janela (já) |
| A2 | Smoke compile + assert run_window | CI/local smoke |
| A3 | PRODUCT-REALITY + este plano | docs |
| A4 | Viewport error banner (GLB fail) | user vê erro, não tela preta muda |

### Fase B — **G1 Primeiro jogo** · meta: vertical slice jogável

Escolher **uma** trilha (recomendado **B3D** se cowboy/level já existem; **B2D** se preferir GM clássico).

#### Trilha B3D (recomendada)

| Slice | Entrega |
|-------|----------|
| B3D-1 | Player loop único + sample só usa ele |
| B3D-2 | Entity dispatch Create/Step no sample (mesmo que só player) |
| B3D-3 | Camera follow runtime |
| B3D-4 | Colliders solids + debug confiável |
| B3D-5 | Mesh player GLB no Play estável |
| B3D-6 | Viewport: GLB instance sempre preview |
| B3D-7 | Level: extrude face **ou** subtract confiável (um bem feito) |
| B3D-8 | Guia “faça o engine_test virar mini-jogo” |

#### Trilha B2D (alternativa)

| Slice | Entrega |
|-------|----------|
| B2D-1… | loop + dispatch |
| B2D-2 | tilemap solids + paint mínimo |
| B2D-3 | sprite sheet + anim |
| B2D-4 | camera top follow |
| B2D-5 | colisão rect |

**Gate G1:** checklist da §0 completo na trilha escolhida.

### Fase C — **Ferramentas de autor sérias** · meta: “edito sem sofrer”

| Slice | Entrega |
|-------|----------|
| C1 | Viewport hardening (dispose, resize, multi-select sem crash) |
| C2 | Content: PNG + GLB preview + copy path/import |
| C3 | Monaco/LSP estáveis; Script IDE não some |
| C4 | Nodes ogs → dispatch real |
| C5 | Decisão final Tauri vs **ImGui Studio** (spike 1–2 semanas) |
| C6 | Se ImGui: Hierarchy + Inspector + Viewport raylib + Play |

### Fase D — **Profundidade de conteúdo** · meta: “mapa e anim de verdade”

| Slice | Entrega |
|-------|----------|
| D1 | Extrude + clip + boolean melhor (estudar QBSP/TrenchBroom patterns) |
| D2 | Tilemap editor completo **ou** timeline anim (o que o 1º jogo exigir) |
| D3 | Materials/texturas no level |
| D4 | Audio buses no sample |

### Fase E — **Engine “profissional” ampla** · meta: pós-primeiro-jogo

| Slice | Entrega |
|-------|----------|
| E1 | Physics 2D/3D integrados no loop |
| E2 | Multi-scene, transitions |
| E3 | Particles/UI toolkit |
| E4 | Net (já há ori-enet) em sample |
| E5 | Packaging multi-OS |
| E6 | Performance / profiling |

---

## 4. Studio: Tauri+React vs Dear ImGui

### 4.1 Situação atual

- **Studio:** Tauri 2 + React + Three (viewport) + dockview.  
- **Runtime jogo:** Ori + raylib.  
- **ori-imgui:** já existe backend raylib + demos/tools (ECO).

### 4.2 Por que ImGui entra na conversa

| Dor atual | ImGui ajuda? |
|-----------|--------------|
| Viewport Three ≠ raylib (duas verdades) | **Sim** — um render path |
| Fragilidade webview / lifecycle | **Sim** — UI imediata no processo nativo |
| Tools in-game + editor | **Sim** — mesmo stack |
| Monaco / tipografia / browser assets | **Não** — pior; precisa ColorTextEdit / file browser próprio |
| Pixel-perfect “Unity pretty” | **Não** — ImGui é **funcional**, não “bonito” |

### 4.3 Decisão recomendada (alinhada ao teu objetivo)

> **Funcional > bonito.**  
> **Curto prazo (Fases A–B):** manter Tauri, mas **estabilizar** e não investir em polish visual.  
> **Médio prazo (Fase C):** **spike ImGui Studio nativo** (raylib + ori-imgui + docks):  
> - Hierarchy, Inspector, Console, Play  
> - Viewport = **mesma** cena raylib (ou FBO)  
> Se o spike fechar G1 mais fácil que consertar Three, **adota ImGui como Studio principal**.

### 4.4 Critérios do spike ImGui (go/no-go)

| Critério | Go se |
|----------|--------|
| Abrir scene JSON + desenhar solids/player | < 2 semanas |
| Edit transform + save | funciona |
| Play in-process ou subprocess estável | funciona |
| Texto de script | aceitável (externo editor OK no G1) |
| Menos crashes que Three viewport | sim |

**Não** reescrever Monaco no ImGui no G1 — abrir `.orl` no editor externo ou manter janela mínima.

---

## 5. Protocolo de estudo (Godot source + docs)

### 5.1 Local do clone

```text
/home/raillen/Documentos/Projetos/study-godot/   # git clone --depth 1 godotengine/godot
```

(Se o clone ainda estiver em andamento, reexecutar o comando no README de estudo.)

### 5.2 Pastas para ler primeiro (não compilar o editor inteiro no dia 1)

| Pasta | Por quê |
|-------|---------|
| `scene/main/` | Node, Viewport, SceneTree — modelo mental de cena |
| `scene/resources/` | Resource / load |
| `servers/rendering/` | o que **não** copiar cedo (complexidade) |
| `servers/physics_*` | interface de physics server |
| `editor/scene/` | como o editor observa a scene tree |
| `modules/gdscript/` | analogia “linguagem de jogo” (Ori é a nossa) |
| `core/object/` | Object/ClassDB (inspiração de registro de tipos) |

### 5.3 Perguntas de estudo (template)

Para cada sistema (câmera, colisão, scene load, anim):

1. Qual o **contrato público** (API do usuário)?  
2. Onde vive o **estado** (node vs server)?  
3. Qual a **ordem no frame**?  
4. O que é **Resource** serializável vs runtime-only?  
5. Qual o **mínimo** que Ori precisa para G1?

### 5.4 Unity / GM sem source

- Unity: [Execution Order](https://docs.unity3d.com/Manual/ExecutionOrder.html), Manual de Physics/Camera/Prefab.  
- GameMaker: manual de Events, Rooms, Instances (documentação YoYo).  
- Não precisamos do source da Unity/GM para G1.

### 5.5 Licença

Godot: **MIT** — estudo e reimplementação de **ideias** ok; **não** copiar código literal sem respeito à licença/atribuição. Preferir **reexpressar** em Ori.

---

## 6. Mapa “inspiração → módulo Ori”

| Sistema | GM | Unity | Godot | Ori (alvo G1) | Ori (hoje) |
|---------|----|-------|-------|---------------|------------|
| Loop | Events | PlayerLoop | SceneTree process | `game.app` + loop doc | app.run_window |
| Tipo | Object | Prefab/MB | PackedScene/script | Entity pasta | sim |
| Instância | Instance | GO | Node instance | scene instance | sim |
| Script | GML | C# | GDScript | `.orl` | sim |
| Alarme | Alarm | Invoke/Coroutine | Timer | ogame_alarms | partial |
| Colisão | Collision event | OnCollision | signals/body | resolve + event | partial |
| Câmera | view | Camera | Camera3D | camera3d + follow | partial |
| Mapa 2D | tile | Tilemap | TileMap | tiled + paint | load only |
| Mapa 3D | — | ProBuilder | CSG/GridMap | brushes + extrude | AABB CSG |
| Anim | sprite | Animator | AnimationPlayer | aseprite/anim_player | runtime |
| Editor | IDE | Editor | Editor | Studio / ImGui | Tauri fragile |
| Render | runtime | pipelines | RenderingServer | raylib | ok L0 |

---

## 7. Ordem de implementação recomendada (fila única)

```text
A1–A4 destravar ──► B3D-1 loop ──► B3D-2 dispatch ──► B3D-3 camera follow
       ──► B3D-4 colliders ──► B3D-5/6 mesh Play+viewport ──► B3D-7 level tool
       ──► B3D-8 guia ──► GATE G1
       ──► C1–C4 ferramentas ──► C5 spike ImGui ──► (go?) C6 ImGui Studio
       ──► D* profundidade ──► E* amplitude
```

**Regra de ouro:** se um item não desbloqueia G1 ou estabilidade do autor, **não entra na sprint**.

---

## 8. Critérios de maturidade (níveis)

| Nível | Nome | Significado |
|-------|------|-------------|
| 0 | Lab | Código existe; usuário sofre |
| 1 | Destravado | Play abre; sample roda |
| 2 | **G1** | Primeiro jogo possível |
| 3 | Autorável | Editor confiável para iterar |
| 4 | Conteúdo rico | Level/anim/tile profundos |
| 5 | Profissional amplo | Física/net/multi-OS (anos) |

**Hoje: ~0.5–1** (Play sample destravando).  
**Meta imediata: 2 (G1).**

---

## 9. O que **não** fazer (ainda)

- Reescrever tudo em ECS/DOTS.  
- Polimento visual Unity HDRP.  
- Marketplace / store.  
- Multi-OS packaging agressivo.  
- Feature parity com Godot 4 inteiro.  
- Dois Studios em paralelo sem decisão (Tauri + ImGui full-time).

---

## 10. Ligações

| Doc | Papel |
|-----|--------|
| [`PRODUCT-REALITY.md`](PRODUCT-REALITY.md) | Honestidade do estado atual |
| [`STUDIO-PRODUCT-DECISIONS.md`](STUDIO-PRODUCT-DECISIONS.md) | Decisões de vocabulário/schema |
| [`PLAY-PACKAGING.md`](PLAY-PACKAGING.md) | AOT / native_libs |
| [`ROADMAP-GAME-ECO.md`](ROADMAP-GAME-ECO.md) | Pacotes ori-* |
| [`STUDIO-ECO-HOOKS.md`](STUDIO-ECO-HOOKS.md) | Superfícies Studio↔runtime |
| `study-godot/` | Código Godot para estudo |

---

## 11. Próxima ação concreta (após este plano)

1. Confirmar trilha **B3D** (default) ou **B2D**.  
2. Implementar **B3D-1 + B3D-2** (loop + dispatch player).  
3. **B3D-3** camera follow no sample.  
4. Só então extrude/viewport polish.

_Este plano é vivo: atualizar status das tabelas a cada slice fechado com evidência de Play/smoke._
