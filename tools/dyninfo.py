#!/usr/bin/env python3
"""Dump the dynamic-linking metadata from an unpacked NSO flat image.

The .dynamic array (found via MOD0) drives everything: it points at .dynsym,
.dynstr, the relocation tables and the symbol hash table.  This is where the
real symbol names live, so it is the single most useful thing to extract before
starting to carve the binary into translation units.
"""

from __future__ import annotations

import struct
import sys
from pathlib import Path

import nso as nsomod

DT = {
    0: "NULL", 1: "NEEDED", 2: "PLTRELSZ", 3: "PLTGOT", 4: "HASH", 5: "STRTAB",
    6: "SYMTAB", 7: "RELA", 8: "RELASZ", 9: "RELAENT", 10: "STRSZ", 11: "SYMENT",
    12: "INIT", 13: "FINI", 14: "SONAME", 15: "RPATH", 16: "SYMBOLIC", 17: "REL",
    18: "RELSZ", 19: "RELENT", 20: "PLTREL", 21: "DEBUG", 22: "TEXTREL",
    23: "JMPREL", 24: "BIND_NOW", 25: "INIT_ARRAY", 26: "FINI_ARRAY",
    27: "INIT_ARRAYSZ", 28: "FINI_ARRAYSZ", 29: "RUNPATH", 30: "FLAGS",
    0x6ffffef5: "GNU_HASH", 0x6ffffff9: "RELACOUNT", 0x6ffffffb: "FLAGS_1",
    0x6ffffff0: "VERSYM", 0x6ffffffe: "VERNEED", 0x6fffffff: "VERNEEDNUM",
}

R_AARCH64 = {257: "ABS64", 1024: "COPY", 1025: "GLOB_DAT", 1026: "JUMP_SLOT",
             1027: "RELATIVE", 1028: "TLS_TPREL64", 1029: "TLS_DTPREL64"}

STT = {0: "NOTYPE", 1: "OBJECT", 2: "FUNC", 3: "SECTION", 4: "FILE",
       5: "COMMON", 6: "TLS", 10: "GNU_IFUNC"}
STB = {0: "LOCAL", 1: "GLOBAL", 2: "WEAK"}


def main(argv):
    nso = nsomod.parse(argv[0] if argv else "fury.nso")
    nsomod.unpack(nso)
    lo = nso.segments[0].memory_offset
    hi = nso.segments[-1].memory_offset + nso.segments[-1].decompressed_size
    img = bytearray(hi - lo)
    for s in nso.segments:
        img[s.memory_offset - lo:s.memory_offset - lo + len(s.data)] = s.data

    text = nso.seg(".text").data
    mod_off = struct.unpack_from("<I", text, 4)[0]
    dyn_rel = struct.unpack_from("<i", text, mod_off + 4)[0]
    dyn_va = mod_off + dyn_rel

    dyn = {}
    off = dyn_va
    while True:
        tag, val = struct.unpack_from("<qQ", img, off)
        off += 16
        if tag == 0:
            break
        dyn.setdefault(DT.get(tag, hex(tag)), []).append(val)
    print("== .dynamic ==")
    for k, v in dyn.items():
        print(f"  {k:12} {', '.join(hex(x) for x in v)}")

    strtab = dyn["STRTAB"][0]
    strsz = dyn["STRSZ"][0]
    symtab = dyn["SYMTAB"][0]
    syment = dyn.get("SYMENT", [24])[0]

    def s(offset):
        end = img.index(b"\0", strtab + offset)
        return img[strtab + offset:end].decode("latin1")

    # symbol count: from GNU_HASH or up to strtab
    nsym = (strtab - symtab) // syment
    funcs = objs = imports = 0
    sample = []
    for i in range(nsym):
        st_name, st_info, st_other, st_shndx, st_value, st_size = struct.unpack_from(
            "<IBBHQQ", img, symtab + i * syment)
        typ = st_info & 0xF
        name = s(st_name)
        if st_shndx == 0 and name:
            imports += 1
        elif typ == 2:
            funcs += 1
        elif typ == 1:
            objs += 1
        if 20 <= i < 40:
            sample.append(f"  [{i}] {STB.get(st_info >> 4,'?'):6} {STT.get(typ,'?'):7} "
                          f"shndx={st_shndx:5} val=0x{st_value:<9x} sz={st_size:<6} {name}")

    print(f"\n== .dynsym ==  {nsym} symbols  (ent={syment})")
    print(f"  defined funcs : {funcs}")
    print(f"  defined objs  : {objs}")
    print(f"  undefined/imports : {imports}")
    print("  sample:")
    print("\n".join(sample))

    if "RELA" in dyn:
        rela = dyn["RELA"][0]
        relasz = dyn["RELASZ"][0]
        kinds = {}
        for r in range(rela, rela + relasz, 24):
            r_off, r_info, r_add = struct.unpack_from("<QQq", img, r)
            kinds[r_info & 0xffffffff] = kinds.get(r_info & 0xffffffff, 0) + 1
        print(f"\n== .rela.dyn ==  {relasz // 24} relocs")
        for k, c in sorted(kinds.items(), key=lambda x: -x[1]):
            print(f"  {R_AARCH64.get(k, k):12} {c}")

    if "JMPREL" in dyn:
        jr = dyn["JMPREL"][0]
        jrsz = dyn["PLTRELSZ"][0]
        print(f"\n== .rela.plt ==  {jrsz // 24} relocs")


if __name__ == "__main__":
    main(sys.argv[1:])
