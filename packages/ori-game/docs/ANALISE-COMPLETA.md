# Análise completa — ori-game (2026-07-10)

> Estado (2026-07-10, fases A–E): typecheck limpo; Linux com `libraylib` stub headless;
> `game.app`, engine real, particles update/draw, save JSON, camera, testes e `tools/smoke_linux.sh`.
> Próximo tema de produto: **editor de mapa** (discussão).

---

## 1. Resumo executivo

| Dimensão | Status | Nota |
|----------|--------|------|
| **Sintaxe Ori moderna** | ✅ Feito | `pub`, `apply`, namespaces `game.*`/`raylib`, mutação por retorno |
| **`ori check` lib + demos + tests** | ✅ 45/45 | Nenhuma falha de typecheck |
| **`ori compile` / link real** | ❌ Linux | `libraylib.a` é **placeholder**; só Windows tem `raylib.lib` real |
| **Loop de jogo de alto nível** | ❌ | Docs prometem `run_game`; **não existe** no código |
| **ECS de verdade** | ⚠️ Parcial | Há `Engine`/`Entity`/`System`, mas builtins de gravidade/movimento são **no-ops** |
| **Documentação** | ❌ Desatualizada | README / getting-started / api.md ainda em `ori.game.*`, `do() =>`, defaults de campo |
| **Persistência (save)** | ⚠️ Stub | Checkpoints em memória; `save_game`/`load_game` placeholders |
| **Partículas** | ⚠️ Incompleto | `emit` existe; faltam `update` / `draw` / lerp de vida |
| **Camera** | ⚠️ Mínima | offset/target/zoom; sem shake, limits, lerp de follow |
| **Pacote / native_libs** | ⚠️ Frágil | `native_libs = ["raylib"]` + lib Linux ausente → demos com raylib **não linkam** |
| **Cobertura de testes** | ⚠️ Rasa | Só shapes + engine básico; zero testes de physics/tilemap/combat/AI |

**Conclusão:** o repositório está **sintaticamente moderno e typecheck-limpo**, mas **ainda não é um motor de jogo instalável/jogável** no Ubuntu sem providenciar Raylib e fechar lacunas de runtime (loop, sistemas, I/O, docs).

---

## 2. Mapa da arquitetura atual

```
ori-game/
├── raylib.orl              # FFI cru (window, draw, input, audio)
├── color.orl / shape.orl / collision.orl
├── game/
│   ├── engine.orl          # Engine + Entity + System (parcial)
│   ├── input / draw / camera / timer / audio
│   ├── particles / physics / tilemap / dialogue
│   ├── inventory / save / state_machine
│   ├── entity_collision.orl
│   └── mechanics/          # jumps, dash, movement, combat, ai, …
├── examples/               # 15 demos (check OK; compile depende de raylib)
├── tests/                  # 2 testes de unidade leves
└── lib/<triple>/           # native raylib (Linux/macOS = placeholder)
```

### Camadas

| Camada | Módulos | Papel |
|--------|---------|--------|
| **L0 FFI** | `raylib` | `extern c` raylib |
| **L1 wrappers** | `input`, `draw`, `audio`, `camera` | Tipagem + helpers sobre FFI |
| **L2 core** | `engine`, `color`, `shape`, `collision`, `timer` | Modelo de jogo e utilitários |
| **L3 systems** | `physics`, `tilemap`, `particles`, `dialogue`, `inventory`, `save`, `state_machine` | Subsystems |
| **L4 mechanics** | `mechanics/*` | Gameplay plugável (jump, dash, AI, …) |
| **L5 demos** | `examples/*` | Programas de demonstração |

Namespaces reais: `game.*`, `raylib` — **não** `ori.game.*` (colide com classificação de stdlib `ori.*`).

---

## 3. Inventário por módulo (honestidade técnica)

### 3.1 Sólidos (úteis e coerentes)

