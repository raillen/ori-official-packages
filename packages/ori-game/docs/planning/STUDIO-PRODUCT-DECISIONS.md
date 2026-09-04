# Ori Game Studio — decisões de produto (fechadas)

> **Data:** 2026-07-14 · **Revisão visão:** 2026-07-15  
> **Status:** decisões de vocabulário/schema mantidas · **host Studio reorientado**  
> **Runtime:** ori-game + raylib · **World:** ECS-lite / híbrido (EntityId + components + systems + script opcional)  
> **Studio destino:** **ori-imgui** + raylib · **Studio agora:** Tauri+React (transição)  
> **Código:** IDE **externa** + ori-lsp  
> **Honestidade:** `PRODUCT-REALITY.md` · **Implementação:** `PLANO-IMPLEMENTACAO-STUDIO.md` (2026-07-15)  
> **Maturação:** `PLANO-AMADURECIMENTO-ENGINE.md` · **ImGui ports:** `ROADMAP-GAME-ECO.md` + `IMGUI-EXTENSIONS-RANKING.md`

---

## 1. Vocabulário de UI (GameMaker-simple)

| UI (português/EN curto) | Significado | Disco (automático; dev não edita à mão no dia 1) |
|-------------------------|-------------|---------------------------------------------------|
| **Entity** (tipo) | Tipo reutilizável, como Object do GM | `entities/<id>/` |
| **Instance** | Colocação na cena | entrada em `scenes/*.scene.json` |
| **Script** | Código acoplado 1:1 à Entity | `entities/<id>/<id>.orl` |
| **Scene** | Room / nível de instances | `scenes/<name>.scene.json` |
| **Notes** | Anotações do projeto | `notes/**` |

**Proibido na UI v1:** “kind”, “kind.ron”, “criar kind manualmente”.  
Internamente o runtime pode usar o termo `entity_type` / `kind` no schema JSON.

### Fluxo do autor (obrigatório ser trivial)

1. **New Entity** → wizard (nome + template Player/Enemy/Solid/Empty).  
2. Studio grava pasta + script stub + meta.  
3. **Place Entity** na cena.  
4. Duplo clique → Monaco no `.orl` (e, em P1, aba Nodes).  
5. Inspector: vars da Entity + overrides da Instance.

Não existe “script por instance” (estilo Godot node script).

---

## 2. Decisões fechadas (2026-07-14)

| # | Tema | Decisão |
|---|------|---------|
| 1 | Notas | **Rich text Tiptap** (não Markdown-only no v1) |
| 2 | Nós (programação visual) | **P1** — depois do loop Entity + Scene + Play |
| 3 | Brushes / CSG / chunks | **Fase 2** (track 3D level; não bloqueia Studio 2D) |
| 4 | Nome do “Object” GM | **Entity** (tipo); Instance = colocação |

### 2.1 Wizard / i18n / ordem (diálogo seguinte)

| # | Tema | Decisão |
|---|------|---------|
| D1 | Templates New Entity | **Player, Enemy, Solid, Empty** (Spawner/Pickup = P1) |
| D2 | Idioma UI Studio | **en canônico** (chaves/código) + **pt-BR** (locale; i18n desde scaffold) |
| D3 | Ordem de build | **R→S**: runtime/schema no `ori-game` → depois app Tauri |
| D4 | Detalhes de id/pasta | Defaults abaixo (slug, `entity.json`, stub events) |

Outras decisões já aceitas neste fio:

| Tema | Decisão |
|------|---------|
| Shell IDE | **Tauri 2 + React**, não ImGui |
| Tools in-game | **ori-imgui** embed (opcional), não shell |
| Cena | JSON próprio versionado |
| Script canônico | `.orl`; nós = layout + lower/lift |
| Play | `ori check` + `ori run` / compile no projeto do user |
| Old engine | Aproveitar **contratos** (cena/entity/API mental); **não** egui/wgpu/Rapier no v1 |
| **Espaço / 2D** | **3D-first (2026-07-14+).** Uma cena 3D; “2D” = workflow **top view** (câmera ortho), **não** segundo motor. |

### 2.2 Espaço 3D-first (decisão de produto)

