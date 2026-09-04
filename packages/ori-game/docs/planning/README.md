# Planning — ori-game (canônico no cluster)

Estes `.md` são a **fonte canônica** dos planos de Studio/runtime no monorepo de games.

**Cluster root:** `/home/raillen/Documentos/Projetos/game-engine-full/`  
**Espelho** (mesmos arquivos): `game-engine-full/docs/planning/`

```bash
# após editar aqui:
../../scripts/sync_planning.sh
# ou a partir da raiz do cluster:
./scripts/sync_planning.sh
```

| Doc | Papel |
|------|--------|
| [**PLANO-IMPLEMENTACAO-STUDIO.md**](PLANO-IMPLEMENTACAO-STUDIO.md) | **Canônico A→M** + checkboxes (§4; fila atual = **G**) |
| [PLANO-IMPLEMENTACAO-FALTANTES.md](PLANO-IMPLEMENTACAO-FALTANTES.md) | Narrativa / sprints dos faltantes (sem checkbox rival) |
| [PLANO-AMADURECIMENTO-ENGINE.md](PLANO-AMADURECIMENTO-ENGINE.md) | Maturidade motor / G1 |
| [FEATURE-CATALOG.md](FEATURE-CATALOG.md) | Features + pipeline retro + protótipos (Auk → Ori) |
| [PRODUCT-REALITY.md](PRODUCT-REALITY.md) | Honestidade de status |
| [STUDIO-PRODUCT-DECISIONS.md](STUDIO-PRODUCT-DECISIONS.md) | Decisões de produto |
| [ROADMAP-GAME-ECO.md](ROADMAP-GAME-ECO.md) | ECO + catálogo ImGui |
| [IMGUI-EXTENSIONS-RANKING.md](IMGUI-EXTENSIONS-RANKING.md) | Rank extensões ImGui |
| [STUDY-ENGINES-DEEP.md](STUDY-ENGINES-DEEP.md) | Estudo source 4 engines |

**Specs de implementação (modulares, fora deste espelho):**  
[`docs/specs/`](../../../docs/specs/) · índice de módulos [`MODULES.md`](../../../docs/specs/MODULES.md) ·  
primeiro módulo: [`studio-shell/`](../../../docs/specs/studio-shell/).
