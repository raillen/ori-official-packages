# ori-official-packages

Official **Ori** libraries and packages maintained by the language team.

| | |
|--|--|
| **Language** | [ori-lang](https://github.com/raillen/ori-lang) |
| **Public registry (OriLamp)** | [ori-lamp](https://github.com/raillen/ori-lamp) |
| **Web stack (Lantern)** | [ori-web-framework](https://github.com/raillen/ori-web-framework) |

## Layout

```text
packages/
  ori-web/              # name = "web"
  ori-templates/        # name = "templates"
  ori-raylib/           # name = "raylib"
  ori-sqlite/           # name = "sqlite"
  …
```

Each package has `ori.pkg.toml`. The **package name** in the manifest is without
the `ori-` prefix (import / registry id). Folder names keep the `ori-` prefix
for GitHub discoverability.

## Install (registry)

```bash
export ORI_REGISTRY=https://<your-orilamp-host>/registry
ori install raylib@0.1.0
ori install web@0.1.0
```

## Local path deps

```toml
# ori.proj
[dependencies]
raylib = { path = "../packages/ori-raylib", version = "0.1.0" }
web = { path = "../packages/ori-web", version = "0.1.0" }
```

## Categories

| Category | Packages |
|----------|----------|
| **Web (Lantern)** | `web`, `templates`, `web_app`, `web_auth`, `web_session_sqlite`, `web_framework` |
| **Storage** | `sqlite` |
| **Graphics / game** | `raylib`, `ori_game`, `raygui`, `rres`, `freetype`, `harfbuzz`, `stb`, `cgltf`, `fast_obj`, `noise` |
| **Physics** | `box2d`, `jolt`, `recast` |
| **Systems** | `enet`, `physfs`, `lz4`, `miniz`, `nfd`, `tracy`, `enkits` |
| **UI (ImGui family)** | `imgui`, `implot`, `implot3d`, `imnodes`, `imguizmo`, `clay`, extras… |

## Notes

- Prebuilt native libs live under each package’s `lib/` when available.
- Upstream C/C++ trees (`vendor/`) and large demos are **not** shipped here;
  rebuild scripts remain under `tools/` where present.
- Source of truth for day-to-day web work: `ori-web-framework`.
- Source of truth for game/FFI bindings during development: `game-engine-full`
  (local monorepo). This repo is the **publishable / official** snapshot.

## License

Per-package (see each `ori.pkg.toml` / README). Most packages are MIT;
exceptions (e.g. Tracy) keep their upstream licenses.