| Item | Valor |
|------|--------|
| Mundo | **Y-up** (raylib / Three alinhados no Studio) |
| Instance | `x`, `y`, `z` — **x/z = chão (plano horizontal)**, **y = altura** |
| Legado `x`/`y` 2D | Arquivos antigos só com `x,y`: `x`→x, **`y` legado → `z` (chão)**, `y` altura = 0 |
| Viewport Studio | **Um** viewport 3D (Three.js); modos **Top** (ortho) e **Orbit** (persp) |
| Brushes / CSG | Continuam Fase 2, mas no **mesmo** editor 3D (não track paralelo “só 2D”) |
| Tilemap “2D clássico” | P1 opcional / especialização, **não** o eixo do produto |

---

## 3. Layout de projeto (v1)

```text
my_game/
  ori.pkg.toml                 -- pacote Ori
  project.ogame.toml           -- manifesto Studio (active scene, paths)
  entities/
    player/
      entity.json              -- meta: id, name, events, vars defaults, tags
      player.orl               -- script 1:1 (Create/Step/…)
      player.ogn               -- layout de nós (P1; opcional)
    slime/
      entity.json
      slime.orl
  scenes/
    room1.scene.json
  notes/
    index.json                 -- índice / ordem do painel
    design.note.json           -- doc Tiptap (ProseMirror JSON) ou .html export
    entities/
      player.note.json
    scenes/
      room1.note.json
  assets/
    ...
  src/                         -- módulos Ori extras (opcional)
```

Nomes de pasta `entities/` (não `kinds/`) na UI e no default do scaffold.

---

## 4. Schema mínimo (**congelado** P0)

### Convenções (D4)

| Item | Valor |
|------|--------|
| Slug `id` | de `Player` → `player` (ascii lowercase, `[a-z0-9_]`) |
| Pasta | `entities/<id>/` |
| Meta | `entities/<id>/entity.json` |
| Script | `entities/<id>/<id>.orl` (campo `script` relativo à pasta) |
| Instance id | `inst_<entity>_<n>` no Place; uuid só se colidir |
| Events no stub | Empty/Player/Enemy: `on_create`, `on_step`, `on_destroy`; Solid: `on_create` opcional + tag solid |
| UI strings | chaves i18n en + ficheiros `pt-BR` (ex. `New Entity` / `Nova Entity`) |

### Templates wizard (D1)

| Template | `id` exemplo | tags | vars default | events no stub |
|----------|--------------|------|--------------|----------------|
| Player | `player` | `["player"]` | `move_speed: float = 5` | create, step, destroy |
| Enemy | `enemy` | `["enemy"]` | `hp: float = 3` | create, step, destroy |
| Solid | `solid` | `["solid"]` | — | create (vazio) |
| Empty | (user slug) | `[]` | — | create, step, destroy |

### `entities/<id>/entity.json`

```json
{
  "format_version": 1,
  "id": "player",
  "name": "Player",
  "script": "player.orl",
  "tags": ["player"],
  "events": ["create", "step", "destroy"],
  "vars": [
    { "name": "move_speed", "type": "float", "default": 5.0 }
  ],
  "parent": null
}
```

### `scenes/<name>.scene.json`

```json
{
  "format_version": 1,
  "name": "room1",
  "instances": [
    {
      "id": "inst_player_01",
      "entity": "player",
      "x": 64.0,
      "y": 0.0,
      "z": 128.0,
      "var_overrides": { "move_speed": 8.0 }
    }
  ]
}
```

- `x`, `z`: posição no chão · `y`: altura (default `0`).  
- Compat: se `z` ausente e só `x`/`y` legados → Studio/runtime leem `z = y_legado`, `y = 0`.

### Notes (Tiptap)

- Conteúdo: **JSON do documento Tiptap/ProseMirror** (portável, diffável).  
- Metadados: `title`, `links[]` para `entity:` / `scene:` / paths.  
- Arquivos em `notes/`; painel lista + editor.  
- Locale do **chrome** do Studio: en + pt-BR; conteúdo das notas = o que o autor escrever.

---

## 5. Fases de implementação (atualizado)

### Fase R — Runtime first (**R→S**, agora)

| Slice | Entrega | Repo |
|-------|---------|------|
| **R0** | Schema docs + sample `entities/` + `scenes/` no repo (fixtures) | ori-game | **done** `assets/sample_project/` |
| **R1** | Parse `entity.json` + `scene.json`; helpers tag/event/instance | ori-game | **done** `game/ogame.orl` + `tests/test_ogame.orl` |
| **R2** | Catalog + `spawn_scene` + var merge + `spawn_into_engine` | ori-game | **done** |
| **R3** | Demo/smoke headless sample project | ori-game | **done** `ogame_scene_smoke.orl` |

