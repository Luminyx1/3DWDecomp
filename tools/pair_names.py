#!/usr/bin/env python3
"""Give the game's unnamed symbols the names our source uses.

The binary only exports global symbols.  Local functions (nerve `execute`s,
anonymous-namespace helpers, statics) show up in the function map as
`sub_XXXXXXXXXX` / `nullsub_N`, and unnamed data the splitter references as
`lbl_XXXXXXXXXX`.  Once a unit's source exists, this tool pairs them with the
names in our compiled object and writes the names back:

  * functions -> the Mangled/Demangled Name columns of config/<ver>/function_map.tsv
  * data      -> config/<ver>/symbols.txt

Pairing is done by walking the relocations of functions that already match
by name (target `bl sub_7100...` vs ours `bl _ZNK12_GLOBAL__N_1...`), then by
order + size for leftover local functions (e.g. nerve executes, which are only
referenced from vtables).  Review the printed list; `--dry-run` writes nothing.

    python tools/pair_names.py src/Game/MapObj/BgmStopObj [--dry-run]

Rebuild afterwards (`ninja`): the target objects are re-carved with the names.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

ROOT = Path(__file__).resolve().parent.parent
BASE = 0x7100000000
UNNAMED = re.compile(r"^(sub_|nullsub_|lbl_|loc_|off_|unk_|byte_|word_|dword_|qword_)")


class Obj:
    def __init__(self, path: Path):
        self.funcs: dict[str, dict] = {}     # name -> {sec, size, relocs[(off,type,sym,addend)]}
        self.order: list[str] = []
        self.objects: dict[str, int] = {}    # data symbol -> size
        self.local: set[str] = set()
        with path.open("rb") as fh:
            elf = ELFFile(fh)
            symtab = elf.get_section_by_name(".symtab")
            syms = list(symtab.iter_symbols())
            by_sec: dict[int, list] = {}
            for s in syms:
                if s.name and isinstance(s["st_shndx"], int):
                    by_sec.setdefault(s["st_shndx"], []).append(s)
                    t = s["st_info"]["type"]
                    if t == "STT_FUNC":
                        self.funcs.setdefault(s.name, {"sec": s["st_shndx"], "size": s["st_size"], "relocs": []})
                        self.order.append(s.name)
                    elif t == "STT_OBJECT":
                        self.objects[s.name] = s["st_size"]
                    if s["st_info"]["bind"] == "STB_LOCAL":
                        self.local.add(s.name)
            secnames = {i: sec.name for i, sec in enumerate(elf.iter_sections())}

            def resolve(sym, addend):
                if sym["st_info"]["type"] == "STT_SECTION":
                    cands = [c for c in by_sec.get(sym["st_shndx"], [])
                             if c["st_info"]["type"] in ("STT_OBJECT", "STT_FUNC")
                             and c["st_value"] <= addend < c["st_value"] + max(c["st_size"], 1)]
                    if cands:
                        c = cands[0]
                        return c.name, addend - c["st_value"]
                    return "[" + secnames.get(sym["st_shndx"], "?") + "]", addend
                return sym.name, addend

            text_of = {f["sec"]: n for n, f in self.funcs.items()}
            for sec in elf.iter_sections():
                if not isinstance(sec, RelocationSection):
                    continue
                owner = text_of.get(sec["sh_info"])
                if owner is None:
                    continue
                for r in sec.iter_relocations():
                    sym = syms[r["r_info_sym"]]
                    name, add = resolve(sym, r["r_addend"])
                    self.funcs[owner]["relocs"].append((r["r_offset"], r["r_info_type"], name, add))
            for f in self.funcs.values():
                f["relocs"].sort()


def demangle(names: list[str]) -> dict[str, str]:
    for tool in ("llvm-cxxfilt", "c++filt"):
        try:
            out = subprocess.run([tool], input="\n".join(names), capture_output=True, text=True)
            if out.returncode == 0:
                return dict(zip(names, out.stdout.splitlines()))
        except FileNotFoundError:
            continue
    return {n: n for n in names}


def unit_objects(q: str) -> tuple[Path, Path]:
    units = json.loads((ROOT / "objdiff.json").read_text())["units"]
    hits = [u for u in units if u["name"] == q or q in u["name"]]
    if len(hits) != 1:
        raise SystemExit(f"unit {q!r}: {len(hits)} matches")
    u = hits[0]
    if "base_path" not in u:
        raise SystemExit("unit has no source yet")
    return ROOT / u["target_path"], ROOT / u["base_path"]


def pair(tgt: Obj, base: Obj) -> dict[str, str]:
    renames: dict[str, str] = {}
    changed = True
    while changed:
        changed = False
        for name, tf in tgt.funcs.items():
            mapped = renames.get(name, name)
            bf = base.funcs.get(mapped)
            if bf is None:
                continue
            tr, br = tf["relocs"], bf["relocs"]
            if tf["size"] == bf["size"]:
                bmap = {r[0]: r for r in br}
                pairs = [(a, bmap[a[0]]) for a in tr if a[0] in bmap]
            else:
                pairs = list(zip(tr, br))
            for a, b in pairs:
                if a[1] != b[1] or a[3] != b[3] and not UNNAMED.match(a[2]):
                    continue
                tn, bn = a[2], b[2]
                if UNNAMED.match(tn) and tn not in renames and not bn.startswith("[") \
                        and not UNNAMED.match(bn) and bn not in renames.values():
                    renames[tn] = bn
                    changed = True
    # leftover local functions: pair by order, requiring equal size
    t_left = [n for n in tgt.order if UNNAMED.match(n) and n not in renames]
    b_left = [n for n in base.order if n in base.local and n not in renames.values()
              and n not in tgt.funcs and not n.startswith("$")]
    for tn in t_left:
        size = tgt.funcs[tn]["size"]
        for bn in b_left:
            if base.funcs[bn]["size"] == size:
                renames[tn] = bn
                b_left.remove(bn)
                break
    return renames


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("unit")
    ap.add_argument("--version", default="1.0.0")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args(argv)

    tpath, bpath = unit_objects(a.unit)
    tgt, base = Obj(tpath), Obj(bpath)
    renames = pair(tgt, base)
    if not renames:
        print("nothing to rename")
        return 0

    cfg = ROOT / "config" / a.version
    fmap = cfg / "function_map.tsv"
    lines = fmap.read_text(encoding="utf-8").split("\n")
    by_name = {}
    for i, line in enumerate(lines[1:], 1):
        c = line.split("\t")
        if len(c) > 4:
            by_name.setdefault(c[4], i)
    dem = demangle(list(renames.values()))
    func_renames, data_adds = [], []
    for old, new in renames.items():
        if new in base.funcs:
            idx = by_name.get(old)
            if idx is None:
                m = re.match(r"^(?:sub_|nullsub_)([0-9A-Fa-f]{6,})$", old)
                print(f"  ! {old}: not in the function map, skipped")
                continue
            func_renames.append((idx, old, new))
        else:
            m = re.match(r"^lbl_([0-9A-Fa-f]+)$", old)
            if not m:
                print(f"  ! {old} -> {new}: can't tell its address, skipped")
                continue
            data_adds.append((int(m.group(1), 16), base.objects.get(new, 0), new))

    for idx, old, new in func_renames:
        print(f"  func  {old:24s} -> {new}")
    for addr, size, new in data_adds:
        print(f"  data  lbl_{addr:X} -> {new} ({size} bytes)")
    if a.dry_run:
        return 0
    for idx, old, new in func_renames:
        c = lines[idx].split("\t")
        c[4] = new
        c[5] = dem.get(new, new)
        lines[idx] = "\t".join(c)
    fmap.write_text("\n".join(lines), encoding="utf-8")
    if data_adds:
        syms = cfg / "symbols.txt"
        text = syms.read_text(encoding="utf-8") if syms.exists() else ""
        unit = a.unit if "/" in a.unit else a.unit
        text = text.rstrip("\n") + f"\n\n# {unit}\n" + "".join(
            f"0x{addr:X} {size} object {new}\n" for addr, size, new in data_adds)
        syms.write_text(text, encoding="utf-8")
    print(f"renamed {len(func_renames)} function(s), added {len(data_adds)} data symbol(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
