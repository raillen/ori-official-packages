# ori-rres

Resource packs for Ori games.

**Package:** `rres` · **Module:** `rres.pack` · **Version:** 0.3.0

## ORPK format

Product pack format used by this package (little-endian integers):

```
offset  size  field
0       4     magic: ASCII "ORPK"
4       4     version: u32 = 1
8       4     count: u32 number of entries
12      …     entries[count]
```

Each entry:

```
u32   name_len          (1..255)
bytes name              (name_len bytes, no required trailing NUL on disk)
u32   data_len
bytes data              (data_len bytes; 0 allowed)
```

**Runtime resource id** = CRC32 of the UTF-8 name bytes, using the same
`rresComputeCRC32` as [raysan5/rres](https://github.com/raysan5/rres).  
Lookup is by name (CRC); the id is not stored on disk in v1.

Limits (shim): max 8 open packs, 64 entries/pack, 16 MiB/entry.

`open` and `validate` reject bad magic, unsupported version, oversized
`name_len`/`count`, and truncated streams.

## API

| Function | Role |
|----------|------|
| `create` / `open` / `close` / `is_open` | lifecycle |
| `validate` / `validate_ok` | structural check without loading |
| `is_valid` | handle passed open validation |
| `add_file` / `put_file` / `remove` | mutate |
| `save` | write `.orpk` |
| `has` / `count` / `size_of` | inspect |
| `name_at` | entry name by index |
| `list_names` | all names, one per line |
| `export` / `export_ok` / `read_bytes_to_path` | extract raw bytes to path |
| `read_text` | extract + read as text |
| `last_error` | last validate/open error message |

## CLI

```bash
./tools/pack_cli out.orpk hello.txt=assets/hello.txt icon.png=assets/icon.png
```

## Smoke

```bash
ORI_BIN=/home/raillen/.local/bin/ori ORI_USE_SYSTEM_LINKER=1 ./tools/smoke_linux.sh
```
