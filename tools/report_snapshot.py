#!/usr/bin/env python3
"""Record or verify which source inputs produced the public progress snapshot.

tools/progress.py records this metadata after measuring progress. CI checks it
without the game binary or compiler, and refuses to publish stale reports.
"""

from __future__ import annotations

import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INPUTS = ["src", "include", "config", "configure.py", "tools/progress.py",
          "tools/elf_split.py", "tools/furyimg.py", "tools/report_snapshot.py"]


def input_digest(root: Path) -> str:
    paths = subprocess.check_output(
        ["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard", "--", *INPUTS],
        cwd=root,
    ).decode("utf-8").split("\0")
    digest = hashlib.sha256()
    for name in sorted(set(paths) - {""}):
        path = root / name
        if not path.is_file():
            continue  # A locally deleted file is no longer a build input.
        # Git checkouts may use CRLF on Windows and LF on Linux.
        content = path.read_bytes().replace(b"\r\n", b"\n")
        digest.update(name.encode("utf-8") + b"\0")
        digest.update(hashlib.sha256(content).digest())
    return digest.hexdigest()


def snapshot(root: Path) -> dict:
    return {
        "version": 1,
        "inputs_sha256": input_digest(root),
        "report_sha256": hashlib.sha256((root / "report.json").read_bytes()).hexdigest(),
    }


def write_snapshot(root: Path = ROOT) -> None:
    (root / "report-metadata.json").write_text(
        json.dumps(snapshot(root), indent=2) + "\n", encoding="utf-8", newline="\n")


def check_snapshot(root: Path = ROOT) -> None:
    expected = json.loads((root / "report-metadata.json").read_text(encoding="utf-8"))
    if expected != snapshot(root):
        raise SystemExit(
            "Stale progress snapshot: run python tools/progress.py and commit "
            "README.md, report.json, and report-metadata.json with the source changes.")


if __name__ == "__main__":
    check_snapshot()
    print("Progress report matches the recorded source inputs and report digest.")
