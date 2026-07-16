# ADR — Studio ImGui fino (sem pivot fyrox-ui)

| Campo | Valor |
|-------|--------|
| **Status** | Accepted |
| **Data** | 2026-07-15 |
| **Decisores** | Produto Ori Game / agents session |
| **Contexto** | Studio lento/disfuncional; dúvida ImGui vs fyrox-ui vs rehost |

## Contexto

O Studio nativo (`ori-imgui` + raylib + shell Ori) ficou pesado: muitos painéis, docking “Godot-like” completo, draw 3D full + ImGui full todo frame, histórico de CSG/load no hot path. Surgiu a hipótese de trocar ImGui por **fyrox-ui** (ou host Rust/C++) ou abandonar UI de engine.

Filosofia de produto (inalterada):

- Funcionalidade ≫ polish  
- G1 = primeiro jogo jogável  
- IDE de **código = externa**  
- Studio = author **Hierarchy / Viewport / Inspector / Play**  
- L0 já é C (raylib, Jolt, ImGui shims)  
- Log de study: `study/SOURCE-LEARNINGS.md`

## Decisão

1. **Permanecer em Dear ImGui** como UI de *tools* do Studio.  
2. **Não** adotar fyrox-ui / egui / Qt **neste ciclo** (não é drop-in; implica pivot de stack).  
3. **Pivot de *produto de UI*** (não de linguagem):  
   - **De:** shell denso (Project, Tools, Console, Output, Timeline, Nodes sempre no layout).  
   - **Para:** **ImGui fino** — só **Hierarchy + Viewport (passthrough) + Inspector + Toolbar (Play/Save/fps)**.  
   - Painéis extra: **opt-in** via menu View (off por default).  
4. **F6** continua como modo *sem ImGui* (diagnóstico linguagem vs UI).  
5. Reabrir host UI em Rust/C++ **somente** se, com shell fino + F6, o profiler ainda apontar ImGui/dock como único gargalo de G1.

## Consequências

### Positivas

- Zero rewrite de ECO ImGui/gizmo/nfd.  
- Alinha com study (tools IMGUI + ResourceCache + preview barato).  
- Reduz custo por frame e superfície de bugs.  
- Mensagem clara: “Studio tool, não Godot Editor”.

### Negativas / trade-offs

- Menos “IDE visual” out-of-the-box (Tools/Nodes/Timeline escondidos).  
- Layout dock antigo em `imgui.ini` do usuário pode precisar **View → Reset dock layout**.  
- Gizmo/level tools ficam atrás de View → Tools (ainda existem).

### Explicitamente fora

- fyrox-ui como shell.  
- Reescrita da engine em C++/Rust por causa do editor.  
- Tauri/Three como destino de Studio.

## Implementação (checklist)

- [x] Este ADR  
- [x] Defaults `show_*` off (exceto Hierarchy/Inspector/Toolbar)  
- [x] `on_draw`: só painéis on  
- [x] Dock layout fino (L/R, sem bottom row)  
- [x] Spec `studio-shell` + `SOURCE-LEARNINGS` + handoff  

## Alternativas rejeitadas (resumo)

| Opção | Por que não agora |
|-------|-------------------|
| fyrox-ui | Host Rust/Fyrox; outro produto |
| Qt | Stack paralelo; foge ECO |
| raygui only | Fraco demais para dock/inspector |
| Sem Studio visual | Válido G0, mas recua author 3D |

## Links

- `study/SOURCE-LEARNINGS.md`  
- `STUDY-TO-ORI-PERF.md`  
- `docs/specs/studio-shell/`  
- `ori-imgui/demos/studio_shell/main.orl`
