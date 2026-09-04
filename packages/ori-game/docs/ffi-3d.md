# Raylib FFI design (R0) — ori-game

## Dual surface

| Layer | Module | Naming | Role |
|-------|--------|--------|------|
| **L0** | `raylib` | `ori_rl_*` C symbols | Scalarized ABI; no C struct layout in Ori |
| **L1** | `game.*` | snake_case | `Color`, `Vec2`/`Vec3`, handle aliases |

## Why `ori_rl_*`

Raw raylib passes `Vector2`/`Vector3`/`Color`/`Camera3D`/`Model` **by value**.
Ori `extern c` cannot safely mirror that layout yet.

Policy:

1. **Opaque `int` handles** for `Texture`, `Sound`, `Music`, `Model`, `Font` (tables in the shim).
2. **Scalars** for vectors, colors, and camera fields.
3. **Prefix `ori_rl_`** so shim + real `libraylib.a` link without symbol clashes.
4. Stub CI implements the same `ori_rl_*` surface as no-ops.

## Link line

```
native_libs = ["ori_raylib_shim", "raylib"]
```

| Mode | `libori_raylib_shim.a` | `libraylib.a` |
|------|------------------------|---------------|
| **stub** (`setup --stub`) | no-op `ori_rl_*` | dummy symbol |
| **desktop** | pack/unpack → raylib | real static raylib |

## MVP inventory (done)

- Window / frame / input / audio (2D path)
- Mode2D / Mode3D
- Primitives 2D + 3D (cube, sphere, plane, grid, ray)
- Textures, models (load/draw/unload)
- Not yet: UpdateCamera, GetMouseRay, materials, shaders, skeletal anim

## Files

- `raylib.orl` — L0 declarations
- `native/ori_raylib_shim.c` — real wrappers
- `tools/raylib_stub.c` — headless
- `tools/setup_raylib_linux.sh` — stage libs under `lib/<triple>/`
