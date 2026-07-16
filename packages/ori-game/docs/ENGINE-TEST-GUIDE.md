# Guia curto — `engine_test` (primeiro loop G1)

> **Slice B8** · Sample: `ori-game/assets/sample_project/`  
> Specs: `docs/specs/runtime-*`, `level-brushes`, `platform-packaging`

## O que é

Um **projeto de jogo mínimo** que prova o gate G1:

- carrega `scenes/engine_test.scene.json` (chão + parede + player + **enemy** + goal)
- spawna **`game.world`** a partir do catalog de entities
- loop: input → script → **patrol path_follow** → AI chase → combat → move → collision → cam → draw
- mesh cowboy **ou** esfera fallback se o GLB falhar; inimigo = esfera vermelha
- **C** debug colliders / aggro; **R** restart; win grava coin em save slot

## Como rodar

```bash
cd /home/raillen/Documentos/Projetos/game-engine-full/ori-game/assets/sample_project
export ORI_GAME_ROOT=../..
export ORI_USE_AOT=1
export ORI_USE_SYSTEM_LINKER=1
export LIBRARY_PATH="$ORI_GAME_ROOT/lib/x86_64-unknown-linux-gnu${LIBRARY_PATH:+:$LIBRARY_PATH}"
export LD_LIBRARY_PATH="$ORI_GAME_ROOT/lib/x86_64-unknown-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

ori run .
# ou: ori compile . -o /tmp/engine_test && /tmp/engine_test
```

Smoke de package (check + compile, sem janela longa):

```bash
cd /home/raillen/Documentos/Projetos/game-engine-full
export ORI_GAME_ROOT="$PWD/ori-game" ORI_USE_AOT=1 ORI_USE_SYSTEM_LINKER=1
./ori-game/tools/smoke_sample_play.sh
```

## Studio (S5.1 product entry)

```bash
cd /home/raillen/Documentos/Projetos/game-engine-full
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_AOT=1
./ori-studio/tools/smoke.sh    # product smoke → studio_shell
./ori-studio/tools/run.sh      # interactive Studio
# equivalente direto:
#   ori-imgui/demos/studio_shell/tools/smoke.sh
```

| Ação | Como |
|------|------|
| Place instance | Hierarchy **Add instance** / Toolbar Place / palette **place** |
| Play | F5 ou Toolbar Play (WASD; HUD se gameplay on) |
| Pick 3D | LMB no viewport free zone |
| Palette | Ctrl+P / Ctrl+K |
| Save | Ctrl+S (path da cena atual) |
| Open project | File → Open project folder (exige `ori.pkg.toml` ou `project.ogame.toml`) |

## Controles

| Tecla | Ação |
|-------|------|
| WASD | move no chão (X/Z) |
| Space / Shift | sobe / desce (Y) |
| Setas | offset da câmera follow |
| V / 2–4 | stack multi-cam |
| T | timeline drive cam (R5.2) |
| C | debug colliders / aggro |
| F | fog postfx |
| R | soft restart (cena + coins do save) |
| Esc | sair (raylib default) |

## Objetivo (win)

Esfera **amarela** (`inst_goal` em z≈36). Chegue perto → **YOU WIN** + 1 coin + save  
(`/tmp/ori_engine_test_g1.json`). Inimigo vermelho patrulha e chase; toque = dano.  
**R** reinicia sem fechar a janela.

## Segunda cena (`room1`)

Multi-scene / Studio: `scenes/room1.scene.json` — chão + parede + pilar, player com mesh, enemy, goal marker.  
No shell: painel **Multi-scene** ou Tools → Next multi-scene.

## Arquivos que importam

| Path | Papel |
|------|--------|
| `main.orl` | entry `run_window` + systems |
| `scenes/engine_test.scene.json` | brushes + player + enemy + goal |
| `scenes/room1.scene.json` | 2ª sala (switch multi-scene) |
| `entities/player/` | tipo + vars (mesh, radius, speed) |
| `assets/models/brazilian-cowboy.glb` | mesh do player |

## O que você deve ver

1. Janela 1280×720  
2. Chão verde + parede cinza em +Z  
3. Cowboy (ou esfera verde se mesh falhar) + **inimigo vermelho** em patrulha  
4. Câmera seguindo o player (V/T para stack/drive)  
5. Ao andar em +Z, a **parede bloqueia** (colisão)  
6. HUD com **fps≈**, HP/score, coins/save  
7. Com **C**, wires brancos + aggro  

## Se algo falhar

| Sintoma | Checagem |
|---------|----------|
| Abre e fecha na hora | Rebuild com libs reais (não stub-only); ver split tick (sem TickResult dual) |
| Sem janela | `main()` chama `app.run_window`? |
| Link / .so | use `ORI_USE_AOT=1` |
| Sem mesh | HUD “mesh FALLBACK”; path `assets/models/...` relativo ao package |
| Atravessa parede | `collision_system` + solids do level; layer 1 no player e brushes |

## Próximo depois do G1

Studio ImGui: `docs/specs/studio-shell/` (Fase C).  
Catálogo de features: `docs/planning/FEATURE-CATALOG.md`.
