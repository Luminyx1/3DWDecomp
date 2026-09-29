# Getting started

See `ABOUT.md` for the goal. This file is the working state of the toolchain.

## Layout

```
fury.nso                        the target (NSO0, not committed)
configure.py                    config/1.0.0/*  ->  build.ninja + objdiff.json
config/1.0.0/
  config.json                   toolchain, compiler flags, include roots
  function_map.tsv              every .text function: folder / object / name (see below)
  splits.txt                    translation units (hand-made at the top, generated below)
  symbols.txt                   names for unnamed local data (nerves, statics)
src/                            the decompilation, one file per translation unit
  Game/<Folder>/                  game code (RedCarpet)
  al/<Library|Project>/<Folder>/  ActionLibrary
  sead/  agl/  eui/  nw/<lib>/    libraries (LMS lives in sead/lms)
include/
  Game/  al/  sead/  agl/  eui/   headers (include roots; started from the 3dcomp project)
  nnheaders/                      community NintendoSDK headers (-isystem)
tools/
  nso.py  nx2elf.py             NSO parsing, NSO -> ELF (build/fury.elf)
  elf_split.py                  carve a unit's ranges out of fury.elf into a target .o
  furyimg.py  elfobj.py         support libs for elf_split (symbol index is cached)
  nxcc.py                       compiler wrapper (wine off-Windows, depfile fix, strips $x/$d)
  strip_mapsyms.py              removes AArch64 mapping symbols from our objects
  complete_map.py               fill in missing objects in the function map
  gen_splits.py                 function map -> generated part of splits.txt
  diff.py                       text diffs / match summary via objdiff-cli
  vtable.py                     print a class's vtable (slot offsets -> method names)
  pair_names.py                 name the game's sub_/lbl_ symbols after our source
  bin/objdiff-cli(.exe)         objdiff 3.8.1
  nnsdk/                        Nintendo SDK compiler (Clang for NX 1.8.14) - not committed
build/                          generated - not committed
```

## The loop

```bash
python configure.py            # regenerate build.ninja + objdiff.json from config/1.0.0/
ninja                          # fury.elf + every unit that has source (target and ours)
ninja all                      # also carve every other unit (needed for a full report)
tools/bin/objdiff-cli report generate -o build/report.json -f json-pretty

python tools/diff.py BgmStopObj                  # match % per function of one unit
python tools/diff.py BgmStopObj _ZN10BgmStopObj4initERKN2al13ActorInitInfoE
python tools/diff.py --status                    # every unit that has source
python tools/diff.py --mark-done                 # ... and mark the fully matching ones done
```

For interactive work, point the objdiff GUI / VS Code extension at `objdiff.json`.

The compiler is always the SDK's own `clang.exe` (Clang for NX 1.8.14, LLVM 8.0.1).
On Windows it runs directly; on Linux/macOS it runs under wine (`wine` on PATH,
or set `$WINE`; start `wineserver -p` once so each compile doesn't pay wine's
startup). Upstream LLVM 8.0.1 is *not* a substitute: it schedules prologues
differently (see `LMS_CloseMessage`).

## Decompiling a unit

Every object in the function map already has a unit in `splits.txt` with its
`.text` range, so:

1. Create the source file at the unit's path (e.g. `src/Game/MapObj/BgmStopObj.cpp`)
   and run `python configure.py` (units only get a compile step once their
   source exists).
2. `ninja`, then `python tools/diff.py <unit>`; iterate until every function is 100%.
3. Local functions and data have no names in the binary: the target shows
   `sub_7100...`, `nullsub_N` or `lbl_7100...`. Once our object compiles, run
   `python tools/pair_names.py <unit>` - it matches them to our names through
   the relocations (and by order/size for nerve `execute`s) and writes them to
   `function_map.tsv` / `symbols.txt`. Re-run `ninja` to re-carve.
4. `python tools/diff.py --mark-done` adds `done` to every fully matching unit's
   header in `splits.txt`; rerun `configure.py`.

### Conventions learned so far

* **Flags** (config.json): `-O3 -mno-implicit-float`, no `-ffunction-sections`, `-std=gnu++17 -fno-rtti
  -fno-exceptions` for C++; C (LMS) also needs `-fno-strict-aliasing`.
  `-mno-implicit-float` is what keeps pointer copies in `ldp/stp x` registers and
  stops loop vectorization, as the game does.
* **Nerves** are one constant object per nerve in an anonymous namespace
  (`Nrv<Class><Action>`, placed right after its vtable), not a struct:
  `NERVE_DECL(Foo, Wait) ... NERVES_MAKE_NOSTRUCT(Foo, Wait, Run)` then
  `al::setNerve(this, &NrvFooWait)`. See `src/Game/MapObj/BgmStopObj.cpp`.
