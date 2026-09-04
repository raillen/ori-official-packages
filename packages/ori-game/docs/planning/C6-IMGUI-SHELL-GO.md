# C6 — Spike go/no-go: ImGui como shell principal

> **Data:** 2026-07-15  
> **Status:** **GO** — ImGui + raylib é o shell **principal** do Studio  
> **Demo:** `ori-imgui/demos/studio_shell/` (C0–C5)  
> **Base da fila:** [`PLANO-IMPLEMENTACAO-STUDIO.md`](PLANO-IMPLEMENTACAO-STUDIO.md) §4

## Critérios avaliados

| Critério | Resultado |
|----------|-----------|
| Hierarchy + select scene JSON | OK (C1) |
| Inspector + save instances | OK (C2) |
| Viewport = mesma verdade raylib do jogo | OK (C3) |
| Play in-process (World + systems) | OK (C4) |
| Open script → OS/IDE (`app.open_path`) | OK (C5) |
| Sem dual Three/raylib no destino | OK (política) |
| Tauri | Transição / bugfix only |

## Decisão

**GO:** o Studio canônico passa a ser o **shell nativo ImGui + raylib**, alinhado a Fyrox/Urho (editor = mesmo stack do jogo).

| Manter | Arquivar / não investir |
|--------|-------------------------|
| `demos/studio_shell` → evoluir para app Studio | Novos painéis Three no Tauri |
| Play AOT do sample como smoke packaging | Monaco como IDE principal |
| Tauri só se bloquear workflow legado | Polish docks Tauri |

## Próximos passos (pós-C)

1. **D1** ImGuizmo no viewport  
2. Extrair package/app `ori-studio` se o demo crescer  
3. Save de cena **com brushes** (writer completo)  
4. Pick no viewport (ray)  

## Não-GO teria sido se

- Viewport ImGui não pudesse mostrar solids/mesh reais  
- Play in-process corrompesse scene de edição sem isolamento  
- Dependência permanente de WebGL  

Nada disso se confirmou no spike C0–C5.
