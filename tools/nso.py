#!/usr/bin/env python3
"""NSO0 parser / unpacker for the 3DWDecomp project.

The Switch loads code as NSO ("Nintendo Shared Object") images.  An NSO0 file is
a small header followed by three LZ4-block-compressed segments (.text, .rodata,
.data).  BSS is not stored; only its size is recorded.  Each decompressed
segment has a SHA-256 stored in the header so we can prove a rebuild matches.

This module can:
  * parse the header
  * decompress every segment and verify its hash
  * dump the flat segments to disk
  * locate the MOD0 / dynamic metadata that follows .text

Usage:
    python tools/nso.py info   fury.nso
    python tools/nso.py unpack fury.nso -o build/fury
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from dataclasses import dataclass, asdict
from pathlib import Path

import lz4.block


@dataclass
class Segment:
    name: str
    file_offset: int
    memory_offset: int
    decompressed_size: int
    compressed_size: int
    compressed: bool
    check_hash: bool
    sha256: str  # expected, from header

    data: bytes = b""          # filled in by unpack()
    actual_sha256: str = ""    # filled in by unpack()

    @property
    def hash_ok(self) -> bool:
        return (not self.check_hash) or self.actual_sha256 == self.sha256


@dataclass
class Nso:
    path: str
    version: int
    flags: int
    module_id: str          # build id, hex
    bss_size: int
    segments: list[Segment]
    module_name: str
    api_info: tuple[int, int]
    dynstr: tuple[int, int]
    dynsym: tuple[int, int]

    def seg(self, name: str) -> Segment:
        for s in self.segments:
            if s.name == name:
                return s
        raise KeyError(name)


_SEG_NAMES = (".text", ".rodata", ".data")


def parse(path: str | Path) -> Nso:
    raw = Path(path).read_bytes()
    if raw[:4] != b"NSO0":
        raise ValueError("not an NSO0 file (bad magic)")

    version, _reserved, flags = struct.unpack_from("<III", raw, 0x4)

    # three SegmentHeaders, interleaved with a few standalone u32s
    seg_hdr_offs = (0x10, 0x20, 0x30)
    module_name_offset, module_name_size = struct.unpack_from("<I", raw, 0x1C)[0], struct.unpack_from("<I", raw, 0x2C)[0]
    bss_size = struct.unpack_from("<I", raw, 0x3C)[0]
    module_id = raw[0x40:0x60].hex()
    comp_sizes = struct.unpack_from("<III", raw, 0x60)

    api_info = struct.unpack_from("<II", raw, 0x88)
    dynstr = struct.unpack_from("<II", raw, 0x90)
    dynsym = struct.unpack_from("<II", raw, 0x98)
    hashes = [raw[0xA0 + i * 0x20: 0xA0 + (i + 1) * 0x20].hex() for i in range(3)]

    segments: list[Segment] = []
    for i, off in enumerate(seg_hdr_offs):
        f_off, m_off, dsize = struct.unpack_from("<III", raw, off)
        segments.append(Segment(
            name=_SEG_NAMES[i],
            file_offset=f_off,
            memory_offset=m_off,
            decompressed_size=dsize,
            compressed_size=comp_sizes[i],
            compressed=bool(flags & (1 << i)),
            check_hash=bool(flags & (1 << (i + 3))),
            sha256=hashes[i],
        ))

    module_name = ""
    if module_name_size and module_name_offset:
        module_name = raw[module_name_offset:module_name_offset + module_name_size].split(b"\0")[0].decode("latin1")

    nso = Nso(
        path=str(path),
        version=version,
        flags=flags,
        module_id=module_id,
        bss_size=bss_size,
        segments=segments,
        module_name=module_name,
        api_info=api_info,
        dynstr=dynstr,
        dynsym=dynsym,
    )
    nso._raw = raw  # type: ignore[attr-defined]
    return nso


def unpack(nso: Nso) -> None:
    """Decompress every segment in-place and compute hashes."""
    raw: bytes = nso._raw  # type: ignore[attr-defined]
    for s in nso.segments:
        blob = raw[s.file_offset:s.file_offset + s.compressed_size]
        if s.compressed:
            data = lz4.block.decompress(blob, uncompressed_size=s.decompressed_size)
        else:
            data = blob[:s.decompressed_size]
        if len(data) != s.decompressed_size:
            raise ValueError(f"{s.name}: decompressed {len(data)} != {s.decompressed_size}")
        s.data = data
        s.actual_sha256 = hashlib.sha256(data).hexdigest()


def _fmt(n: int) -> str:
    return f"0x{n:X} ({n:,})"


def cmd_info(args) -> int:
    nso = parse(args.file)
    unpack(nso)
    print(f"NSO      : {nso.path}")
    print(f"version  : {nso.version}")
    print(f"flags    : 0x{nso.flags:02X}")
    print(f"build id : {nso.module_id}")
    print(f"module   : {nso.module_name or '(none)'}")
    print(f"bss size : {_fmt(nso.bss_size)}")
    print()
    hdr = f"{'seg':8} {'file_off':>12} {'mem_off':>12} {'size':>14} {'comp':>12}  hash"
    print(hdr)
    print("-" * len(hdr))
    for s in nso.segments:
        status = "n/a" if not s.check_hash else ("OK" if s.hash_ok else "MISMATCH")
        print(f"{s.name:8} {s.file_offset:>12} {s.memory_offset:>12} "
              f"{s.decompressed_size:>14} {s.compressed_size:>12}  {status}")
    end = nso.seg(".data").memory_offset + nso.seg(".data").decompressed_size
    print()
    print(f".bss     : {_fmt(end)} .. {_fmt(end + nso.bss_size)}")
    print(f"image end : {_fmt(end + nso.bss_size)}")

    # MOD0 header: first 4 bytes of .text are a branch, next u32 points to Mod0
    text = nso.seg(".text").data
    mod_off = struct.unpack_from("<I", text, 4)[0]
    if text[mod_off:mod_off + 4] == b"MOD0":
        dyn_off, bss_start, bss_end, eh_start, eh_end, mod_obj = struct.unpack_from("<6i", text, mod_off + 4)
        print()
        print(f"MOD0 @ .text+0x{mod_off:X}")
        print(f"  .dynamic   : text+0x{mod_off + dyn_off:X}")
        print(f"  .bss       : 0x{mod_off + bss_start:X} .. 0x{mod_off + bss_end:X}")
        print(f"  .eh_frame  : 0x{mod_off + eh_start:X} .. 0x{mod_off + eh_end:X}")
    return 0 if all(s.hash_ok for s in nso.segments) else 1


def cmd_unpack(args) -> int:
    nso = parse(args.file)
    unpack(nso)
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)

    manifest = {
        "source": nso.path,
        "build_id": nso.module_id,
        "module_name": nso.module_name,
        "bss_size": nso.bss_size,
        "segments": [],
    }
    for s in nso.segments:
        fn = out.with_suffix("").name + s.name.replace(".", "_") + ".bin"
        (out.parent / fn).write_bytes(s.data)
        d = asdict(s)
        d.pop("data")
        d["file"] = fn
        d["hash_ok"] = s.hash_ok
        manifest["segments"].append(d)
        print(f"wrote {fn:24} {len(s.data):>10} bytes  hash {'OK' if s.hash_ok else 'MISMATCH'}")

    # flat image (text .. data, no bss) at its virtual layout
    lo = nso.segments[0].memory_offset
    hi = nso.segments[-1].memory_offset + nso.segments[-1].decompressed_size
    flat = bytearray(hi - lo)
    for s in nso.segments:
        flat[s.memory_offset - lo:s.memory_offset - lo + len(s.data)] = s.data
    flat_fn = out.with_suffix("").name + "_flat.bin"
    (out.parent / flat_fn).write_bytes(flat)
    manifest["flat"] = {"file": flat_fn, "base": lo, "size": len(flat)}
    print(f"wrote {flat_fn:24} {len(flat):>10} bytes  (virtual base 0x{lo:X})")

    man_fn = out.parent / (out.with_suffix("").name + "_manifest.json")
    man_fn.write_text(json.dumps(manifest, indent=2))
    print(f"wrote {man_fn.name}")
    return 0 if all(s.hash_ok for s in nso.segments) else 1


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("info", help="parse header, verify segment hashes")
    p.add_argument("file")
    p.set_defaults(func=cmd_info)

    p = sub.add_parser("unpack", help="decompress segments to disk")
    p.add_argument("file")
    p.add_argument("-o", "--out", default="build/fury", help="output path prefix")
    p.set_defaults(func=cmd_unpack)

    args = ap.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
