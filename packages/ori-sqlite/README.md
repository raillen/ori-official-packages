# ori-sqlite

[SQLite](https://sqlite.org) for Ori (S3) — amalgamation + thin C shim.

**Package:** `sqlite` · **Module:** `sqlite.db` · **Version:** 0.3.0

## API

### Connection

| Function | Role |
|----------|------|
| `open` / `close` / `is_open` | connection |
| `exec` / `exec_ok` | run SQL |
| `query_int` | first column, first row as int |
| `query_text` / `query_text_to_path` | first column as string |
| `query_all_json` / `query_all_to_json_path` | multi-row JSON array |
| `last_insert_rowid` / `changes` | write stats |
| `begin` / `commit` / `rollback` | transactions |
| `has_error` / `last_error` | last error message |

### Prepared statements

| Function | Role |
|----------|------|
| `prepare` / `is_prepared` / `finalize` | lifecycle |
| `bind_int` / `bind_text` | bind (1-based indices) |
| `step` | advances; compare to `STEP_ROW` / `STEP_DONE` |
| `step_row` / `step_done` | convenience bools (each calls `step`) |
| `column_int` / `column_text` / `column_text_to_path` | read current row |
| `column_count` | column count |
| `reset` / `clear_bindings` | reuse statement |

## Build / smoke

```bash
./tools/build_linux.sh
# produces under lib/x86_64-unknown-linux-gnu/:
#   libsqlite3.a
#   libori_sqlite_shim.a          # AOT / ori compile
#   libori_sqlite_shim.so         # JIT / ori run (cdylib)
ORI_BIN=/home/raillen/.local/bin/ori ORI_USE_SYSTEM_LINKER=1 ./tools/smoke_linux.sh
```

| Artifact | Used by |
|----------|---------|
| `libori_sqlite_shim.a` + `libsqlite3.a` | `ori compile` / `ORI_USE_AOT=1` |
| `libori_sqlite_shim.so` + `libsqlite3.so` | `ori run` JIT (`native_libs` order; shim rpath `$ORIGIN`) |

Demos: `kv_store.orl`, `tx_rollback.orl`, `prepared_demo.orl`.
