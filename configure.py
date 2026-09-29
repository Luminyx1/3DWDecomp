#!/usr/bin/env python3
"""Generate build.ninja and objdiff.json from config/<version>/.

  config.json        toolchain + compiler flags + include paths
  splits.txt         one block per translation unit -> the section ranges it owns
  function_map.tsv   every function in .text, with its folder/object (the map
                     splits.txt is generated from, see tools/gen_splits.py) and
                     the names we give local (sub_XXXX) functions

Pipeline per translation unit:

    fury.nso --nx2elf--> build/fury.elf --elf_split--> build/target/<u>.o   (expected)
    src/<u>.(c|cpp) --clang--> build/obj/<u>.o                              (actual)

objdiff diffs the two objects.  A unit whose source file does not exist yet is
still carved (so it counts toward progress as 0%) but has no compile step.  A
unit marked `done` in splits.txt is one whose compiled object matches.

The compiler is the SDK's Clang for NX (Windows).  On Windows it runs directly;
anywhere else it runs under wine through tools/nxcc.py (set $WINE to override
the wine binary).
"""

from __future__ import annotations

import json
import os
import shlex
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).parent
VERSION = "1.0.0"
CFG = ROOT / "config" / VERSION
PY = Path(sys.executable).as_posix()
IS_WINDOWS = os.name == "nt"


SECTIONS = (".text", ".rodata", ".data", ".bss")


@dataclass
class Unit:
    src: str
    ranges: dict[str, tuple[int, int]] = field(default_factory=dict)  # section -> (start, end)
    done: bool = False
    lang: str | None = None            # 'c' | 'c++'
    cflags: str = ""

    @property
    def name(self) -> str:
        return str(Path(self.src).with_suffix("")).replace("\\", "/")

    @property
    def is_cxx(self) -> bool:
        if self.lang:
            return self.lang == "c++"
        return self.src.endswith((".cpp", ".cc", ".cxx", ".C"))

    @property
    def has_source(self) -> bool:
        return (ROOT / self.src).is_file()

    @property
    def range_args(self) -> list[str]:
        return [f"{s}:{lo:#x}:{hi:#x}" for s, (lo, hi) in self.ranges.items()]

    @property
    def category(self) -> str:
        n = self.name
        for prefix, cat in (("src/Game/", "game"), ("src/al/", "al"), ("src/sead/", "sead"),
                            ("src/agl/", "agl"), ("src/eui/", "eui"), ("src/nw/", "nw"),
                            ("src/NintendoSDK/", "nw"), ("src/aal/", "aal"),
                            ("src/erepo/", "erepo")):
            if n.startswith(prefix):
                return cat
        return "other"


CATEGORIES = [
    ("game", "Game (RedCarpet)"),
    ("al", "ActionLibrary (al)"),
    ("sead", "sead"),
    ("agl", "agl"),
    ("eui", "eui"),
    ("nw", "NintendoWare / SDK"),
    ("aal", "aal"),
    ("erepo", "erepo"),
    ("other", "Other"),
]


def _parse_range_line(text: str, where: str) -> tuple[str, int, int]:
    # ".text   start:0x123 end:0x456"  /  ".text 0x123 0x456"  /  ".text 0x123-0x456"
    parts = text.replace("-", " ").split()
    sec = parts[0]
    nums = [p.split(":", 1)[-1] for p in parts[1:]]
    if sec not in SECTIONS or len(nums) != 2:
        raise SystemExit(f"{where}: bad range line {text!r} "
                         f"(expected '<section> start:0x.. end:0x..')")
    return sec, int(nums[0], 16), int(nums[1], 16)


def parse_splits(path: Path) -> list[Unit]:
    units: list[Unit] = []
    cur: Unit | None = None
    for lineno, raw in enumerate(path.read_text().splitlines(), 1):
        line = raw.split("#", 1)[0].rstrip()
        if not line.strip():
            continue
        where = f"{path}:{lineno}"
        is_header = ":" in line and not line.lstrip().startswith(".")
        if is_header:
            head, _, rest = line.partition(":")
            cur = Unit(src=head.strip())
            for tok in shlex.split(rest.strip()):
                if tok == "done":
                    cur.done = True
                elif tok in ("c", "c++"):
                    cur.lang = tok
                elif tok.startswith("cflags="):
                    cur.cflags = tok[len("cflags="):]
                else:
                    hint = '  (multi-word cflags must be quoted: cflags="-a -b")' if tok.startswith("-") else ""
                    raise SystemExit(f"{where}: unknown option {tok!r}{hint}")
            units.append(cur)
        else:
            if cur is None:
                raise SystemExit(f"{where}: range line before any unit header")
            sec, lo, hi = _parse_range_line(line.strip(), where)
            if hi <= lo:
                raise SystemExit(f"{where}: end 0x{hi:x} <= start 0x{lo:x}")
            cur.ranges[sec] = (lo, hi)
    seen = set()
    for u in units:
        if not u.ranges:
            raise SystemExit(f"unit {u.src} has no section ranges")
        if u.name in seen:
            raise SystemExit(f"unit {u.src} is listed twice")
        seen.add(u.name)
    return units


def ninja_escape(p: str) -> str:
    return p.replace("$", "$$").replace(":", "$:").replace(" ", "$ ")