| Módulo | Avaliação |
|--------|-----------|
| `color` | Completo o bastante (constantes + `from_rgb`/`from_hex`) |
| `shape` | Structs geométricos básicos |
| `collision` | Bom conjunto (AABB, círculo, linha, SAT parcial) |
| `input` | Bom mapeamento de teclas/mouse via raylib |
| `draw` | Wrappers de primitivas + texto |
| `timer` | create/tick/reset com `loops` (nome seguro; `repeat` é keyword) |
| `jumps` / `movement` / `dash` / `combat` | APIs return-style modernas, usáveis em demos |
| `dialogue` | Fluxo completo typewriter + choices (sem I/O de arquivo) |
| `inventory` | create/add/count/use após expansão recente |
| `state_machine` | map-based, callbacks nomeados |
| `tilemap` | get/set tile + resolve_collision simples |
| `physics` | world + integrate + bounds; `set_body`/`resolve_pair` |

### 3.2 Incompletos ou enganosos

| Módulo | Gap |
|--------|-----|
| **`engine`** | `gravity_system` / `movement_system` **não fazem nada**; sem `enable_system`/`disable_system`/`cleanup_dead`/`set_entity`; `spawn_entity` não aceita callbacks (só noops); `tick` não roda `update_fn` por entidade |
| **`particles`** | Sem `update_particles` / `draw_particles`; emit determinístico (sem random real); demo só “spawn visual” |
| **`camera`** | Sem shake, limites, deadzone, smooth follow; docs mentem |
| **`save`** | `save_game` sempre `true`; `load_game` sempre `none` |
| **`audio`** | Depende de handles raylib; sem asset pipeline documentado |
| **`behavior_tree`** | Estrutura OK, mas `sequence_status` é frágil; sem integração com `Entity` |
| **`ai`** | Patrol/chase/flee/wander OK; sem obstacle avoidance / pathfinding |
| **`grab` / `ropes` / `vehicles` / `platforms`** | Lógica isolada; pouco exercitada por demos dedicados |
| **`entity_collision`** | Mínimo (AABB fixo por half-size) |
| **`raylib`** | Só bindings; sem módulo `game.app` / `run_game` |

### 3.3 Documentação vs código (divergências críticas)

Docs atuais (`README`, `docs/getting-started.md`, `docs/api.md`) ainda ensinam:

- `import ori.game.*` → **errado** (use `game.*`)
- `game.run_game(...)` → **não existe**
- `do() =>` closures / field defaults → **sintaxe inválida** na Ori atual
- `spawn_entity(..., update_fn = ...)` → API real é só `(eng, pos) -> Engine`
- `enable_system` / `cleanup_dead` / camera shake → **não implementados**
- `ori-game` vs package name real **`ori_game`**

---

## 4. Pipeline de build / distribuição

### O que funciona hoje

```bash
ori check <qualquer .orl do pacote>   # 100%
```

### O que quebra no Ubuntu

```bash
ori compile examples/hello_game.orl
# ld: não encontra .../lib/x86_64-unknown-linux-gnu/libraylib.a
```

Causa: `libraylib.a.placeholder` (texto), não a static lib real.  
Windows MSVC tem `raylib.lib` binário; Linux/macOS não.

### Pacote Ori

```toml
name = "ori_game"
entry = "raylib.orl"
native_libs = ["raylib"]
```

Implicações:

1. Entry aponta só para FFI; não há “prelude” de jogo.
2. `native_libs` exige artefato em `lib/<triple>/libraylib.a` no link.
3. Nome do pacote é `ori_game` (underscore) — docs dizem `ori-game`.

---

## 5. Exemplos e testes

### Exemplos (15) — `check` OK

| Demo | Exercita | Roda de verdade? |
|------|----------|------------------|
| hello_game, platformer, space_shooter, … | raylib loop + módulos game | **Não**, sem raylib linkado |
| simple_game, game | lógica / FFI local | simple_game evita pacote; game tem extern próprio |
| window_test / wrapper_test | FFI mínimo | depende de raylib |

### Testes (2)

- `test_shapes` — struct color/shape + `ori.test`
- `test_engine` — create/spawn/timer/systems length  

**Faltam:** physics step, collision cases, inventory add/stack, tilemap solid, jump coyote, dialogue advance, save checkpoint.

---

## 6. Lacunas de design (Ori × game engine)

Estas não são “bugs de sintaxe”; são limitações do modelo atual:

1. **Params imutáveis** → toda mutação de `Entity`/`Engine` é copy-return; listas de entidades exigem rebuild ou `lists.set` cuidadoso.
2. **Sem closures capturantes** → callbacks de state machine / BT / dialogue precisam de funções nomeadas de módulo.
3. **FFI raylib** → cada wrapper redeclares `extern` ou depende de link global; `camera` redeclara `BeginMode2D` em vez de importar `raylib`.
4. **Sem ECS real** → Entity é struct monólito com function pointers, não componentes.
5. **JSON/FS save** → stdlib tem `ori.json` / `ori.fs`, mas save ainda não os usa.

---

## 7. Roadmap de implementação (prioridade)

### Fase A — “Dá para compilar e rodar um demo no Linux” (P0)

1. **Obter `libraylib.a` real** para `x86_64-unknown-linux-gnu`  
   - script `tools/fetch_or_build_raylib.sh` (cmake/raylib release)  
   - documentar `apt install libraylib-dev` **ou** vendor static  
2. Validar `ori compile examples/hello_game.orl` + execução  
3. Smoke script `tools/smoke_linux.sh` (check + 1 compile+run)

### Fase B — “Motor mínimo honesto” (P0/P1)

4. **`game/app.orl`**: `run_window(title, w, h, update, draw)`  
   - encapsula InitWindow / loop / ClearBackground / CloseWindow  
   - demos param de reescrever o boilerplate raylib  
5. **Engine de verdade**  
   - `spawn_entity` com callbacks opcionais (via overloads manuais / builders)  
   - `tick` chama `update_fn` por entity viva  
   - `cleanup_dead`  
   - `enable_system` / `disable_system`  
   - gravity/movement systems que **atualizam** pos/vel  
6. **Particles**: `update_particles`, `draw_particles` (via `game.draw`)

### Fase C — “Docs e contrato público” (P1)

7. Reescrever `README.md`, `docs/getting-started.md`, `docs/api.md` para API real  
8. Completar `.oridoc` faltantes **ou** gerar de `ori doc`  
9. Alinhar nome do pacote na doc (`ori_game`) e exemplos de `ori.pkg.toml`

### Fase D — “Sistemas de jogo” (P1/P2)

10. **Save real** com `ori.json` + `ori.fs` (slots em `~/.ori/saves/...` ou path explícito)  
11. Camera: follow smooth, shake, limits  
12. Physics: body-body no `step` (multi-body), sizes por body  
13. Tilemap: chunks / WorldMap (removido na modernização; reintroduzir se necessário)

### Fase E — “Qualidade” (P2)

14. Testes L2: physics, collision matrix, inventory stack, dialogue, jumps  
15. Um demo “vertical slice” (platformer) que **compila e roda** no CI Linux  
16. CONTRIBUTING: como rebuildar raylib; política de native_libs

---

## 8. Critérios de “ori-game 1.0 usável”

Checklist mínimo:

- [ ] `libraylib` real no triple Linux (e doc macOS)
- [ ] `ori compile examples/hello_game.orl && ./hello_game` no Ubuntu
- [ ] `game.app.run_window` (ou equivalente) usado por ≥3 demos
- [ ] Engine: tick aplica movement/gravity reais; cleanup de mortos
- [ ] Particles update+draw
- [ ] Save/load JSON funcional em 1 slot
- [ ] Docs 100% alinhadas aos namespaces e APIs
- [ ] CI: `ori check` em todos + smoke compile de 1 demo

---

## 9. Ordem recomendada para a próxima sessão de código

1. Script de Raylib Linux + smoke compile `hello_game`  
2. `game/app.orl` (loop de janela)  
3. Completar `engine` (tick entity + systems reais + cleanup)  
4. Particles update/draw  
5. Docs (getting-started + README + api.md)  
6. Save JSON  
7. Testes + CI smoke  

---

## 10. Métricas atuais (snapshot)

| Métrica | Valor |
|---------|--------|
| Arquivos `.orl` | 45 |
| `ori check` pass | 45 |
| Módulos de biblioteca | 28 |
| Exemplos | 15 |
| Testes | 2 |
| `.oridoc` presentes | ~18 / 28 módulos |
| LOC lib (aprox.) | ~3.5k |
| Triple Linux raylib | placeholder |
| Package version | 0.1.0 |
| Ori target | 0.2.0 |

---

*Gerado na análise de 2026-07-10. Atualizar este doc quando fechar cada fase do roadmap.*
