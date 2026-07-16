# Ori Game Studio — Visão (documento histórico)

> **REVISÃO 2026-07-15:** este arquivo permanece como **contexto histórico** de produto (GM-like, Entity, Tauri early).  
> A **visão operacional atual** está em:  
> - [`PLANO-IMPLEMENTACAO-STUDIO.md`](PLANO-IMPLEMENTACAO-STUDIO.md) — ECS-lite/híbrido, Studio **ori-imgui** destino, IDE externa, fases A–F / G1  
> - [`PLANO-AMADURECIMENTO-ENGINE.md`](PLANO-AMADURECIMENTO-ENGINE.md) — maturidade motor  
> - [`PRODUCT-REALITY.md`](PRODUCT-REALITY.md) — honestidade de “done”  
> Onde conflitar, **prevalecem** os três docs acima.

---

# Ori Game Studio — visão de produto

> **Status:** proposta de produto (2026-07-10)  
> **Não é:** só um tilemap editor  
> **É:** ambiente 2D completo, fácil como GameMaker, sem POO de herança, com código Ori integrado

---

## 1. A ideia em uma frase

Um **estúdio 2D** em que a pessoa abre o projeto, **vê a cena**, **escreve Ori** ao lado, **aperta Play** e o jogo roda — com a mesma legibilidade e honestidade da linguagem Ori, **sem** class inheritance, prefabs OOP nem “magic events” opacos.

---

## 2. O que se pega do GameMaker (e o que se rejeita)

### Manter (ergonomia)

| Ideia GameMaker | Tradução Ori Studio |
|-----------------|---------------------|
| Room / Room editor | **Cena** (scene) com camadas e objetos colocados |
| Object + events | **Entidade + scripts nomeados** (não métodos de classe) |
| Sprite / animation | **Sprite sheet + anim clip** (dados + funções) |
| Drag-and-drop + code | **Editor visual + painel de código Ori** (código é first-class) |
| F5 Play | **Run / Hot reload** do projeto atual |
| Asset browser | **Projeto = pastas + manifesto** (`ori.pkg.toml` + `assets/`) |
| Tile layers | **Camadas de tile** (uma parte da cena, não o produto inteiro) |

### Rejeitar (filosofia)

| GameMaker / engines OOP | Posição Ori |
|-------------------------|-------------|
| Herança de Object (`obj_enemy` extends `obj_parent`) | **Composição de dados** + **funções livres** + traits `apply` quando fizer sentido |
| “Events” mágicos escondidos (`Create`, `Step`, `Draw` como magic methods) | **Callbacks explícitos** e/ou **sistemas nomeados** no loop (`update`, `draw`, `on_collide`) — visíveis no código |
| GML como scripting fraco | **Ori completo** (tipos, `result`/`optional`, módulos) |
| Room como dono de tudo | **Cena = dados**; **lógica = módulos `.orl`** |
| Black-box runtime | Runtime e assets **legíveis e auditáveis** |

---

## 3. Filosofia de design (contrato com o usuário)

1. **Reading-first** — o que roda é o que está no `.orl` e nos arquivos de cena; o editor não esconde comportamento em checkboxes mágicos sem gerar código/dados claros.
2. **Dados ≠ comportamento** — sprites, tiles, cenas são **dados**; comportamento é **código Ori** (funções em módulos).
3. **Sem herança de objetos** — reuso por:
   - composição de structs (`Enemy = Health + Patrol + SpriteRef`);
   - funções compartilhadas (`game.mechanics.ai.update_patrol`);
   - traits (`apply Drawable to …`) quando o tipo precisa de contrato.
4. **Código integrado, não opcional** — o painel de código é cidadão de primeira classe (não um “script afterthought”).
5. **Progressive disclosure** — iniciante: arrasta sprite, cola tile, escreve 20 linhas; avançado: sistemas custom, physics, save, multi-file packages.
6. **Um clique para rodar** — `Play` usa o mesmo pipeline `ori check/compile/run` (ou JIT) que o CLI.

---

## 4. Arquitetura do produto (três peças)