def compiler_cmd(path: str) -> str:
    # tools/nxcc.py runs the SDK compiler (under wine when not on Windows) and
    # post-processes the object; see its docstring
    exe = (ROOT / path).resolve().as_posix()
    return f"{PY} tools/nxcc.py {ninja_escape(exe)}"


def gen_ninja(cfg: dict, units: list[Unit]) -> str:
    tc = cfg["toolchain"]
    fl = cfg["flags"]
    cc = compiler_cmd(tc["cc"])
    cxx = compiler_cmd(tc["cxx"])
    incs = " ".join(f"-I{p}" for p in cfg.get("include_dirs", ["include"]))
    sys_incs = " ".join(f"-isystem {p}" for p in cfg.get("system_include_dirs", []))
    common = f'--target={tc["target"]} {fl["common"]} {incs} {sys_incs}'.rstrip()
    fmap = f"config/{VERSION}/function_map.tsv"
    fmap_arg = f"--map {fmap}" if (ROOT / fmap).is_file() else ""
    fmap_dep = f" {fmap}" if fmap_arg else ""

    L: list[str] = []
    w = L.append
    w("# AUTO-GENERATED by configure.py - edit config/%s/ instead\n" % VERSION)
    w("ninja_required_version = 1.3")
    w("builddir = build")
    w(f"cflags = {common} {fl['c']}")
    w(f"cxxflags = {common} {fl['cxx']}")
    w("")
    w(f"rule nx2elf\n  command = {PY} tools/nx2elf.py fury.nso -o $out\n  description = nx2elf $out\n")
    w(f"rule split\n  command = {PY} tools/elf_split.py --elf $elf {fmap_arg} --out $out $ranges\n"
      f"  description = split $out\n")
    w(f"rule cc\n  command = {cc} $cflags -MD -MF $out.d -c $in -o $out\n  depfile = $out.d\n  deps = gcc\n  description = cc $in\n")
    w(f"rule cxx\n  command = {cxx} $cxxflags -MD -MF $out.d -c $in -o $out\n  depfile = $out.d\n  deps = gcc\n  description = cxx $in\n")
    w("build build/fury.elf: nx2elf | tools/nx2elf.py tools/nso.py fury.nso\n")

    targets: list[str] = []
    split_deps = f"tools/elf_split.py tools/elfobj.py tools/furyimg.py{fmap_dep}"
    syms = f"config/{VERSION}/symbols.txt"
    if (ROOT / syms).is_file():
        split_deps += f" {syms}"
    for u in units:
        tobj = f"build/target/{u.name}.o"
        ranges = " ".join("--range " + r for r in u.range_args)
        w(f"build {ninja_escape(tobj)}: split build/fury.elf | {split_deps}")
        w(f"  elf = build/fury.elf")
        w(f"  ranges = {ranges}")
        targets.append(tobj)
        if u.has_source:
            aobj = f"build/obj/{u.name}.o"
            rule = "cxx" if u.is_cxx else "cc"
            w(f"build {ninja_escape(aobj)}: {rule} {ninja_escape(u.src)} | tools/nxcc.py tools/strip_mapsyms.py")
            if u.cflags:
                key = "cxxflags" if u.is_cxx else "cflags"
                base = fl["cxx"] if u.is_cxx else fl["c"]
                w(f"  {key} = {common} {base} {u.cflags}")
            targets.append(aobj)
        w("")

    # `ninja target` carves every unit; `ninja` (default) only builds the units
    # that have source plus their targets, which is what objdiff needs day to day
    w("build target: phony " + " ".join(ninja_escape(t) for t in targets if t.startswith("build/target/")))
    w("build all: phony " + " ".join(ninja_escape(t) for t in targets))
    active = []
    for u in units:
        if u.has_source:
            active += [f"build/target/{u.name}.o", f"build/obj/{u.name}.o"]
    w("build src: phony " + " ".join(ninja_escape(t) for t in active))
    w("default src")
    return "\n".join(L) + "\n"


def gen_objdiff(units: list[Unit]) -> dict:
    out_units = []
    for u in units:
        ent = {
            "name": u.name,
            "target_path": f"build/target/{u.name}.o",
            "metadata": {"source_path": u.src, "complete": u.done,
                         "progress_categories": [u.category]},
        }
        if u.has_source:
            ent["base_path"] = f"build/obj/{u.name}.o"
        out_units.append(ent)
    return {
        "min_version": "3.0.0",
        "custom_make": "ninja",
        "build_target": True,
        "build_base": True,
        "watch_patterns": ["*.c", "*.cpp", "*.cc", "*.h", "*.hpp",
                           "tools/*.py", "config/**/*"],
        "progress_categories": [{"id": i, "name": n} for i, n in CATEGORIES],
        "units": out_units,
    }


def main():
    cfg = json.loads((CFG / "config.json").read_text())
    units = parse_splits(CFG / "splits.txt")
    (ROOT / "build.ninja").write_text(gen_ninja(cfg, units))
    (ROOT / "objdiff.json").write_text(json.dumps(gen_objdiff(units), indent=2) + "\n")
    done = sum(u.done for u in units)
    have = sum(u.has_source for u in units)
    print(f"configured {len(units)} unit(s): {have} with source, {done} done -> build.ninja, objdiff.json")


if __name__ == "__main__":
    main()
