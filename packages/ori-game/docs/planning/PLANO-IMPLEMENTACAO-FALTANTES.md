# Plano — faltantes (narrativa pós A–F)

> **Atualizado:** 2026-07-15  
> **Status:** A–F MVP done · fila ativa = fases **G→M**  
> **Workspace:** `game-engine-full/`

---

## Fonte de verdade (checkboxes)

| Papel | Doc |
|-------|-----|
| **Checkboxes + fases A→M** | **[`PLANO-IMPLEMENTACAO-STUDIO.md`](PLANO-IMPLEMENTACAO-STUDIO.md) §4** ← **marcar `[x]` só lá** |
| Este arquivo | Narrativa, mapa de prioridade, sprints, “done quando” |
| Continuidade | `DEV-HANDOFF.md` §3 |
| Como implementar | `docs/specs/<módulo>/` |

**Regra para agentes:** ao fechar S1.1 / V2.1 / L3 / … → `[x]` em **STUDIO §4**, não aqui · atualizar `DEV-HANDOFF` · `./scripts/sync_planning.sh`.

---

## 0. O que “A–F done” não significa

- Engine paridade Unity/Godot  
- App Studio de produto (ainda `demos/studio_shell`)  
- Pipeline de conteúdo profundo ou gameplay shipável  

### Fora da fila (salvo pedido)

| Item | Motivo |
|------|--------|
| flecs / DOTS | só se medido; default ECS-lite |
| Polish Three no Tauri | destino = ori-imgui |
| IDE de código embutida | IDE externa |
| Marketplace / feature dump | YAGNI |

---

## 1. Mapa rápido (prioridade → fase no STUDIO)

```text
  P0  Studio usável (S*)     → Fase G
  P1  Viewport + assets (V*) → Fase H
  P2  Pipeline profundo      → Fase I  (L* 3D  OU  T* 2D)
  P3  Runtime de jogo (R*)   → Fase J
  P4  Look/VFX/áudio (X*)    → Fase K
  P5  Gameplay systems       → Fase L  (IDs GP* — não confundir com gate G1)
  P6  Plataforma (P*)        → Fase M
```

**Recomendação default:** G(S*) → H(V*) → **I(L*)** → J(R*) → resto.

Pós-F já no produto base (STUDIO **PF1–PF7**): save brushes, sample root, room1+win, box2d/jolt dynamics, dock Godot-like, Spectrum, viewport free zone.

---

## 2. Fase G — Studio usável (P0)

> Specs: `studio-shell`, `studio-project-io`, `studio-play`  
> Código: `ori-imgui/demos/studio_shell/`

| Bloco | IDs (STUDIO) | Done quando |
|-------|--------------|-------------|
| Place instance | S1.1–S1.3 | cria player/enemy sem editar JSON |
| Inspector completo | S2.1–S2.4 | rot/scale/overrides → save round-trip |
| Project IO | S3.1–S3.5 | open sample → edit → save → reopen |
| Toolbar | S4.1–S4.4 | Play/tool/snap/debug sem menus profundos |
| App Studio | S5.1–S5.3 | um comando documentado sobe o produto |
| UX commands | S6.1–S6.2 | atalho palette + comandos place/save/play |

**Próximo default:** **S1.1** — Hierarchy “Add instance”.

---

## 3. Fase H — Viewport + assets (P1)

> Specs: `studio-viewport`, `runtime-assets`, `runtime-camera`

| Bloco | IDs | Done quando |
|-------|-----|-------------|
| Viewport estável | V1.1–V1.4 | resize/reopen ok; grid; frame F |
| Pick + gizmo | V2.1–V2.3 | click seleciona; ImGuizmo na câmera editor |
| Preview assets | V3.1–V3.3 | GLB/PNG + tree Project |
| Script open | V4.1 | feedback + stub `.orl` |

Base já feita: free zone + orbit (PF7 / V1.0), open path (C5).

---

## 4. Fase I — Pipeline de conteúdo (P2)

Escolher **um** eixo (recomendação: **3D level**). Não metade L e metade T no mesmo sprint.

### Trilha L — Level 3D

| ID | Escopo |
|----|--------|
| L1 | Select brush na Hierarchy |
| L2 | Extrude/clip com preview no viewport |
| L3 | Subtract = mesmo `collect_solids` runtime |
| L4 | Clip plane (pós L2) |
| L5 | Displacements (depois L1–L3) |
| L6 | CSG boolean/merge/axis-BSP — **done** (AABB, not Hammer poly) |

**Done trilha L:** editar brush no Studio → Play no mesmo solid.

### Trilha T — Tilemap 2D

| ID | Escopo |
|----|--------|
| T1 | Save/load tilemap no projeto |
| T2 | solid rects → colliders Play 2D |
| T3 | Sample 2D mínimo |
| T4 | Viewport 2D / dual mode |

MVP paint/fill já existe (E3).

---

## 5. Fase J — Runtime de jogo (P3)

> Specs: `runtime-*`

