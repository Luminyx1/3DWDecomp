#!/usr/bin/env python3
"""Compiler wrapper used by build.ninja:

    python tools/nxcc.py <path/to/clang.exe> <clang args...>

* On Windows the SDK's Clang for NX runs directly; anywhere else it runs under
  wine ($WINE, default `wine`; keep `wineserver -p` running to avoid its ~1s
  startup per compile).  Under wine the -MF depfile comes out with Windows
  paths (`Z:\\home\\...`, `include\\foo.h`); they are rewritten to POSIX so
  ninja can track header dependencies.
* After a successful compile, AArch64 mapping symbols ($x/$d) are stripped
  from the -o object (see tools/strip_mapsyms.py for why).
"""

from __future__ import annotations

import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import strip_mapsyms  # noqa: E402

IS_WINDOWS = os.name == "nt"


def fix_depfile(path: str) -> None:
    try:
        with open(path, "r", encoding="utf-8", errors="surrogateescape") as fh:
            text = fh.read()
    except FileNotFoundError:
        return
    out_lines = []
    for line in text.splitlines():
        cont = line.endswith(" \\")
        body = line[:-2] if cont else line
        body = re.sub(r"(?i)\bZ:\\", "/", body)      # wine maps Z: to /
        body = re.sub(r"\\(?! )", "/", body)          # keep '\ ' escapes
        out_lines.append(body + (" \\" if cont else ""))
    with open(path, "w", encoding="utf-8", errors="surrogateescape") as fh:
        fh.write("\n".join(out_lines) + "\n")


def arg_after(argv: list[str], flag: str) -> str | None:
    if flag in argv:
        i = argv.index(flag)
        if i + 1 < len(argv):
            return argv[i + 1]
    return None


def main(argv: list[str]) -> int:
    if not argv:
        print(__doc__, file=sys.stderr)
        return 2
    if IS_WINDOWS:
        rc = subprocess.call(argv)
    else:
        env = dict(os.environ)
        env.setdefault("WINEDEBUG", "-all")
        rc = subprocess.call([os.environ.get("WINE", "wine")] + argv, env=env)
    if rc != 0:
        return rc
    dep = arg_after(argv, "-MF")
    if dep and not IS_WINDOWS:
        fix_depfile(dep)
    out = arg_after(argv, "-o")
    if out and out.endswith(".o") and os.path.isfile(out):
        strip_mapsyms.strip(out)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
