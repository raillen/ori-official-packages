# ori-raylib

Standalone **L0** [raylib](https://www.raylib.com/) bindings for Ori (S3).

**Package:** `raylib` · **Module:** `raylib` · **Version:** 0.1.0

All C entry points are **`ori_rl_*`** (scalar ABI via `ori_raylib_shim`).  
Game helpers (`game.draw`, …) live in **`ori-game`**, which path-depends on this package.

## Layout

```
ori-raylib/
  raylib.orl                 # L0 extern surface
  native/ori_raylib_shim.c   # handle tables + wrappers
  tools/raylib_stub.c        # headless stub
  tools/setup_raylib_linux.sh
  tools/setup_raylib_windows.ps1
  lib/<triple>/              # staged static libs
```

## Stage (Linux)

```bash
./tools/setup_raylib_linux.sh --stub   # CI / no X11
# or real raylib if available:
./tools/setup_raylib_linux.sh
```

## Use

```toml
[package]
name = "my_app"
native_libs = ["ori_raylib_shim", "raylib"]

[dependencies]
raylib = { path = "../ori-raylib", version = "0.1.0" }
# or with game helpers:
ori_game = { path = "../ori-game", version = "0.3.0" }
```

```ori
import raylib as rl
-- rl.ori_rl_InitWindow(...)
```

## Smoke

```bash
ORI_BIN=ori ORI_USE_SYSTEM_LINKER=1 ./tools/smoke_linux.sh
```
