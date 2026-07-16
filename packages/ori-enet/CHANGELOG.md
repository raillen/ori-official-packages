# Changelog

## [0.3.0] - 2026-07-14

### Added
- Channel helpers: `CHANNEL_RELIABLE` / `CHANNEL_UNRELIABLE`,
  `send_reliable` / `send_unreliable`, `broadcast_reliable` / `broadcast_unreliable`
- Peer state constants + `peer_is_connected`
- `enet.protocol` — pure Ori `TYPE|body` encode/decode for jam framing
- Examples: `channels_demo`, `broadcast_demo` (2 clients), `protocol_demo`
- Cwd-relative packet temps (Windows-safe, no `/tmp`)
- Shim: `#pragma comment(lib, ws2_32/winmm)` on MSVC
- Phase OS: hardened `build_windows.ps1` (vswhere) + expanded `smoke_windows.ps1`

### Changed
- Package version **0.3.0** — Linux product surface for multiplayer jam use

## [0.2.0] - 2026-07-14

### Added
- Host stats: `host_total_sent` / `host_total_received` / `host_peer_count`
- Peer: `peer_state`, `peer_address` (`ip:port`)
- `service_drain` — non-blocking event pump (discards RECEIVE payloads)
- Example `examples/multi_message.orl` (3 reliable messages loopback)

## [0.1.0] - 2026-07-14

### Added
- **P1-B MVP:** vendored [lsalzman/enet](https://github.com/lsalzman/enet) +
  `native/ori_enet_shim.c` (host/peer slots, service events, reliable send,
  packet extract to path/text).
- L1 module `enet.host` (S3).
- Example `examples/loopback_ping.orl` (in-process ping/pong).
- `tools/build_linux.sh`, `smoke_linux.sh`; Windows PS1 scaffolds.
