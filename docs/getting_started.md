# Getting started

See `ABOUT.md` for the goal. This file is the working state of the toolchain.

## Layout

```
fury.nso                     the target (NSO0, not committed)
configure.py                 config/1.0.0/*  ->  build.ninja + objdiff.json
config/1.0.0/splits.txt      the translation-unit manifest (one block per TU)
config/1.0.0/config.json     toolchain + compiler flags
src/  include/               the decompilation
tools/
  nso.py                     parse / unpack / hash-verify the NSO
  nx2elf.py                  NSO  ->  static AArch64 ELF (vaddr == game address)
  elf_split.py               carve a TU's section ranges out of fury.elf into a .o
  furyimg.py  elfobj.py      support libs for elf_split
  dyninfo.py  symbols.py     analysis dumps
  bin/objdiff-cli.exe        objdiff 3.8.1 (Windows x86_64)
  nnsdk/                     Nintendo SDK (compiler + MakeNso) - not committed
build/                       generated - not committed
```

## The loop

```bash
python configure.py            # regenerate build.ninja + objdiff.json from config/1.0.0/
ninja                          # build fury.elf, carve targets, compile sources
tools/bin/objdiff-cli.exe report generate -o build/report.json -f json-pretty
```

For interactive work, point the objdiff GUI / VS Code extension at `objdiff.json`.

### Adding a translation unit

A unit is the contiguous span the linker gave one object file in each section.

1. Find its boundaries. `python tools/symbols.py fury.nso` writes
   `build/fury_symbols.csv` (addr, size, section, name for all 69k). An object
   file's functions sit in one unbroken run of `.text`; the block's `.text`
   range is `[first function's addr, last function's addr + size)`, and likewise
   for `.rodata` / `.data` / `.bss` if the TU has any.
2. Add a block to `config/1.0.0/splits.txt`:
   ```
   src/foo/bar.cpp:
       .text     start:0x00240000 end:0x00240180
       .rodata   start:0x00A50000 end:0x00A50040
   ```
   Header options after the `:` - `done` (marks complete in objdiff metadata),
   `c` / `c++` (force language), `cflags="-x -y"` (per-TU flags).
3. Create `src/foo/bar.cpp`, then run `python configure.py` followed by `ninja`.
   The splitter carves those ranges, discovers
   the symbols inside them, and reconstructs relocations.
4. Implement the source; iterate until objdiff shows 100%.
5. Add `done` to the header and rerun `python configure.py` to update objdiff metadata.

`done` currently affects only objdiff metadata. Both reference and source objects
are built regardless of this flag; the build does not yet link a rebuilt game.

Within a range each function is carved into its own `.text.<symbol>` section,
mirroring clang's `-ffunction-sections` output so objdiff lines the two objects
up section-for-section. A reference to unnamed data *outside* the unit's
declared ranges is left unrelocated (and shows in the diff) - widen the range or
add the missing section.

## How the target objects are made

The final image no longer contains the original object-file relocations.
`elf_split.py` reconstructs code relocations by disassembling each carved function
(capstone) and uses retained dynamic relocations for data pointers:

| pattern | reconstructed relocation |
|---|---|
| `bl <addr>` | `R_AARCH64_CALL26` -> symbol at addr |
| `b <addr>` (tail call, outside the function) | `R_AARCH64_JUMP26` |
| `adrp`+`add` to a direct address | `ADR_PREL_PG_HI21` + `ADD_ABS_LO12_NC` |
| `adrp`+`ldr/str` to a direct address | `ADR_PREL_PG_HI21` + `LDSTn_ABS_LO12_NC` |
| `adrp`+`ldr` through `.got` | `ADR_GOT_PAGE` + `LD64_GOT_LO12_NC` |
| pointer word in `.data`/`.rodata` | `R_AARCH64_ABS64` (from `.rela.dyn`) |

Data targets with no symbol are pulled in as anonymous local blobs and
referenced section-relative. This part is heuristic and will need tuning as more
of the binary is attempted.

## Proven facts

* NSO round-trips **byte-exact**: `nx2elf.py fury.nso` -> `MakeNso --compress`
  reproduces `fury.nso` with an identical SHA-256.
  This validates format conversion; it does not yet validate linking rebuilt objects.
* `fury.nso` is **not stripped**: ~69k symbols with mangled C++ names
  (~59,160 functions, ~10.2 MB of `.text`).
* Compiler: **Clang for NX 1.8.14 (LLVM 8.0.1)**, `--target=aarch64-nintendo-nx-elf`.
* Built against **NintendoSDK 10.4.0**. Compile flags follow the SDK manual
  (`docs/nnsdk/.../Page_170694349.html`): `-std=gnu++14 -fno-common
  -fno-short-enums -ffunction-sections -fdata-sections -fPIC
  -mcpu=cortex-a57+fp+simd+crypto+crc -fno-omit-frame-pointer -O3`, with
  `-DNN_NINTENDO_SDK -DNN_SDK_BUILD_RELEASE`.
* First matched function: `LMS_CloseMessage` (100%, 44/44 bytes).

## TODO

* Tune the default compiler flag set against real matches.
* Seed `config/1.0.0/splits.txt` from symbol scopes (`al::`, `sead::`, ...),
  validating each candidate unit's section boundaries.
* Mine `docs/nnsdk` for the exact NSO/DSO spec and linker script.
* Teach `elf_split.py` about jump tables and `.init_array`/`.fini_array` entries.
* Implement a real link step (lld) using units marked `done` and the remaining
  original code and data, then generate and hash-check the NSO.
