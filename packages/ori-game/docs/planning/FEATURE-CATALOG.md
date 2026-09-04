# Catálogo de features — Ori Game Engine (produto)

> **Atualizado:** 2026-07-15  
> **Origem:** recorte de produto da Auk Game Engine (`docs/AUK GAME ENGINE.md` na raiz do cluster),  
> **adaptado** ao stack Ori atual (raylib L0, packages ECO, `game.world` ECS-lite, Studio ImGui destino).  
> **Não** reinstala Rapier/wgpu/egui/Auk9 — só **features, pipeline visual e protótipos-guia**.  
> **Ordem de implementação canônica:** [`PLANO-IMPLEMENTACAO-STUDIO.md`](PLANO-IMPLEMENTACAO-STUDIO.md) (A→M + checkboxes; G1 em B) ·  
> backlog fino: [`PLANO-AMADURECIMENTO-ENGINE.md`](PLANO-AMADURECIMENTO-ENGINE.md).

---

## 0. Como ler este catálogo

| Coluna / termo | Significado |
|----------------|-------------|
| **Fase Ori** | Onde entra no plano atual (G1 / C / D / E / F) |
| **Auk M\*** | Milestone legado (referência histórica) |
| **Ori hoje** | `ok` · `partial` · `missing` · `package` (existe no ECO, falta wiring) |
| **Aceito** | No roadmap de produto (não = “faça agora”) |

**Regra:** G1 primeiro. Linhas `pós-G1` / `E` / `F` **não** bloqueiam B5–B8.

### Mapa Auk M0–M4 → Ori A–F

| Auk | Ori (aproximado) | Meta |
|-----|------------------|------|
| M0 | **G1 (B\*)** + base A | Loop, world, colisão, mesh, Play |
| M1 | **G1 rest + C** | Editor mínimo, VFX base, audio, prefabs |
| M2 | **E** + gameplay support | AI, nav, stealth/horda, mapa extrude, inventário |
| M3 | **E–F** | Bake, timeline UI, water/snow, build multi-OS |
| M4 | **F** | Chunks, hot-reload, polish, vehicles |

### Stack (substituições vs Auk)

| Auk (legado) | Ori (canônico) |
|--------------|----------------|
| Rapier3D | `ori-jolt` / `ori-box2d` + resolve level (G1) |
| wgpu render | **raylib** + shaders `game.shader*` |
| egui docks | **ori-imgui** (destino); Tauri transição |
| Auk9 scripting | **Ori** + ogs + IDE externa |
| heightmap-world | **não** — brushes/displacements |

---

## 1. Osso (runtime)

