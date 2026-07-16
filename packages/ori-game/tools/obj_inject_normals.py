#!/usr/bin/env python3
"""Inject face normals into Wavefront OBJ so raylib 5.5 LoadOBJ does not SEGV.

raylib 5.5 LoadOBJ (tinyobj path) indexes normals unconditionally; OBJs exported
without `vn` (common from Blender/Tripo `f v/vt`) crash. This rewrites faces as
`f v/vt/vn` with one face normal per polygon (triangulated meshes preferred).
"""
from __future__ import annotations

import argparse
import math
import re
from pathlib import Path


def inject_normals(src: Path, dst: Path) -> None:
    verts: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []
    faces: list[list[tuple[int, int]]] = []
    mtllib = None
    usemtl = None
    o_name = None

    for line in src.read_text(errors="replace").splitlines():
        if line.startswith("v "):
            p = line.split()
            verts.append((float(p[1]), float(p[2]), float(p[3])))
        elif line.startswith("vt "):
            p = line.split()
            uvs.append((float(p[1]), float(p[2])))
        elif line.startswith("f "):
            corners: list[tuple[int, int]] = []
            for tok in line.split()[1:]:
                bits = tok.split("/")
                vi = int(bits[0])
                ti = int(bits[1]) if len(bits) > 1 and bits[1] else 0
                corners.append((vi, ti))
            faces.append(corners)
        elif line.startswith("mtllib "):
            mtllib = line
        elif line.startswith("usemtl "):
            usemtl = line
        elif line.startswith("o "):
            o_name = line

    normals: list[tuple[float, float, float]] = []
    face_nidx: list[int] = []
    for corners in faces:
        i0, i1, i2 = corners[0][0] - 1, corners[1][0] - 1, corners[2][0] - 1
        ax, ay, az = verts[i0]
        bx, by, bz = verts[i1]
        cx, cy, cz = verts[i2]
        ux, uy, uz = bx - ax, by - ay, bz - az
        vx, vy, vz = cx - ax, cy - ay, cz - az
        nx = uy * vz - uz * vy
        ny = uz * vx - ux * vz
        nz = ux * vy - uy * vx
        ln = math.sqrt(nx * nx + ny * ny + nz * nz) or 1.0
        normals.append((nx / ln, ny / ln, nz / ln))
        face_nidx.append(len(normals))

    out: list[str] = [
        "# normals injected by tools/obj_inject_normals.py (raylib 5.5 LoadOBJ safety)"
    ]
    if mtllib:
        out.append(mtllib)
    if o_name:
        out.append(o_name)
    for x, y, z in verts:
        out.append(f"v {x:.6f} {y:.6f} {z:.6f}")
    for u, v in uvs:
        out.append(f"vt {u:.6f} {v:.6f}")
    for x, y, z in normals:
        out.append(f"vn {x:.6f} {y:.6f} {z:.6f}")
    out.append("s off")
    if usemtl:
        out.append(usemtl)
    for corners, ni in zip(faces, face_nidx):
        parts = []
        for vi, ti in corners:
            parts.append(f"{vi}/{ti}/{ni}" if ti else f"{vi}//{ni}")
        out.append("f " + " ".join(parts))
    dst.write_text("\n".join(out) + "\n")
    print(
        f"wrote {dst} verts={len(verts)} uvs={len(uvs)} "
        f"faces={len(faces)} normals={len(normals)}"
    )


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("input", type=Path)
    ap.add_argument("-o", "--output", type=Path, default=None)
    ap.add_argument(
        "--in-place",
        action="store_true",
        help="overwrite input (keeps input.raw.obj backup once)",
    )
    args = ap.parse_args()
    src = args.input
    if args.in_place:
        backup = src.with_suffix(src.suffix + ".raw")
        if not backup.exists():
            backup.write_bytes(src.read_bytes())
        dst = src
    else:
        dst = args.output or src.with_name(src.stem + "_normals.obj")
    inject_normals(src if not args.in_place else (src.with_suffix(src.suffix + ".raw") if src.with_suffix(src.suffix + ".raw").exists() else src), dst)


if __name__ == "__main__":
    main()