| Bloco | IDs | Foco |
|-------|-----|------|
| Character controller | R1.1–R1.3 | capsule/sphere + sample |
| Script / events | R2.1–R2.3 | destroy, alarms, **play_freshness done** |
| Collision / queries | R3.1–R3.4 | layers nomeados, raycast, triggers, box |
| Physics ECO no jogo | R4.1–R4.3 | PhysicsHost + **jolt.world_character done** |
| Câmera | R5.1–R5.3 | **multi-cam stack done**; timeline drive; docs editor |
| World model | R6.1–R6.3 | parent/child, despawn, multi-scene runtime |
| Net | R7.1–R7.2 | host+peer; replicate opcional |

Já fechados no STUDIO: ScriptHost base, sphere–AABB, PhysicsHost+smokes, follow_offset, flecs **não** (R6.4).

---

## 6. Fase K — Look, VFX, áudio (P4)

| ID | Escopo |
|----|--------|
| X1 | Materials lit + textures (sample + Studio cwd) |
| X2 | PostFX presets toggable |
| X3 | Particles author + Play |
| X4 | Decals pool (após trilha L) |
| X5 | Audio buses + 1 som sample |
| X6 | Anim clips + timeline → mesh/sprite |

---

## 7. Fase L — Gameplay systems (P5)

> Specs: `gameplay-*` · IDs no STUDIO = **GP\*** (gate de produto continua **G1**)

| ID | Sistema | Done quando |
|----|---------|-------------|
| GP1 | Combat hitbox/hurtbox + damage | inimigo morre no sample |
| GP2 | Health + death + i-frames | player sobrevive hit |
| GP3 | AI FSM (patrol/chase) | 1 enemy reage |
| GP4 | Nav / path (recast) | enemy navega obstacles |
| GP5 | Waves / spawner | 2 waves |
| GP6 | Inventory + save | pick + save slot |
| GP7 | HUD in-game (≠ Studio) | HP/score on screen |
| GP8 | Stealth (opcional prot B) | noise/visibility |

Protótipos: **A** waves+combat+HUD · **B** stealth+AI.  
Só após G–J úteis (ou protótipo explícito).

---

## 8. Fase M — Plataforma e ship (P6)

| ID | Escopo |
|----|--------|
| P1 | `smoke_eco_windows.ps1` em host MSVC real |
| P2 | Export/package jogo (bin + assets + native_libs) |
| P3 | macOS smoke (quando host) |
| P4 | CI smoke Linux shell + sample |

Linux AOT + scripts Win já existem (STUDIO).

---

## 9. Dívidas técnicas (IDs DT* no STUDIO)

| ID | Dívida |
|----|--------|
| DT1 | Shaders PBR path depende de cwd |
| DT2 | Clash `*.world` → `*.dynamics` (**feito**) |
| DT3 | Split return World+Host AOT (**política ok**) |
| DT4 | `begin_window` + `end_window` (**shell ok**) |
| DT5 | Refresh PRODUCT-REALITY vs tilemap/timeline/dock |

---

## 10. Critérios de fatia done

1. Código com **clean-code** + AGENTS  
2. Teste **ou** smoke executável  
3. Checkbox **`[x]` em `PLANO-IMPLEMENTACAO-STUDIO.md` §4**  
4. Visível no Studio **ou** sample  
5. Spec/CHANGELOG se contrato user-facing mudou  
6. `DEV-HANDOFF` §3 se mudou o “próximo” da fila  

---

## 11. Ordem sugerida de sprints

| Sprint | IDs | Resultado |
|--------|-----|-----------|
| **1** | S1, S2, S4 | place + inspector + toolbar |
| **2** | S3, V3.3 | open project real + tree assets |
| **3** | V1, V2 | pick + gizmo + viewport estável |
| **4** | L1–L3 | level author = runtime solids |
| **5** | R1, R2, R3.1–R3.3 | controller + events + triggers |
| **6** | S5, P2 | app Studio + export mínimo |
| **7+** | X*, GP*, R4/R7, P1/P3 | polish e gameplay |

Checklist vivo com o mesmo mapa: **STUDIO §4** (Fases G–M).

---

## 12. Ligações

| Doc | Papel |
|-----|--------|
| [PLANO-IMPLEMENTACAO-STUDIO.md](PLANO-IMPLEMENTACAO-STUDIO.md) | **Canônico A→M + checkboxes** |
| [PRODUCT-REALITY.md](PRODUCT-REALITY.md) | bar “sério o bastante” |
| [FEATURE-CATALOG.md](FEATURE-CATALOG.md) | inventário M0–M4 |
| [PLANO-AMADURECIMENTO-ENGINE.md](PLANO-AMADURECIMENTO-ENGINE.md) | backlog motor longo |
| [PHASE-OS.md](PHASE-OS.md) | multi-OS scripts |
| [`docs/specs/MODULES.md`](../../../docs/specs/MODULES.md) | mapa de módulos |
| `DEV-HANDOFF.md` (cluster root) | continuidade de sessão |

---

## 13. Próximo ID (default)

**Linux ultra-deep:** L6 CSG · R2.3 freshness · R4.3 Jolt character — **done**
**OS depois:** P1 Win · P3 macOS  

→ checkboxes em **`PLANO-IMPLEMENTACAO-STUDIO.md` §4** (deep L5/V3-GPU/R4.1/path_follow done 2026-07-15).

---

*Este arquivo não compete com o STUDIO: narrativa + sprints. Progresso = checkboxes no plano central.*