```
┌─────────────────────────────────────────────────────────────┐
│                    Ori Game Studio (IDE)                      │
│  ┌──────────┐  ┌────────────┐  ┌────────────┐  ┌─────────┐ │
│  │ Assets   │  │ Scene      │  │ Code Ori   │  │ Play /  │ │
│  │ browser  │  │ editor 2D  │  │ (LSP)      │  │ Inspector│ │
│  └────┬─────┘  └─────┬──────┘  └─────┬──────┘  └────┬────┘ │
└───────┼──────────────┼───────────────┼───────────────┼──────┘
        │              │               │               │
        ▼              ▼               ▼               ▼
   assets/*      scenes/*.scene   src/**/*.orl    ori run/JIT
        │              │               │               │
        └──────────────┴───────┬───────┴───────────────┘
                               ▼
                    runtime: ori-game (lib) + raylib
```

| Peça | Responsabilidade | Base tecnológica (proposta) |
|------|------------------|-----------------------------|
| **A. Runtime (`ori-game`)** | Engine 2D, assets, cena, input, draw, physics… | Já existe (este repo); evoluir para “runtime de Studio” |
| **B. Formato de projeto** | Manifesto + pastas + schema de cena/sprite | `ori.pkg.toml` + `project.ogame.toml` + JSON/YAML de cena |
| **C. Shell do Studio** | UI do editor + host do LSP + Play | Desktop: **Tauri 2 + frontend** *ou* app nativo Ori+raylib (v2) |

**Recomendação de host do editor (v1):**  
**Tauri 2 + UI web (Svelte/React)** falando com:

- `ori-lsp` (código);
- processo `ori run` / API de projeto;
- preview opcional em canvas/WebGL **ou** janela raylib filha.

Motivo: editor de código + painéis + docks é muito mais barato em web do que reinventar IDE em raylib. O **jogo** continua nativo Ori+raylib; o **Studio** é o shell.

*(v2 possível: Studio self-hosted em Ori se a linguagem e o runtime UI amadurecerem.)*

---

## 5. Modelo mental do jogo (sem POO)

### 5.1 Entidade de cena (dados)

```text
SceneEntity {
  id: string
  kind: string          -- "player", "slime", "checkpoint" (rótulo, não classe)
  pos: Vec2
  sprite: AssetRef
  solid: bool
  tags: list of string
  props: map of string to string   -- dados leves editáveis no Inspector
  script: ModulePath               -- módulo Ori que "dirige" este kind
}
```

Não há `class Slime extends Enemy`.  
Há: **kind + props + módulo de comportamento**.

### 5.2 Comportamento (código)

```ori
namespace game.scripts.slime

import game.engine as engine
import game.mechanics.ai as ai

pub func on_spawn(e: engine.Entity) -> engine.Entity
    -- setup
    return e
end

pub func on_update(e: engine.Entity, dt: float) -> engine.Entity
    -- patrol / chase
    return e
end

pub func on_draw(e: engine.Entity) -> void
end

pub func on_collide(e: engine.Entity, other: engine.Entity) -> engine.Entity
    return e
end
```

O Studio **gera o esqueleto** do módulo ao criar um kind; o autor edita Ori de verdade.

### 5.3 Loop de runtime (explícito)

```text
load project → load scene → spawn entities from scene data
while running:
  input
  for each entity: scripts.on_update  (ou systems por tag)
  physics / collisions
  for each entity: scripts.on_draw
  camera
```

Nada de “event order” mágico não documentado: a ordem é a do runtime e está na spec.

---

## 6. Superfícies do Studio (UI)

### 6.1 Painéis v1 (MVP “faz um platformer”)

| Painel | Função |
|--------|--------|
| **Project** | Árvore `src/`, `assets/`, `scenes/` |
| **Scene** | Viewport 2D: pan/zoom, grid, colocar/mover entidades e tiles |
| **Assets** | Import sprite, tileset, audio; preview |
| **Inspector** | Props da seleção (pos, kind, tags, props) |
| **Code** | Editor Ori com `ori-lsp` (hover, diagnostics, complete) |
| **Console** | Output de `ori check` / run / erros |
| **Play** | Compila/roda o entry do projeto; Stop |

