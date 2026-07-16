#!/usr/bin/env bash
# R2.3: write `.ori_play_stamp` for a project (fingerprint of play sources).
# Mirrors game.play_freshness djb2 so Studio Play freshness check stays green
# after you recompile and stamp.
#
# Usage:
#   ./tools/write_play_stamp.sh
#   ./tools/write_play_stamp.sh /path/to/sample_project
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PROJ="${1:-$ROOT/assets/sample_project}"

python3 - "$PROJ" <<'PY'
import sys
from pathlib import Path

root = Path(sys.argv[1]).resolve()
rels = [
    "main.orl",
    "ori.pkg.toml",
    "entities/player/player.orl",
    "entities/player/entity.json",
    "entities/enemy/enemy.orl",
    "entities/enemy/entity.json",
    "entities/solid/solid.orl",
    "entities/solid/entity.json",
    "entities/empty/empty.orl",
    "entities/empty/entity.json",
    "scenes/engine_test.scene.json",
    "scenes/room1.scene.json",
]

# Match Ori i64-ish fold used in game.play_freshness.hash_text
def fold(h: int) -> int:
    # keep in a modest range like the Ori clamp loop intent
    if h < 0:
        h = -h
    while h > 2000000000:
        h -= 1000000000
    return h


def hash_text(h: int, text: bytes) -> int:
    for b in text:
        h = fold(h * 33 + b)
    return h


h = 5381
missing = 0
n = len(rels)
for rel in rels:
    p = root / rel
    h = hash_text(h, rel.encode("utf-8"))
    h = hash_text(h, b"|")
    if p.is_file():
        h = hash_text(h, p.read_bytes())
    else:
        missing += 1
        h = hash_text(h, b"?missing")
    h = hash_text(h, b";")

fp = f"p{h}:{n}m{missing}"
stamp = root / ".ori_play_stamp"
stamp.write_text(fp + "\n", encoding="utf-8")
print(f"stamp_ok {stamp}")
print(f"fingerprint {fp}")
PY
