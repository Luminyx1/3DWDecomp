#!/usr/bin/env python3
"""Fill in the object-file assignments the hand-made function map is missing.

The function map (a spreadsheet exported as TSV: Start Address, Folder, Object,
Not In Obj, Mangled Name, Demangled Name) marks the first function of every
folder and object.  Where the map is unfinished, an object "absorbs" every
function up to the next marker.  This tool proposes the missing object starts:

  * the linker laid out game + al objects folder by folder, alphabetically
    (case-insensitive, compared upper-cased) within a folder, so an object name
    that sorts *before* the current one means a new folder started;
  * a function whose mangled name also exists in Super Mario Odyssey tells us
    the object SMO put it in (OdysseyDecomp's data/file_list.yml) - al, sead,
    agl, eui and NintendoWare are shared libraries, so that is a strong hint;
  * otherwise, in an unfinished stretch, a new class name (that still sorts
    after the current object) starts a new `<Class>.o`.

Every marker the tool adds is recorded in an extra `Inferred` column (the
user's own markers are kept as-is), so the result can be pasted back into the
spreadsheet and reviewed.

    python tools/complete_map.py <user map.tsv> --smo <OdysseyDecomp>/data/file_list.yml \
        -o config/1.0.0/function_map.tsv
"""

from __future__ import annotations

import argparse
import collections
import re
import sys

RESET = "\0reset"     # placeholder for a folder start named in a post-pass

NON_ALPHA_PREFIXES = ("NintendoWare", "NintendoSDK", "sead", "agl", "eui", ".dynlink")

NAMESPACES = {"al", "sead", "nn", "agl", "eui", "rc", "nw", "ui2d", "atk", "font", "g3d", "gfx",
              "vfx", "detail", "lyt", "utl", "lght", "pfx", "fx", "lyr", "ptcl", "nerd", "nst",
              "erepo", "aal", "std", "Vessel", "driver", "util", "prepo", "os", "fs"}

TEMPLATE_WRAPPERS = (
    "al::FunctorV", "sead::Delegate", "sead::IDelegate", "sead::AnyDelegate", "al::createActorFunction",
    "al::createSceneFunction", "sead::StrTreeMap", "sead::TreeMap", "sead::FixedPtrArray",
    "sead::PtrArray", "sead::Buffer", "sead::ObjArray", "sead::SafeStringBase", "sead::FixedSafeString",
    "sead::WFixedSafeString", "sead::BufferedSafeString", "sead::TList", "sead::OffsetList",
    "sead::FixedObjArray", "sead::Matrix", "sead::Vector", "sead::Quat", "sead::RingBuffer",
    "al::HostStateBase", "al::StateMachine", "al::DeriveActorGroup", "sead::FixedRingBuffer",
    "sead::SingletonDisposer", "al::LayoutActorGroupTemplate", "sead::TaskBase::TaskCreatorImpl",
)


def split_qualified(dem: str) -> list[str]:
    """'a::B<x::y>::c(int)' -> ['a', 'B<x::y>', 'c']"""
    depth = 0
    s = dem
    for i, ch in enumerate(dem):
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth -= 1
        elif ch == "(" and depth == 0:
            s = dem[:i]
            break
    parts, cur, depth, i = [], "", 0, 0
    while i < len(s):
        ch = s[i]
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth -= 1
        if depth == 0 and s.startswith("::", i):
            parts.append(cur)
            cur = ""
            i += 2
            continue
        cur += ch
        i += 1
    parts.append(cur)
    return parts


WEAK: set[str] = set()      # mangled names bound STB_WEAK in the binary (inline/template code)