### 6.2 Além do mapa (por isso não é “só tilemap”)

- **Sprite editor leve:** frames, origin, collision rect (AABB)
- **Animation clips:** lista de frames + fps
- **Object kinds:** catálogo de kinds → módulo de script
- **Collision layers** (bitmasks simples ou tags)
- **Audio cues** referenciáveis por id
- **Room/scene transitions** (dados: target scene + spawn id)
- **Build export** (pacote jogável: binário + assets)

Tilemap é **um editor entre vários**, não o produto.

---

## 7. Formatos de arquivo (propostos)

```
my_platformer/
  ori.pkg.toml                 # pacote Ori + dep ori_game
  project.ogame.toml           # meta do Studio (entry scene, window size)
  src/
    main.orl                   # bootstrap: carrega cena, chama run
    scripts/
      player.orl
      slime.orl
  assets/
    sprites/player.png
    tilesets/forest.png
    audio/jump.wav
  scenes/
    level_01.scene.json
```

**`level_01.scene.json` (esboço):**

```json
{
  "name": "level_01",
  "width": 40,
  "height": 23,
  "tile_size": 16,
  "layers": [
    { "name": "ground", "type": "tile", "tileset": "forest", "data": [/*…*/] },
    { "name": "collision", "type": "tile", "data": [/* 0/1 */] }
  ],
  "entities": [
    { "id": "p1", "kind": "player", "x": 64, "y": 180, "script": "scripts.player" },
    { "id": "e1", "kind": "slime", "x": 200, "y": 180, "script": "scripts.slime",
      "props": { "patrol": "left_right" } }
  ]
}
```

Runtime: `game.scene.load("scenes/level_01.scene.json")` → entidades + tilemap.

---

## 8. Roadmap de produto (alto nível)

| Fase | Entrega | Critério de “pronto” |
|------|---------|----------------------|
| **R0** | Runtime maduro (este repo) | demos linkam, engine/tick, tilemap, save |
| **R1** | **Schema + loader de cena** em Ori | `main.orl` carrega JSON de cena e spawna kinds |
| **R2** | **Studio shell v0** (Tauri): Project + Code(LSP) + Play | editar `.orl`, F5 roda |
| **R3** | **Scene editor v0** | colocar entities + tile paint + save `.scene.json` |
| **R4** | **Asset pipeline** | import sprite, origin, collision box |
| **R5** | **Kinds + scaffold de script** | “New kind” gera `scripts/foo.orl` e registra no projeto |
| **R6** | **Polish GM-like** | hot reload, templates (platformer/shmup), export release |
| **R7** | (Opcional) Studio em Ori nativo | se UI Ori existir e valer a pena |

**R1–R3** já entregam a sensação “abro o Studio e faço um jogo”.  
Editor de mapa sozinho = só R3 parcial.

---

## 9. Como isso se encaixa no que já existe (`ori-game`)

| Já temos | Vira no Studio |
|----------|----------------|
| `game.app.run_window` | Play window / game view |
| `game.engine` | Runtime de entidades da cena |
| `game.tilemap` | Camada tile da cena |
| `game.draw` / `input` / `camera` | Render e câmera da cena |
| `game.mechanics.*` | Biblioteca de comportamentos reutilizáveis (não herança) |
| `ori-lsp` + VS Code ext | Motor do painel Code (embutir ou acoplar) |
| `ori.pkg.toml` | Projeto do Studio |

**Próximo bloco de runtime (antes/during R1):**

1. `game.scene` — load/save scene JSON, spawn por kind  
2. `game.assets` — registro de sprites/tilesets por id  
3. `game.script_registry` — mapa `kind → módulo` sem vtables OOP  
4. Contrato estável de callbacks `on_update` / `on_draw` / `on_collide`

---

## 10. Decisões de produto

### 10.1 Fechadas (2026-07-10)

