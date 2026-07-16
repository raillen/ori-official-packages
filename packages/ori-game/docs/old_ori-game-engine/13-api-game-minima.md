# 13 — API mínima `game.*` / `ori_engine_*`

Esboço de contrato host ↔ gameplay Ori.  
Nomes finais em **inglês** no código; este doc descreve o desenho.

Status: **API v4** — `GameHostApi` ND-first (input, spawn/despawn, combat,
vfx/audio, HUD, save, inv, **object variables `var_*`**, **alarms `alarm_*`**);
guest = intenção; host systems por behaviours.
E2E: [guides/host-api-e2e.md](guides/host-api-e2e.md).  
**EB-6 host load:** feito (`libloading` + `OriHostApiV1` + sample) —
ver [guides/eb6-dylib-abi.md](guides/eb6-dylib-abi.md).  
**Ori compiler → dylib** (`.orl` export): ainda aberto (toolchain).

---

## Princípios

1. **ND-first (obrigatório):** a API deve ser **extremamente fácil de entender e
   de escrever**. Poucos conceitos, nomes óbvios, pouca cerimônia, exemplos
   curtos. Se uma chamada exigir “manual mental”, redesenhar. Público-alvo
   inclui neurodivergentes (mesma filosofia Ori).
2. API **pequena e estável** > espelho de tudo que o Rust tem.
3. **Uma forma óbvia** de fazer a tarefa comum (move, spawn, play sound).
4. IDs opacos (`EntityId`, …) — sem ponteiros crus no Ori.
5. Erros: `result` / códigos claros; sem panic no host por input de jogo.
6. Determinismo onde possível em fixed update.
7. Versionar (`ORI_ENGINE_API_VERSION`).
8. Preferir verbos de domínio: `player_move`, `spawn_prefab`, `play_sound_at`
   em vez de APIs genéricas do tipo `world_invoke("...")`.

### Anti-padrões da API

| Evitar | Preferir |
|--------|----------|
| 8 parâmetros sem nome semântico | 2–4 args claros ou struct documentada |
| Nomes genéricos (`do`, `exec`, `handle`) | Verbo + objeto (`damage_apply`) |
| Side effects invisíveis | Nome que diz o efeito |
| Dois jeitos de spawn | Um `spawn` + dados no prefab |

---

## Lifecycle

| Symbol / hook | Descrição |
|---------------|-----------|
| `game_init()` | cena carregada |
| `game_fixed_update(dt: float)` | física/gameplay fixo |
| `game_update(dt: float)` | frame |
| `game_shutdown()` | unload |

---

## Entities e transforms

| API | Status |
|-----|--------|
| `spawn_prefab` / `spawn_prefab_at` → id? | ✅ |
| `entity_despawn(id)` | ✅ |
| `transform_get/set_position` | ✅ |
| `scene_has_behaviour/kind` | ✅ |
| `entity_count`, `first_entity_with_*`, `nearest_with_tag` | ✅ |
| `damage_apply`, `health_get/max`, `health_ensure` | ✅ |
| `vfx_spawn`, `audio_play_2d/3d` | ✅ (queue; kira depois) |
| `ui_set_bar`, `ui_set_text` | ✅ |
| `save_game` / `load_game` / `list_saves` | ✅ RON slots |
| `inv_add` / `inv_remove` / `inv_has` | ✅ |
| `input_axis` / `input_button` (+ actions) | ✅ |
| `player_move` / `player_position` / `raycast` | ✅ |
| `var_get_f` / `var_set_f` / `var_get_i` / `var_set_i` / `var_get_bool` / `var_set_bool` / `var_get_str` / `var_set_str` | ✅ Object Variables (sessão Play; defaults do kind; missing → `None` / set `false` se entity sumiu) |
| `alarm_set(entity, index, seconds)` / `alarm_get(entity, index)` | ✅ Alarms lite (4 slots `0..=3` por instance; sessão Play; `seconds < 0` cancela; NaN/inf → set `false`; `0.0` fire no próximo fixed tick; fire → `on_alarm`) |
| `transform_set_rotation` / parent API | 🔲 |