Só depois disso o Studio grava o mesmo formato.

### Fase S — Studio P0 (2D GM-like)

| Slice | Entrega |
|-------|---------|
| S0 | Scaffold Tauri + docks + i18n en/pt-BR + open project | **done** (`ori-game-studio` repo) |
| S0b | **New Project** scaffold (pasta + manifest + scene) | **done** |
| S1 | **New Entity** wizard (4 templates) + tree + Place Instance | **done** |
| S2 | Scene JSON load + Inspector vars/overrides + viewport | **done** |
| S3 | Monaco + save/load `.orl` + diagnostics via `ori check` | **done** |
| S3b | LanguageClient bridge ↔ `ori-lsp` (hover/complete/diagnostics) | **done** |
| S4 | **Notes** panel (Tiptap) + `notes/*.note.json` | **done** |
| S5 | Play (`ori check` / `ori run`) + logs | **done** |
| S6 | Undo/redo editor + command palette | **done** |

### Fase V3D — Viewport / espaço unificado (3D-first)

| Slice | Entrega |
|-------|---------|
| **V3D0** | Schema `x,y,z` + parse runtime + viewport Three (Top/Orbit) + Place/Inspector z | **done** |
| **V3D1** | Tools Select/Place/Move · click-to-place · gizmo X/Y/Z · save on release | **done** |
| **V3D2** | Preferências viewport (Top/Orbit, grid, axes, FOV) + paths ori/ori-lsp | **done** |
| **V3D3+** | Brushes / CSG AABB / chunks (Studio + `game.ogame_level`) | **done** (MVP) |

### Fase P1 — Studio (sobre o espaço 3D)

| Slice | Entrega |
|-------|---------|
| N1 | **Nodes** (@xyflow) + lower/lift subset → `.orl` | **done** (deepen: set_var/if/wait/alarm/call, then/else, undo, Ctrl+S) |
| **N2** | Events Create/Step stubs + **alarms** (`game.ogame_alarms`) | **done** (MVP API) |
| N3 | Tilemap / anim browser + meta timeline + runtime load hints | **done** (browser MVP; full editors later) |

### Fase 2 — Level tools (absorvida em V3D3+)

| Slice | Entrega | Nota |
|-------|---------|------|
| M1–M3 | Brushes / CSG / chunks | Mesmo track V3D; não “segundo Studio 2D” |
| Level residual | Player vars from scene, layers, albedo path, mesh scale | **done** (sample `main` + Inspector) |

### Packaging / Play / ECO hooks

| Item | Doc | Status |
|------|-----|--------|
| Play package root + native env | [`PLAY-PACKAGING.md`](PLAY-PACKAGING.md) | **done** |
| Smoke sample | `tools/smoke_sample_play.sh` | **done** |
| Studio ↔ ECO surfaces | [`STUDIO-ECO-HOOKS.md`](STUDIO-ECO-HOOKS.md) | **done** |

---

## 6. Stack Camada A — acréscimos vs stack base

Base: `~/.grok/skills/ori-game-studio/references/stack-tauri-react.md`

| Pacote | Uso | Fase |
|--------|-----|------|
| **`@tiptap/react` + starter-kit** (+ extensions link/placeholder) | Painel Notes rich text | P0 (S4) |
| `@tiptap/extension-link` | Links `entity:` / `scene:` | P0 |
| `elkjs` | Auto-layout nós | P1 |
| `use-debounce` | Sync nós→código | P1 |
| `three` | Viewport cena 3D (Top/Orbit) | V3D0 |
| `react-zoom-pan-pinch` | *(legado)* só se precisar 2D UI; preferir Three | — |

Não adicionar: Excalidraw, Electron, MUI.
**ImGui-as-Studio é o destino** (ori-imgui); não confundir com “não usar ImGui”.
Não adicionar polish Three.js como caminho longo.

---


## 6b. Visão de arquitetura (2026-07-15) — obrigatória