| Decisão | Escolha |
|---------|---------|
| **Nome** | **Ori Game Studio** |
| **Formato de cena** | **JSON próprio** (schema versionado; import Tiled opcional depois) |
| **Script por kind** | **1 arquivo Ori por kind** (`src/scripts/<kind>.orl`) |
| **Programação visual** | **Sim — um ambiente só**: nós ↔ código Ori em sync (ver §13.7) |
| **Host do Studio v1** | **Tauri + web** (jogo = Ori+raylib) |
| **UI visual** | **Unity light-mode first**, dark secundário; simplicidade GM + look Unity moderno (ver §14) |
| **Stack Studio** | Máximo de lógica em **Rust**; webview fino (ver §15) |

### 10.2 Host do Studio: Tauri+web vs Ori+raylib

#### A) Tauri 2 + UI web (recomendado para v1)

| Vantagens | Desvantagens |
|-----------|--------------|
| Editor de texto / Monaco / CodeMirror maduros | Duas stacks (jogo nativo + shell web) |
| Integração natural com **ori-lsp** (stdio/WebSocket) | Dependência Node/npm no *desenvolvimento* do Studio (usuário final recebe binário) |
| Docks, abas, split, temas, a11y web (ARIA, teclado, contraste) | “Sinto que abri um app web” se a UI for genérica |
| Canvas de **nós** (Blueprint) é o caminho mais barato (SVG/canvas/WebGL libs) | Precisa bridge clara: Studio ↔ `ori check/run` ↔ preview |
| Hot reload de UI do editor | Performance de viewport 2D boa, mas preview *pixel-perfect* do jogo pode ser janela filha raylib |
| Ecossistema (file dialogs, menus, updates) | Tamanho do installer maior que um binário raylib puro |

#### B) Studio 100% Ori + raylib

| Vantagens | Desvantagens |
|-----------|--------------|
| Uma filosofia, um runtime, “tudo é Ori” | Reinventar IDE: texto, seleção, undo, multi-cursor, LSP embed |
| Play e editor no mesmo processo (mais “GameMaker clássico”) | Canvas de nós e a11y avançada custam **muito** mais |
| Menos camadas de IPC | Menos libs de UI; risco de UX pobre por anos |
| Bom para *game view* nativo | Ruim para *code editor* de verdade no curto prazo |

**Síntese:** para a promessa “fácil como GameMaker **+ código de verdade + nós**”, Tauri+web vence no prazo. O **jogo** continua Ori+raylib; o **Studio** é o casco. Um Studio 100% Ori fica como horizonte se Ori ganhar UI toolkit.

**Default:** Tauri+web no v1, com **Play** abrindo/atualizando janela nativa do jogo.

---

## 11. Critério de sucesso (sensação GameMaker)

Um usuário novo consegue em **uma tarde**:

1. Criar projeto no **Ori Game Studio**  
2. Importar um sprite  
3. Pintar um chão de tiles  
4. Colocar um “player” e um “slime”  
5. Escrever movimento em Ori **ou** montar um grafo de nós equivalente  
6. Apertar Play e ver a cena rodar  
7. Sem `class` / `extends` / herança de objetos  

Se isso funcionar, a filosofia venceu.

---

## 12. Próximo passo de implementação

**R1 (runtime, sem UI do Studio ainda):**  
`game.scene` + loader JSON próprio + demo que carrega cena e spawna kinds (1 `.orl` por kind).

**R2:** shell Ori Game Studio (Tauri) — Project + Code(LSP) + Play.  
**R3:** Scene editor.  
**R4+:** assets, kinds wizard.  
**R-vis (paralelo a partir de R2/R3):** grafo de nós ↔ Ori (ver §13).

---

## 13. Programação visual por nós (Blueprints / Bolt) — viabilidade e desenho ND

### 13.1 Dá para fazer?

**Sim.** É viável e alinhado ao produto — desde que os nós **não substituam** Ori como verdade final, e sim **compõem ou geram** Ori (ou um IR legível).

| Referência | O que copiar | O que evitar |
|------------|--------------|--------------|
| Unreal Blueprints | grafo de eventos, pins tipados, debug visual | explosão de “spaguetti”, herança de Blueprint classes |
| Unity Bolt / Visual Scripting | nós de fluxo + dados, variables de grafo | UI densa, pouca legibilidade textual |
| Scratch / MakeCode | blocos grandes, feedback imediato, erros amigáveis | infantilizar; perder poder de Ori |
| Pure Data / Max | clareza de fluxo de dados | curva musical demais |

### 13.2 Princípio: dois modos, uma semântica

```
┌─────────────┐     lower / sync      ┌──────────────────┐
│  Grafo .ogn │  ←──────────────────→ │  scripts/x.orl   │
│  (nós)      │     (sempre legível)   │  (fonte canônica │
└─────────────┘                       │   ou gerada)     │
                                      └──────────────────┘
```

Opções de verdade:

1. **Ori canônico** — nós *editam* / *refletem* um subconjunto do `.orl` (mais seguro).  
2. **Grafo canônico** — compila grafo → Ori gerado (mais “visual-first”; diff de código pior).  
3. **Híbrido** — funções marcadas `@visual` são ida-e-volta; o resto é só texto.

**Recomendação:** híbrido com **Ori como source of truth** e regiões `@visual` / arquivos `.ogn` ao lado do kind (`scripts/slime.orl` + `scripts/slime.ogn`).

### 13.3 Modelo de nós *sem* POO

Nós = **funções e dados**, não métodos de classe.

| Categoria de nó | Exemplos |
|-----------------|----------|
| **Evento** | `On Update`, `On Collide`, `On Spawn`, `On Key` |
| **Fluxo** | `Branch`, `Sequence`, `Wait`, `For Each` (limitado) |
| **Dados** | `Get Pos`, `Set Vel`, `Prop (kind)`, literais |
| **Ação** | `Play Sound`, `Apply Damage`, `Change Scene` |
| **Mecânica** | wrappers de `game.mechanics.*` (jump, dash, patrol) |
| **Math / logic** | `+`, `and`, `clamp`, `distance` |

Pins **tipados** (float, int, bool, Vec2, EntityRef, string) — erros no grafo = diagnostics iguais aos do código.

### 13.4 Foco em facilidade e neurodivergência (ND)

Não é só “bonito”; é **carga cognitiva e sensorial**.

| Princípio ND | Aplicação no grafo |
|--------------|-------------------|
| **Baixa densidade** | poucos nós por tela; zoom + “focus mode” (só o fluxo ativo) |
| **Previsibilidade** | mesma ordem de execução documentada; sem side effects escondidos |
| **Feedback imediato** | highlight do nó atual no Play; erros no nó, não só no console |
| **Chunks grandes** | macros / “receitas” (Move 4-dir, Patrol box) em vez de 30 nós micro |
| **Cor + forma + texto** | nunca só cor; ícones e labels curtos; modo alto contraste |
| **Menos ruído visual** | wires retos opcionais, grid, snap, minimizar wires cruzados |
| **Teclado first** | criar/conectar nós sem mouse obrigatório; atalhos documentados |
| **Uma coisa por vez** | wizard “quero que o slime persiga o player” → grafo mínimo gerado |
| **Desfazer/refazer confiável** | histórico local; sem medo de quebrar |
| **Modo calmo** | desligar animações de wire, partículas de UI, auto-layout agressivo |
| **Leitura linear opcional** | botão “ver como Ori” sempre disponível (reduz ansiedade de caixa-preta) |

**Anti-padrões a evitar (Blueprints ruins):**

- 200 nós numa tela sem grupos  
- herança de grafos  
- side effects em nós “Get”  
- tipos fracos / conversões silenciosas  

### 13.5 Onde encaixa no roadmap

| Fase | Entrega visual |
|------|----------------|
| R2 | Studio shell (sem nós ainda) |
| R3 | Scene editor |
| **R-vis0** | Viewer de grafo read-only gerado a partir de templates |
| **R-vis1** | Editor de nós para `on_update` de um kind; export → `.orl` |
| **R-vis2** | Round-trip `.ogn` ↔ região `@visual` no `.orl` |
| **R-vis3** | Debug step-through no Play + receitas ND |