| Feature | Descrição | Auk | Fase Ori | Ori hoje |
|---------|-----------|-----|----------|----------|
| Time | fixed dt, frame dt, pause, time_scale | M0 | G1 / pós | partial |
| Input actions | devices, rebind básico | M0–M1 | G1 / C | partial (`game.input`) |
| Scene / entities | spawn/despawn, components | M0 | **G1 (B1–B3)** | partial (`game.world`) |
| Transform hierarchy | parent/child, world/local | M0–M1 | pós-G1 / C | missing (flat World) |
| Tags | queries de gameplay | M0–M1 | G1+ | partial (primary_tag) |
| Prefabs | spawn reutilizável | M1 | C / E | partial (entity folder) |
| Map load | brushes → mesh/collider | M0 | **G1 (B7)** | partial (`ogame_level`) |
| Physics 3D/2D | dynamics, queries | M0 | F (G1 = resolve) | package jolt/box2d |
| **Raycast** | gameplay + editor pick + LOS | M0–M1 | C / E | partial (`ray3d`) |
| **Colliders** | box/sphere/capsule…; props no inspector | M0–M1 | **G1 (B5)** + C | partial sphere–AABB |
| Collision layers | World, Player, Enemy… | M0 | G1+ | partial (layer int) |
| Character controller | capsule + controller próprio | M0 | pós-G1 | missing (WASD sample) |
| Cameras | 3rd person + follow/orbit | M0–M1 | **G1 (B4)** | partial follow |
| Materials | unlit / lit simples; textures | M0–M1 | G1 / E | partial shaders |
| **Resolução interna livre** | render target W×H | M1 | E (look) | missing |
| **Presets retro** | PS1 / N64 / GC / None | M1 | E (look) | missing (knobs isolados) |
| PostFX | fog, dither, quantize, upsample | M1 | E | partial fog/shaders |
| **Sombras leves** | blob e/ou 1 shadow map low-res | M1–M3 | E–F | missing |
| **Light bake** | lightmap/vertex estáticos; sem GI RT | M3 | F | missing |
| Animation clips | play + events | M1–M2 | E | partial `anim_player` |
| Timeline keyframes (editor) | camadas | M3+ | E | missing |
| Combat | hitbox/hurtbox, damage | M1 | pós-G1 | partial `mechanics/combat` |
| Health | HP, death, i-frames | M1 | pós-G1 | partial |
| Pooling | projectiles, VFX, enemies | M1 | pós-G1 | missing formal |
| Triggers | volumes enter/exit | M1 | pós-G1 | missing |
| Audio 2D/3D | buses SFX/BGM | M1–M2 | E | partial `game.audio` |
| Events / bus | desacoplar sistemas | M1 | F | missing |
| Asset registry | paths, hot reload | M1 | C / E | partial |
| Ori host | hooks + API | M0+ | **sempre** | ok path |
| Project IO | open/save projeto | M0 | C | partial Tauri |
| Debug draw | colliders, AI, nav, rays | M0+ | **G1 (B5)** | partial sample |
| Debug/profile overlay | fps, draws | M0+ / M4 | C / F | partial; `ori-tracy` package |

---

## 2. Mapa e level design

| Feature | Descrição | Auk | Fase Ori | Ori hoje |
|---------|-----------|-----|----------|----------|
| Brushes convexos | caixas/prismas, grid snap | M0–M1 | **G1** + C | partial |
| Faces + materials/UVs | por face | M0–M1 | E | partial |
| Extrude face | puxar na normal | M1 | **E1** (TB-inspired) | missing |
| **Displacements** | faces subdivididas puxáveis (Source-lite) | M2–M3 | E | missing |
| CSG full | half-space / boolean real | depois | F / backlog | AABB multi-slab only |
| Entities de mapa | player_start, lights, triggers… | M0–M2 | G1 instances + E | partial instances |
| Chunks | load/unload | M4 | F | stub |
| **Não** heightmap-world | outdoor = brushes/displacements | — | **política** | alinhado |

Inspiração de **tools** de mapa: TrenchBroom (estudo) + §2 deste catálogo — não portar TB.

---

## 3. VFX, superfícies, água, clima

**Regra de ouro (herança Auk — manter):**

| Onde | Técnica |
|------|---------|
| **No ar** (chuva, fagulhas) | Billboards / partículas |
| **No solo** (pegada, sangue) | Decals (+ poucas partículas) |
| **Afundar / neve / areia** | Surface tag + **sink** (não billboard) |
| **Poças autoradas** | Decal e/ou mesh multi-layer |

| Feature | Auk | Fase Ori | Ori hoje |
|---------|-----|----------|----------|
| Billboards / particles | M1–M2 | pós-G1 / E | partial `game.particles` |
| Decals (pool + TTL) | M1 | E | missing formal |
| Surface tags | M2 | E | missing |
| Sink (neve/areia) | M2 | E | missing |
| Rain / snow leve | M2–M3 | E–F | missing |
| Poças A/B | M2–M3 | E–F | missing |
| Water W0–W1 | M2–M3 | F | missing |
| Skybox | M1 | E | partial draw3d |
| Day/night | M2 | F | missing |
| Wind | M4+ | opcional | — |

---

## 4. UI de jogo, inventário, save

