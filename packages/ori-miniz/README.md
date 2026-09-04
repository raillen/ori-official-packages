# ori-miniz

[miniz](https://github.com/richgel999/miniz) deflate, CRC32, and single-entry ZIP for Ori.

**Package:** `miniz` · **Module:** `miniz.deflate` · **0.2.0**  
**Maturity:** **5 (Linux)** — G1..G7 met

## Build

```bash
./tools/build_linux.sh
```

Builds vendored miniz (`miniz.c` + tdef + tinfl + zip) into
`lib/x86_64-unknown-linux-gnu/libori_miniz_shim.{a,so}`.

Memory-only archive path: `MINIZ_NO_STDIO` + `MINIZ_NO_TIME` (no disk I/O).

## Smoke + tests

```bash
export ORI_BIN=/path/to/ori
export ORI_RUNTIME_CDYLIB=/path/to/libori_runtime.so
export ORI_RUNTIME_LIB=/path/to/libori_runtime.a
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_JIT=1
./tools/smoke_linux.sh
```

Smoke prints `ok`. Tests cover compress round-trip, CRC32 stability, ZIP
create/extract one entry, and decompress error path.

## API (`miniz.deflate`)

| Function | Description |
|----------|-------------|
| `compress(src_ptr, src_len, level)` | Deflate raw buffer → internal out; returns compressed length (`<0` fail) |
| `compress_text(text, level)` | Deflate UTF-8 text |
| `decompress(src_ptr, src_len, max_out)` | Inflate buffer → out; `max_out` 0 = capacity |
| `roundtrip_len(text)` | Compress then decompress; returns plain length |
| `crc32(ptr, len)` / `crc32_text(text)` | CRC-32 (init over buffer) |
| `zip_create_entry(name, text, level)` | In-memory ZIP with **one** named entry |
| `zip_extract_last(name)` | Extract named entry from last created ZIP |
| `zip_extract(zip_ptr, zip_len, name)` | Extract from external ZIP buffer |
| `zip_roundtrip_len(name, text)` | Create + extract; returns plain length |
| `out_len` / `out_ptr` / `out_byte` / `out_sum` / `last_src_len` | Last buffer accessors |

Native ABI is `int64_t` (pointers and lengths). Shim: `native/ori_miniz_shim.c`.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy
