#!/usr/bin/env python3
"""Move where an object starts in config/<ver>/function_map.tsv.

    python tools/set_object.py seadEventNin.o 0x71006913C0
    python tools/set_object.py seadMessageQueueNin.o 0x7100691600 --new   # start a new object

The object's marker (and its folder marker, if it has one) moves to the row at
the given address; the rows in between join the neighbouring object.  Use it
when a file boundary in the map is a few functions off.  Rerun
tools/gen_splits.py and configure.py afterwards.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("object")
    ap.add_argument("address")
    ap.add_argument("--new", action="store_true", help="the object does not exist yet: start it there")
    ap.add_argument("--folder", help="with --new: the folder it starts (a folder change)")
    ap.add_argument("--version", default="1.0.0")
    a = ap.parse_args(argv)
    path = Path("config") / a.version / "function_map.tsv"
    lines = path.read_text(encoding="utf-8").split("\n")
    rows = [l.split("\t") for l in lines]
    want = int(a.address, 16)
    src = [i for i, c in enumerate(rows) if len(c) > 2 and c[2] == a.object]
    dst = [i for i, c in enumerate(rows) if c and c[0].startswith("0x") and int(c[0], 16) == want]
    if a.new and not src and len(dst) == 1:
        c = rows[dst[0]]
        c += [""] * (7 - len(c))
        if c[2]:
            raise SystemExit(f"row {a.address} already starts {c[2]}")
        c[2] = a.object
        if a.folder:
            c[1] = a.folder
        c[6] = "object start set by hand"
        path.write_text("\n".join("\t".join(r) for r in rows), encoding="utf-8")
        print(f"{a.object}: new at row {dst[0] + 1}")
        return 0
    if len(src) != 1 or len(dst) != 1:
        raise SystemExit(f"need exactly one {a.object} marker and one row at {a.address} (found {len(src)}, {len(dst)})")
    s, d = src[0], dst[0]
    for c in (rows[s], rows[d]):
        c += [""] * (7 - len(c))
    if rows[d][2] and d != s:
        raise SystemExit(f"row {a.address} already starts {rows[d][2]}")
    folder = rows[s][1]
    rows[s][1], rows[s][2] = "", ""
    rows[d][1], rows[d][2] = folder, a.object
    rows[d][6] = "object start fixed by hand"
    path.write_text("\n".join("\t".join(c) for c in rows), encoding="utf-8")
    print(f"{a.object}: row {s + 1} -> row {d + 1}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