| Feature | Auk | Fase Ori | Ori hoje |
|---------|-----|----------|----------|
| HUD data-driven | M1–M2 | pós-G1 | missing sample |
| Widgets (button, panel, slot) | M2–M3 | F / clay/raygui | packages `ori-clay`, `ori-raygui` |
| UI/GUI editor | M3–M4 | F | missing |
| Inventário 1ª classe | M2–M3 | F | partial `game.inventory` |
| Equip ↔ sockets | M2–M3 | F | missing |
| Save fácil | M2–M3 | F | partial `game.save` |
| Localization keys | M3+ | F | missing |

---

## 5. IA e navegação

| Feature | Auk | Fase Ori | Ori hoje |
|---------|-----|----------|----------|
| Blackboard | M2 | F | partial ai |
| Perception (vision, LOS raycast) | M2 | F | missing full |
| FSM | M2 | F | partial `state_machine` / ai |
| Navmesh / path | M2 | F | package `ori-recast` + `game.navmesh` |
| Controllers (Patrol, Chaser…) | M2 | F | partial |
| Crowd separation | M2 | F | missing |
| Goal selector | M3–M4 | F | missing |
| Brains default engine + script params | — | F | ogs/script |
| GOAP full | fora v0 | **fora** | — |

---

## 6. Editor (ferramentas Studio)

| Feature | Auk | Fase Ori | Ori hoje |
|---------|-----|----------|----------|
| Skin Unity-like (~80%) docks | M0+ | **C** (ImGui) | partial Tauri |
| Viewport 3D editável + gizmos | M0–M1 | **C** + **D** ImGuizmo | fragile Three |
| Orbit / pan / zoom | M0 | C | partial |
| Views ortográficas | M1 | C | partial Top |
| Frame selection / fly | M0–M1 | C | missing |
| Snap grid / superfície | M0+ | C / E | partial |
| Undo/redo mapa | M1 | C | partial |
| Multi-seleção | M1 | C | partial |
| Hierarchy reparent | M1 | C | partial |
| Pivot editável | M1–M2 | E | missing |
| Sockets nomeados | M2 | E | missing |
| Brush tools | M0–M2 | E1 | partial |
| Displacement tool | M2–M3 | E | missing |
| Play / Pause / Step | M1 | **G1** + C | Play AOT; step missing |
| Play isolation | M1–M2 | C | partial |
| Console diagnostics | M1 | C | partial |
| Navmesh bake button | M2 | F | missing UI |
| Light bake UI | M3 | F | missing |
| Animation timeline UI | M3+ | E | missing |
| UI layout editor | M3–M4 | F | missing |
| Código `.orl` | externo v0 | **política** | IDE externa |

Packages já portados para o editor nativo: `ori-imgui`, `ori-imguizmo`, `ori-imnodes`, `ori-implot`, `ori-nfd` — **usar na Fase D**, não re-portar.

---

## 7. Protótipos e gameplay support

| Feature | Auk | Fase Ori | Notas |
|---------|-----|----------|--------|
| Spawner / waves | M1–M2 | pós-G1 · prot. A | Megabonk |
| Progression / XP | M2 | prot. A | |
| Auto-attack default | M1 | prot. A | |
| Mission / objectives | M2 | prot. B | Tenchu |
| Stealth metrics | M2 | prot. B | |
| Crouch / noise | M2 | prot. B | |
| Interaction / use | M2 | pós-G1 | |
| Vehicles | M4+ | condicional | |
| Dialogue trees | depois | opcional | `game.dialogue` partial |
| Config data-driven | M1 | sempre | JSON/vars |
| Build Win+Linux | M3–M4 | F / Phase OS | Linux first |
| Hot-reload dylib | M3–M4 | F | AOT Play estável primeiro |

### 7.1 Protótipos-guia (herança Auk §10)

**Só após gate G1.** Escolher **um** para vertical slice de “jogo real”.

#### A — Megabonk-like

Arena, hordas, pooling, auto-attack, XP, VFX, câmera follow, death loop.

**Exige do catálogo:** pooling, combat/health, spawner, particles, camera follow, save leve.

#### B — Tenchu 2-like (PS1)

Mapa vertical brush, 3rd person stealth, guardas (FSM + visão + nav), mission win/lose, low-poly, noise/crouch.

**Exige do catálogo:** brushes sólidos, camera 3rd, AI+nav, raycast LOS, surface/noise, look retro opcional.