def row_key(row) -> str | None:
    """The class a function belongs to, or None (free / weak / anonymous)."""
    if row.notin or row.mangled in WEAK:
        return None
    mg, d = row.mangled, row.dem
    if not mg.startswith("_Z"):
        return None
    for pre in ("non-virtual thunk to ", "virtual thunk to ", "guard variable for "):
        if d.startswith(pre):
            d = d[len(pre):]
    parts = split_qualified(d)
    if " " in parts[0]:
        parts[0] = parts[0].split(" ")[-1]
    full = "::".join(parts)
    if full.startswith(TEMPLATE_WRAPPERS):
        return None
    comps = parts[:-1]
    i = 0
    while i < len(comps) and (comps[i] in NAMESPACES or comps[i].startswith("(anonymous")):
        i += 1
    if i >= len(comps):
        return None
    return re.sub(r"<.*", "", comps[i]) or None


def upper_key(name: str) -> str:
    n = name[:-2] if name.endswith(".o") else name
    return n.upper()


class Row:
    __slots__ = ("cols", "addr", "folder", "obj", "notin", "mangled", "dem",
                 "user_folder", "user_obj", "new_folder", "new_obj", "why")

    def __init__(self, cols):
        cols = cols + [""] * (6 - len(cols))
        self.cols = cols
        self.addr = cols[0]
        self.user_folder = cols[1].strip()
        self.user_obj = cols[2].strip()
        self.notin = cols[3].strip().upper() == "TRUE"
        self.mangled = cols[4].strip()
        self.dem = cols[5].strip()
        self.folder = self.obj = None
        self.new_folder = self.new_obj = ""
        self.why = ""


def load_smo_hints(path: str) -> dict[str, str]:
    cur, cnt, m = None, collections.Counter(), {}
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if line and not line.startswith(" ") and line.rstrip().endswith(":"):
                cur = line.rstrip()[:-1]
                continue
            s = line.strip()
            v = None
            if s.startswith("label:"):
                v = s[6:].strip() or None
            elif s.startswith("- ") and not s.startswith("- offset"):
                v = s[2:].strip()
            if v and cur and cur != "UNKNOWN":
                cnt[v] += 1
                m[v] = cur
    return {k: v for k, v in m.items() if cnt[k] == 1}


HINT_BLACKLIST = {"sead/gfx/seadDrawLockContext.o", "Util/SensorMsgFunction.o"}


def family(folder: str) -> str:
    if folder.startswith(("Library", "Project")):
        return "al"
    for fam in ("sead", "eui", "agl", "NintendoWare", "NintendoSDK", "erepo", ".dynlink"):
        if folder.startswith(fam):
            return fam
    return "game"


def hint_ok(fam: str, hint: str) -> bool:
    if hint in HINT_BLACKLIST or hint.endswith(".a"):
        return False
    if fam == "al":
        return hint.startswith(("Library/", "Project/"))
    if fam in ("sead", "eui"):
        return hint.startswith(fam + "/")
    return False


def top_namespace(row) -> str | None:
    d = row.dem
    for pre in ("non-virtual thunk to ", "virtual thunk to "):
        if d.startswith(pre):
            d = d[len(pre):]
    parts = split_qualified(d)
    if " " in parts[0]:
        parts[0] = parts[0].split(" ")[-1]
    return parts[0] if len(parts) > 1 else None


def related(a: str, b: str) -> bool:
    a, b = a.upper(), b.upper()
    return a.startswith(b) or b.startswith(a)


def complete(rows: list[Row], hints: dict[str, str]) -> None:
    # folder segments from the user's folder markers
    segs, cur = [], None
    for i, r in enumerate(rows):
        if r.user_folder:
            cur = [r.user_folder, i, i]
            segs.append(cur)
        if cur:
            cur[2] = i + 1
    for folder, lo, hi in segs:
        alpha = not folder.startswith(NON_ALPHA_PREFIXES)
        if alpha and family(folder) in ("al", "game"):
            prev = None
            for i in range(lo, hi):
                o = rows[i].user_obj
                if not o:
                    continue
                if prev and upper_key(o) < upper_key(prev) and not o.startswith("_") \
                        and not prev.startswith("_"):
                    # the user's own names are often a secondary class or a typo,
                    # so only flag this for review
                    rows[i].why = "note: sorts before the previous object (folder boundary or misnamed object?)"
                prev = o
        complete_folder(rows, lo, hi, folder, alpha, hints)
    name_resets(rows)
    # propagate
    folder = obj = None
    for r in rows:
        if r.user_folder:
            folder, obj = r.user_folder, None
        if r.new_folder:
            folder, obj = r.new_folder, None
        if r.user_obj:
            obj = r.user_obj
        if r.new_obj:
            obj = r.new_obj
        r.folder, r.obj = folder, obj