* **Weak / inline functions** (header code) are emitted in whichever object
  first used them; objdiff lists our extra copies as "extra in source" - ignore.
  Their bodies stay in headers (see FUNCTIONS.md).
* **Sections**: a unit's own functions are in one `.text` (no
  `-ffunction-sections`); weak/inline functions each get a COMDAT
  `.text.<name>`, both in our objects and in the carved targets.
* **Interfaces** (`IUsePlayerCollision`, `IUsePlayerInput`, ...) have no symbols of
  their own, but their implementers' vtables do: `python tools/vtable.py PlayerCollider`
  lists every slot with its offset and name, and secondary groups (other bases)
  with their offset-to-top. A call through `ldr x8, [x8, #0x50]` on an
  `IUsePlayerCollision*` is slot +0x50 there (`isOnFloor`). See `include/Game/Player/`.
* **Classes whose constructor is missing** from the binary had it inline in the
  header (it was only used in other files).
* **objdiff's report** ignores relocation targets (e.g. which nerve a function
  sets); `tools/diff.py` does not. Only mark a unit `done` when `diff.py` says 100%.

## The function map

`config/1.0.0/function_map.tsv` is the spreadsheet (Start Address, Folder,
Object, Not In Obj, Mangled Name, Demangled Name) plus an `Inferred` column.
Folders and objects are marked on their first function. The hand-made map was
finished with `tools/complete_map.py`, using:

* objects are laid out folder by folder, alphabetically (case-insensitive,
  compared upper-case) inside a folder, so an object that sorts *before* the
  previous one means a new folder began;
* function names shared with Super Mario Odyssey tell us SMO's object for al,
  sead and eui code (OdysseyDecomp `data/file_list.yml`);
* otherwise a new class name in an unfinished stretch starts `<Class>.o`.

Everything it added is labelled in the `Inferred` column (and `note:` flags
places where the hand-made names break the alphabetical order). It is a best
guess - fix it in the sheet, re-export, and run:

```bash
python tools/gen_splits.py && python configure.py
```

`gen_splits.py` rewrites only the part of `splits.txt` below the GENERATED
marker and keeps each unit's `done` / `cflags=` options. Units above the marker
are hand-maintained and take precedence.

## How the target objects are made

The final image no longer contains the original object-file relocations.
`elf_split.py` rebuilds them by disassembling each carved function (capstone),
and uses the retained dynamic relocations for data pointers:

| pattern | reconstructed relocation |
|---|---|
| `bl <addr>` | `R_AARCH64_CALL26` -> symbol at addr |
| `b <addr>` (tail call, outside the function) | `R_AARCH64_JUMP26` |
| `adrp`+`add`/`ldr`/`str` to a direct address | `ADR_PREL_PG_HI21` + `ADD_ABS_LO12_NC` / `LDSTn_ABS_LO12_NC` (every consumer) |
| `adrp`+`ldr` through `.got` | `ADR_GOT_PAGE` + `LD64_GOT_LO12_NC` |
| pointer word in `.data`/`.rodata` | `R_AARCH64_ABS64` (from `.rela.dyn`) |

Merged data is rebuilt the way clang lays it out in an object: string literals
go to `.rodata.str1.1` (first-use order), FP constants to `.rodata.cst4/8/16`,
switch tables to `.rodata`. Named data outside the unit becomes an
undefined reference by name; unnamed data becomes `lbl_<address>` until it is
named in `symbols.txt`. Relocated instruction fields are zeroed, as in a real
object file.

## Proven facts

* NSO round-trips **byte-exact**: `nx2elf.py fury.nso` -> `MakeNso --compress`
  reproduces `fury.nso` with an identical SHA-256.
* `fury.nso` is **not stripped**: ~69k symbols with mangled C++ names
  (~59,160 functions, ~10.2 MB of `.text`). Local functions/data are stripped.
* Compiler: **Clang for NX 1.8.14 (LLVM 8.0.1)**; under wine it produces
  byte-identical objects to the Windows build.
* RTTI exists only for NintendoWare classes; everything else is `-fno-rtti`.
* Matched: `src/sead/lms/lms_memory.c`, `src/Game/MapObj/BgmStopObj.cpp`, most of
  `src/sead/lms/*`.

## TODO

* Data-section splits (`.rodata`/`.data`/`.bss` ranges) for matched units.
* NintendoWare: compile the SDK's own sources (NintendoSDK/Sources/Libraries)
  with the SDK's include folder, and use their symbol sets to fix NW object
  boundaries in the map.
* Port the ~2,000 functions the 3dcomp project matched with upstream clang.
* Implement a real link step (lld) using units marked `done` and the remaining
  original code and data, then generate and hash-check the NSO.