### 13.6 Esforço (honestidade)

- Editor de nós usável: **meses**, não semanas.  
- Com Tauri+web: libs de graph editor existem; ainda há trabalho de **semântica Ori**.  
- Com raylib puro: facilmente **2–3×** o esforço só de UI.  
- Por isso nós **reforçam** a escolha Tauri no v1, se programação visual for prioridade de produto.

### 13.7 Um ambiente só: nós ↔ código em tempo real (anti-GameMaker dual)

**Ideia do produto (aceita):** não existem “dois mundos” (visual *ou* código). Existe **uma semântica**; a pessoa escolhe a **lente**:

```
┌──────────────────────────────────────────────────┐
│  Kind: slime                                     │
│  [ Grafo ]  [ Código ]  [ Split ]   ← mesmas 3  │
│                                                  │
│   nós  ←── lower/lift em tempo real ──→  .orl    │
└──────────────────────────────────────────────────┘
```

| Direção | Comportamento |
|---------|----------------|
| **Nó → código** | Editar grafo **gera/atualiza** o Ori do kind (texto legível, formatado) |
| **Código → nó** | Editar o Ori **reconstrói** o grafo da região visual (quando o subset for suportado) |
| **Split** | Dois painéis; seleção de nó destaca linhas; cursor no código destaca nós |

**Regras para não virar inferno:**

1. **Subset visual** — só um núcleo de Ori é round-trip (eventos, if/branch, calls a mecânicas, math simples).  
2. **Regiões** — funções/`@visual` ou arquivo irmão `.ogn`; o que estiver fora do subset fica **só em código** (nó “Custom Ori” opaco se necessário).  
3. **Source of truth no disco** — preferência: **`.orl` canônico** + `.ogn` como cache de layout (posições dos nós). Se o código mudar e o grafo não parsear, mostra aviso e preserva o texto.  
4. **Debounce + parse incremental** — não reescrever o arquivo a cada pixel de drag; commit em pause/blur/Play.  
5. **Diff honesto** — gerar Ori estável (ordem determinística de nós) para diffs de git limpos.  
6. **Sem dualidade GM** — não há “event list GML” separado do “code editor”; o código **é** o que o grafo expressa.

Isso é exatamente o modelo pedido: **facilita com nós, sem prender quem prefere código**, e sem dois pipelines de produto.

---

## 14. Design de interface (Unity light + simplicidade GameMaker)

### 14.1 Direção visual

| Aspecto | Direção |
|---------|---------|
| **Tema default** | **Light** (estilo Unity Editor light: cinzas frios, painéis claros, seleção azul suave) |
| **Tema secundário** | Dark (opcional; mesma hierarquia, não outro layout) |
| **Densidade** | Ícones **menores**, padding moderado; menos chrome que Unity “pesado” |
| **Abas** | **Poucas abas fixas**; conteúdo muda por **modo** (Scene / Kind / Assets), não 15 tabs soltas |
| **Configurações** | Preferências em **modais** / drawer, não um labirinto de Project Settings |
| **Sensação** | “GameMaker na cabeça, Unity no olho” — hierarquia clara, sem D&D legado |

### 14.2 Layout modular (proposta)

```
┌─────────┬──────────────────────────┬────────────┐
│ Project │     Viewport / Work      │ Inspector  │
│ (tree)  │     (Scene | Graph |     │ (context)  │
│         │      Sprite | Timeline)  │            │
├─────────┴──────────────────────────┴────────────┤
│  Code / Graph strip  OR  Timeline  (dock bottom) │
├──────────────────────────────────────────────────┤
│  Console (colapsável)                            │
└──────────────────────────────────────────────────┘
     Toolbar: Play | Stop | Mode switch | Theme
```

- **Um “modo de trabalho”** por vez no centro (Scene, Graph, Sprite, Animation, Particles…).  
- **Inspector** sempre contextual à seleção.  
- **Modais** para: New Project, Import Asset, New Kind, Export, Preferences, Collision editor detalhado.  
- Evitar: dezenas de janelas flutuantes estilo Unity pro sem layout salvo.

