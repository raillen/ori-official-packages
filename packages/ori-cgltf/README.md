# ori-cgltf

[cgltf](https://github.com/jkuhlmann/cgltf) glTF 2.0 loader bindings for Ori (S3).

**Package:** `cgltf` · **Module:** `cgltf.loader` · **0.2.0**  
**Maturity:** **5 (Linux)** · API + tests + smoke  
**Linux first** · Phase OS deferred

```bash
export ORI_BIN=…/ori
export ORI_RUNTIME_CDYLIB=…/libori_runtime.so
export ORI_RUNTIME_LIB=…/libori_runtime.a
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_JIT=1
./tools/smoke_linux.sh
```

## API (`cgltf.loader`)

| Symbol | Description |
|--------|-------------|
| `load_file(path)` | Parse `.gltf` / `.glb`; returns `bool` |
| `free()` | Release current document + export buffer |
| `mesh_count()` / `node_count()` / `material_count()` / `animation_count()` | Top-level counts |
| `mesh0_vertex_count()` | POSITION count on mesh 0 / first primitive |
| `mesh_primitive_count(mesh_i)` | Primitives in mesh |
| `mesh_vertex_count(mesh_i, prim_i)` | POSITION vertex count |
| `mesh_index_count(mesh_i, prim_i)` | Index count (or vertex count if unindexed) |
| `mesh_has_position` / `mesh_has_normal` | Attribute presence on primitive |
| `node_has_translation` / `node_has_rotation` / `node_has_scale` | TRS flags |
| `node_translation_m(node_i, axis)` | Translation milli (`value * 1000`); missing → 0 |
| `node_rotation_m(node_i, component)` | Quaternion milli (xyzw); missing → identity |
| `node_scale_m(node_i, axis)` | Scale milli; missing → 1000 |
| `material_base_color_m(mat_i, channel)` | PBR base color RGBA milli |
| `export_mesh(mesh_i, prim_i)` | Build interleaved float mesh for raylib upload |
| `export_vertex_count` / `export_index_count` / `export_float_count` | Last export sizes |
| `export_float_m(i)` / `export_pos_m` / `export_normal_m` | Milli samples of export |
| `export_index(i)` | Index value (0-based) |
| `export_floats_ptr` / `export_indices_ptr` | Native pointers for upload helpers |
| `export_sum_m()` | Milli checksum of exported floats (tests) |

### Export layout (raylib-friendly)

Interleaved floats: **`[px, py, pz, nx, ny, nz] * vertex_count`**.

- Missing `NORMAL` → default `(0, 1, 0)` per vertex.
- Missing indices → sequential `0 .. vertex_count-1`.
- Milli accessors round floats as `int(value * 1000)` for Ori tests and L1 glue.

## Native

- Shim: `native/ori_cgltf_shim.c` → `libori_cgltf_shim.{a,so}`
- ABI: Ori `int` is 64-bit; C uses `int64_t` for pointers and counts
- Vendor: `vendor/cgltf.h` (MIT, jkuhlmann/cgltf)

## Fixture

`tests/fixtures/minimal.gltf` — single triangle (3 verts, POSITION+NORMAL, TRS node, tinted PBR material, 0 animations).

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See [`ori-lang` `docs/planning/PHASE-OS.md`](https://github.com/raillen/ori-lang/blob/master/docs/planning/PHASE-OS.md) for the multi-OS staging policy
