# ori-game / Ori Game Studio — Project Context

> Runtime de jogo **3D-first** (Y-up) em Ori — `game.world` ECS-lite/híbrido (EntityId +
> components + systems + script opcional). Studio **destino**: `ori-imgui` + raylib
> (`ori-game-studio/` Tauri 2 + React é a transição em manutenção, não o destino).
> Visão completa: `AGENTS.md` do cluster (`../AGENTS.md`) e `docs/planning/PLANO-IMPLEMENTACAO-STUDIO.md`.

## Precedência

**Este `AGENTS.md` > skills globais > defaults.**

## Skills e agents — **obrigatório**

> Alinhado ao **`game-engine-full/AGENTS.md`** (cluster). Em conflito de escopo
> runtime: **este arquivo**; em política de skills do monorepo games: **cluster**.

### Sempre (qualquer tarefa neste repo)

| Skill | Obrigação |
|-------|-----------|
| **`clean-code`** | Todo código/refactor `.orl`/host: módulos, nomes, KISS, sem `utils` lixeira |
| **`nd-explain`** | Explicações, design, trade-offs, docs conceituais: TL;DR + exemplo + ≤3 opções + recomendação |
| **`living-docs`** | CHANGELOG / docs / specs se contrato ou UX mudar |
| **`ori-game-studio`** | Contexto de produto runtime + Studio |

### Por tipo de mudança

| Skill | Quando |
|-------|--------|
| **`ori-testing`** | Qualquer `.orl` runtime (check/compile/test/smoke) |
| **`rust`** + **`tauri-desktop`** | Shell Studio Tauri / crates |
| **`typescript`** + **`react-spa`** | Frontend React (legado) |
| **`semantic-web`** + **`premium-ui`** | UI Studio (a11y + visual) |
| **`c-secure`** | Stubs/FFI raylib C |
| **`check-work`** | Fechar slices não triviais |
| **`code-review`** / `/review` | Review estrutural |

### Orquestração (agents)

| Comando / agent | Uso |
|-----------------|-----|
| `/design` · `plan` | Features grandes (schema, graph, World) |
| `/execute-plan` | Plano multi-PR |
| `/implement` | Slice aprovado |
| `explore` | Explorar runtime/API |
| `/check-work` | Verificação final |
| `/review` | Review |

### Sob demanda

| Skill | Quando |
|-------|--------|
| `e2e-playwright` | E2E do Studio |
| `redesign-ui` / `minimalist-ui` | Só se pedido |
| `lang-compiled` / `compiler-dev` | Só se a tarefa exigir **ori-lang** |
| `full-output` | Dumps longos sem truncar |

**Não usar** neste produto: `multi-tenant-saas`, `saas-security` (exceto auth futura irrelevante ao v1), `lang-interpreted`, stacks Rails/Laravel/PHP.

### Checklist rápido

- [ ] `clean-code` aplicado no diff  
- [ ] Resposta conceitual em formato **nd-explain**  
- [ ] `ori test` / smoke se tocou runtime  
- [ ] Spec em `docs/specs/` se decisão de implementação mudou  


## Convenções

| Aspecto | Convenção |
|---------|-----------|
| Identificadores | inglês |
| Comentários decisão (Rust/TS do Studio) | pt-BR |
| Comentários / código `.orl` | inglês (alinhado ao runtime Ori) |
| Docs produto | pt-BR |
| Namespaces jogo | `game.*`, `raylib` — **nunca** `ori.game.*` |
| Package | `ori_game` |
| Entity (tipo) | pasta `entities/<id>/` — `entity.json` + `<id>.orl` (não `src/scripts/<kind>.orl`) |
| Cena | JSON próprio |
| Nós | sync bidirecional com código; não dual GM |
| UI default | light (Unity-like); dark secundário |

## Layout do monorepo (atual)

```
ori-game/                 # runtime .orl + raylib + tools (este repo)
ori-game-studio/          # Tauri 2 + React — transição/manutenção (existe; não é o destino)
  src-tauri/
  src/                    # React
ori-imgui/demos/studio_shell/  # Studio destino (ImGui + raylib) — GO em C6-IMGUI-SHELL-GO.md
```

Runtime e planos canônicos ficam em `ori-game/`; espelho em `docs/planning/` na raiz do cluster.

## Build & smoke (runtime)

```bash
./tools/setup_raylib_linux.sh --stub   # ou raylib real
export ORI_BIN=../ori-lang/target/debug/ori
export ORI_STDLIB_ROOT=../ori-lang/stdlib
export ORI_USE_SYSTEM_LINKER=1
export ORI_SYSTEM_LINKER=/usr/bin/ld
./tools/smoke_linux.sh
```

## Planos

- Visão: `docs/planning/ORI-GAME-STUDIO-VISAO.md`
- Implementação: `docs/planning/PLANO-IMPLEMENTACAO-STUDIO.md`
- Análise runtime: `docs/ANALISE-COMPLETA.md`

## Pitfalls

1. `for i in 0..(n-1)` com `n==0` → bounds error; guardar `if n > 0`.  
2. `lists.set` em lista de struct aninhada pode corromper — preferir rebuild.  
3. Function pointers em structs → falha codegen nativo.  
4. Linux sem `libraylib.a` → stub ou setup script.  
5. Docs antigas com `ori.game` / `run_game` / `do() =>` estão obsoletas.
