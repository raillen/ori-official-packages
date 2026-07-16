# ori-physfs

[PhysicsFS](https://github.com/icculus/physfs) virtual filesystem bindings for Ori (S3).

**Package:** `physfs` · **Module:** `physfs.fs` · **0.2.0**  
**Linux first** · maturity **5 (Linux)** · **ABI:** Ori `int` / C `int64_t`

## Build

```bash
./tools/build_linux.sh
./tools/smoke_linux.sh
```

Build prefers **system** `libphysfs` via `pkg-config physfs` (e.g. `libphysfs-dev`).  
If unavailable, compiles **vendored** PhysFS 3.2.0 under `vendor/physfs/` into the shim staticlib/cdylib.

## API (`physfs.fs`)

| Function | Description |
|----------|-------------|
| `init()` / `deinit()` | Start / shut down PhysicsFS |
| `mount(path, mount_point)` | Mount dir/archive at virtual point (`""` = root); mounts **stack** (append) |
| `unmount(path)` | Remove a previous mount by real path |
| `exists(path)` | Virtual path exists |
| `read(path) -> int` | Read whole file into shim buffer; returns length or `<0` |
| `last_len` / `last_sum` / `last_byte` / `last_ptr` | Last read buffer getters |
| `enumerate_count(dir)` | Entry count under virtual dir (`""` = root) |
| `set_write_dir(path)` | Real OS directory for writes (does **not** auto-mount) |
| `mkdir(path)` | Create virtual directory tree under the write dir |
| `write(path, data) -> int` | Write string bytes; returns length or `<0` |
| `write_bytes(path, ptr, len) -> int` | Write raw buffer |
| `last_write_len()` | Bytes written by last successful `write` / `write_bytes` |

### Write path

1. `set_write_dir("/tmp/writable")`  
2. Optionally `mount` the same real path if you need VFS read-back  
3. `mkdir("saves")` then `write("saves/slot.txt", payload)`

### Multi-mount

```ori
const _a: bool = pfs.mount("/game/base", "")
const _b: bool = pfs.mount("/game/mod_pack", "mod")
-- base/hello.txt  → "hello.txt"
-- mod_pack/x.bin  → "mod/x.bin"
```

Earlier mounts win on name conflicts (`appendToPath = 1`).

## Smoke fixture

`tools/smoke_linux.sh` prepares:

| Dir | Contents |
|-----|----------|
| `/tmp/ori_physfs_test` | `hello.txt` (`hello-physfs\n`, 13 bytes) |
| `/tmp/ori_physfs_test_b` | `other.txt` (`from-b\n`, 7 bytes) |
| `/tmp/ori_physfs_write` | empty write dir |

Smoke covers multi-mount read, write-dir write + read-back, unmount, enumerate.

## License

Shim and package: Zlib (same as PhysicsFS). Vendored sources: see `vendor/physfs/LICENSE.txt`.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See [`ori-lang` `docs/planning/PHASE-OS.md`](https://github.com/raillen/ori-lang/blob/master/docs/planning/PHASE-OS.md) for the multi-OS staging policy
