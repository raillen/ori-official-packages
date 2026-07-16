# Changelog

## [0.2.0] - 2026-07-15

### Added
- Product surface for maturity 5: `parallel_for_sum`, explicit `add_range_sum` + `wait_scheduled_range`, `run_n_tasks` (N independent tasks), `wait_all`, `is_initialized`, `is_range_complete`, `tasks_completed`
- Stress + edge tests (re-init, double shutdown, no-init no-ops)
- Smoke covers multi-thread init, parallel_for, add/wait, N tasks, clean shutdown

### Changed
- Version `0.1.0` → `0.2.0`
- README API table + Phase OS note

## [0.1.0] - 2026-07-15

### Added
- enkiTS vendor + C shim (`init` / parallel sum / shutdown)
- Linux smoke + unit tests
- GNU ld script for C++ AOT link (`-lstdc++ -lpthread`)
