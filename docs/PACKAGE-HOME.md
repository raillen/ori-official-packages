# Package home policy (canonical)

**Date:** 2026-07-16  
**Rule:** every Ori package we develop and publish lives **only** under this
repository: [`ori-official-packages`](https://github.com/raillen/ori-official-packages).

## Single home

| Location | Role |
|----------|------|
| **`ori-official-packages/packages/*`** | **Canonical** source + release snapshot |
| `ori-web-framework` | **Product monorepo** for Lantern (web); packages are mirrored **into** official-packages for registry |
| `game-engine-full` | **Lab only** (FFI/game experiment). Not the publish home. Promote → official-packages when ready |
| `ori-lang/packages/` | **Forbidden** for real package trees — pointer README only |
| Root of `~/Documentos/Projetos/` | **Forbidden** for loose `ori-*` package clones |

## Rules

1. **No package trees** under `ori-lang/packages/` (no `ori-web/`, `ori-templates/`, …).
2. **No** top-level `~/Documentos/Projetos/ori-raylib` style clones.
3. New binding/library → create under `ori-official-packages/packages/ori-<name>/` with `ori.pkg.toml`.
4. Registry (OriLamp) reads / syncs from **this** repo (or tarballs built from it).
5. Lab work in `game-engine-full` must be **copied or PR’d** here before anything is “official”.

## Consumers

Apps and demos depend via:

```toml
# preferred: registry
sqlite = "0.3.0"

# or path during monorepo work
sqlite = { path = "../ori-official-packages/packages/ori-sqlite", version = "0.3.0" }
```

Lantern day-to-day: path into `ori-web-framework`; release/registry: same packages under this repo.

## Web (Lantern) note

| Package id | Folder | Day-to-day | Official snapshot |
|------------|--------|------------|-------------------|
| `web`, `templates`, … | `ori-web`, … | `ori-web-framework` | `ori-official-packages/packages/` |

Do not keep a third copy inside `ori-lang`.
