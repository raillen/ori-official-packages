# Play packaging — sample + Studio

> **Updated:** 2026-07-15  
> Makes `ori check` / Play reliable for the Studio sample and user projects
> that path-dep `ori_game`.

---

## Contract

| Item | Value |
|------|--------|
| Package root | Directory with `ori.pkg.toml` (and usually `ori.proj`) |
| Check / run target | **Always `.`** (package root) — never bare `main.orl` alone |
| Path dep | `ori_game = { path = "…", version = "0.3.0" }` |
| Native libs | Declared on **ori_game / raylib** packages that **stage** `lib/<triple>/*.a` — **not** on user projects without that layout |
| Play mode | **`ORI_USE_AOT=1`** (Studio forces this). JIT needs `.so` which is **not** staged today. |
| Windowed entry | `main()` **must** call `game.app.run_window(..., on_update, on_draw)` or the process exits with no window |
| Env | `ORI_GAME_ROOT` → ori-game repo root; `LIBRARY_PATH` for link diagnostics |

Studio Play (`play_runner` + `play_env`) enforces package-root check, AOT, and injects paths.

See also **[PRODUCT-REALITY.md](PRODUCT-REALITY.md)** — honest maturity.

---

## Sample project

```text
ori-game/assets/sample_project/
  ori.pkg.toml          -- path-dep ../../ (ori_game)
  project.ogame.toml    -- active_scene = engine_test
  main.orl              -- Play entry (loads scene vars + cowboy mesh)
  scenes/engine_test.scene.json
  entities/player/
  assets/models/brazilian-cowboy.glb
  assets/shaders/       -- PBR copies for relative load
```

Smoke:

```bash
# from ori-game
./tools/smoke_sample_play.sh
# → ori check . under sample_project with ORI_GAME_ROOT set
```

---

## Studio host

1. Open sample via **Open Sample** (resolves `ori-game/assets/sample_project`).
2. Play runs:
   - `ori check .` (cwd = project root)
   - diagnostics: `ORI_GAME_ROOT`, native lib dir, pkg present
   - `ori run .` with the same env
3. Preferences: optional override path to `ori` binary.

If `import game.*` fails: missing path-dep or wrong cwd (not package root).

### R2.3 — Play freshness (binary/source stamp)

In-process Studio Play warns when sample sources change since the last stamp:

| Piece | Path |
|-------|------|
| Module | `game.play_freshness` |
| Stamp file | `<project>/.ori_play_stamp` |
| CLI | `ori-game/tools/write_play_stamp.sh [project]` |
| Shell | **Play → Check/Write play stamp** (also on every Play start) |

Workflow: edit entity/scene scripts → `ori compile .` → Write play stamp → Play shows **fresh**.

---

## User project checklist

1. `ori.pkg.toml` with `ori_game` path (or registry when published).
2. `entry = "main.orl"` (or package entry of choice).
3. Stage raylib / shim under `ori-game/lib/<host-triple>/` (see `tools/setup_raylib_linux.sh`).
4. Optional: `export ORI_GAME_ROOT=/path/to/ori-game`.
5. `ori check .` then `ori run .` from project root.

---

## P2 — Export package (ship folder)

One-shot AOT export of a project + assets + staged native libs:

```bash
cd /home/raillen/Documentos/Projetos/game-engine-full/ori-game
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_AOT=1
./tools/export_game_package.sh
# or: ./tools/export_game_package.sh /path/to/project /tmp/my_export
```

Layout:

```text
export/
  bin/<name>          # AOT binary
  assets/ scenes/ entities/
  native/             # libraylib + shim (host triple)
  run.sh              # sets LD_LIBRARY_PATH and runs bin
  EXPORT.md
```

Run: `cd export && ./run.sh` (cwd must stay package root so relative scene paths work).

Not yet: installer, multi-OS fat package, code signing. Windows host: compile on MSVC + copy `lib/x86_64-pc-windows-msvc`.

---

## P4 — CI smoke Linux

Gate local (cluster monorepo):

```bash
cd /home/raillen/Documentos/Projetos/game-engine-full
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_AOT=1
./scripts/smoke_ci_linux.sh
# ou: cd ori-game && ./tools/smoke_ci_linux.sh
```

| Step | Script / comando |
|------|------------------|
| Sample Play | `tools/smoke_sample_play.sh` |
| Unit gate | `test_phase_g_l`, `test_camera_matrices`, level/tilemap/physics/multi_scene |
| Studio shell | `ori-imgui/demos/studio_shell/tools/smoke.sh` |
| Export | `tools/export_game_package.sh` |

Env: `ORI_CI_SHELL=0` skip shell · `ORI_CI_EXPORT=0` skip export · `ORI_CI_QUICK=1` fewer tests.

GitHub Actions (repo `ori-game`): `.github/workflows/smoke-linux.yml`  
- **core** job: sample + tests (bootstraps path-dep stubs)  
- **shell** job: `workflow_dispatch` ou repo var `ORI_CI_SHELL=1` (precisa monorepo / multi-checkout)

---

## Restart Studio after host changes

Tauri host rebuild is required when `play_env` / `play_runner` change:

```bash
cd ori-game-studio
pnpm tauri dev
```

Frontend-only (React) hot-reloads; Rust does not.
