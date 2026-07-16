# ori-enkiTS

[enkiTS](https://github.com/dougbinks/enkiTS) task scheduler for Ori (S3).

**Package:** `enkits` · **Module:** `enkits.tasks` · **0.2.0** · **Linux first**

```bash
./tools/smoke_linux.sh
```

## API (`enkits.tasks`)

| API | Role |
|-----|------|
| `init(num_threads)` | Start scheduler; `0` = hardware default (N−1 workers + main). Returns `bool`. |
| `shutdown` | Wait for work, stop threads, destroy scheduler (idempotent). |
| `is_initialized` | Whether a live scheduler exists. |
| `num_threads` | Worker + main count (`enkiGetNumTaskThreads`). |
| `wait_all` | Wait for all pending task sets. |
| `parallel_for_sum(n, min_range)` | Parallel for over `[0,n)` with grain `min_range`; returns `sum(i)`. |
| `parallel_sum(n)` | Same as `parallel_for_sum(n, 1)`. |
| `add_range_sum(n, min_range)` | **Add** a range-sum task set (no wait). |
| `wait_scheduled_range` | **Wait** for the last scheduled range task. |
| `is_range_complete` | Non-blocking complete check for that range task. |
| `run_n_tasks(n)` | **N independent unit tasks** (grain 1), wait; returns sum of indices. |
| `tasks_completed` | How many unit indices ran in the last `run_n_tasks`. |
| `last_range_runs` | Partition count from last `parallel_for` / range sum. |
| `last_sum` | Result of last range sum after wait. |
| `serial_sum(n)` | CPU serial reference sum `0..n-1`. |

## Build notes

- Vendor C++ (`TaskScheduler.cpp` + C API) linked via **GNU ld script** `libori_enkits_shim.a` → `INPUT(libori_enkits_objs.a -lstdc++ -lpthread)` so AOT picks up libstdc++ without a fake empty archive.
- Shared object `libori_enkits_shim.so` for JIT (`ori run` / tests with `ORI_USE_JIT=1`).
- Native symbols use **`int64_t` ABI** (Ori `int`).

## Phase OS

**Linux-first.** Windows/macOS packaging and multi-OS CI are deferred (**Phase OS** — last, non-blocking for maturity 5). Source is mostly portable (enkiTS + pthread/C++).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## Maturity

**5 (Linux)** — broad product API (add/wait/parallel_for/N tasks), ≥4 tests, green `tools/smoke_linux.sh`.
