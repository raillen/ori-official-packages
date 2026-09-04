# ori-lz4

[LZ4](https://github.com/lz4/lz4) block + stream/frame compress/decompress for Ori.

**Package:** `lz4` · **Module:** `lz4.codec` · **0.2.0** · **Maturity 5 (Linux)**

## Build

```bash
./tools/build_linux.sh
```

Prefers system **liblz4** via `pkg-config liblz4`. If unavailable, compiles vendored `vendor/lz4.c` + `vendor/lz4.h` (LZ4 v1.10.0 block + stream API).

## Smoke + tests

```bash
export ORI_BIN=/path/to/ori
export ORI_RUNTIME_CDYLIB=/path/to/libori_runtime.so
export ORI_RUNTIME_LIB=/path/to/libori_runtime.a
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_JIT=1
./tools/smoke_linux.sh
```

Smoke prints `ok`. Tests cover block/stream round-trips, bounds, large pattern fixture, and garbage reject.

## API (`lz4.codec`)

| Function | Description |
|----------|-------------|
| `compress` / `compress_text` | LZ4 block compress (raw / UTF-8 text) |
| `decompress` | LZ4 block decompress |
| `roundtrip_len` | Block compress then decompress; returns plain length |
| `stream_compress` / `stream_compress_text` | OLZ1 multi-block stream frame (`LZ4_compress_fast_continue`) |
| `stream_decompress` | Decompress OLZ1 frame |
| `stream_roundtrip_len` | Stream frame round-trip; returns plain length |
| `compress_bound` | `LZ4_compressBound` for plain size |
| `stream_bound` | Worst-case OLZ1 frame size for plain + chunk |
| `pattern_roundtrip` / `pattern_stream_roundtrip` | Large native pattern fixtures |
| `out_len` / `out_ptr` / `out_byte` / `out_sum` | Last buffer accessors |
| `last_src_len` | Last plain input length (after compress) |

### OLZ1 frame format

Simple length-prefixed multi-block stream (not the official LZ4F container):

```
magic 'O','L','Z','1'
u32le plain_total
u32le chunk_size
repeat:
  u32le compressed_size
  compressed_size bytes   # LZ4 stream continue block
u32le 0                   # terminator
```

Native ABI is `int64_t` (pointers and lengths). Shim: `native/ori_lz4_shim.c`.

**Note:** Avoid Ori parameter name `len` (builtin) — use `nbytes` / `src_nbytes`.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See [`ori-lang` `docs/planning/PHASE-OS.md`](https://github.com/raillen/ori-lang/blob/master/docs/planning/PHASE-OS.md) for the multi-OS staging policy
