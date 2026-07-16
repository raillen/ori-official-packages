# 01 — Arquitetura

## Diagrama de peças

```text
┌────────────────────────────────────────────────────────────┐
│                 Ori Game Editor (processo nativo)         │
│  egui docks: Hierarchy | Scene | Inspector | Project | Log │
│  tools: brush, face, displacement, decal, entity place     │
└────────────────────────────┬───────────────────────────────┘
                             │ lê/escreve
                             ▼
                    project/ (mapa, assets, src/*.orl)
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
        Scene/Map IR    assets/*      gameplay Ori
              │              │              │
              └──────────────┴──────┬───────┘
                                    ▼
┌────────────────────────────────────────────────────────────┐
│              Ori Game Runtime (mesmo binário ou player)   │
│  wgpu render │ Rapier │ audio │ AI │ particles │ decals    │
│  systems loop (fixed + frame) │ cameras │ input            │
└────────────────────────────┬───────────────────────────────┘
                             │ API estável
                             ▼
┌────────────────────────────────────────────────────────────┐
│  Ori gameplay (compilado via ori-lang → link / dylib)    │
│  player.orl, guards.orl, waves.orl, …                      │
└────────────────────────────────────────────────────────────┘
```

## Fronteiras

| Camada | Responsabilidade | Não faz |
|--------|------------------|---------|
| **Editor** | Autoria, gizmos, bake navmesh, Play | Lógica de missão do jogo |
| **Runtime** | Simular, desenhar, áudio, física | Parser da linguagem Ori |
| **Ori gameplay** | Regras do jogo | Rasterizer / solver de física |
| **ori-lang** | Check, compile, diagnostics | Conhecer brushes ou Rapier |

## Crates previstos (scaffold futuro)

Prefixo sugerido: `ori-engine-*`.

| Crate | Papel |
|-------|--------|
| `ori-engine-core` | Math helpers, IDs, time, events, result types |
| `ori-engine-scene` | Entity, components, prefabs, scene IR |
| `ori-engine-map` | Brushes, faces, displacements, CSG-lite, serialize |
| `ori-engine-render` | wgpu, materials, postFX, presets retro |
| `ori-engine-physics` | Rapier backend, character controller, layers |
| `ori-engine-anim` | Clips, skeleton, events |
| `ori-engine-vfx` | Particles, billboards, decals, pools |
| `ori-engine-audio` | Playback, buses, 3D atten |
| `ori-engine-ai` | Blackboard, FSM, perception, goals (depois) |
| `ori-engine-nav` | Navmesh bake + path query |
| `ori-engine-script` | Host API + bridge para binários Ori |
| `ori-engine-app` | Loop, input, cameras, wiring de systems |
| `ori-engine-editor` | egui UI, tools de mapa, project browser |
| `ori-engine-cli` / bin | `ori-engine` editor e `ori-engine-player` |

Nomes finais podem ajustar; a **separação de responsabilidades** é o contrato.

## Loop de runtime

```text
process input
fixed_update (N×):
  physics step
  Ori fixed hooks (se houver)
  AI / gameplay systems
frame_update:
  animation
  cameras
  particles / decals TTL
  Ori frame hooks
render:
  world → low-res target → postFX → present
  (editor) egui overlay
```

## Dados canônicos

| Artefato | Formato sugerido | Dono |
|----------|------------------|------|
| Projeto | `ori.engine.toml` | editor |
| Mapa | `maps/*.map.ron` (brushes) | map crate |
| Cena | `scenes/*.scene.ron` (entities/instances) | scene crate |
| Prefabs | `prefabs/*` (M1) | scene |
| Gameplay | `src/**/*.orl` + `ori.pkg.toml` | ori-lang |
| Navmesh bake | `maps/*.nav.bin` | nav |

## Play

1. Validar projeto (paths, mapa).
2. `ori check` no pacote de gameplay (**depende de ori-lang**).
3. Compilar/linkar gameplay contra `ori_engine_*`.
4. Carregar mapa + assets + spawn world.
5. Entrar no loop (editor embedded ou player).

Hot-reload de dylib Ori é **fase posterior**, não bloqueia M0.

## Chunks (futuro open world)

- Mapa lógico dividido em **chunks** carregáveis.
- Brushes e entities têm `chunk_id` ou bounds.
- Streaming: load/unload com hysteresis de câmera.
- v0 pode ser **single chunk** (arena / missão).

## Segurança e robustez

- Paths de projeto canonicalizados (sem path traversal).
- Fail closed no load de mapa corrompido.
- Budgets: partículas, decals, AI agents (ver `reference/budgets.md`).