### 14.3 Acessibilidade ND (UI shell)

- Contraste WCAG no light e dark  
- Modo calmo (sem motion excessivo)  
- Atalhos documentados; foco teclado visível  
- Densidade “Comfortable / Compact”  

---

## 15. Stack Tauri — máximo Rust, webview fino

### 15.1 Princípio de fronteira

| Em **Rust** (Tauri commands / crates) | Na **webview** (só UI) |
|--------------------------------------|-------------------------|
| FS do projeto, watch, import de assets | Render de painéis, docks, modais |
| Parse/lower grafo ↔ Ori (quando possível) | Canvas de nós, viewport 2D editor |
| Spawn `ori check/run`, pipes, diagnostics | Mostrar diagnostics, Monaco |
| Index de projeto, kinds, cenas | Listas, trees, forms |
| Image processing preview (opcional) | Mostrar texture/preview |
| Undo stack de documento (opcional server-side) | Gestos, drag, selection |

Webview **não** é dona da regra de negócio do jogo.

### 15.2 Bibliotecas recomendadas

#### Core app (Rust / Tauri)

| Lib | Uso |
|-----|-----|
| **Tauri 2** | Shell desktop, IPC, windowing |
| **tauri-plugin-fs / dialog / shell / process** | Arquivos, open/save, spawn ori |
| **tauri-plugin-window-state** | Lembrar layout da janela |
| **notify** | File watch (hot reload assets/código) |
| **serde / serde_json** | Cenas JSON, project files |
| **walkdir** | Scan de projeto |
| **thiserror / anyhow** | Erros de domínio |
| **tokio** | Async IPC, processos |
| **which** / path helpers | Achar `ori` no PATH |
| **image** (Rust) | Resize, import, thumbnails, alguns FX de preview |
| **lopdf / zip** (depois) | Export packs |

#### Frontend (webview mínimo porém premium)

| Lib | Uso |
|-----|-----|
| **Svelte 5** ou **Solid** (preferir leves) **ou React 19** se equipe já fluente | UI reativa |
| **Tailwind 4** + tokens de design (light Unity-like) | Visual moderno sem CSS bagunçado |
| **Bits UI / Melt / Radix** (headless) | Modais, dropdowns, focus trap a11y |
| **Lucide** (ícones pequenos, consistentes) | Toolbar |
| **Monaco Editor** (ou CodeMirror 6 se quiser mais leve) | Painel de código Ori |
| **@xyflow/react** (React Flow) **ou** **Svelte Flow** | **Editor de nós** |
| **zustand** ou **nanostores** | Estado UI fino (não duplicar estado do Rust) |
| **cmdk** | Command palette (Ctrl+K) — reduz menus confusos |
| **floating-ui** | Popovers/tooltips |
| **panzoom** / custom canvas | Viewport de cena 2D no editor |
| **immer** (se React) | Updates imutáveis no client |

#### Nodes (escolha)

| Opção | Nota |
|-------|------|
| **Svelte Flow** / **React Flow (@xyflow)** | Melhor DX, custom nodes, minimap, ND-friendly com CSS |
| **Rete.js** | Poderoso; mais cerimônia |
| **Elkjs** | Layout automático opcional (desligar no “modo calmo”) |

#### Não recomendar no v1

- Electron (mais pesado que Tauri para o mesmo fim)  
- Unity-as-editor-host  
- Duplicar motor de jogo na web (WebGL clone completo) — só **editor view**, Play nativo  

### 15.3 Arquitetura modular de “módulos do Studio”

Cada módulo = crate Rust opcional + painel web preguiçoso:

```
studio/
  crates/
    ogs-core/          # project model, paths
    ogs-scene/         # scene json schema
    ogs-graph/         # graph IR ↔ ori lower/lift
    ogs-assets/        # import, thumbs
    ogs-runner/        # ori process
  ui/
    modules/
      scene-editor/
      code-editor/
      node-graph/
      animation-timeline/
      particles/
      dialogue/
      postfx/
```

Ativar módulos por feature flags / settings, sem poluir a toolbar default.

---