Attach: `behaviours: [id,…]` + tags `prefab:<name>` no spawner.  
Registry: `player_controller`, `health`, `static_prop`, `spawner`, `ai_agent`.  
**Console:** Check Ori. **ORI_BIN** override do path do CLI.

### Prefabs (EB-5)

Arquivo: `prefabs/<name>.prefab.ron` (ex.: `prop_crate.prefab.ron` → nome `prop_crate`).

Campos: `format_version`, `name?`, `kind`, `transform`, `tags`, `mesh_path?`,
`behaviours`, `children` (mesma shape de node: name/kind/transform/tags/
mesh_path/behaviours/children). **Sem** path `.orl` em behaviours ou mesh.

```text
// host / guest
if game.spawn_prefab("prop_crate") {
    // Hierarchy mostra root + children com stable ids únicos
}
```

`spawn_prefab` retorna `false` se o nome não existir no catálogo do projeto.
Transform default do arquivo; offset opcional fica no runtime Rust
(`SceneWorld::spawn_prefab(..., Some(offset))`) — a API ND v0 só passa o nome.

---

## Input

| API | Descrição |
|-----|-----------|
| `input_axis(name) -> float` | "move_x", "move_y" |
| `input_button(name) -> bool` | held |
| `input_button_down(name) -> bool` | edge |

Bindings configuráveis no host.

---

## Physics / queries

| API | Descrição |
|-----|-----------|
| `raycast(origin, dir, max_dist, mask) -> Hit?` | **1ª classe** — picking, LOS, hitscan |
| `raycast_all(...)` | opcional |
| `overlap_sphere(center, r, mask) -> list` | |
| `character_move(id, velocity)` | controller host |
| `ground_info(id)` | tag + normal + sink |
| Colliders: edit via dados/prefab (box/sphere/capsule…) | inspector + load |

## Save / inventário / UI (esboço)

| API | Descrição |
|-----|-----------|
| `save_game(slot)` / `load_game(slot)` / `list_saves()` | ND-first |
| `inv_add` / `inv_remove` / `inv_has` / `inv_equip` | inventário |
| `ui_set_text` / `ui_set_bar` / … | HUD data-driven (U0) |

---

## Combat / gameplay commons

| API | Descrição |
|-----|-----------|
| `damage_apply(target, amount, source, kind)` | |
| `health_get / health_set` | |
| `hitbox_enable(id, name, on)` | |

---

## VFX / audio

| API | Descrição |
|-----|-----------|
| `vfx_spawn(name, pos, rot)` | |
| `decal_spawn(name, pos, normal, ttl)` | |
| `audio_play_2d / 3d(name, pos?)` | |

---

## AI helpers (host-side brains)

| API | Descrição |
|-----|-----------|
| `ai_set_state(id, state_name)` | |
| `ai_blackboard_set/get` | tipos limitados |
| `nav_path(id, to) -> path handle / points` | |
| `perception_can_see(viewer, target) -> bool` | |

---

## Time / world

| API | Descrição |
|-----|-----------|
| `time_dt / time_fixed_dt / time_scale` | |
| `world_get_time_of_day()` | 0–1 ou horas |
| `debug_draw_line / text` | só dev builds |

---

## Exemplo mental (Ori)

```ori
// ilustrativo — sintaxe sujeita à spec ori-lang

namespace game.player

func game_fixed_update(dt: float) -> void
    // ler input, mover character, etc.
end
```

A forma exata de export de hooks depende do design de binding (símbolos C
vs tabela de entry). Decidir em ADR de binding antes do M0 fechar.

---

## Não-objetivos da API v0

- Reflexão total de components
- ECS queries arbitrárias do Ori
- Criar brushes em runtime (mapa é dado)
- Trocar backend de física do script