| Tema | Decisão |
|------|---------|
| Modelo de jogo | **ECS-lite / híbrido**: components = structs; systems = funções; Script component = events GM-like |
| ECS hard (flecs/DOTS) | **Não** default; package opcional só com profiling |
| Studio canônico | **ori-imgui** + raylib (Hierarchy, Inspector, Viewport, Play) |
| Studio Tauri+React+Three | **Transição** — manutenção e bugfix; sem roadmap de features novas cosméticas |
| Preview 3D destino | **raylib** (mesma verdade do Play), não Three |
| Edição de `.orl` | **IDE externa** (+ ori-lsp); Monaco/Tiptap = legados / opcionais |
| Gate de produto | **G1** (primeiro jogo) em `PLANO-AMADURECIMENTO-ENGINE.md` / `PLANO-IMPLEMENTACAO-STUDIO.md` |
| Fila de implementação Studio | **`PLANO-IMPLEMENTACAO-STUDIO.md`** supersede fases R* antigas centradas só em Tauri |

---

## 7. Anti-padrões (produto)

- Pedir ao user que escreva `kind.ron` / paths de script à mão no onboarding.  
- Script por instance.  
- Herança OOP de Entity (parent opcional só como defaults + event chain, depois).  
- Misturar Save Project com save de jogo.  
- Brushes no P0 “simples como GM 2D”.  
- Nós sem `.orl` canônico.

---

## 8. Próximo passo de implementação

**Contrato de diálogo fechado.** Progresso:

1. **R0–R3** em `ori-game`: **done** (`game.ogame`, sample_project, smoke).  
2. **S0** em `ori-game-studio`: **done**.  
3. **S1** New Entity + Place: **done**.  
4. **S2** scene + Inspector vars: **done**.  
5. **S3** Monaco + `ori check`: **done**.  
6. **S4** Notes (Tiptap): **done**.  
7. **S5** Play + Console: **done**.  
8. **3D-first** + **V3D0–V3D2** viewport: **done**.  
9. **S6** Undo/redo + command palette: **done**.  
10. **S3b** ori-lsp bridge (hover/complete/diagnostics): **done**.  
11. **Shell Unity-like:** Hierarchy esquerda · Project em baixo · Script IDE janela: **done**.  
12. **V3D3** level brushes/CSG/chunks · **N2** alarms · **dock** · Hierarchy++ · Play++ · Notes links: **done** (MVP).  
13. **N1 Nodes** (2026-07-15): dock **Nodes**, IR `.ogn`, lower/lift Print/Comment/Pass/SetVar/If/Wait/Alarm/Call ↔ `ogs:*` no `.orl`; undo + then/else.  
14. **N3 Content** browser: maps/anim meta, frame scrubber, Ori load hints.  
15. **Play packaging** + **ECO hooks** docs; sample `main` loads player vars from scene JSON.  
16. **ogs:* runtime** (2026-07-15): `game.ogame_script` interpreter + sample host in `main.orl`; Studio lower emits markers + `log.info` for print.  
17. **N3 solids:** `game.tiled.solid_tiles` / `first_layer_solid_tiles` for collision boxes from TMJ.  
18. **Maturation plan (2026-07-15):** [`PLANO-AMADURECIMENTO-ENGINE.md`](PLANO-AMADURECIMENTO-ENGINE.md) — backlog completo, estudo GM/Unity/Godot, fases até G1 (primeiro jogo), decisão Tauri vs ImGui.  
19. **Engine study:** [`STUDY-ENGINES.md`](STUDY-ENGINES.md) + clone Godot `Documentos/Projetos/study-godot`.  
20. Next implement: **B3D-1/2** (player loop + entity dispatch) no sample — não feature list.

_Este arquivo é a fonte de verdade das decisões de 2026-07-14; atualizar ao mudar produto._  
_Maturidade e fila de implementação: **PLANO-AMADURECIMENTO-ENGINE.md** (prevalece sobre “done” cosmético)._

21. **Visão ECS-lite + Studio ImGui (2026-07-15):** plano de implementação reescrito —  
    `PLANO-IMPLEMENTACAO-STUDIO.md` fases **A→F** (B = G1 World/systems; C = ori-imgui shell).  
22. Próximo código: **B1–B3** (`game.world` + systems + script dispatch no sample).

_Implementação executável: **PLANO-IMPLEMENTACAO-STUDIO.md** (visão 2026-07-15)._