---

## 8. Pipeline look retro (produto)

### 8.1 Pipeline

```text
Meshes + materials
  → raster (unlit / gouraud / simple lit)     # raylib + shaders Ori
  → render target (resolução INTERNA livre)
  → post (fog, dither, quantize, grade)
  → upsample (nearest / crisp)
  → tela
```

### 8.2 Presets (opcionais)

| Preset | Típico se ativo |
|--------|-----------------|
| PS1 | nearest, vertex snap, dither forte |
| N64 | bilinear, fog, cores mais limpas |
| GC-like | multi-texture, menos banding, sombra simples |
| None | knobs manuais + res livre |

### 8.3 Iluminação v1 (alvo produto)

- Ambient + 1 directional + poucas lights dinâmicas  
- **Bake** em estáticos (brushes/props)  
- Dinâmicos: lit simples + blob — **sem** rebake por frame  
- Sem GI tempo real  

### 8.4 Budgets artísticos (guias)

| Alvo | Tris na view (ordem) |
|------|----------------------|
| PS1-like | ~1k–5k |
| N64-like | ~2k–10k |
| GC-like | ~10k–40k |

| VFX | Guia v0 |
|-----|---------|
| Partículas ativas | 500–2000 |
| Decals vivos | 128–256 |

**Fase Ori:** implementação séria do pipeline = **E (look)** após G1 e Studio mínimo.  
G1 pode usar lit simples atual (`pbr_lit` sample) sem presets.

---

## 9. Matriz resumo → fase Ori

| Bloco | G1 (B*) | C Studio | D ImGui ports | E Conteúdo | F Ampliar |
|-------|---------|----------|---------------|------------|-----------|
| World + script + cam + col + mesh | ● | | | | |
| Play AOT estável | ● | ● | | | |
| Hierarchy/Inspector/Viewport nativo | | ● | gizmos ● | | |
| Extrude / displacements / materials level | | | | ● | |
| Retro pipeline / postFX / bake | | | | ● | ● bake |
| Particles formal / decals / surface | | | | ● | |
| AI + nav wiring + stealth/horda | | | | | ● |
| Inventário / save / UI editor | | | | | ● |
| Chunks / multi-OS / net sample | | | | | ● |
| Protótipo A ou B | | | | após G1 | ● |

---

## 10. Explicitamente fora (v0 / v1 Ori alinhado a Auk)

- Multiplayer / netcode como requisito G1 (sample enet = depois)  
- Segunda linguagem de script  
- **Three.js como viewport principal / verdade do jogo**  
- Heightmap open-world como paradigma de mapa  
- Fluid sim / soft-body neve  
- GI tempo real / sombras cascaded AAA  
- Blueprint visual full (ogs subset ≠ Unreal BP)  
- Export macOS / mobile / consoles no G1  
- Asset store  
- Sequencer cinematográfico completo  
- flecs/DOTS como default  
- GPL como licença da engine  

---

## 11. Ligações

| Doc | Papel |
|------|--------|
| [`docs/AUK GAME ENGINE.md`](../../../docs/AUK%20GAME%20ENGINE.md) | Fonte bruta (recorte Auk) |
| [PLANO-IMPLEMENTACAO-STUDIO.md](PLANO-IMPLEMENTACAO-STUDIO.md) | Fila A→M + checkboxes / World / Studio |
| [PLANO-AMADURECIMENTO-ENGINE.md](PLANO-AMADURECIMENTO-ENGINE.md) | Backlog IDs R-*/LVL-* |
| [STUDY-ENGINES-DEEP.md](STUDY-ENGINES-DEEP.md) | Godot/TB/rbfx/Fyrox |
| [PRODUCT-REALITY.md](PRODUCT-REALITY.md) | Honestidade de status |
| [ROADMAP-GAME-ECO.md](ROADMAP-GAME-ECO.md) | Packages ECO |

---

*Catálogo vivo. Preferir atualizar **Ori hoje** e **Fase Ori** aqui quando fechar slices; não duplicar checklists longas no AUK bruto.*
