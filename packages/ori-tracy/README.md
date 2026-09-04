# ori-tracy

[Tracy](https://github.com/wolfpld/tracy) instrumentation for Ori (S3).

**Package:** `tracy` · **Module:** `tracy.profiler` · **0.2.0** · Linux first  
**Claim:** `maturity 5 (Linux) — G1..G7 met`

```bash
# Headless counters + FULL static artifact (default CI)
./tools/smoke_linux.sh

# Force FULL-only build path for product link experiments
ORI_TRACY_FULL=1 ./tools/build_linux.sh

# Headless only
ORI_TRACY_FULL=0 ./tools/build_linux.sh
```

| API | Role |
|-----|------|
| `zone_begin` / `zone_end` | Nested zones |
| `zone_depth` / `zone_begins` / `zone_ends` | Stack + counters |
| `overflow_begins` / `orphan_ends` | Capacity / unmatched-end edges |
| `frame_mark` / `frame_count` | Frame boundaries |
| `message` / `message_count` | Log string (FULL emits to Tracy) |
| `plot_m` / `plot_count` / `last_plot_value_m` | Plot (milli-float → Tracy double/1000) |
| `reset_stats` | Clear counters (ends open FULL zones) |
| `is_full_build` | `true` when linked against FULL artifact |

## Dual build

| Artifact | Contents | Use |
|----------|----------|-----|
| `libori_tracy_shim.{a,so}` | Headless bookkeeping | `ori test` / `ori run` smoke (no Tracy GUI) |
| `libori_tracy_full.a` | Shim + `TracyClient.cpp` (`TRACY_ENABLE`) | Product AOT; connect Tracy GUI / capture |

Default `tools/build_linux.sh` builds **both** when `vendor/public/TracyClient.cpp` is present. Smoke requires the FULL archive (symbol check via `nm`) but runs tests against the **headless** cdylib so CI never needs a Tracy GUI.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy
