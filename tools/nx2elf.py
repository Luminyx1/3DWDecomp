#!/usr/bin/env python3
"""Convert an unpacked NSO into a normal AArch64 ELF shared object.

The result is a static, self-consistent ET_DYN ELF whose virtual addresses match
the NSO's memory layout (file offset == vaddr), so any address seen in a
disassembler is the same address the game uses.  Sections are reconstructed from
the .dynamic metadata: .dynsym/.dynstr give real symbol names, and the SHT_NOBITS
.bss is appended.  This ELF is what objdiff, llvm-objdump, Ghidra and IDA consume.

    python tools/nx2elf.py fury.nso -o build/fury.elf
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

import nso as nsomod

EM_AARCH64 = 183
ET_DYN = 3
PT_LOAD, PT_DYNAMIC = 1, 2
PF_X, PF_W, PF_R = 1, 2, 4
SHT_PROGBITS, SHT_SYMTAB, SHT_STRTAB, SHT_RELA, SHT_HASH, SHT_DYNAMIC = 1, 2, 3, 4, 5, 6
SHT_NOBITS, SHT_DYNSYM, SHT_GNU_HASH = 8, 11, 0x6FFFFFF6
SHF_WRITE, SHF_ALLOC, SHF_EXEC = 1, 2, 4


def build(nso: nsomod.Nso) -> bytes:
    nsomod.unpack(nso)
    text, ro, data = (nso.seg(n) for n in (".text", ".rodata", ".data"))
    bss_start = data.memory_offset + data.decompressed_size
    image_end = bss_start + nso.bss_size

    # flat image covering text..data at their vaddrs
    img = bytearray(image_end)
    for s in (text, ro, data):
        img[s.memory_offset:s.memory_offset + s.decompressed_size] = s.data

    # ---- read .dynamic ----
    mod_off = struct.unpack_from("<I", text.data, 4)[0]
    dyn_va = mod_off + struct.unpack_from("<i", text.data, mod_off + 4)[0]
    dyn = {}
    o = dyn_va
    while True:
        tag, val = struct.unpack_from("<qQ", img, o)
        o += 16
        if tag == 0:
            break
        dyn.setdefault(tag, []).append(val)
    g = lambda t, d=0: dyn[t][0] if t in dyn else d

    strtab, strsz = g(5), g(10)
    symtab = g(6)
    nsym = (strtab - symtab) // 24
    rela, relasz = g(7), g(8)
    jmprel, pltrelsz = g(23), g(2)
    gnu_hash = g(0x6FFFFEF5)
    hash_ = g(4)
    init_array, init_arraysz = g(25), g(27)
    fini_array, fini_arraysz = g(26), g(28)
    pltgot = g(3)

    dyn_end = o
    dyn_size = dyn_end - dyn_va

    # ---- section table ----
    shstr = bytearray(b"\0")
    def name(s: str) -> int:
        i = len(shstr)
        shstr.extend(s.encode() + b"\0")
        return i

    secs = []  # [name, type, flags, addr, offset, size, link, info, align, entsize]
    def add(nm, typ, flags, addr, size, link=0, info=0, align=8, entsize=0):
        secs.append([name(nm), typ, flags, addr, addr, size, link, info, align, entsize])
        return len(secs) - 1  # 0-based index of the section just added

    add("", 0, 0, 0, 0, align=0)
    add(".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXEC, text.memory_offset, text.decompressed_size, align=0x1000)
    add(".rodata", SHT_PROGBITS, SHF_ALLOC, ro.memory_offset, ro.decompressed_size, align=0x1000)
    add(".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, data.memory_offset, data.decompressed_size, align=0x1000)
    add(".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE, bss_start, nso.bss_size, align=0x1000)

    if gnu_hash:
        add(".gnu.hash", SHT_GNU_HASH, SHF_ALLOC, gnu_hash, (symtab - gnu_hash), align=8)
    if hash_:
        nbucket = struct.unpack_from("<I", img, hash_)[0]
        nchain = struct.unpack_from("<I", img, hash_ + 4)[0]
        add(".hash", SHT_HASH, SHF_ALLOC, hash_, 8 + 4 * (nbucket + nchain), align=8, entsize=4)

    i_dynstr = add(".dynstr", SHT_STRTAB, SHF_ALLOC, strtab, strsz, align=1)

    # Rebuild .dynsym with st_shndx remapped to *our* section indices (the NSO's
    # original indices are meaningless here).  Emitted into the trailing area.
    SHN_ABS = 0xFFF1
    ranges = [
        (1, text.memory_offset, text.memory_offset + text.decompressed_size),
        (2, ro.memory_offset, ro.memory_offset + ro.decompressed_size),
        (3, data.memory_offset, data.memory_offset + data.decompressed_size),
        (4, bss_start, image_end),
    ]
    new_dynsym = bytearray()
    first_global = nsym
    for i in range(nsym):
        st_name, st_info, st_other, st_shndx, st_value, st_size = struct.unpack_from(
            "<IBBHQQ", img, symtab + i * 24)
        if st_shndx == 0:
            shndx = 0
        else:
            shndx = SHN_ABS
            for idx, lo_, hi_ in ranges:
                if lo_ <= st_value < hi_:
                    shndx = idx
                    break
        if (st_info >> 4) != 0 and i < first_global:
            first_global = i
        new_dynsym += struct.pack("<IBBHQQ", st_name, st_info, st_other, shndx, st_value, st_size)

    i_dynsym = add(".dynsym", SHT_DYNSYM, SHF_ALLOC, symtab, len(new_dynsym),
                   link=i_dynstr, info=first_global, align=8, entsize=24)

    if rela:
        add(".rela.dyn", SHT_RELA, SHF_ALLOC, rela, relasz, link=i_dynsym, align=8, entsize=24)
    if jmprel:
        add(".rela.plt", SHT_RELA, SHF_ALLOC, jmprel, pltrelsz, link=i_dynsym, align=8, entsize=24)
    if init_array:
        add(".init_array", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, init_array, init_arraysz, align=8)
    if fini_array and fini_arraysz:
        add(".fini_array", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, fini_array, fini_arraysz, align=8)
    add(".dynamic", SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE, dyn_va, dyn_size, link=i_dynstr, align=8, entsize=16)
    if pltgot:
        add(".got", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, pltgot, (data.memory_offset + data.decompressed_size) - pltgot, align=8)

    if nso.api_info[1]:
        add(".api_info", SHT_PROGBITS, SHF_ALLOC,
            ro.memory_offset + nso.api_info[0], nso.api_info[1], align=1)

    build_id = bytes.fromhex(nso.module_id)[:20]
    note = struct.pack("<III", 4, len(build_id), 3) + b"GNU\0" + build_id
    i_note = add(".note.gnu.build-id", 7, SHF_ALLOC, 0, len(note), align=4)  # SHT_NOTE

    i_shstr = add(".shstrtab", SHT_STRTAB, 0, 0, 0, align=1)

    # ---- file layout ----
    # EHDR + PHDRs live at offset 0, followed by the build-id note; the three
    # loadable segments follow on page boundaries; non-alloc data (rebuilt
    # .dynsym, .shstrtab, section headers) is appended last.  File offset !=
    # vaddr, which every analysis tool handles.
    ehsize, phentsize, shentsize = 64, 56, 64
    n_ph = 6
    seg_file = {}

    out = bytearray(ehsize + n_ph * phentsize)
    note_off = len(out)
    out.extend(note)
    secs[i_note][4] = note_off
    for s in (text, ro, data):
        while len(out) % 0x1000:
            out.append(0)
        seg_file[s.name] = len(out)
        out.extend(s.data)

    def va_to_off(va: int) -> int:
        for s in (text, ro, data):
            if s.memory_offset <= va < s.memory_offset + s.decompressed_size:
                return seg_file[s.name] + (va - s.memory_offset)
        if bss_start <= va <= image_end:
            return seg_file[".data"] + data.decompressed_size  # nominal
        raise ValueError(f"va 0x{va:x} not mapped")

    # fix up every alloc section's file offset from its vaddr (skip .bss and the
    # sections whose offset was already placed by hand: the note, and .dynsym
    # whose rebuilt copy goes in the trailing area below)
    explicit = {i_note, i_dynsym}
    for i, s in enumerate(secs):
        if i in explicit or s[1] == SHT_NOBITS:
            continue
        if s[2] & SHF_ALLOC:
            s[4] = va_to_off(s[3])

    while len(out) % 8:
        out.append(0)
    secs[i_dynsym][4] = len(out)          # rebuilt .dynsym
    out.extend(new_dynsym)

    secs[i_shstr][4] = len(out)
    secs[i_shstr][5] = len(shstr)
    out.extend(shstr)
    for s in secs:
        if s[1] == SHT_NOBITS:
            s[4] = seg_file[".data"] + data.decompressed_size

    while len(out) % 8:
        out.append(0)
    sh_off = len(out)
    for nm, typ, flags, addr, offset, size, link, info, align, entsize in secs:
        out += struct.pack("<IIQQQQIIQQ", nm, typ, flags, addr, offset, size, link, info, align, entsize)

    ph_off = ehsize
    phdrs = [
        (PT_LOAD, PF_R | PF_X, seg_file[".text"], text.memory_offset, text.decompressed_size, text.decompressed_size, 0x1000),
        (PT_LOAD, PF_R, seg_file[".rodata"], ro.memory_offset, ro.decompressed_size, ro.decompressed_size, 0x1000),
        (PT_LOAD, PF_R | PF_W, seg_file[".data"], data.memory_offset, data.decompressed_size,
         data.decompressed_size, 0x1000),
        (PT_LOAD, PF_R | PF_W, seg_file[".data"] + data.decompressed_size, bss_start, 0,
         nso.bss_size, 0x1000),
        (PT_DYNAMIC, PF_R | PF_W, va_to_off(dyn_va), dyn_va, dyn_size, dyn_size, 8),
        (4, PF_R, note_off, note_off, len(note), len(note), 4),  # PT_NOTE
    ]
    ph_blob = b"".join(
        struct.pack("<IIQQQQQQ", p_type, p_flags, p_off, p_va, p_va, p_fsz, p_msz, p_al)
        for p_type, p_flags, p_off, p_va, p_fsz, p_msz, p_al in phdrs
    )
    out[ph_off:ph_off + len(ph_blob)] = ph_blob

    out[0:ehsize] = struct.pack(
        "<16sHHIQQQIHHHHHH",
        b"\x7fELF" + bytes([2, 1, 1, 0]) + b"\0" * 8,
        ET_DYN, EM_AARCH64, 1,
        0,  # e_entry
        ph_off, sh_off, 0,
        ehsize, phentsize, len(phdrs), shentsize, len(secs), i_shstr,
    )
    return bytes(out)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("file")
    ap.add_argument("-o", "--out", default="build/fury.elf")
    args = ap.parse_args(argv)
    nso = nsomod.parse(args.file)
    blob = build(nso)
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    Path(args.out).write_bytes(blob)
    print(f"wrote {args.out}  ({len(blob):,} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
