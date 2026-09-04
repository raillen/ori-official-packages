# Ranking — extensões Dear ImGui para Ori Studio / tools

> **Data:** 2026-07-15  
> **Fontes de plano:** `ROADMAP-GAME-ECO.md` (**catálogo completo** ImGui + P0–P4), `ori-imgui` 0.4.0,  
> catalog ports (`ori-implot`, `ori-imnodes`, `ori-imguizmo`, `ori-nfd`, …)  
> **Critérios de rank:** Necessidade (N) · Dificuldade de port/bind (D) · Valor profissional (V)  
> Escala 1–5 (5 = alta necessidade / alta dificuldade / alto valor).  
> **Score** ≈ `(N×2 + V×2 − D)` — prioriza necessidade e valor, penaliza dificuldade.

---

## 1. O que já está no plano / no código

### 1.1 Já no `ori-imgui` (implementado ou MVP)

| Item | Status real | Upstream equivalente |
|------|-------------|----------------------|
| **Dear ImGui core** | **done** (`vendor/cimgui` + `imgui.ui`) | [ocornut/imgui](https://github.com/ocornut/imgui) |
| Docking / tables / selectable | **done** Tier 0 | core ImGui |
| `file_browser` | **done** (lista dirs Ori, não full dialog nativo) | ≈ file dialog minimal |
| `inspector` | **done** MVP (int/bool/milli) | ≈ property rows, não ImReflect |
| `nodes` | **MVP próprio** (cards + links), **não** imnodes/node-editor | imnodes *later* no plano |
| `plot` | **MVP** (buffer PlotLines), **não** ImPlot | implot *later* |
| `curves` | **MVP** pure Ori (points milli) | ImCurveEdit S2 no plano |
| `timeline` | **MVP** pure Ori | ImSequencer-like S2 “custom” |
| Backend raylib embed | **done** P2-A | — |
| Multi-context editor/game | **no plano** S0 open | Multi-Context Compositor |

### 1.2 Explicitamente no ROADMAP ECO (Tier 1–3 / ports)

| Upstream / ideia | Onde no plano | Status |
|------------------|---------------|--------|
| **implot** → `ori-implot` | Tier A ports + plot S1 | **planned**, não portado |
| **imnodes** → `ori-imnodes` (alt: imgui-node-editor) | Tier A + nodes S1 | **planned** |
| **ImGuizmo** → `ori-imguizmo` | Tier A + gizmos S2 | **planned** |
| ImCurveEdit / ImGradient | Tier 2 S2 | **planned** (ou custom) |
| Animation timeline | Tier 2 S2 custom | **MVP** timeline Ori |
| Knobs / spinners / toggles | Tier 2 S2 | **planned** generically |
| Style / themes | Tier 2 S2 | **planned** generically |
| Image helpers | Tier 2 S2 | **planned** |
| **imgui_markdown** | Tier 3 S3 | **planned** |
| IME | Tier 3 S3 | **planned** |
| Remoting (imgui-ws) | Tier 3 S3 | **planned** |
| Terminal/console | Tier 3 S3 | **planned** |
| Layout generator / GUI editor | Tier 3 optional | **planned** deprioritized |
| **ori-nfd** (native file dialog) | Tier A ports | **planned** (paralelo a file_browser) |
| ColorTextEdit | S1 “later” | **mentioned**, não portado |
| Wiki Useful Extensions | referência canônica | — |

### 1.3 Na sua lista, **fora** do plano detalhado (ou só implícito)

A maioria dos itens “nice” (notify, ImSearch, Zep, ImPlot3D, Hello ImGui, MetricsGui, netImGui, ImTui, knobs específicos, ImRAD, spectrum, software renderer, command palette, etc.) **não** tem linha própria no roadmap — só o bucket “Tier 2 knobs” / “Tier 3 remoting/markdown”.

---

## 2. Ranking unificado (todas as libs da sua lista)

**Legenda status:**  
`done` · `mvp` (nosso, não o upstream) · `planned` · `gap` (não no plano)

**Ordem = score decrescente** (fazer primeiro o que está no topo).

| # | Biblioteca | Status plano | N | D | V | Score | O que faz (1 linha) |
|---|------------|--------------|---|---|---|-------|---------------------|
| 1 | **Dear ImGui** (core) | **done** | 5 | 3 | 5 | 14 | UI imediata C++: janelas, widgets, docking base. |
| 2 | **ImGuizmo** | **planned** S2/Tier A | 5 | 3 | 5 | 14 | Gizmos move/rotate/scale 3D no viewport — **editor de cena**. |
| 3 | **imnodes** (Nelarius) | **planned** Tier A | 5 | 2 | 4 | 14 | Node graph leve, zero deps — **visual script / materials**. |
| 4 | **ImPlot** | **planned** Tier A | 4 | 2 | 5 | 14 | Plots 2D GPU — FPS, curves de debug, profiling. |
| 5 | **imgui-node-editor** (thedmd) | planned alt. a imnodes | 4 | 4 | 5 | 10 | Node editor “blueprint” completo (mais pesado/rico). |
| 6 | **ImGuiFileDialog** | partial via file_browser; nfd planned | 4 | 3 | 4 | 10 | Open/Save com preview/bookmarks. |
| 7 | **ImGuiColorTextEdit** (pthom fork) | mentioned S1 later | 3 | 3 | 5 | 10 | Editor de código c/ syntax highlight **in-UI** (se não usar IDE externa). |
| 8 | **ImCurveEdit** (+ ImGuizmo repo) | planned S2 | 4 | 3 | 4 | 10 | Editor de curvas Bezier (easing/anim). |
| 9 | **ImSequencer** (mesmo ecossistema Cedric) | planned “timeline custom” | 4 | 3 | 4 | 10 | Timeline de tracks — anim/cinemática. |
| 10 | **ImGradient** | planned S2 | 3 | 2 | 3 | 8 | Editor de gradiente de cor. |
| 11 | **ori file_browser / ori-nfd** | file_browser done; nfd planned | 4 | 2 | 4 | 12 | Diálogo nativo OS (nfd) vs lista in-ImGui. |
| 12 | **imgui-notify** | **gap** | 3 | 1 | 3 | 10 | Toasts de sucesso/erro — UX de tools. |
| 13 | **ImSearch** | **gap** | 3 | 2 | 3 | 8 | Busca em listas grandes (assets/entities). |
| 14 | **ImHotKey** | **gap** | 3 | 2 | 3 | 8 | Captura/atalhos de teclado configuráveis. |
| 15 | **MetricsGui** | **gap** (parcial plot MVP) | 3 | 2 | 4 | 10 | Widgets de FPS/frame time (GameTechDev). |
| 16 | **imgui_memory_editor** (imgui_club) | **gap** | 2 | 1 | 4 | 10 | Hex dump — debug runtime/compilador. |
| 17 | **Command Palette** (extensão) | **gap** | 3 | 2 | 4 | 10 | Ctrl+Shift+P de comandos do editor. |
| 18 | **Multi-Context Compositor** | planned S0 open | 4 | 4 | 4 | 8 | Dois contextos ImGui (game + editor). |
| 19 | **ImViewGuizmo** / view cube | **gap** (Studio React tem view cube) | 3 | 3 | 3 | 6 | Cubo de navegação de câmera. |
| 20 | **imGuIZMO.quat** | **gap** (alt ImGuizmo) | 2 | 3 | 3 | 4 | Gizmo quaternion alternativo — **só se ImGuizmo falhar**. |
| 21 | **ImNodeFlow** | **gap** | 2 | 3 | 3 | 4 | Outro node editor — **redundante** com imnodes/node-editor. |
| 22 | **ImPlot3D** | **gap** | 2 | 3 | 3 | 4 | Plots 3D — raro no G1 Studio. |
| 23 | **Zep** | **gap** | 2 | 4 | 4 | 4 | Editor texto robusto (+Vim) — pesado; prefira IDE externa. |
| 24 | **ColorTextEdit** (Balazs original) | superseded by pthom | 2 | 3 | 3 | 4 | Preferir **fork pthom**. |
| 25 | **ImZoomSlider** | **gap** (ImGuizmo bundle) | 2 | 2 | 2 | 4 | Slider de zoom — acessório. |
| 26 | **imgui-knobs** | planned bucket S2 | 2 | 2 | 3 | 6 | Dials para áudio/params. |
| 27 | **imgui_toggle** | planned bucket S2 | 2 | 1 | 2 | 6 | Switch visual vs checkbox. |
| 28 | **imspinner** | planned bucket S2 | 2 | 1 | 2 | 6 | Spinners de loading. |
| 29 | **ImAnim** | **gap** | 2 | 3 | 2 | 2 | Anima widgets da UI — cosmético. |
| 30 | **imgui_markdown** | planned S3 | 2 | 2 | 3 | 6 | Docs/help in-engine. |
| 31 | **ImGuiTextSelect** | **gap** | 2 | 2 | 2 | 4 | Seleção/cópia de texto em labels. |
| 32 | **ImRefl / ImGui::Auto / ImReflect** | **gap** (inspector MVP existe) | 3 | 4 | 4 | 6 | UI auto a partir de structs — forte, mas C++ reflection hard em Ori. |
| 33 | **Hello ImGui** | **gap** | 2 | 3 | 3 | 4 | Framework de app ImGui — **concorre** com shell próprio. |
| 34 | **Stack Layout / Splitters** | partial (docking core) | 3 | 2 | 3 | 8 | Layout flex/split — docking ImGui já cobre muito. |
| 35 | **ImRAD** | planned optional S3 | 1 | 4 | 3 | 0 | Designer WYSIWYG → C++ — estudo, não port cedo. |
| 36 | **imgui-spectrum** | planned themes S2 | 2 | 1 | 3 | 8 | Tema Adobe-like — polish barato. |
| 37 | **Flix01 Addons** | **gap** (catálogo) | 2 | 2 | 2 | 4 | Coleção de addons — cherry-pick, não monorepo. |
| 38 | **imgui_hex_editor** (Teselka) | **gap** | 1 | 2 | 2 | 2 | Alt hex — prefira ocluba memory_editor. |
| 39 | **InAppGpuProfiler** | **gap** | 2 | 3 | 4 | 6 | Profiler GPU in-UI — pós G1 / com Tracy. |
| 40 | **netImGui / imgui-ws** | planned remoting S3 | 2 | 4 | 3 | 2 | UI remota — dev tools avançados. |
| 41 | **ImTui** | planned console S3-ish | 1 | 3 | 2 | 0 | ImGui no terminal — nicho headless. |
| 42 | **Software Renderer (emilk)** | **gap** | 1 | 4 | 2 | −2 | UI sem GPU — só se target sem GL. |
| 43 | **ImGuiTexInspect / imgInspect** | planned image S2 | 3 | 2 | 3 | 8 | Inspetor de textura (canais, zoom). |
| 44 | **ImGuiDatePicker** | **gap** | 1 | 1 | 1 | 2 | Date picker — quase irrelevante p/ engine. |
| 45 | **DearImGui-with-IMM32** | planned IME S3 | 2 | 3 | 3 | 4 | IME Windows CJK — quando i18n for serio. |
| 46 | **imgui_test_engine** | **gap** | 3 | 4 | 4 | 6 | Testes automatizados de UI — valor QA alto, custo setup alto. |
| 47 | **ImGuiFD** (zero STL) | **gap** | 2 | 2 | 2 | 4 | File dialog minimalista — file_browser/nfd cobrem. |
| 48 | **Wiki Useful Extensions** | referência | — | — | — | — | Catálogo oficial, não é lib. |

---

## 3. Ranking “o que falta portar de verdade” (só gaps / planned, não done)

Ordem de implementação sugerida **depois do core já ter**:

### P0 — Studio nativo “parece engine” (alto score + no plano)

1. **ImGuizmo** — sem isso não há editor 3D sério  
2. **imnodes** (preferir ao node-editor no 1º port: D menor)  
3. **ImPlot** — debug/profiling profissional  
4. **ori-nfd** ou polish `file_browser` / FileDialog — open/save real  

### P1 — Conteúdo / anim (plano Tier 2)

5. **ImCurveEdit** + **ImGradient** (mesmo autor do ImGuizmo — bom bundle)  
6. **ImSequencer** ou aprofundar `imgui.timeline` Ori  
7. **TexInspect** (image tools)  
8. **imgui-notify** + **ImSearch** + **Command Palette** (UX barata, V alto)  
9. **MetricsGui** ou ImPlot dashboards  

### P2 — Código in-UI (só se **não** confiar 100% em IDE externa)

10. **ColorTextEdit (pthom)** — se quiser shader/script embutido  
11. **Zep** — só se ColorTextEdit for pouco (D alta)  

### P3 — Profundidade / polish

12. Multi-context compositor  
13. Knobs / toggle / spinner / spectrum theme  
14. markdown help  
15. memory_editor (debug compilador)  
16. imgui_test_engine  
17. Reflection auto-UI (difícil em Ori — talvez gerar código, não port C++)  

### P4 — Adiar / evitar cedo

- ImNodeFlow, imGuIZMO.quat, ImPlot3D, Hello ImGui, ImRAD, netImGui, ImTui, software renderer, DatePicker, ImAnim  

---

## 4. Descritivos curtos por categoria (sua lista)

### Core
- **Dear ImGui** — biblioteca base de UI imediata (já no `ori-imgui`).

### Node editors
- **imgui-node-editor** — blueprints completos (pin, zoom, save).  
- **imnodes** — nós simples e leves (recomendado 1º port).  
- **ImNodeFlow** — outra opção de flow; redundante.

### Gizmos / view
- **ImGuizmo** — transform 3D no viewport.  
- **ImCurveEdit / ImGradient / ImZoomSlider** — curvas, gradiente, zoom (mesmo ecossistema).  
- **imGuIZMO.quat / ImViewGuizmo** — alts de gizmo / view cube.

### Plot
- **ImPlot** — charts 2D profissionais.  
- **ImPlot3D** — charts 3D (nicho).

### Texto / hex
- **ColorTextEdit** — code editor com highlight.  
- **Zep** — editor “IDE-like”/Vim.  
- **memory_editor / hex_editor** — memória em hex.

### Files / notify / search
- **ImGuiFileDialog / Flix01 / ImGuiFD** — diálogos de arquivo.  
- **imgui-notify** — toasts.  
- **ImSearch** — busca em listas.

### Métricas / reflection / app
- **MetricsGui** — FPS/widgets de perf.  
- **ImRefl / Auto / ImReflect** — inspector automático via reflection C++.  
- **Hello ImGui** — bootstrap de aplicação ImGui.

### Outros (2ª lista)
- **imgui_test_engine** — testes de UI.  
- **ImSequencer / ImAnim** — timeline e anim de UI.  
- **markdown / IME / knobs / spinners / toggles** — docs, i18n, polish.  
- **Stack Layout / Splitters** — layout (docking já ajuda).  
- **ImRAD / spectrum** — designer / tema.  
- **Software renderer / netImGui / imgui-ws / ImTui** — alvos especiais.  
- **TexInspect** — debug de texturas.  
- **DatePicker** — datas.  
- **Command Palette / InAppGpuProfiler** — power-user e GPU.

---

## 5. Recomendação alinhada à Ori

| Decisão | Motivo |
|---------|--------|
| **IDE de código = externa** | Zep/ColorTextEdit caem para P2 opcional |
| **Port order oficial** | ImGuizmo → imnodes → ImPlot → nfd/FileDialog → Curve/Sequencer |
| **Não portar 3 node editors** | Escolher **imnodes** primeiro; node-editor só se faltar power |
| **MVP Ori** (nodes/plot/curves/timeline) | Manter até o port C valer a pena; não jogar fora |
| **Reflection C++** | Baixa prioridade: Ori não é C++; inspector manual/gerado é mais realista |
| **Hello ImGui** | Não adotar como shell — vocês controlam o loop raylib |

---

## 6. Mapa score → ação

```text
done/mvp agora     → Dear ImGui, file_browser, inspector, nodes/plot/curves/timeline MVP
próximos ports C   → ImGuizmo, imnodes, ImPlot, nfd
UX barata Ori-side → notify, search, command palette, spectrum (muitos em pure Ori + ui.*)
depois G1 Studio   → CurveEdit, Sequencer, TexInspect, multi-context
quase nunca cedo   → ImTui, software GL, DatePicker, 3º node editor, Hello ImGui
```

_Documento vivo: atualizar Status quando um port entrar no `ori-imgui` ou repo `ori-im*`. _
