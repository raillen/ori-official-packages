# ADR-0013 — Object Types estilo GameMaker

- **Status:** Aceito
- **Data:** 2026-07-13

## Contexto

O modelo `behaviours[]` + módulo global de jogo é correto para AOT e
cena=dados, mas confuso para quem vem de Unity/GameMaker (“onde coloco o
script do objeto?”).

## Decisão

Adotar o **modelo mental GameMaker**:

| Conceito | Ori Game Engine |
|----------|-----------------|
| Object | **Kind / Object Type** (`kinds/<id>.kind.ron`) |
| Instance | **Entity** na cena com `kind = id` |
| Room | **Scene** |
| Object Events | Create / Step / Destroy / **Collision** (dispatch por kind) |
| Object Events | Create / Step / Destroy / Alarm (dispatch por kind) |
| Object Variables | vars no kind + overrides na instance |
| Alarms | 4 slots por instance (`alarm_set` / `on_alarm`) |
| Parent object | `parent_kind` (opcional) |

**Não** script por instância (Godot).  
**Não** MonoBehaviour genérico Unity no v1.

`behaviours[]` passa a ser **engine modules** (systems do host: health,
spawner, …), subset opcional dos defaults do kind.

## Consequências

- UX: Place Object / Object Type no Inspector.
- Runtime: dispatch Create→Step→Destroy por instance, ordenado por stable_id.
- Código do tipo: `kinds/<id>.orl` (check agora; execução Ori = EB-6).
- Até dylib: shim Rust `KindBehaviour` com o mesmo contrato de eventos.
- **Parent kind:** defaults (tags/behaviours/vars/mesh) são achatados no
  catálogo; **eventos** andam na cadeia em runtime (ver abaixo).

## Parent kind — cadeia de eventos (decisão)

| Aspecto | Escolha |
|---------|---------|
| Ordem | **Filho primeiro, depois pai** (`child → parent → …`) |
| Quando | Cada kind da cadeia que **declara** o evento em `events[]` e tem handler |
| Eventos v1 | Create / Step / Destroy (mesma ordem nos três) |
| Ciclo / profundidade | Soft-fail: warning no catálogo; depth ≤ 8 (`MAX_PARENT_DEPTH`) |
| Defaults | Merge no load (`resolve_parents`); child sobrescreve var de mesmo nome |

**Por quê child→parent:** espelha o padrão GameMaker em que o evento do
objecto filho corre e, em seguida, a lógica partilhada do pai (equivalente a
chamar `event_inherited` no fim do evento filho). Assim o filho pode preparar
estado (ex.: `health_ensure` com HP próprio) antes do pai aplicar setup
comum. A alternativa “só pai se o filho não tiver handler” foi rejeitada:
ambos podem definir o mesmo evento e ambos correm.

API: `KindCatalog::parent_event_chain_for` + `KindDispatcher::dispatch_chain`.

Demo: `kinds/enemy_base.kind.ron` + `enemy_grunt` com
`parent_kind: Some("enemy_base")`.

## Alternativas rejeitadas

| Alternativa | Motivo |
|-------------|--------|
| Script `.orl` por entity | Confusão × N, AOT, cena poluída |
| MonoBehaviour livre | Custo alto, ND-first fraco |
| Manter só behaviours sem kind asset | Continua ilegível para o autor |
| Parent-only se child sem handler | Perde lógica partilhada quando ambos declaram o evento |
| Parent → child automático | Contraria o hábito GM de specialized-first + inherited |

## Relacionados

- ADR-0001 (Ori-only)
- ADR-0011 (projeto/cenas)
- Guia: `docs/guides/object-types-gamemaker.md`
