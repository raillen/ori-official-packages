# Getting Started with ori-game

## Prerequisites

1. **Ori compiler** ≥ 0.2.0 (`ori --version`)
2. **System linker** for AOT: `build-essential` on Ubuntu
3. **Raylib static library** for your triple:
   - Linux: `./tools/setup_raylib_linux.sh` (real build) or `--stub` for headless smoke
   - Windows: `lib/x86_64-pc-windows-msvc/raylib.lib` is already vendored

## Add the package

```toml
# my_game/ori.pkg.toml
[package]
name = "my_game"
version = "0.1.0"
entry = "main.orl"
ori_version = "0.2.0"

[dependencies]
ori_game = { path = "../ori-game", version = "0.1.0" }
```

Package **name** is `ori_game`. Import **namespaces** are `game.*` and `raylib`.

## First window

```ori
namespace app.main

import game.app as app
import game.color as color
import game.draw as draw
import ori.math.vec2 as vec2

func update(_dt: float) -> void
end

func draw_frame() -> void
    draw.draw_text("Hello, Ori!", vec2.Vec2(x: 280.0, y: 280.0), 32, color.WHITE)
end

func main()
    app.run_window("Hello", 800, 600, update, draw_frame)
end
```

```bash
ori compile main.orl --out my_game
./my_game
```

## Engine tick (entities)

```ori
import game.engine as engine
import ori.math.vec2 as vec2

func main()
    var eng: engine.Engine = engine.with_builtins(engine.create_engine())
    eng = engine.spawn_entity(eng, vec2.Vec2(x: 0.0, y: 0.0))
    var e: engine.Entity = eng.entities[0]
    e.has_gravity = true
    e.vel = vec2.Vec2(x: 10.0, y: 0.0)
    eng = engine.set_entity(eng, 0, e)
    eng = engine.tick(eng, 0.016)
end
```

Entity fields: `id`, `pos`, `vel`, `alive`, `has_gravity`, `has_collision`, `tag`.  
Mutators return a new `Engine` / `Entity` (parameters are immutable).

## Input helpers

```ori
import game.input as input

if input.key_down(input.KEY_RIGHT)
    -- move
end
```

## Smoke on Linux

```bash
cd ori-game
./tools/setup_raylib_linux.sh --stub
./tools/smoke_linux.sh
```

## Next

- Examples under `examples/` (platformer, physics, dialogue, …)
- API map: [api.md](api.md)
- Deep analysis: [ANALISE-COMPLETA.md](ANALISE-COMPLETA.md)
