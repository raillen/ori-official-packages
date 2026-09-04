# 05 — Mapas: brushes, faces e displacements

## Objetivo visual e de workflow

Mapas no espírito **Quake / Half-Life / Source-lite**:

- sólidos (brushes) editáveis;
- faces com material e UV;
- **displacements**: faces subdivididas cujos vértices se puxam;
- **não** landscape heightmap infinito como forma principal.

Open world futuro = **chunks de brushes/entities**, não um heightmap único.

## Conceitos

### Brush

Sólido convexo (v0: caixa/prismas; depois wedges etc.).

```text
Brush {
  id,
  transform,          // ou definido por planos
  faces[],            // material + UV + flags
  optional displacement on some faces
}
```

### Face

Polígono (ou plano clipado) com:

- material id
- UV projection (planar / atlas)
- flags: solid, invisible, sky, water, trigger-only, …

### Displacement (Source-lite)

Em **algumas** faces:

1. subdividir em grid (ex. 2×2 … 8×8);
2. permitir puxar vértices na normal (e opcionalmente tangente);
3. gerar mesh de render + collider (trimesh ou height patch local).

Isso dá terreno “artesanal” sem visual de open-world moderno.

## Pipeline

```text
Brushes (edição)
  → build mesh render (indexed)
  → build collision (compound / trimesh)
  → optional navmesh input
  → serialize map
```

### CSG (escada Hammer-like — ADR-0012)

| Nível | Descrição | Quando |
|-------|-----------|--------|
| **v0** | Brushes **Add** (caixas convexas); rooms por adjacência + grid | **Agora** (editor) |
| **v1** | **Subtract de caixas** (carve janela/porta AABB) | M1.5–M2 |
| **v2** | Subtract de brushes **convexos** (half-space / solid clip clássico) | M2+ |
| **v3** | Mais primitivas (wedge) + cleanup coplanar | depois |
| **fora cedo** | Mesh-boolean tri genérico | só se v2 não bastar |

Meta de sensação: **Hammer 2004** (sólidos + carve), implementação **tranquila**:
não recriar VBSP; preferir convexos + algoritmo de clip documentado.

Muitos editores amadores morrem no CSG perfeito. Preferir **brushes limpos + grid snap**
e carve **fail closed**.

### Primitivas no editor (v0 — implementado)

| Kind | Collider | Notas |
|------|----------|--------|
| **Box** | cuboid | blockout padrão |
| **Wedge** | convex hull | rampa / half-space visual |
| **Cylinder** | convex hull | eixo Y |
| **Sphere** | ball | radius = min(size)/2 |

| Ação | Comportamento |
|------|----------------|
| **+ Add** | Cria kind selecionado no grid |
| **Dup / Del** | Copia / remove brush selecionado |
| **LMB pick** | Seleciona brush por AABB |
| **Gizmo Move/Scale** | Eixos RGB no solid selecionado |
| **View** | Solid / Solid Lit / Wireframe / Solid+Wire |
| **Inspector** | Center/size com snap |
| **Grid** | Off / 0.25 / 0.5 / 1 m |

## Grid e snap

- Grid world configurável (ex. **0.25 / 0.5 / 1.0** m no editor; depois 16 uu se quiser escala clássica).
- Snap de center/size no grid.
- Essencial para look clássico e colliders estáveis.

## Entities no mapa

Além de geometria:

| Entity | Uso |
|--------|-----|
| `player_start` | spawn |
| `light_point` / `light_spot` | iluminação |
| `trigger_*` | volumes |
| `ai_spawn` / `path_node` | stealth / horda |
| `decal_author` | poça, mancha fixa |
| `water_volume` / `puddle` | água |
| `sky` / env | céu |

## Chunks

```text
Map
  chunks[]
    brushes[]
    entities[]
    bounds
```

v0: um chunk. Depois: streaming por distância da câmera.

## Formato

Sugestão: JSON ou RON versionado com `format_version`.

Requisitos:

- diffável o suficiente para git (aceitar verbosidade);
- validação fail-closed no load;
- bake artifacts (nav, light) em arquivos satélite.

## Ferramentas de editor (mapa)

| Tool | Função | Status |
|------|--------|--------|
| Create brush | caixa no grid | **Feito** |
| Extrude face | puxar face na normal | depois |
| Clip / knife (depois) | cortar brush | depois |
| Material paint | aplicar em faces | material por brush |
| Displacement (top Box) | subdiv power + raise/noise | **Feito** (Inspector) |
| Entity place | spawns e triggers | **Feito** |
| Measure / grid | produtividade | grid snap **Feito** |
| Surface water/snow | create brush + tags | **Feito** |
| Vertex light bake | Tools menu | **Feito** |

### Displacement v1 (implementado)

- Só **Box** + face **Top** (local +Y).
- `power` 1..=4 → segmentos `2^power`; heights row-major em RON.
- Inspector: Enable, Power, Raise center, Noise, Flatten.
- Mesh: top grid + paredes até a borda displaced; collider ainda AABB do brush.

## Relação com física e nav

- Collider gerado do mesh final (ou por brush convex hull).
- Navmesh bake usa walkable surfaces (filtros por angle/material).
- Superfícies (`Snow`, `Sand`, `Water`, `Rock`) vêm de material tags — ver
  [07-decals-billboards-superficies.md](07-decals-billboards-superficies.md).
