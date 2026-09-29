#!/usr/bin/env python3
"""Remove AArch64 mapping symbols ($x, $x.N, $d, $d.N) from an ELF64 object, in place.

clang emits a `$d.N` / `$x.N` mapping symbol at the start of every data/code
section.  objdiff treats them as ordinary symbols, and when it resolves a
section-relative relocation (e.g. `.bss..L_MergedGlobals+0`) to "the best
symbol at that address" it picks the zero-sized `$d.N` instead of the real
variable, so our side of the diff shows `$d.10` where the game shows `sMalloc`.
Stripping them after compiling avoids that; they carry no meaning for matching.

    python tools/strip_mapsyms.py build/obj/foo.o [more.o ...]
"""

from __future__ import annotations

import struct
import sys

SHT_SYMTAB, SHT_RELA, SHT_REL = 2, 4, 9


def is_mapping(name: bytes) -> bool:
    return name in (b"$x", b"$d") or name.startswith((b"$x.", b"$d."))


def strip(path: str) -> int:
    with open(path, "rb") as fh:
        buf = bytearray(fh.read())
    if buf[:4] != b"\x7fELF" or buf[4] != 2 or buf[5] != 1:
        return 0
    e_shoff, = struct.unpack_from("<Q", buf, 0x28)
    e_shentsize, e_shnum = struct.unpack_from("<HH", buf, 0x3A)
    shdrs = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        shdrs.append(list(struct.unpack_from("<IIQQQQIIQQ", buf, off)) + [off])
    removed_total = 0
    for sh in shdrs:
        if sh[1] != SHT_SYMTAB:
            continue
        sym_off, sym_size, link, info, entsize = sh[4], sh[5], sh[6], sh[7], sh[9] or 24
        str_off = shdrs[link][4]
        n = sym_size // entsize
        keep, remap = [], {}
        first_global_new = None
        for i in range(n):
            ent = bytes(buf[sym_off + i * entsize: sym_off + (i + 1) * entsize])
            st_name, st_info = struct.unpack_from("<IB", ent, 0)
            end = buf.index(b"\0", str_off + st_name)
            name = bytes(buf[str_off + st_name:end])
            if i != 0 and (st_info >> 4) == 0 and is_mapping(name):
                continue
            remap[i] = len(keep)
            if i >= info and first_global_new is None:
                first_global_new = len(keep)
            keep.append(ent)
        removed = n - len(keep)
        if not removed:
            continue
        removed_total += removed
        if first_global_new is None:
            first_global_new = len(keep)
        new = b"".join(keep)
        buf[sym_off:sym_off + sym_size] = new + b"\0" * (sym_size - len(new))
        sh[5] = len(new)
        sh[7] = first_global_new
        struct.pack_into("<IIQQQQIIQQ", buf, sh[10], *sh[:10])
        symtab_idx = shdrs.index(sh)
        for rs in shdrs:
            if rs[1] not in (SHT_RELA, SHT_REL) or rs[6] != symtab_idx:
                continue
            ent = 24 if rs[1] == SHT_RELA else 16
            for j in range(rs[5] // ent):
                o = rs[4] + j * ent
                r_info, = struct.unpack_from("<Q", buf, o + 8)
                sym, typ = r_info >> 32, r_info & 0xFFFFFFFF
                if sym:
                    struct.pack_into("<Q", buf, o + 8, (remap[sym] << 32) | typ)
    if removed_total:
        with open(path, "wb") as fh:
            fh.write(buf)
    return removed_total


def main(argv: list[str]) -> int:
    for p in argv:
        strip(p)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
