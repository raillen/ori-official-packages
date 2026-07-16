# Phase OS — multi-OS staging for ECO packages

> **Status:** **P1 Windows scripts polished** · **P3 macOS scaffolding ready** (2026-07-15)  
> Execution of Win/mac still requires a **real host**. Linux-5 remains the CI gate.  
> **Validate without host:** `ori-game/tools/validate_os_scripts.sh`

## Goal

| Triple | Priority | Status |
|--------|----------|--------|
| `x86_64-unknown-linux-gnu` | done | **Linux-5** + CI (`smoke_ci_linux.sh`) |
| `x86_64-pc-windows-msvc` | **P1** | **scripts polished** — execute on MSVC host |
| `x86_64-apple-darwin` / `aarch64-apple-darwin` | **P3** | **scaffold** — `smoke_macos` / `smoke_eco_macos` |

## P1 — Windows (MSVC)

### Prerequisites

1. **Visual Studio 2022 Build Tools** (*Desktop development with C++*)
2. **cmake** on PATH (Box2D / optional real Jolt)
3. **ori.exe** ≥ 0.3.x with staged `runtime/x86_64-pc-windows-msvc/`
4. Shell: **x64 Native Tools Command Prompt** *or* scripts auto-load via `vswhere`

```powershell
$env:ORI_BIN = "C:\path\to\ori.exe"
$env:ORI_USE_SYSTEM_LINKER = "1"
cd C:\path\to\ori-game
.\tools\smoke_eco_windows.ps1 -Stub          # first run (raylib stub)
.\tools\smoke_eco_windows.ps1                # full when libs staged
.\tools\smoke_eco_windows.ps1 -ValidateOnly  # inventory only
```

### Naming (MSVC)

Package `native_libs = ["foo"]` → **`foo.lib`** under `lib/x86_64-pc-windows-msvc/`.

| Linux | Windows |
|-------|---------|
| `libori_raylib_shim.a` | `ori_raylib_shim.lib` |
| `libraylib.a` | `raylib.lib` |
| `libori_box2d_shim.a` | `ori_box2d_shim.lib` |
| `libbox2d.a` | `box2d.lib` |
| `libori_enet_shim.a` / `libenet.a` | `ori_enet_shim.lib` / `enet.lib` |
| `libori_jolt_shim.a` / `libJolt.a` | `ori_jolt_shim.lib` / `Jolt.lib` |

### Per-package scripts (core 8)

| Package | Build | Smoke |
|---------|-------|-------|
| **ori-game** | `tools/setup_raylib_windows.ps1` [`-Stub`] | `tools/smoke_windows.ps1` [`-Stub`] |
| **ori-box2d** | `tools/build_windows.ps1` | `tools/smoke_windows.ps1` (**dynamics.orl**) |
| **ori-jolt** | `tools/build_windows.ps1` (stub default) | `tools/smoke_windows.ps1` (**dynamics** + character examples) |
| **ori-sqlite** | `tools/build_windows.ps1` | `tools/smoke_windows.ps1` |
| **ori-rres** | `tools/build_windows.ps1` | `tools/smoke_windows.ps1` |
| **ori-imgui** | `tools/build_windows.ps1` (stub host) | `tools/smoke_windows.ps1` |
| **ori-raygui** | `tools/build_windows.ps1` | `tools/smoke_windows.ps1` |
| **ori-enet** | `tools/build_windows.ps1` | `tools/smoke_windows.ps1` (5 demos) |

Umbrella also probes (SKIP if missing): nfd, imguizmo, imnodes, implot, imgui-extras.

### Checklist (execute on Windows)

| # | Package | Build | Smoke | Notes |
|---|---------|-------|-------|-------|
| 1 | ori-game | [ ] | [ ] | Start with `-Stub` |
| 2 | ori-box2d | [ ] | [ ] | needs cmake · module `dynamics` |
| 3 | ori-sqlite | [ ] | [ ] | amalgamation under vendor/ |
| 4 | ori-rres | [ ] | [ ] | |
| 5 | ori-jolt | [ ] | [ ] | stub ABI · includes R4.3 character_world |
| 6 | ori-imgui | [ ] | [ ] | stub until GLFW full |
| 7 | ori-raygui | [ ] | [ ] | real raylib for GUI demos |
| 8 | ori-enet | [ ] | [ ] | vendored enet; ws2_32 via pragma |

