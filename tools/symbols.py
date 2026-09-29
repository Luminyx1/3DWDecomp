#!/usr/bin/env python3
"""Export the NSO symbol table and summarise it by top-level namespace.

Produces build/fury_symbols.csv (addr,size,bind,type,section,name) and prints a
breakdown of defined functions by demangled top-level scope, which is the first
cut at how the binary should be split into translation units.
"""

from __future__ import annotations

import csv
import re
import struct
import sys
from collections import Counter
from pathlib import Path

import nso as nsomod

STT = {0: "NOTYPE", 1: "OBJECT", 2: "FUNC", 6: "TLS"}
STB = {0: "LOCAL", 1: "GLOBAL", 2: "WEAK"}


def top_scope(mangled: str) -> str:
    """Cheap heuristic: pull the first namespace/class off an Itanium name."""
    if not mangled.startswith("_Z"):
        return "(C)"
    m = re.match(r"_ZN?(\d+)", mangled)
    if not m:
        return "(other)"
    n = int(m.group(1))
    start = m.end()
    return mangled[start:start + n] or "(other)"


def main(argv):
    nso = nsomod.parse(argv[0] if argv else "fury.nso")
    nsomod.unpack(nso)
    lo = nso.segments[0].memory_offset
    hi = nso.segments[-1].memory_offset + nso.segments[-1].decompressed_size
    img = bytearray(hi - lo)
    for s in nso.segments:
        img[s.memory_offset:s.memory_offset + s.decompressed_size] = s.data

    text = nso.seg(".text").data
    mod_off = struct.unpack_from("<I", text, 4)[0]
    dyn_va = mod_off + struct.unpack_from("<i", text, mod_off + 4)[0]
    dyn = {}
    o = dyn_va
    while True:
        tag, val = struct.unpack_from("<qQ", img, o)
        o += 16
        if tag == 0:
            break
        dyn.setdefault(tag, []).append(val)
    strtab, symtab = dyn[5][0], dyn[6][0]
    nsym = (strtab - symtab) // 24

    def name(off):
        end = img.index(b"\0", strtab + off)
        return img[strtab + off:end].decode("latin1")

    ranges = [(".text", *[(s.memory_offset, s.memory_offset + s.decompressed_size) for s in nso.segments if s.name == ".text"][0]),
              (".rodata", *[(s.memory_offset, s.memory_offset + s.decompressed_size) for s in nso.segments if s.name == ".rodata"][0]),
              (".data", *[(s.memory_offset, s.memory_offset + s.decompressed_size) for s in nso.segments if s.name == ".data"][0])]

    def section_of(v):
        for nm, a, b in ranges:
            if a <= v < b:
                return nm
        return "(abs)"

    out = Path("build/fury_symbols.csv")
    out.parent.mkdir(parents=True, exist_ok=True)
    func_bytes = Counter()
    func_count = Counter()
    total_func_bytes = 0
    with out.open("w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["addr", "size", "bind", "type", "section", "name"])
        for i in range(nsym):
            st_name, st_info, _o, st_shndx, st_value, st_size = struct.unpack_from(
                "<IBBHQQ", img, symtab + i * 24)
            nm = name(st_name)
            if not nm:
                continue
            typ = STT.get(st_info & 0xF, str(st_info & 0xF))
            bind = STB.get(st_info >> 4, "?")
            sec = "UND" if st_shndx == 0 else section_of(st_value)
            w.writerow([f"0x{st_value:x}", st_size, bind, typ, sec, nm])
            if typ == "FUNC" and sec == ".text":
                scope = top_scope(nm)
                func_bytes[scope] += st_size
                func_count[scope] += 1
                total_func_bytes += st_size

    print(f"wrote {out}  ({nsym} symbols)")
    print(f"\ndefined .text functions: {sum(func_count.values())}  "
          f"({total_func_bytes:,} bytes)\n")
    print(f"{'scope':28} {'funcs':>7} {'bytes':>12}  {'%':>5}")
    print("-" * 56)
    for scope, b in func_bytes.most_common(40):
        print(f"{scope:28} {func_count[scope]:>7} {b:>12,}  {100*b/total_func_bytes:>4.1f}%")


if __name__ == "__main__":
    main(sys.argv[1:])
