# ori-fast-obj

[fast_obj](https://github.com/thisistherk/fast_obj) Wavefront OBJ loader for Ori (S3).

**Package:** `fast_obj` · **Module:** `fast_obj.mesh` · **0.2.0**  
**Linux first** · maturity **5 (Linux)** · API + smoke/tests

```bash
./tools/smoke_linux.sh
```

| Module | API |
|--------|-----|
| `fast_obj.mesh` | `load(path)` / `free` |
| | Counts: `position_count` / `face_count` / `normal_count` / `texcoord_count` / `index_count` / `material_count` |
| | Samples (milli): `pos_m(i, axis)` / `normal_m(i, axis)` / `texcoord_m(i, axis)` |
| | Face indices: `index_p(i)` / `index_t(i)` / `index_n(i)` (1-based; 0 = missing) |
| | Materials: `material_name_len(i)` / `material_name_byte(i, offset)` |
| | Flatten: `export_mesh()` → interleaved float mesh (see layout) |
| | Export queries: `export_vertex_count` / `export_index_count` / `export_float_count` / `export_float_m` / `export_index` / `export_pos_m` / `export_normal_m` / `export_uv_m` / `export_floats_ptr` / `export_indices_ptr` / `export_sum_m` |

## Flatten layout (raylib helper)

Interleaved floats after fan triangulation of faces:

**`[px, py, pz, nx, ny, nz, u, v] * vertex_count`**

- One unique vertex per triangle corner (shared OBJ vertices with different normal/uv expand).
- Missing normal → default `(0, 0, 1)`.
- Missing texcoord → `(0, 0)`.
- Indices: sequential `0..V-1` triangle list.
- `export_floats_ptr` / `export_indices_ptr` for native upload.
- Milli accessors for Ori tests / L1 glue (`value * 1000`).

## Notes

- Upstream single-header: `vendor/fast_obj.h` (MIT, thisistherk/fast_obj).
- Counts follow fast_obj: `position_count` / `normal_count` / `texcoord_count` include a dummy zero entry at index 0.
- `pos_m` / `normal_m` / `texcoord_m` use 0-based real attribute indices and return milli-float.
- Material names come from the companion `.mtl` (`mtllib`).
- Module leaf remains **`fast_obj.mesh`** so path-dep + `game.obj` do not collide.
- Native shim: `libori_fast_obj_shim` (`int64_t` ABI for pointers and counts).

## License

- Package bindings: MIT
- Vendor `fast_obj.h`: MIT (see header copyright)

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See [`ori-lang` `docs/planning/PHASE-OS.md`](https://github.com/raillen/ori-lang/blob/master/docs/planning/PHASE-OS.md) for the multi-OS staging policy