def name_resets(rows):
    """Give every inferred folder start a name: the lead word most of its
    objects share (e.g. 'Player'), else '<parent>_<n>'."""
    parent = None
    taken = collections.Counter()
    i = 0
    n = len(rows)
    while i < n:
        r = rows[i]
        if r.user_folder:
            parent = r.user_folder
            taken[parent] += 1
        if r.new_folder == RESET:
            objs = []
            j = i
            while j < n and (j == i or (not rows[j].user_folder and rows[j].new_folder != RESET)):
                o = rows[j].new_obj or rows[j].user_obj
                if o:
                    objs.append(o[:-2] if o.endswith(".o") else o)
                j += 1
            words = collections.Counter(re.match(r"[A-Z]?[a-z0-9]*", o).group(0) or o for o in objs)
            name = None
            if objs:
                w, c = words.most_common(1)[0]
                if c * 10 >= len(objs) * 6 and len(w) >= 3:
                    name = (parent.split("/")[0] + "/" + w) if family(parent) == "al" else w
            if not name:
                name = parent
            base = name
            k = 2
            while taken[name]:
                name = f"{base}_{k}"
                k += 1
            taken[name] += 1
            r.new_folder = name
            if not r.why.startswith("folder"):
                r.why = "folder (alphabetical reset); " + r.why
            i = j
            continue
        i += 1


