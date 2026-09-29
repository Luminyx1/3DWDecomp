#!/usr/bin/env python3
"""Print a class's vtable from fury.elf: every slot with its offset and the
function it points to, split into the primary and secondary (base-class)
groups.

    python tools/vtable.py PlayerAirTurnChecker          # by class name
    python tools/vtable.py _ZTV20PlayerAirTurnChecker    # by vtable symbol
    python tools/vtable.py --grep AirTurn                # list vtables matching a pattern

Slot offsets are relative to the address point (vtable + 0x10 for a group),
which is what the code loads: `ldr x8, [x0]; ldr x8, [x8, #0x18]` calls the
slot printed as +0x18.  Secondary groups show their offset-to-top, i.e. at
which offset in the object that base's vtable pointer lives.
"""

from __future__ import annotations

import argparse
import re
import struct
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from furyimg import FuryImage  # noqa: E402


def demangle(names: list[str]) -> list[str]:
    try:
        out = subprocess.run(["c++filt"], input="\n".join(names), capture_output=True, text=True).stdout
        return out.splitlines()
    except OSError:
        return names


def vtable_symbol(img: FuryImage, cls: str) -> str:
    if cls.startswith("_ZTV"):
        return cls
    if "::" in cls:
        parts = cls.split("::")
        return "_ZTVN" + "".join(f"{len(p)}{p}" for p in parts) + "E"
    return f"_ZTV{len(cls)}{cls}"


def slot(img: FuryImage, addr: int) -> str:
    ent = img.dyn_relocs.get(addr)
    if ent:
        _, nm, add = ent
        if nm:
            return nm + (f"+{add:#x}" if add else "")
        cov = img.sym_covering(add)
        if cov:
            return cov[0].name + (f"+{cov[1]:#x}" if cov[1] else "")
        return f"{add:#x}"
    v = struct.unpack("<q", img.bytes_at(addr, 8))[0]
    return str(v) if v else "0"


def dump(img: FuryImage, name: str) -> int:
    s = img.sym(name)
    if s is None:
        print(f"{name}: no such symbol", file=sys.stderr)
        return 1
    n = s.size // 8
    raw = [slot(img, s.addr + 8 * i) for i in range(n)]
    dem = demangle(raw)
    print(f"{demangle([name])[0]}  @ {s.addr + 0x7100000000:#x}  ({n} words)")
    i = 0
    while i < n:
        top = raw[i]
        print(f"  -- group at +{8 * i:#x}: offset-to-top {top}, typeinfo {raw[i + 1] if i + 1 < n else '?'}")
        j = i + 2
        k = 0
        while j < n:
            # a new group starts with a negative offset-to-top followed by 0 typeinfo
            if j + 1 < n and re.fullmatch(r"-\d+", raw[j]) and raw[j + 1] == "0":
                break
            print(f"    +{8 * k:#05x}  {dem[j]}")
            j += 1
            k += 1
        i = j
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cls", nargs="*")
    ap.add_argument("--grep", help="list vtable symbols whose demangled name matches this regex")
    a = ap.parse_args(argv)
    img = FuryImage("build/fury.elf")
    if a.grep:
        names = sorted(s.name for s in img.symbols if s.name.startswith("_ZTV") and s.section != "UND")
        for m, d in zip(names, demangle(names)):
            if re.search(a.grep, d):
                print(d)
        return 0
    rc = 0
    for c in a.cls:
        rc |= dump(img, vtable_symbol(img, c))
    return rc


if __name__ == "__main__":
    sys.exit(main())
