# ADR-0011 — Salvamento de projeto e modelo de cenas

- **Status:** Aceito
- **Data:** 2026-07-13

## Contexto

O editor precisa de contratos claros para:

1. o que **Save Project** grava;
2. como **cenas** se relacionam com **mapas** (brushes);
3. se o modelo é Godot-nodes ou Unity-like.

## Decisões

### Projeto

- Projeto = **pasta** versionável com `ori.engine.toml`.
- **Save Project** grava artefatos de autoria dirty: manifesto + **mapa ativo** + **cena ativa**.
- Save de **jogo** (runtime / slots) é outro pipeline (M2+), não misturar.
- Paths relativos sem `..`; load **fail closed**.

### Mapa vs cena

| Artefato | Conteúdo | Arquivo |
|----------|----------|---------|
| **Map** | Brushes / geometria de nível | `maps/*.map.ron` |
| **Scene** | Instances de entities (spawn, props, tags) | `scenes/*.scene.ron` |
| **Prefab** | Receita reutilizável | `prefabs/*` (M1) |

Manifesto referencia `active_map` e `active_scene`.

### Modelo de cena

- **Unity-like em espírito:** entities + parent/child + prefabs depois.
- **Não** Godot SceneTree com script por node.
- **Dados ≠ comportamento:** cena = dados; lógica = Ori em `src/`.

## Consequências

- Hierarchy do editor espelha a **cena** (não inventa entities só em RAM).
- Play carrega map + scene.
- Dirty separado ou unificado no projeto; Save limpa ambos.

## Alternativas rejeitadas

| Alternativa | Motivo |
|-------------|--------|
| Tudo num único level.ron | Dirty/merge ruins |
| Nodes Godot + script embutido | Conflita com Ori-only e ND-first |
| Só mapa, entities efêmeras | Perde autoria de spawns |