def complete_folder(rows, lo, hi, folder, alpha, hints):
    fam = family(folder)
    user_idx = [i for i in range(lo, hi) if rows[i].user_obj]
    used = {rows[i].user_obj.upper() for i in user_idx}
    next_user = {}
    nxt = None
    for i in range(hi - 1, lo - 1, -1):
        next_user[i] = nxt
        if rows[i].user_obj:
            nxt = rows[i].user_obj

    cur_obj = None          # current object name
    cur_src = None          # 'user' | 'hint' | 'key'
    cur_keys: set[str] = set()
    absorbing = False       # past the end of the current user object's own classes
    cur_folder = folder
    after_last_user = False

    def hinted(i):
        h = hints.get(rows[i].mangled)
        if h and hint_ok(fam, h):
            return h
        return None

    def hbase(h):
        return h.rsplit("/", 1)[-1] if h else None

    def confirm_hint(i, h):
        seen = agree = 0
        for j in range(i, min(i + 40, hi)):
            if rows[j].user_obj and j != i:
                break
            hj = hbase(hinted(j))
            if hj:
                seen += 1
                agree += hj == h
                if seen >= 4:
                    break
        return agree * 2 >= seen and agree >= 1

    def order_ok(name, i):
        if not alpha:
            return True
        if cur_obj and upper_key(name) <= upper_key(cur_obj):
            return False
        return True

    def stable_reset(i, k):
        """A new folder must look like one: the run of k is not tiny and the
        next distinct classes keep increasing from k."""
        n_k = 0
        keys = [k]
        for j in range(i, min(i + 400, hi)):
            if rows[j].user_obj:
                break
            kj = row_key(rows[j])
            if kj == k:
                n_k += 1
            elif kj and kj not in keys:
                keys.append(kj)
                if len(keys) >= 4:
                    break
        if n_k < 4:
            return False
        ups = [upper_key(x) for x in keys]
        return all(a < b for a, b in zip(ups, ups[1:]))

    def lead_word(k):
        m = re.match(r"[A-Z][a-z0-9]+|[A-Z]+(?![a-z])|[a-z]+", k)
        return m.group(0) if m else k

    def new_folder_name(i, k):
        # SMO hint folder for the next few rows, else the class name
        for j in range(i, min(i + 30, hi)):
            h = hinted(j)
            if h and "/" in h:
                return (folder.split("/")[0] if fam == "al" else "") + "/" + h.rsplit("/", 2)[-2] \
                    if fam == "al" else h.rsplit("/", 1)[0]
        if fam == "al":
            return f"{folder.split('/')[0]}/{lead_word(k)}"
        return lead_word(k)

    for i in range(lo, hi):
        r = rows[i]
        if r.user_obj:
            cur_obj, cur_src = r.user_obj, "user"
            k = row_key(r)
            cur_keys = {k} if k else set()
            absorbing = False
            after_last_user = next_user.get(i) is None
            continue
        k = row_key(r)
        h = hbase(hinted(i))
        start = None
        # a different library namespace at the end of al (erepo, aal, ...)
        ns = top_namespace(r)
        if fam == "al" and ns in ("erepo", "aal") and cur_folder != ns:
            cur_folder = ns
            r.new_folder, r.why = ns, f"folder ({ns} namespace)"
            used = set()
            cur_obj = None
        cand = src = None
        if h and cur_obj != h and h.upper() not in used and confirm_hint(i, h):
            cand, src = h, "smo"
        elif k and alpha and cur_obj is not None and not h:
            if cur_src == "user" and not absorbing:
                stem = cur_obj[:-2] if cur_obj.endswith(".o") else cur_obj
                if k not in cur_keys and not related(k, stem) and upper_key(k) > upper_key(stem):
                    absorbing = True
            if (absorbing or cur_src in ("key", "hint")) and k not in cur_keys \
                    and (k + ".o").upper() not in used:
                cand, src = k + ".o", "class"
        elif k and not alpha and fam == "agl" and k not in cur_keys and (k + ".o").upper() not in used:
            cand, src = k + ".o", "class"
        elif k and cur_obj is None and (k + ".o").upper() not in used:
            cand, src = k + ".o", "class"
        if cand:
            if order_ok(cand, i):
                start = (cand, src)
            elif (fam in ("al", "game") and cur_obj and not related(cand[:-2], cur_obj[:-2])
                  and (src == "class" or after_last_user) and stable_reset(i, k or cand[:-2])):
                # sorts before the current object: the linker moved on to a new folder
                r.new_folder = RESET
                start = (cand, src + ", new folder (alphabetical reset)")
        if start:
            r.new_obj = start[0]
            r.why = (r.why + "; " if r.why else "") + f"object ({start[1]})"
            used.add(start[0].upper())
            cur_obj, cur_src = start[0], ("hint" if start[1] == "smo" else "key")
            cur_keys = set()
        if k:
            cur_keys.add(k)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("tsv")
    ap.add_argument("--smo", required=True, help="OdysseyDecomp data/file_list.yml")
    ap.add_argument("--elf", default="build/fury.elf", help="for symbol bindings (weak = header code)")
    ap.add_argument("-o", "--out", required=True)
    a = ap.parse_args(argv)
    import os
    if os.path.isfile(a.elf):
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        from furyimg import FuryImage
        img = FuryImage(a.elf)
        WEAK.update(s.name for s in img.symbols if s.bind == "STB_WEAK")

    with open(a.tsv, encoding="utf-8", errors="replace") as fh:
        header = fh.readline().rstrip("\n").split("\t")
        rows = [Row(line.rstrip("\n").split("\t")) for line in fh if line.strip()]
    hints = load_smo_hints(a.smo)
    complete(rows, hints)

    added = 0
    with open(a.out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\t".join(header[:6] + ["Inferred"]) + "\n")
        for r in rows:
            c = list(r.cols[:6])
            if r.new_folder:
                c[1] = r.new_folder
            if r.new_obj:
                c[2] = r.new_obj
                added += 1
            fh.write("\t".join(c + [r.why]) + "\n")
    print(f"{len(rows)} functions, {added} object starts inferred -> {a.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
