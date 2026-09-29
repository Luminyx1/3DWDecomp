#!/usr/bin/env python3
"""Name every nerve in the binary from the `exe` function it calls.

A nerve's `execute` is a tiny local function, `ldr x0, [x1]; b Foo::exeWait()`
(NERVE_DECL: `keeper->getParent<Foo>()->exeWait()`).  Its vtable points at it
and the nerve object (vptr only) follows that vtable.  So from the exe name we
get, with the project's naming (see NERVE_DECL / NERVES_MAKE_NOSTRUCT):

    execute  (anonymous namespace)::FooNrvWait::execute(al::NerveKeeper*) const
    object   (anonymous namespace)::NrvFooWait            -> symbols.txt
    vtable   vtable for (anonymous namespace)::FooNrvWait  -> symbols.txt

Only `sub_`/`nullsub_` rows of the function map are renamed, and names that
would clash (the same Foo::exeWait reached by two nerves) are skipped.

    python tools/name_nerves.py [--dry-run]
"""

from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

import capstone

sys.path.insert(0, str(Path(__file__).parent))
from furyimg import FuryImage, BASE, R_AARCH64_RELATIVE  # noqa: E402

UNNAMED = re.compile(r"^(sub_|nullsub_)")
EXE = re.compile(r"^_ZN(.*?)(\d+)exe(\w+)Ev$")
MD = capstone.Cs(capstone.CS_ARCH_ARM64, capstone.CS_MODE_LITTLE_ENDIAN)


def src_name(n: str) -> str:
    return f"{len(n)}{n}"


def parse_exe(name: str):
    """_ZN3Foo7exeWaitEv -> ('Foo', 'Wait'); nested classes use the last name."""
    if not name.startswith("_ZN") or not name.endswith("Ev"):
        return None
    q, toks = name[3:-2], []
    while q:
        mm = re.match(r"^(\d+)", q)
        if not mm:
            return None
        n, start = int(mm.group(1)), len(mm.group(1))
        toks.append(q[start:start + n])
        q = q[start + n:]
    if len(toks) < 2 or not toks[-1].startswith("exe") or len(toks[-1]) <= 3:
        return None
    return toks[-2], toks[-1][3:]


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", default="1.0.0")
    ap.add_argument("--elf", default="build/fury.elf")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args(argv)

    cfg = Path("config") / a.version
    fmap, syms = cfg / "function_map.tsv", cfg / "symbols.txt"
    img = FuryImage(a.elf, extra_map=str(fmap), extra_syms=str(syms))

    # pointer word -> its target, for RELATIVE relocs (vtable slots, vptrs)
    rel_to = collections.defaultdict(list)
    for off, (typ, _nm, add) in img.dyn_relocs.items():
        if typ == R_AARCH64_RELATIVE:
            rel_to[add].append(off)

    proposals = {}   # func addr -> (cls, action)
    for s in img.symbols:
        if s.section != ".text" or s.bind != "STB_LOCAL" or not UNNAMED.match(s.name) or s.size > 16:
            continue
        insns = list(MD.disasm(img.bytes_at(s.addr, s.size), s.addr))
        if not insns or insns[-1].mnemonic != "b" or not insns[0].op_str.startswith(("x0, [x1", )):
            continue
        try:
            tgt = int(insns[-1].op_str.lstrip("#"), 16)
        except ValueError:
            continue
        cov = img.sym_covering(tgt)
        if not cov or cov[1] != 0:
            continue
        pe = parse_exe(cov[0].name)
        if pe:
            proposals[s.addr] = pe

    names = collections.Counter(f"{c}Nrv{act}" for c, act in proposals.values())
    func_new, data_new = {}, []
    for addr, (cls, act) in proposals.items():
        struct = f"{cls}Nrv{act}"
        if names[struct] > 1:
            continue
        mangled = f"_ZNK12_GLOBAL__N_1{src_name(struct)}7executeEPN2al11NerveKeeperE"
        dem = f"(anonymous namespace)::{struct}::execute(al::NerveKeeper*) const"
        func_new[addr] = (mangled, dem)
        slots = rel_to.get(addr, [])
        if len(slots) == 1:
            slot = slots[0]
            vt = slot - 0x10
            objs = rel_to.get(slot, [])
            data_new.append((vt, 0x20, f"_ZTVN12_GLOBAL__N_1{src_name(struct)}E"))
            if len(objs) == 1:
                data_new.append((objs[0], 8, f"_ZN12_GLOBAL__N_1{src_name('Nrv' + cls + act)}E"))

    lines = fmap.read_text(encoding="utf-8").split("\n")
    renamed = 0
    for i in range(1, len(lines)):
        c = lines[i].split("\t")
        if len(c) < 6 or not UNNAMED.match(c[4]):
            continue
        addr = int(c[0], 16) - BASE
        if addr in func_new:
            c[4], c[5] = func_new[addr]
            lines[i] = "\t".join(c)
            renamed += 1
    existing = syms.read_text(encoding="utf-8") if syms.exists() else ""
    have = set(re.findall(r"^0x([0-9A-Fa-f]+)", existing, re.M))
    have = {int(h, 16) for h in have}
    have_names = set(re.findall(r"\s(\S+)\s*$", existing, re.M))
    add = [(ad, sz, nm) for ad, sz, nm in sorted(data_new)
           if ad + BASE not in have and ad not in have and nm not in have_names]
    print(f"{len(proposals)} nerve executes found, {renamed} renamed in the function map "
          f"({sum(1 for v in names.values() if v > 1)} ambiguous names skipped), "
          f"{len(add)} nerve objects/vtables added to symbols.txt")
    if a.dry_run:
        for ad, (m, d) in list(func_new.items())[:10]:
            print(f"  {ad + BASE:#x}  {d}")
        return 0
    fmap.write_text("\n".join(lines), encoding="utf-8")
    if add:
        text = existing.rstrip("\n") + "\n\n# nerves (tools/name_nerves.py)\n" + "".join(
            f"0x{ad + BASE:X} {sz} object {nm}\n" for ad, sz, nm in add)
        syms.write_text(text, encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
