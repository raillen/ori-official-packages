# ori-game

Game helpers and [raylib](https://www.raylib.com/) bindings for the
[Ori](https://github.com/raillen/ori-lang) language (**S3 / 0.3.x**).

**Package name:** `ori_game` · **Modules:** `raylib`, `game.*`  
**Requires:** Ori ≥ 0.3.0 · system linker for AOT · Raylib static lib (or headless stub)

## Features

- Window loop helper (`game.app`) + manual frame escape hatch
- Entity store + gravity/movement tick (`game.engine`)
- Raylib dual surface: **L0** `raylib` (PascalCase scalar FFI) + **L1** `game.*` (snake_case Ori)
- 2D: draw (shapes, text, textures), input, audio (sound + music), camera2D
- 3D: `game.camera3d` + `game.draw3d` (cube/sphere/grid/plane/ray + model load/draw)
- Handle aliases: `game.ids` (`TextureId`, `ModelId`, …)
- Collision, physics, tilemap, particles, dialogue, inventory, save (in-memory)
- Gameplay mechanics: jumps, dash, movement, combat, AI helpers, …


## Layers (L0 / L1)

| Layer | Module(s) | Naming | Types |
|-------|-----------|--------|-------|
| **L0** | `raylib` | PascalCase (C ABI, scalarized) | ints/floats/handles |
| **L1** | `game.draw`, `game.camera`, `game.draw3d`, `game.camera3d`, … | **snake_case** (spec 0.3.x) | `Color`, `Vec2`/`Vec3`, aliases |

Normative Ori style (`docs/spec/02-lexical.md`): functions/modules = **snake_case**,
types = **PascalCase**, visibility = **`public`** (not `pub`).

## Gameplay layer (trilha O)

| Module | Role |
|--------|------|
| `game.tween` | Easings + `Tween` over time |
| `game.scene` | Named screen stack (`push` / `pop` / `replace`) |
| `game.assets` | Pure path→handle cache |
| `game.asset_loader` | Raylib `LoadTexture` / `LoadSound` + unload; `load_*_as` logical keys |
| `game.rres_assets` | ORPK (`rres.pack`) → extract → AssetCache (path-dep `rres`) |
| `game.ldtk` | LDtk project JSON (IntGrid / Entities / Tiles) |
| `game.spine` | Spine JSON bones + rotate/translate timelines (milli-int pose) |
| `game.save` | In-memory checkpoints + `save_json` / `load_json` files |

Example: `examples/scene_menu.orl` (menu → play → pause).

## Install (path)

```bash
# Linux: stage libori_raylib_shim.a + libraylib.a
./tools/setup_raylib_linux.sh          # real raylib + scalar shim (if available)
# or headless CI:
./tools/setup_raylib_linux.sh --stub   # ori_rl_* no-ops + dummy libraylib
```

Native link: `ori_raylib_shim` then `raylib` (see `ori.pkg.toml`). All C entry
points are **`ori_rl_*`** — never call raw raylib struct ABI from Ori.

```toml
# your_game/ori.pkg.toml
[package]
name = "my_game"
version = "0.1.0"
entry = "main.orl"
ori_version = "0.3.0"

[dependencies]
ori_game = { path = "../ori-game", version = "0.2.0" }
```

## Quick start (S3)

```ori
module app.main

imports
    game.app = app
    game.color = color
    game.draw = draw
    game.input = input
    ori.math.vec2 = vec2
end

var px: float = 400.0
var py: float = 300.0

on_update(dt: float) -> void
    if input.key_down(input.KEY_RIGHT)
        px = px + 200.0 * dt
    end
end

on_draw() -> void
    draw.draw_circle(vec2.Vec2 { x: px, y: py }, 16.0, color.RED)
end

main()
    app.run_window("My Game", 800, 600, on_update, on_draw)
end
```

## Layout

| Path | Role |
|------|------|
| `raylib.orl` | Low-level raylib `extern c` API |
| `color.orl` / `shape.orl` / `collision.orl` | Shared types + geometry |
| `game/app.orl` | `run_window` loop |
| `game/engine.orl` | Entities, gravity/movement tick |
| `game/input.orl` · `draw.orl` · `camera.orl` · `audio.orl` | Wrappers |
| `game/physics.orl` · `tilemap.orl` · `particles.orl` · … | Systems |
| `game/mechanics/*` | Plug-in gameplay helpers |
| `lib/<triple>/libraylib.a` | Native raylib (or headless stub) |
| `examples/` | Demos (`hello_game`, `app_smoke`, `simple_game` are canonic for smoke) |
| `tests/` | `@test` unit tests |

## Develop

```bash
# typecheck lib
for f in raylib.orl color.orl shape.orl collision.orl game/*.orl game/mechanics/*.orl; do
  ori check "$f" || exit 1
done

# unit tests
ori test tests/test_engine.orl
ori test tests/test_shapes.orl
# …

# full Linux smoke (trilha G)
./tools/smoke_linux.sh
```

## Syntax (S3)

- `module path` (not `namespace`)
- `import path = alias` (not `as`)
- Declarations without `func` keyword: `public name(...) -> T`
- Struct literals: `Type { field: value }`
- Generics: `list[T]` (not `list of T`)

## Status

Trilha **G** (S3 adapt + raylib 2D link + smoke): see `CHANGELOG.md`.  
Further ECO work (raygui, Box2D, Jolt, …) lives in the language planning doc
`eco-game-imgui-raylib3d-plan.md` (ori-lang repo).
