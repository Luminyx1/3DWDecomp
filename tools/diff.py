#!/usr/bin/env python3
"""Text front-end for objdiff-cli: per-unit match summary and per-function
side-by-side instruction diffs, for terminals and scripted use.

    python tools/diff.py <unit>                 # every function's match %
    python tools/diff.py <unit> <symbol>        # side-by-side diff of one function
    python tools/diff.py <unit> --all           # diffs of every non-matching function
    python tools/diff.py --status               # all units that have source

<unit> is the unit name from objdiff.json (the source path without its
extension, e.g. src/nw/lms/lms_message) or just a unique part of it.
Build first (`ninja`); this reads build/target/... and build/obj/...

Uses tools/bin/objdiff-cli(.exe), or `objdiff-cli` on PATH.
"""

from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def objdiff_cli() -> str:
    for cand in (ROOT / "tools/bin/objdiff-cli.exe", ROOT / "tools/bin/objdiff-cli"):
        if cand.is_file():
            return str(cand)
    exe = shutil.which("objdiff-cli")
    if exe:
        return exe
    raise SystemExit("objdiff-cli not found (put it in tools/bin/ or on PATH)")


def load_units() -> list[dict]:
    return json.loads((ROOT / "objdiff.json").read_text())["units"]


def find_unit(q: str) -> dict:
    units = load_units()
    q = q.replace("\\", "/")
    for u in units:
        if u["name"] == q or u["metadata"].get("source_path") == q:
            return u
    hits = [u for u in units if q in u["name"]]
    if len(hits) == 1:
        return hits[0]
    if not hits:
        raise SystemExit(f"no unit matches {q!r}")
    raise SystemExit(f"{q!r} is ambiguous: " + ", ".join(u["name"] for u in hits[:10]))


def run_diff(unit: dict, symbol: str | None = None) -> dict:
    tgt = ROOT / unit["target_path"]
    base = ROOT / unit["base_path"] if "base_path" in unit else None
    if not tgt.is_file():
        raise SystemExit(f"missing {tgt} (run ninja)")
    if base is None or not base.is_file():
        raise SystemExit(f"missing compiled object for {unit['name']} (no source yet, or run ninja)")
    cmd = [objdiff_cli(), "diff", "-1", str(tgt), "-2", str(base), "-o", "-", "--format", "json"]
    if symbol:
        cmd.append(symbol)
    else:
        cmd.append("__none__")
    out = subprocess.run(cmd, capture_output=True, text=True)
    if out.returncode != 0 and not out.stdout:
        raise SystemExit(out.stderr.strip())
    return json.loads(out.stdout)


def functions(side: dict) -> list[dict]:
    return [s for s in side.get("symbols", []) if s.get("kind") == "SYMBOL_FUNCTION"]


def summary(unit: dict) -> tuple[int, int]:
    d = run_diff(unit)
    left = functions(d["left"])
    right = {s["name"]: s for s in functions(d.get("right", {}))}
    total = matched = 0
    rows = []
    for s in left:
        size = int(s.get("size", 0))
        pct = s.get("match_percent")
        total += size
        if pct is not None and pct >= 100.0:
            matched += size
        mark = "  " if s["name"] in right else "? "
        rows.append((pct, size, s["name"], mark))
    print(f"{unit['name']}:")
    for pct, size, name, mark in rows:
        p = "  --  " if pct is None else f"{pct:6.2f}"
        print(f"  {p}%  {size:6d}  {mark}{name}")
    missing = [n for n in right if n not in {r[2] for r in rows}]
    for n in missing:
        print(f"    (extra in source) {n}")
    print(f"  => {matched}/{total} bytes fully matching")
    return matched, total


def render(unit: dict, symbol: str) -> None:
    d = run_diff(unit, symbol)
    lsym = next((s for s in d["left"].get("symbols", []) if s.get("name") == symbol), None)
    if lsym is None:
        raise SystemExit(f"{symbol} not found in target {unit['target_path']}")
    rsym = None
    if "target_symbol" in lsym:
        rsyms = d.get("right", {}).get("symbols", [])
        idx = lsym["target_symbol"]
        rsym = rsyms[idx] if idx < len(rsyms) else None
    lins = lsym.get("instructions", [])
    rins = rsym.get("instructions", []) if rsym else []
    pct = lsym.get("match_percent")
    print(f"{symbol}  ({'no match' if pct is None else f'{pct:.2f}%'})")
    width = 52
    marks = {"DIFF_NONE": " ", "DIFF_REPLACE": "|", "DIFF_DELETE": "<", "DIFF_INSERT": ">",
             "DIFF_OP_MISMATCH": "|", "DIFF_ARG_MISMATCH": "~"}
    for i in range(max(len(lins), len(rins))):
        l = lins[i] if i < len(lins) else {}
        r = rins[i] if i < len(rins) else {}
        lt = l.get("instruction", {}).get("formatted", "") if l else ""
        rt = r.get("instruction", {}).get("formatted", "") if r else ""
        kind = l.get("diff_kind") or r.get("diff_kind") or "DIFF_NONE"
        m = marks.get(kind, "?")
        print(f"{m} {lt[:width]:<{width}}  {rt[:width]}")


def main(argv: list[str]) -> int:
    if not argv or argv[0] in ("-h", "--help"):
        print(__doc__)
        return 0
    if argv[0] == "--status":
        tot_m = tot = 0
        for u in load_units():
            if "base_path" not in u or not (ROOT / u["base_path"]).is_file():
                continue
            m, t = summary(u)
            tot_m += m
            tot += t
        print(f"\nall units with source: {tot_m}/{tot} bytes fully matching")
        return 0
    unit = find_unit(argv[0])
    if len(argv) == 1:
        summary(unit)
    elif argv[1] == "--all":
        d = run_diff(unit)
        for s in functions(d["left"]):
            if (s.get("match_percent") or 0) < 100.0:
                render(unit, s["name"])
                print()
    else:
        render(unit, argv[1])
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv[1:]))
    except BrokenPipeError:
        sys.exit(0)