### Acceptance for “5 (Linux+Win)”

- [ ] All eight package smokes green on Windows MSVC (stub OK for graphics)
- [ ] Matrix Table A: maturity **5 (Linux+Win)** where applicable
- [ ] Optional: studio_shell via Git Bash / WSL

---

## P3 — macOS (Apple Darwin)

### Prerequisites

1. **Xcode CLT** (`xcode-select --install`)
2. **ori** binary with darwin runtime (`x86_64-apple-darwin` or `aarch64-apple-darwin`)
3. Optional staged raylib/shim under `ori-game/lib/<triple>/`

```bash
export ORI_BIN=ori ORI_USE_SYSTEM_LINKER=1 ORI_USE_AOT=1
# optional: export ORI_HOST_TRIPLE=aarch64-apple-darwin
cd /path/to/ori-game
./tools/smoke_macos.sh              # game gate (check + tests + sample)
./tools/smoke_eco_macos.sh          # umbrella ECO
./tools/smoke_eco_macos.sh           # CORE_ONLY=1 for faster loop
ECO_SMOKE_CORE_ONLY=1 ./tools/smoke_eco_macos.sh
```

### Naming (Darwin)

Same as Linux: `libfoo.a` under `lib/<triple>/`.  
Linker: system `clang`/`ld` with `ORI_USE_SYSTEM_LINKER=1`.  
Runtime dylib search: `DYLD_LIBRARY_PATH` (script sets from staged lib dir).

### What smoke does today

| Script | Scope |
|--------|--------|
| `smoke_macos.sh` | ori-game modules + unit subset + sample `ori check .` |
| `smoke_eco_macos.sh` | core 8 + studio-critical; prefers `smoke_macos.sh`, else reuses `smoke_linux.sh` |

### Gaps (honest)

- Many packages still use **Linux-named** build scripts (`build_linux.sh`) — on mac they may work if clang is used, or need `build_macos.sh` later.
- Full raylib stage for windowed Studio on mac is **not** automated here.
- CI macOS runner: optional future GHA (not blocking G1).

### Checklist (execute on Mac)

| # | Gate | Done |
|---|------|------|
| 1 | `./tools/smoke_macos.sh` green | [ ] |
| 2 | Core ECO `smoke_eco_macos.sh` green | [ ] |
| 3 | Sample Play window (optional) | [ ] |
| 4 | studio_shell smoke (optional) | [ ] |

---

## Validate from Linux (no Win/mac hardware)

```bash
cd /path/to/ori-game
chmod +x tools/validate_os_scripts.sh tools/smoke_macos.sh tools/smoke_eco_macos.sh
./tools/validate_os_scripts.sh
# → PASS if umbrellas + core Win scripts + rename guards OK
```

---

## What this host already validated

- Linux: `smoke_eco_linux.sh` + enet + CI gate
- Linux **P4 CI:** `scripts/smoke_ci_linux.sh` / GHA `smoke-linux.yml`
- **P1/P3 scaffolding:** `validate_os_scripts.sh` PASS on Linux
- Windows/mac **execution:** host-only (checkbox above)

## Linux reference

```bash
export ORI_BIN=ori ORI_USE_SYSTEM_LINKER=1
./tools/smoke_eco_linux.sh
./tools/smoke_ci_linux.sh
```

## Export (P2) multi-triple

```bash
# Linux default
./tools/export_game_package.sh

# macOS (on Mac host)
ORI_HOST_TRIPLE=aarch64-apple-darwin ./tools/export_game_package.sh
```

`export_game_package.sh` honors `ORI_HOST_TRIPLE` for native lib copy + binary name.
