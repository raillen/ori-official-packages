# ori-enet

[ENet](https://github.com/lsalzman/enet) UDP multiplayer for Ori (S3) — vendored
upstream + thin C shim with int handles.

**Package:** `enet` · **Modules:** `enet.host`, `enet.protocol` · **Version:** 0.3.0  
**Maturity:** **5 (Linux)** jam product surface

## Why ENet

Reliable/unreliable channels over UDP, connect/disconnect events, low latency —
the usual jam multiplayer transport. Not full netcode (no rollback/prediction).

## API (`enet.host`)

| Function | Role |
|----------|------|
| `initialize` / `deinitialize` | global ENet lifecycle |
| `create_server` / `create_client` / `destroy` | hosts |
| `connect` / `peer_is_valid` / `peer_is_connected` | peers |
| `service` / `flush` / `service_drain` | poll events |
| `send_text` / `send_reliable` / `send_unreliable` | send |
| `broadcast_*` | fan-out |
| `packet_text` / `destroy_packet` | last RECEIVE payload |
| `host_total_sent` / `host_peer_count` / `peer_rtt_ms` / `peer_address` | diagnostics |

Event constants: `EVENT_NONE|CONNECT|DISCONNECT|RECEIVE`.  
Channels (convention): `CHANNEL_RELIABLE` (0), `CHANNEL_UNRELIABLE` (1).

## Framing (`enet.protocol`)

Pure Ori helpers: `encode("CHAT", "hi")` → `"CHAT|hi"`; `decode` → `{ kind, body }`.

## Build / smoke

```bash
# Linux
./tools/build_linux.sh
ORI_BIN=ori ORI_USE_SYSTEM_LINKER=1 ./tools/smoke_linux.sh

# Windows (x64 Native Tools / VS Build Tools)
$env:ORI_BIN = "C:\path\to\ori.exe"
$env:ORI_USE_SYSTEM_LINKER = "1"
.\tools\smoke_windows.ps1
```

Examples (all print `ok`): `loopback_ping`, `multi_message`, `channels_demo`,
`broadcast_demo`, `protocol_demo`.

## Path dep

```toml
[package]
name = "my_game"
native_libs = ["ori_enet_shim", "enet"]

[dependencies]
enet = { path = "../ori-enet", version = "0.3.0" }
```

On Windows the shim pulls `ws2_32` / `winmm` via `#pragma comment`.

## License

Shim and Ori surface: MIT. Upstream ENet: MIT (`vendor/enet/LICENSE`).
