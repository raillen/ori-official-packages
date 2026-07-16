# ori-nfd

Native file dialogs for Ori via
[portable-file-dialogs](https://github.com/samhocevar/portable-file-dialogs).

**Package:** `nfd` · **Module:** `nfd.dialog` · **0.2.0**  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Build

```bash
./tools/build_linux.sh
```

C++17 shim + pfd header → `lib/x86_64-unknown-linux-gnu/libori_nfd_shim.{a,so}`  
(`.a` is a GNU ld script that pulls objects + `-lstdc++ -lpthread` for AOT.)

## Smoke + tests

```bash
export ORI_BIN=/path/to/ori
export ORI_RUNTIME_CDYLIB=/path/to/libori_runtime.so
export ORI_RUNTIME_LIB=/path/to/libori_runtime.a
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_JIT=1
# Headless: never open a real dialog (default in smoke_linux.sh)
export NFD_SMOKE_SKIP_UI=1
./tools/smoke_linux.sh
```

Smoke uses `set_path_for_test` / multi / `clear` only — **no GUI**.  
If any code path calls `open_file` / `save_file` / `pick_folder` / `open_files`
while `NFD_SMOKE_SKIP_UI=1`, the shim returns **cancel** (false / empty) without
blocking.

## API (`nfd.dialog`)

| Symbol | Role |
|--------|------|
| `open_file(title, filter) -> bool` | Single-file open dialog |
| `open_files(title, filter) -> bool` | Multi-select open (`pfd::opt::multiselect`) |
| `save_file(title, default_name) -> bool` | Save-as dialog |
| `pick_folder(title) -> bool` | Folder picker |
| `is_ok() -> bool` | Last dialog accepted with a path |
| `path_len() / path_ptr() / path_byte(i) / path_sum()` | Primary path marshalling |
| `path_count() / path_at_len(idx) / path_at_byte(idx, i)` | Multi-open path slots |
| `clear()` | Reset to cancel / empty (no UI) |
| `set_path_for_test(path) -> bool` | Headless inject one path |
| `set_paths_for_test("a;b") -> bool` | Headless multi (`;`-separated) |

**Cancel:** user cancel, empty selection, `clear()`, empty test inject, or
`NFD_SMOKE_SKIP_UI=1` → `is_ok() == false`, `path_len() == 0`, `path_count() == 0`.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (package). portable-file-dialogs is WTFPL / zlib (see vendor header).
