#!/usr/bin/env python3
"""Load build/fury.elf and expose the game image for analysis / splitting.

Wraps pyelftools with the lookups the splitter needs: address -> symbol,
name -> symbol, the .text/.rodata/.data/.bss ranges, the GOT, and the dynamic
relocations (ABS64 / RELATIVE / GLOB_DAT) keyed by target address.
"""

from __future__ import annotations

import bisect
from dataclasses import dataclass
from functools import cached_property
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from elftools.elf.relocation import RelocationSection

R_AARCH64_ABS64 = 257
R_AARCH64_GLOB_DAT = 1025
R_AARCH64_JUMP_SLOT = 1026
R_AARCH64_RELATIVE = 1027


@dataclass(frozen=True)
class Symbol:
    name: str
    addr: int
    size: int
    type: str        # 'STT_FUNC' | 'STT_OBJECT' | ...
    bind: str        # 'STB_GLOBAL' | 'STB_WEAK' | 'STB_LOCAL'
    section: str     # '.text' | '.rodata' | '.data' | '.bss' | 'UND' | 'ABS'


class FuryImage:
    def __init__(self, path="build/fury.elf"):
        self._f = open(path, "rb")
        self.elf = ELFFile(self._f)

        self.ranges: dict[str, tuple[int, int]] = {}
        for nm in (".text", ".rodata", ".data", ".bss"):
            s = self.elf.get_section_by_name(nm)
            self.ranges[nm] = (s["sh_addr"], s["sh_addr"] + s["sh_size"])
        got = self.elf.get_section_by_name(".got")
        self.got_range = (got["sh_addr"], got["sh_addr"] + got["sh_size"]) if got else (0, 0)

        self._sec_data: dict[str, bytes] = {}
        for nm in (".text", ".rodata", ".data"):
            self._sec_data[nm] = self.elf.get_section_by_name(nm).data()

        self.symbols: list[Symbol] = []
        self._by_name: dict[str, Symbol] = {}
        dynsym = self.elf.get_section_by_name(".dynsym")
        for sym in dynsym.iter_symbols():
            if not sym.name:
                continue
            addr = sym["st_value"]
            shn = sym["st_shndx"]
            if shn == "SHN_UNDEF":
                sec = "UND"
            elif shn == "SHN_ABS":
                sec = "ABS"
            else:
                sec = self._section_of(addr)
            s = Symbol(sym.name, addr, sym["st_size"],
                       sym["st_info"]["type"], sym["st_info"]["bind"], sec)
            self.symbols.append(s)
            # first definition wins for name lookup; prefer defined over undef
            if s.name not in self._by_name or (self._by_name[s.name].section == "UND" and sec != "UND"):
                self._by_name[s.name] = s

        # address index over *defined* symbols with a real section
        self._defs = sorted((s for s in self.symbols if s.section in (".text", ".rodata", ".data", ".bss")),
                            key=lambda s: s.addr)
        self._def_addrs = [s.addr for s in self._defs]

        # dynamic relocations, keyed by r_offset (the patched location)
        self.dyn_relocs: dict[int, tuple[int, str, int]] = {}   # off -> (type, symname, addend)
        for sec in self.elf.iter_sections():
            if not isinstance(sec, RelocationSection):
                continue
            symsec = self.elf.get_section(sec["sh_link"])
            for r in sec.iter_relocations():
                info_sym = r["r_info_sym"]
                nm = ""
                if isinstance(symsec, SymbolTableSection) and info_sym:
                    nm = symsec.get_symbol(info_sym).name
                self.dyn_relocs[r["r_offset"]] = (r["r_info_type"], nm, r["r_addend"])

    # ------------------------------------------------------------------
    def _section_of(self, addr: int) -> str:
        for nm, (lo, hi) in self.ranges.items():
            if lo <= addr < hi:
                return nm
        return "ABS"

    def section_of(self, addr: int) -> str:
        return self._section_of(addr)

    def bytes_at(self, addr: int, size: int) -> bytes:
        nm = self._section_of(addr)
        if nm == ".bss":
            return b"\0" * size
        lo, _ = self.ranges[nm]
        return self._sec_data[nm][addr - lo: addr - lo + size]

    def sym(self, name: str) -> Symbol | None:
        return self._by_name.get(name)

    def defs_in(self, lo: int, hi: int) -> list[Symbol]:
        i = bisect.bisect_left(self._def_addrs, lo)
        j = bisect.bisect_left(self._def_addrs, hi)
        return self._defs[i:j]

    def defs_in_sized(self, lo: int, hi: int, section: str | None = None):
        """Defined symbols in [lo, hi), each with an effective size (a 0-size
        symbol's size is inferred as the gap to the next symbol / hi)."""
        i = bisect.bisect_left(self._def_addrs, lo)
        j = bisect.bisect_left(self._def_addrs, hi)
        out = []
        for k in range(i, j):
            s = self._defs[k]
            if section and s.section != section:
                continue
            nxt = self._def_addrs[k + 1] if k + 1 < len(self._defs) else hi
            size = s.size if s.size else max(min(nxt, hi) - s.addr, 0)
            out.append((s, size))
        return out

    def sym_covering(self, addr: int) -> tuple[Symbol, int] | None:
        """Return (symbol, addend) for the defined symbol whose range covers addr."""
        i = bisect.bisect_right(self._def_addrs, addr) - 1
        while i >= 0:
            s = self._defs[i]
            span = s.size if s.size else (self._def_addrs[i + 1] - s.addr if i + 1 < len(self._defs) else 1 << 30)
            if s.addr <= addr < s.addr + max(span, 1):
                return s, addr - s.addr
            # zero-size symbol exactly at addr
            if s.addr == addr:
                return s, 0
            i -= 1
        return None

    def got_target(self, got_addr: int) -> tuple[str, int] | None:
        """Resolve a .got slot to (symname, addend) via its dynamic reloc."""
        ent = self.dyn_relocs.get(got_addr)
        if ent is None:
            return None
        rtype, nm, add = ent
        if rtype in (R_AARCH64_GLOB_DAT, R_AARCH64_JUMP_SLOT) and nm:
            return nm, add
        if rtype == R_AARCH64_RELATIVE:
            cov = self.sym_covering(add)
            if cov:
                return cov[0].name, cov[1]
        return None
