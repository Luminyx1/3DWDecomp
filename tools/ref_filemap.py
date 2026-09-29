#!/usr/bin/env python3
"""Recover function -> source file for other games' debug builds.

Debug builds of other Nintendo games (ref/*.nso: ARMS, Splatoon 2, MK8 Deluxe
dev builds) keep their asserts, and every assert passes __FILE__ - the full
path of the .cpp it is in, e.g.
`D:/home/Project/P4ARMS/App/Lib/sead/engine/library/modules/src/container/seadPtrArray.cpp`.
For every exported function of such a build this finds the .cpp paths it
references (adrp+add to a string in .rodata); the most common one is its file.
Functions without asserts take the file of their neighbours when the functions
on both sides agree (objects are contiguous).

The result is a TSV of `mangled name <TAB> library-relative path`, e.g.
`_ZN4sead8PtrArrayImpl...  sead/container/seadPtrArray.cpp`, which
tools/complete_map.py can use (--ref) the same way it uses Odyssey's file list.

    python tools/ref_filemap.py ref/ARMS-mainuncompressed.nso ... -o config/1.0.0/ref_files.tsv
"""

from __future__ import annotations

import argparse
import collections
import re
import subprocess
import sys
from pathlib import Path

import capstone
from capstone.arm64 import ARM64_INS_ADRP, ARM64_INS_ADD, ARM64_OP_IMM, ARM64_OP_REG

sys.path.insert(0, str(Path(__file__).parent))
from furyimg import FuryImage  # noqa: E402

MD = capstone.Cs(capstone.CS_ARCH_ARM64, capstone.CS_MODE_LITTLE_ENDIAN)
MD.detail = True
SRC = re.compile(rb"^[^\0]{0,300}?\.(?:cpp|c|cc)\0")


def normalize(path: str) -> str | None:
    """Library-relative path: .../Lib/sead/engine/library/modules/src/container/seadPtrArray.cpp
    -> sead/container/seadPtrArray.cpp; NintendoSDK paths -> NintendoSDK/...; game code ->
    None (a different game's files are no use)."""
    p = path.replace("\\", "/")
    if "/" not in p:
        return "?/" + p          # bare file name (Splatoon 2): resolved by basename later
    m = re.search(r"/sead/(?:engine/)?(?:library/)?(?:modules/)?(?:src/)?(.*)$", p)
    if m:
        return "sead/" + m.group(1)
    for lib in ("agl", "eui", "NintendoSDK", "nn", "NintendoWare", "aal", "erepo", "LibMessageStudio"):
        m = re.search(rf"/({lib})/(.*)$", p)
        if m:
            rest = re.sub(r"^(?:[Ll]ibrary/|engine/|modules/|src/)+", "", m.group(2))
            return f"{m.group(1)}/{rest}"
    return None


def scan(elf: Path) -> dict[str, str]:
    img = FuryImage(str(elf))
    ro_lo, ro_hi = img.ranges[".rodata"]
    funcs = sorted((s for s in img.symbols if s.section == ".text" and s.type == "STT_FUNC" and s.size),
                   key=lambda s: s.addr)
    files: list[str | None] = []
    for s in funcs:
        code = img.bytes_at(s.addr, s.size)
        seen = collections.Counter()
        insns = list(MD.disasm(code, s.addr))
        for i, ins in enumerate(insns):
            if ins.id != ARM64_INS_ADRP:
                continue
            reg, page = ins.operands[0].reg, ins.operands[1].imm
            for nxt in insns[i + 1:i + 6]:
                ops = nxt.operands
                if nxt.id == ARM64_INS_ADD and len(ops) == 3 and ops[1].type == ARM64_OP_REG \
                        and ops[1].reg == reg and ops[2].type == ARM64_OP_IMM:
                    t = page + ops[2].imm
                    if ro_lo <= t < ro_hi:
                        m = SRC.match(img.bytes_at(t, 320))
                        if m:
                            try:
                                seen[m.group(0)[:-1].decode("utf-8")] += 1
                            except UnicodeDecodeError:
                                pass
                    break
        files.append(seen.most_common(1)[0][0] if seen else None)
    # fill gaps where both neighbours agree
    out = {}
    last = None
    for k, s in enumerate(funcs):
        f = files[k]
        if f is None:
            nxt = next((files[j] for j in range(k + 1, min(k + 40, len(funcs))) if files[j]), None)
            if last and nxt == last:
                f = last
        if f:
            last = f
            n = normalize(f)
            if n and s.name not in out:
                out[s.name] = n
    return out


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("nso", nargs="+")
    ap.add_argument("-o", "--out", required=True)
    a = ap.parse_args(argv)
    merged: dict[str, collections.Counter] = collections.defaultdict(collections.Counter)
    for nso in a.nso:
        elf = Path("build/ref") / (Path(nso).stem + ".elf")
        if not elf.is_file():
            elf.parent.mkdir(parents=True, exist_ok=True)
            subprocess.run([sys.executable, "tools/nx2elf.py", nso, "-o", str(elf)], check=True)
        got = scan(elf)
        print(f"{nso}: {len(got)} functions with a library source file", file=sys.stderr)
        for k, v in got.items():
            merged[k][v] += 1
    # resolve bare file names through the full paths other builds gave
    full = {}
    for k, c in merged.items():
        for v in c:
            if not v.startswith("?/"):
                full.setdefault(v.rsplit("/", 1)[-1], v)
    for k in list(merged):
        c = collections.Counter()
        for v, n in merged[k].items():
            if v.startswith("?/"):
                v = full.get(v[2:])
            if v:
                c[v] += n
        if c:
            merged[k] = c
        else:
            del merged[k]
    with open(a.out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("# mangled name\tsource file (from debug builds' assert paths; tools/ref_filemap.py)\n")
        for k in sorted(merged):
            fh.write(f"{k}\t{merged[k].most_common(1)[0][0]}\n")
    print(f"{len(merged)} functions -> {a.out}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
