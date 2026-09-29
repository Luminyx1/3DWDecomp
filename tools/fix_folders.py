#!/usr/bin/env python3
"""Resolve the inferred folder resets in config/<ver>/function_map.tsv.

tools/complete_map.py starts a new folder wherever an inferred object sorts
before the previous one.  Many of those "resets" are really a secondary class
that lives in a neighbouring file.  For every inferred reset at object K
(previous object X, the one before that W, next Y) this tries, in order:

  a) K is a secondary class of X's file  - if W < X < Y still sorts, drop K;
  b) X was a secondary class of K's file - if W < K < Y, rename X to K, drop K;
  c) otherwise it is a real folder boundary.  Folders form a tree
     (Folder1, Folder1/Folder2, Folder3), so it becomes a subfolder of the
     folder it interrupts, named after what its objects have in common.

Only markers the tools added (Inferred column) are touched; the hand-made
ones are kept.  Idempotent.

    python tools/fix_folders.py [--dry-run]
"""

from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

RESET = "alphabetical reset"


def up(name: str) -> str:
    return (name[:-2] if name.endswith(".o") else name).upper()


def words(name: str) -> list[str]:
    n = name[:-2] if name.endswith(".o") else name
    return re.findall(r"[A-Z]+(?![a-z])|[A-Z][a-z0-9]*|[a-z0-9]+", n)


class Map:
    def __init__(self, path: Path):
        self.path = path
        lines = path.read_text(encoding="utf-8").split("\n")
        self.header = lines[0]
        self.rows = []
        for ln in lines[1:]:
            if ln == "":
                continue
            c = ln.split("\t")
            c += [""] * (7 - len(c))
            self.rows.append(c)

    def save(self):
        out = [self.header] + ["\t".join(c) for c in self.rows]
        self.path.write_text("\n".join(out) + "\n", encoding="utf-8")

    def objects(self):
        """[(row index, folder marker, object, inferred note)] for rows starting objects/folders"""
        return [(i, c[1], c[2], c[6]) for i, c in enumerate(self.rows) if c[1] or c[2]]

    def size_of(self, i: int) -> int:
        """number of functions in the object starting at row i"""
        n = 1
        while i + n < len(self.rows) and not (self.rows[i + n][1] or self.rows[i + n][2]):
            n += 1
        return n


def inferred_reset(note: str) -> bool:
    return RESET in note and "between mapped objects" not in note


GENERIC = {"INFO", "PARAM", "HOLDER", "KEEPER", "UTIL", "DATA", "BASE", "DIRECTOR", "CTRL",
           "FUNCTION", "LIST", "STATE", "GROUP", "TYPE", "OBJ", "PARTS", "ACTOR", "CONTROLLER"}

# names picked by hand where the objects share no telling word (keyed by first object)
OVERRIDES = {
    "GodRayDirector.o": "Library/PostProcessing",
    "FurActor.o": "Player",
    "PlayReport.o": "Raidon",
    "GigaNormalRollingParam.o": "Player/Giga",
    "GigaNormalRollingAttackParam.o": "Player/Normal",
    "SeListener.o": "Project/Se",
}


def related(a: str, b: str) -> bool:
    """same lead word (FooBar / FooBaz), ignoring generic words"""
    wa, wb = words(a), words(b)
    return bool(wa and wb and wa[0].upper() == wb[0].upper() and wa[0].upper() not in GENERIC)


def movable(o) -> bool:
    """an object the tools inferred from a class name (not hand-made, not an SMO name)"""
    return "object" in o[3] and "smo" not in o[3] and not o[1]


