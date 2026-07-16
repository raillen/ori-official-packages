# Estudo de engines — Godot / Unity / GameMaker

> Companion de [`PLANO-AMADURECIMENTO-ENGINE.md`](PLANO-AMADURECIMENTO-ENGINE.md).  
> **Clone local Godot:** `/home/raillen/Documentos/Projetos/game-engine-full/study/godot`  
> (shallow `git clone --depth 1` · ~430 MB · MIT)  
>  
> **Log vivo (append obrigatório ao ler source):**  
> [`study/SOURCE-LEARNINGS.md`](../../../study/SOURCE-LEARNINGS.md) — journal + catálogo `P-*`.

---

## 1. Godot (source local)

### 1.1 Layout (o que mapear)

| Path no clone | Papel | Relevância Ori |
|---------------|--------|----------------|
| `core/` | Object, Variant, OS, templates | Registro de tipos / IDs |
| `servers/` | Rendering, Physics, Audio, Text, Navigation… | “Serviços” finos (raylib = display/audio L0) |
| `scene/main/` | `Node`, `SceneTree`, `Viewport`, `Window` | Hierarchy + process loop |
| `scene/resources/` | `Resource`, loaders | entity.json / scene / assets |
| `scene/3d/` | Node3D, Camera3D, MeshInstance… | analogia draw3d / camera3d |
| `scene/2d/` | Node2D, Sprite2D, TileMap… | tilemap / sprites |
| `editor/` | Editor inteiro | Studio (Tauri ou ImGui) — **ler ideias, não portar** |
| `modules/gdscript/` | Linguagem de script | Ori é o nosso GDScript |
| `main/` | Entry, MainLoop | `game.app.run_window` |
| `drivers/` | GL/Vulkan backends | fora do G1 |

### 1.2 Ideias a extrair (não copiar código)

1. **SceneTree process:** `_process` / `_physics_process` por nó → nosso Create/Step.  
2. **Resource vs Node:** dados serializáveis vs runtime.  
3. **Servers:** API de baixo nível sem misturar com árvore de cena.  
4. **RID:** handles opacos (`ModelId`, `TextureId`).  
5. **Editor observa a mesma scene tree** do runtime (nosso gap: Three ≠ raylib).

### 1.3 Como estudar sem se perder

```bash
cd /home/raillen/Documentos/Projetos/game-engine-full/study/godot
# Árvore de nós e loop
ls scene/main/
# Physics server surface
ls servers/physics_3d/ servers/physics_2d/ 2>/dev/null | head
# Editor scene docks (inspiração UI)
ls editor/scene/ | head
```

Perguntas por arquivo-chave: ver §5 do plano de amadurecimento.

### 1.4 Atualizar o clone

```bash
cd /home/raillen/Documentos/Projetos/game-engine-full/study/godot && git pull --depth 1
```

---

## 2. Unity (docs, sem source)

| Tópico | Onde ler | Pegar para Ori |
|--------|----------|----------------|
| Ordem de execução | [Execution Order](https://docs.unity3d.com/Manual/ExecutionOrder.html) | Player loop G1 |
| GameObject/Component | Manual “The GameObject” | Instance + vars + 1 script (sem ECS) |
| Physics | Colliders / Rigidbody | layers + resolve + (depois) box2d/jolt |
| Camera | Cameras overview | follow + editor orbit |
| Prefabs | Prefab workflow | Entity folder |
| DOTS | Entities package | **adiado** |

Filosofia útil: **ordem de frame previsível** e composição;  
filosofia a evitar no G1: DOTS, multi-pipeline, package hell.

---

## 3. GameMaker (docs / mental model)

| Conceito | Equivalente Ori |
|----------|-----------------|
| Object | Entity |
| Instance | scene instance |
| Create / Step / Draw / Alarm / Collision | handlers + dispatch |
| Room | scene.json |
| GML | .orl |
| Paths / timelines | post-G1 |

Filosofia: **simplicidade de autor** — o que o instance faz a cada event.

---

## 4. Decisões de arquitetura (congeladas para G1)

| Tema | Decisão |
|------|---------|
| Modelo de mundo | Entity + Instance + Scene JSON (GM+Unity light) |
| Loop | Ordem fixa documentada (Unity light) |
| Render L0 | raylib (server fino) |
| Script | Ori only |
| ECS | não no G1 |
| Editor | Tauri agora; spike ImGui se viewport bloquear G1 |
| Estudo Godot | ideias/API mental; reimplementar em Ori |

---

## 5. Relação com o plano

Backlog e fases: **`PLANO-AMADURECIMENTO-ENGINE.md`**.  
Realidade atual: **`PRODUCT-REALITY.md`**.


---

## Base de estudo local (4 engines)

Raiz: `game-engine-full/study/` (shallow clones).

| Pasta | Foco |
|-------|------|
| `godot/` | SceneTree, servers, Resource |
| `trenchbroom/` | Brushes, extrude, mapa 3D |
| `rbfx/` | Scene 3D, components, subsystems |
| `fyrox/` | Scene graph 3D + editor nativo |

Ver `study/README.md` e `DEV-HANDOFF.md` na raiz do cluster.

### Análise profunda (código + o que trazer)

Documento canônico da sessão de estudo source-level:

**[`STUDY-ENGINES-DEEP.md`](STUDY-ENGINES-DEEP.md)** — Godot / TrenchBroom / rbfx / Fyrox  
(arquivos-chave no clone, filosofia, matriz trazer/adiar/nunca, mapa ECO, fases B–F).
