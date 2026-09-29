#!/usr/bin/env python3
"""Use the debug builds' source files (config/<ver>/ref_files.tsv, made by
tools/ref_filemap.py) to set objects and folders in the library parts of the
function map: sead, agl, eui, aal and NintendoWare.

These libraries are linked from archives, so their objects are not in
alphabetical order and the function map's guesses there were weakest.  Here a
function whose mangled name appears in a debug build gets that build's file:

  sead/packages/agl/src/utility/aglX.cpp   -> folder agl/utility, object aglX.o
  sead/container/seadPtrArray.cpp          -> folder sead/container, object seadPtrArray.o
  NintendoSDK/Sources/Libraries/atk/detail/atk_X.cpp -> NintendoWare/atk/detail, atk_X.o

Functions that no debug build has stay in the current object.  A new object
starts only when at least two of the next few known functions agree, so a
stray inline function doesn't split a file.  Game and al folders are left
alone.  Idempotent.

    python tools/apply_ref_files.py [--dry-run]
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

LIB_FOLDERS = ("sead", "agl", "eui", "aal", "NintendoWare", "NintendoSDK")


def place(path: str) -> tuple[str, str] | None:
    """ref path -> (folder, object)"""
    d, _, f = path.rpartition("/")
    obj = re.sub(r"\.(cpp|cc|c)$", ".o", f)
    m = re.match(r"^sead/packages/(agl|aal|eui)/(?:src/)?(.*)$", d + "/")
    if m:
        sub = m.group(2).strip("/")
        return (f"{m.group(1)}/{sub}" if sub else m.group(1)), obj
    if d.startswith("sead/packages/"):
        return None          # other games' packages (gsys, LinkCommon, ...)
    if d.startswith("sead/") or d == "sead":
        return d, obj
    m = re.match(r"^NintendoSDK/Sources/Libraries/(.*)$", d)
    if m:
        return f"NintendoWare/{m.group(1)}", obj
    return None


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", default="1.0.0")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args(argv)
    cfg = Path("config") / a.version
    ref = {}
    for line in (cfg / "ref_files.tsv").read_text(encoding="utf-8").splitlines():
        if line.startswith("#") or "\t" not in line:
            continue
        k, v = line.split("\t", 1)
        p = place(v)
        if p:
            ref[k] = p
    fmap = cfg / "function_map.tsv"
    lines = fmap.read_text(encoding="utf-8").split("\n")
    header, rows = lines[0], [l.split("\t") for l in lines[1:] if l]
    for c in rows:
        c += [""] * (7 - len(c))

    folder = None
    in_lib = []
    for i, c in enumerate(rows):
        if c[1]:
            folder = c[1]
        in_lib.append(bool(folder) and folder.split("/")[0] in LIB_FOLDERS)

    known = [ref.get(c[4]) if in_lib[i] else None for i, c in enumerate(rows)]
    folder_of = {obj: fold for fold, obj in ref.values()}
    cur = None
    changed = 0
    for i, c in enumerate(rows):
        if not in_lib[i]:
            cur = None
            continue
        p = known[i]
        start = False
        if p and p != cur:
            ahead = [known[j] for j in range(i, min(i + 60, len(rows))) if known[j] and in_lib[j]][:3]
            start = sum(1 for x in ahead if x == p) >= min(2, len(ahead))
        if start:
            new_f = p[0] if (cur is None or p[0] != cur[0]) else ""
            if c[1] != new_f or c[2] != p[1]:
                if new_f or c[1]:
                    c[1] = new_f or (cur[0] if cur else c[1])
                c[2] = p[1]
                c[6] = "object/folder from debug builds' assert paths"
                changed += 1
            cur = p
        elif (c[2] or c[1]) and "debug builds" not in c[6]:
            nxt = next((known[j] for j in range(i, min(i + 60, len(rows))) if known[j]), None)
            if cur and (nxt == cur or c[2] == cur[1]):
                # an older marker inside a file the debug builds identified: drop it
                c[1] = c[2] = ""
                c[6] = f"part of {cur[1]} (debug builds)"
                changed += 1
            elif cur and c[2] in folder_of and folder_of[c[2]] != cur[0]:
                # a file the debug builds know, in another folder
                c[1] = folder_of[c[2]]
                cur = (c[1], c[2])
                changed += 1
            elif cur and c[2]:
                cur = (cur[0], c[2])
    print(f"{changed} marker(s) set or removed from {len(ref)} known functions")
    if not a.dry_run:
        fmap.write_text("\n".join([header] + ["\t".join(c) for c in rows]) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