def resolve(m: Map, log) -> int:
    changes = 0
    while True:
        objs = [o for o in m.objects() if o[2]]
        done = True
        for k, (i, fold, obj, note) in enumerate(objs):
            if not (fold and inferred_reset(note)) or k < 1:
                continue
            kn = up(obj)
            X = objs[k - 1]
            Y = objs[k + 1] if k + 1 < len(objs) and not objs[k + 1][1] else None
            yn = up(Y[2]) if Y else None
            # (a') a short run of inferred objects before K were secondary classes of
            #      the object before them: fold them in if that restores the order
            run = []
            j = k - 1
            while j >= 0 and len(run) < 2 and movable(objs[j]) and m.size_of(objs[j][0]) <= 12:
                run.append(objs[j])
                j -= 1
            if run and j >= 0 and not objs[j + 1][1] and up(objs[j][2]) < kn:
                P = objs[j]
                for r in run:
                    rr = m.rows[r[0]]
                    rr[2] = ""
                    rr[6] = f"merged into {P[2]} (secondary class {r[2]})"
                row = m.rows[i]
                row[1] = ""
                row[6] = row[6].replace("folder (alphabetical reset); ", "").replace(
                    ", new folder (alphabetical reset)", "")
                log(f"  {', '.join(r[2] for r in run)}: secondary classes of {P[2]} - merged; {obj} stays in the folder")
            elif movable(X) and "smo" not in note and related(X[2], obj) and (yn is None or kn < yn) \
                    and (k < 2 or objs[k - 1][1] or up(objs[k - 2][2]) < kn):
                # (b) X was a secondary class of K's file
                xr = m.rows[X[0]]
                xr[6] = f"object {obj} (was {X[2]}, a secondary class)"
                xr[2] = obj
                row = m.rows[i]
                row[1] = row[2] = ""
                row[6] = f"part of {obj}"
                log(f"  {X[2]}: secondary class of {obj} - object renamed")
            elif "smo" not in note and related(X[2], obj) and (yn is None or yn > up(X[2])):
                # (a) K is a secondary class of X's file
                row = m.rows[i]
                row[1] = row[2] = ""
                row[6] = f"merged into {X[2]} (secondary class {obj})"
                log(f"  {obj}: secondary class of {X[2]} - merged")
            else:
                continue
            changes += 1
            done = False
            break
        if done:
            return changes


def name_folders(m: Map, log) -> int:
    """Name the remaining inferred resets as subfolders of the folder they interrupt."""
    parent = None
    taken = collections.Counter(c[1] for c in m.rows if c[1])
    objs = m.objects()
    renamed = 0
    for n, (i, fold, obj, note) in enumerate(objs):
        if fold and not inferred_reset(note):
            parent = fold
            continue
        if not (fold and inferred_reset(note)) or parent is None:
            continue
        seg = [obj]
        for j in range(n + 1, len(objs)):
            if objs[j][1]:
                break
            seg.append(objs[j][2])
        seg = [s for s in seg if s]
        # the word the segment's objects share most (ignoring the parent's own name)
        pw = set(w.upper() for w in re.split(r"[/_]", parent))
        cnt = collections.Counter()
        for s in seg:
            for w in dict.fromkeys(words(s)):
                if w.upper() not in pw and len(w) > 2:
                    cnt[w] += 1
        sub = cnt.most_common(1)[0][0] if cnt else "Sub"
        # a sibling if the name fits between this folder and the next hand-made one,
        # otherwise a subfolder
        nxt = next((o[1] for o in objs[n + 1:] if o[1] and not inferred_reset(o[3])), None)
        head = parent.rsplit("/", 1)[0] + "/" if "/" in parent else ""
        sib = head + sub
        cur_name = m.rows[i][1]
        if seg and seg[0] in OVERRIDES:
            name = OVERRIDES[seg[0]]
        elif up(parent) < up(sib) and (nxt is None or up(sib) < up(nxt)) and (not taken[sib] or sib == cur_name):
            name = sib
        else:
            name = f"{parent}/{sub}"
        base, k = name, 2
        while taken[name] and cur_name != name:
            name = f"{base}{k}"
            k += 1
        if not name.startswith(parent + "/"):
            parent = name
        if m.rows[i][1] != name:
            log(f"  {m.rows[i][1]:28s} -> {name}   ({', '.join(seg[:4])}{'...' if len(seg) > 4 else ''})")
            taken[m.rows[i][1]] -= 1
            taken[name] += 1
            m.rows[i][1] = name
            renamed += 1
    return renamed


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", default="1.0.0")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args(argv)
    m = Map(Path("config") / a.version / "function_map.tsv")
    merged = resolve(m, print)
    named = name_folders(m, print)
    print(f"{merged} reset(s) merged away, {named} folder(s) named")
    if not a.dry_run:
        m.save()
    return 0


if __name__ == "__main__":
    sys.exit(main())