## 16. Catálogo de módulos de conteúdo (runtime + editors)

### 16.1 Pedidos do produto (mapa)

| Módulo | Runtime (`ori-game`) | Editor no Studio |
|--------|----------------------|------------------|
| **Diálogos** | já há base `game.dialogue`; evoluir para data-driven | Graph de falas + choices + preview |
| **Animação + timeline** | clips (frames, fps, loop) em dados; player no draw | Timeline de frames / keys |
| **Partículas** | `game.particles` (update/draw); expandir emitters | Inspector de emitter + preview |
| **Object / Kind editor** | kinds + scripts 1:1 | “New Object” wizard (estilo GM) + props |
| **FX de imagem** | shaders/material no draw path | Toggle + params no Inspector |
| **Pós-processamento** | full-screen pass (blur, grade, vignette…) | Stack de FX na câmera/cena |
| **Iluminação 2D** | lights + normal maps opcionais / lightmap | Colocar light, raio, cor; máscaras |
| **Colisores visuais** | AABB/circle/poly no kind ou sprite | Editar handles no viewport |

### 16.2 Imagem / pós / luz (abordagem técnica)

| Efeito | Abordagem realista com raylib/OpenGL |
|--------|--------------------------------------|
| Grayscale, tint, flash | shader de sprite / multiply color (fácil) |
| Neon / outline | multi-pass outline + glow (médio) |
| Blur, bloom, vignette | **render texture** + fullscreen shader (médio/alto) |
| Lighting 2D | lights aditivos + **normal map** opcional; colliders como **blockers** de luz (raymarch 2D ou shadow mesh — alto) |
| Collider edit | dados no JSON do kind/sprite; gizmo no editor |

**Fases honestas:**  
1) tint/grayscale/flash + outline  
2) post stack (blur/bloom/color grade) via RT  
3) lights pontuais  
4) shadows/blockers com colliders  

### 16.3 O que mais recomendo (prioridade de “sinto GameMaker/Unity 2D”)

| Prioridade | Módulo | Por quê |
|------------|--------|---------|
| P0 | **Scene + Kind + Play** | Coração do Studio |
| P0 | **Code + Graph sync** | Diferencial vs GM dual |
| P0 | **Sprites + colliders editáveis** | Base de qualquer 2D |
| P1 | **Tilemap layers** | Levels rápidos |
| P1 | **Animation clips + timeline** | Personagens vivos |
| P1 | **Audio bus** (sfx/music + volumes) | Feedback imediato |
| P1 | **Dialogue** data + editor | RPGs / narrative |
| P1 | **Particles** editor | Juice |
| P2 | **Post-FX stack** | Polish visual |
| P2 | **2D lights** | Atmosfera |
| P2 | **Camera zones / rooms** | Transições estilo GM |
| P2 | **Input map** (ações, não scancodes crus) | Portabilidade |
| P2 | **Prefab-of-data** (templates de entity, não herança) | Reuso sem OOP |
| P3 | **Pathfinding grid** | AI de chase decente |
| P3 | **Cutscene timeline** (câmera + dialogue + waits) | Direção |
| P3 | **Localization table** | Shipping |
| P3 | **Build/export** one-click | Distribuir jogo |

### 16.4 O que *não* priorizar cedo

- Física 3D, multiplayer netcode, editor de shaders genérico completo  
- Visual scripting *sem* sync com código  
- 50 abas de settings estilo Unity full  

---

## 17. Próximos passos de implementação (atualizado)

1. **R1** — `game.scene` + JSON schema + load + 1 script/kind  
2. **R2** — scaffold **ori-game-studio** (Tauri): Project tree + Monaco + Play + tema light  
3. **R3** — Scene viewport + Inspector + modais New/Import  
4. **R-vis1** — Graph → Ori generate; depois lift Código → Graph  
5. **Módulos** na ordem P0→P1 da §16.3  

---

*Documento vivo. Nome: Ori Game Studio · JSON próprio · 1 kind = 1 `.orl` · nós↔código em sync · UI Unity-light/GM-simple · Tauri+Rust-first · módulos §16.*
