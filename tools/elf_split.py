#!/usr/bin/env python3
"""Carve a per-translation-unit relocatable object out of build/fury.elf.

Given the section address ranges a TU owns (or an explicit list of symbol
names), copy the bytes into a fresh ELF `.o`, discover the symbols inside, and
reconstruct the AArch64 relocations the linker discarded:

  * BL / B  to another symbol            -> R_AARCH64_CALL26 / R_AARCH64_JUMP26
  * ADRP (+ADD)  direct data/text addr   -> ADR_PREL_PG_HI21 + ADD_ABS_LO12_NC
  * ADRP (+LDR/STR) direct addr          -> ADR_PREL_PG_HI21 + LDSTn_ABS_LO12_NC
  * ADRP (+LDR) through .got             -> ADR_GOT_PAGE + LD64_GOT_LO12_NC
  * ABS64 pointers in .data/.rodata      -> R_AARCH64_ABS64   (from .rela.dyn)

Data targets with no symbol are emitted as section-relative references and the
containing rodata/data bytes are pulled in as an anonymous local blob.

    python tools/elf_split.py --out build/foo.o --range .text:0x6b2540:0x6b256c
    python tools/elf_split.py --out build/foo.o SymbolA SymbolB      # by name
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import capstone
from capstone.arm64 import (ARM64_OP_IMM, ARM64_OP_MEM, ARM64_OP_REG,
                            ARM64_INS_ADRP, ARM64_INS_ADR, ARM64_INS_BL,
                            ARM64_INS_B, ARM64_INS_ADD, ARM64_INS_LDR,
                            ARM64_INS_LDRB, ARM64_INS_LDRH, ARM64_INS_LDRSW,
                            ARM64_INS_STR, ARM64_INS_STRB, ARM64_INS_STRH)

import elfobj as E
from furyimg import FuryImage

# AArch64 relocation type numbers
R_CALL26 = 283
R_JUMP26 = 282
R_ADR_PREL_PG_HI21 = 275
R_ADD_ABS_LO12_NC = 277
R_LDST8_ABS_LO12_NC = 278
R_LDST16_ABS_LO12_NC = 284
R_LDST32_ABS_LO12_NC = 285
R_LDST64_ABS_LO12_NC = 286
R_LDST128_ABS_LO12_NC = 299
R_ADR_GOT_PAGE = 311
R_LD64_GOT_LO12_NC = 312
R_ABS64 = 257
R_PREL32 = 261

_LDST_BY_ACCESS = {
    1: R_LDST8_ABS_LO12_NC, 2: R_LDST16_ABS_LO12_NC, 4: R_LDST32_ABS_LO12_NC,
    8: R_LDST64_ABS_LO12_NC, 16: R_LDST128_ABS_LO12_NC,
}
_ACCESS_BITS = {
    ARM64_INS_LDRB: 1, ARM64_INS_STRB: 1, ARM64_INS_LDRH: 2, ARM64_INS_STRH: 2,
    ARM64_INS_LDRSW: 4,
}

_MD = capstone.Cs(capstone.CS_ARCH_ARM64, capstone.CS_MODE_LITTLE_ENDIAN)
_MD.detail = True


_SEC_FLAGS = {
    ".text": E.SHF_ALLOC | E.SHF_EXEC,
    ".rodata": E.SHF_ALLOC,
    ".data": E.SHF_ALLOC | E.SHF_WRITE,
    ".bss": E.SHF_ALLOC | E.SHF_WRITE,
}


class Carver:
    def __init__(self, img: FuryImage):
        self.img = img
        self.o = E.ElfObject()
        # one section per included symbol, named like clang -ffunction-sections /
        # -fdata-sections output: ".text.<sym>", ".rodata.<sym>", etc.
        self.owner: dict[str, E.Section] = {}          # symname -> its section
        self.placed: dict[str, E.Section] = {}         # symname -> section (offset always 0)
        self.included: set[str] = set()
        self.undefs: set[str] = set()
        self.anon: dict[int, tuple[E.Section, int]] = {}   # src addr -> (section, offset)
        self.sizes: dict[str, int] = {}                    # symname -> effective size
        self.ranges: list[tuple[str, int, int]] = []       # (section, lo, hi) owned by this TU

    def _in_ranges(self, addr: int) -> bool:
        return any(lo <= addr < hi for _, lo, hi in self.ranges)

    def _own_section(self, base: str, sym_name: str) -> E.Section:
        nm = f"{base}.{sym_name}"
        typ = E.SHT_NOBITS if base == ".bss" else E.SHT_PROGBITS
        return self.o.section(nm, typ, _SEC_FLAGS[base],
                              align=4 if base == ".text" else 8)

    def _anon_section(self, base: str) -> E.Section:
        return self.o.section(base, E.SHT_PROGBITS, _SEC_FLAGS[base], align=8)

    # -- placement -------------------------------------------------------
    def _place(self, s, size):
        base = s.section
        sec = self._own_section(base, s.name)
        if base == ".bss":
            sec.nobits_size = size
        else:
            sec.data += self.img.bytes_at(s.addr, size)
        self.owner[s.name] = sec
        self.placed[s.name] = sec
        self.sizes[s.name] = size
        self.included.add(s.name)
        self.o.define(s.name, sec, 0, size,
                      typ=E.STT_FUNC if base == ".text" else E.STT_OBJECT,
                      bind=E.STB_WEAK if s.bind == "STB_WEAK" else E.STB_GLOBAL)

    def include(self, names: list[str]):
        for n in names:
            s = self.img.sym(n)
            if s is None:
                raise SystemExit(f"unknown symbol: {n}")
            if s.section not in _SEC_FLAGS:
                raise SystemExit(f"{n}: not a text/data symbol (section {s.section})")
        for n in sorted(names, key=lambda n: self.img.sym(n).addr):
            s = self.img.sym(n)
            self._place(s, s.size)

    def include_ranges(self, ranges: list[tuple[str, int, int]]):
        """ranges: list of (section, start, end).  Every defined symbol that
        falls in a range is carved out (in address order)."""
        self.ranges = ranges
        seen: list[tuple[int, int]] = []
        for base, lo, hi in ranges:
            if base not in _SEC_FLAGS:
                raise SystemExit(f"bad section in range: {base}")
            got = self.img.defs_in_sized(lo, hi, section=base)
            if not got:
                print(f"  warning: no symbols in {base} [{lo:#x}, {hi:#x})", file=sys.stderr)
            for s, size in got:
                self._place(s, size)
                seen.append((s.addr, s.addr + size))

    # -- relocation reconstruction -------------------------------------
    def process(self):
        for name in list(self.included):
            if self.img.sym(name).section == ".text":
                self._recon_text(name)
        self._recon_data_abs64()
        for u in sorted(self.undefs - self.included):
            self.o.undef(u)

    def _ref(self, target_addr: int):
        """Map an absolute target address to (symname_or_section, addend)."""
        cov = self.img.sym_covering(target_addr)
        if cov and cov[0].name in self.included:
            return cov[0].name, cov[1]
        if cov and cov[0].size and cov[1] < cov[0].size:
            self.undefs.add(cov[0].name)
            return cov[0].name, cov[1]
        if cov and cov[1] == 0:
            self.undefs.add(cov[0].name)
            return cov[0].name, 0
        # no symbol: anonymous data — pull the bytes into a local blob
        return self._anon(target_addr)

    def _anon(self, addr: int):
        nm = self.img.section_of(addr)
        if nm not in (".rodata", ".data"):
            return None
        # only pull in unnamed bytes that this TU actually owns; a reference to
        # unnamed data outside our ranges is a bug in the split (or a missing
        # symbol) - leave it unrelocated and let the diff show it
        if self.ranges and not self._in_ranges(addr):
            return None
        if addr in self.anon:
            sec, o = self.anon[addr]
            return sec.name, o
        # extent: to next defined symbol, the end of our range, or +64 bytes
        nxt = self.img.defs_in(addr + 1, addr + 0x400)
        end = nxt[0].addr if nxt else addr + 0x40
        for _, lo, hi in self.ranges:
            if lo <= addr < hi:
                end = min(end, hi)
        sec = self._anon_section(nm)
        while len(sec.data) % 8:
            sec.data.append(0)
        o = len(sec.data)
        sec.data += self.img.bytes_at(addr, end - addr)
        self.anon[addr] = (sec, o)
        return sec.name, o

    def _recon_text(self, name: str):
        s = self.img.sym(name)
        sec = self.owner[name]
        sz = self.sizes[name]
        code = self.img.bytes_at(s.addr, sz)
        fn_lo, fn_hi = s.addr, s.addr + sz
        insns = list(_MD.disasm(code, s.addr))
        for idx, ins in enumerate(insns):
            rel_off = ins.address - s.addr
            gid = ins.id
            if gid == ARM64_INS_BL:
                ref = self._ref(ins.operands[0].imm)
                if ref:
                    sec.reloc(rel_off, ref[0], R_CALL26, ref[1])
            elif gid == ARM64_INS_B:
                tgt = ins.operands[0].imm
                if fn_lo <= tgt < fn_hi:
                    continue  # local branch
                ref = self._ref(tgt)
                if ref:
                    sec.reloc(rel_off, ref[0], R_JUMP26, ref[1])
            elif gid == ARM64_INS_ADRP:
                page = ins.operands[1].imm
                reg = ins.operands[0].reg
                lo12, acc, kind = self._find_lo12(insns, idx, reg)
                if lo12 is None:
                    ref = self._ref(page)
                    if ref:
                        sec.reloc(rel_off, ref[0], R_ADR_PREL_PG_HI21, ref[1])
                    continue
                tgt = page + lo12
                lo12_off = kind[1].address - s.addr
                if self.img.got_range[0] <= tgt < self.img.got_range[1]:
                    got = self.img.got_target(tgt)
                    if got:
                        self.undefs.add(got[0])
                        sec.reloc(rel_off, got[0], R_ADR_GOT_PAGE, got[1])
                        sec.reloc(lo12_off, got[0], R_LD64_GOT_LO12_NC, got[1])
                    continue
                ref = self._ref(tgt)
                if not ref:
                    continue
                sec.reloc(rel_off, ref[0], R_ADR_PREL_PG_HI21, ref[1])
                if kind[0] == "add":
                    sec.reloc(lo12_off, ref[0], R_ADD_ABS_LO12_NC, ref[1])
                else:
                    sec.reloc(lo12_off, ref[0], _LDST_BY_ACCESS.get(acc, R_LDST64_ABS_LO12_NC), ref[1])

    def _find_lo12(self, insns, idx, reg):
        """Scan forward for the instruction that consumes `reg` with an imm."""
        for j in range(idx + 1, min(idx + 12, len(insns))):
            ins = insns[j]
            ops = ins.operands
            if ins.id == ARM64_INS_ADD and len(ops) >= 3 and ops[1].type == ARM64_OP_REG \
                    and ops[1].reg == reg and ops[2].type == ARM64_OP_IMM:
                return ops[2].imm, 8, ("add", ins)
            if ins.id in (ARM64_INS_LDR, ARM64_INS_LDRB, ARM64_INS_LDRH, ARM64_INS_LDRSW,
                          ARM64_INS_STR, ARM64_INS_STRB, ARM64_INS_STRH):
                for op in ops:
                    if op.type == ARM64_OP_MEM and op.mem.base == reg:
                        acc = _ACCESS_BITS.get(ins.id)
                        if acc is None:
                            # LDR/STR: width from the dest/src register name
                            rn = ins.reg_name(ops[0].reg) or "x"
                            acc = 4 if rn[0] == "w" else (16 if rn[0] in "qv" else 8)
                        return op.mem.disp, acc, ("ldst", ins)
            # reg overwritten before use -> give up
            if ops and ops[0].type == ARM64_OP_REG and ops[0].reg == reg and ins.id == ARM64_INS_ADRP:
                return None, None, None
        return None, None, None

    def _recon_data_abs64(self):
        for name in list(self.included):
            s = self.img.sym(name)
            if s.section not in (".rodata", ".data"):
                continue
            sec = self.owner[name]
            for addr in range(s.addr, s.addr + self.sizes[name], 8):
                ent = self.img.dyn_relocs.get(addr)
                if not ent:
                    continue
                rtype, nm, add = ent
                if rtype == R_ABS64 and nm:
                    self.undefs.add(nm)
                    sec.reloc(addr - s.addr, nm, R_ABS64, add)
                elif rtype in (R_ABS64, 1027):  # RELATIVE: addend is the absolute VA
                    cov = self.img.sym_covering(add)
                    if cov:
                        self.undefs.add(cov[0].name)
                        sec.reloc(addr - s.addr, cov[0].name, R_ABS64, cov[1])

    def write(self, path: str):
        # drop any empty section we speculatively created
        for s in list(self.o._secs):
            if s.type != E.SHT_NOBITS and not s.data and not s.relocs:
                self.o._secs.remove(s)
                del self.o._by_name[s.name]
        Path(path).parent.mkdir(parents=True, exist_ok=True)
        Path(path).write_bytes(self.o.build())


def _parse_range(tok: str) -> tuple[str, int, int]:
    # ".text:0x1000:0x1234"  or  ".text:0x1000-0x1234"
    sec, _, rest = tok.partition(":")
    rest = rest.replace("-", ":")
    lo, _, hi = rest.partition(":")
    return sec, int(lo, 0), int(hi, 0)


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    ap.add_argument("--elf", default="build/fury.elf")
    ap.add_argument("--range", action="append", default=[], metavar="SEC:LO:HI",
                    help="a section range this TU owns (repeatable)")
    ap.add_argument("symbols", nargs="*", help="explicit symbol names (alternative to --range)")
    a = ap.parse_args(argv)

    if not a.range and not a.symbols:
        ap.error("give at least one --range or symbol name")

    img = FuryImage(a.elf)
    c = Carver(img)
    if a.range:
        c.include_ranges([_parse_range(t) for t in a.range])
    if a.symbols:
        c.include(list(a.symbols))
    c.process()
    c.write(a.out)
    print(f"wrote {a.out}: {len(c.included)} symbols, "
          f"{sum(len(s.relocs) for s in c.o._secs)} relocs, "
          f"{len(c.undefs - c.included)} undefined refs")


if __name__ == "__main__":
    main()
