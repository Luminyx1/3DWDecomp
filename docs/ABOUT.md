# 3DWDecomp

3DWDecomp is exactly what the name says. It's a matching decompilation effort for Super Mario 3D World + Bowser's Fury for the Nintendo Switch. On the root is "fury.nso", which is the game code, in NSO format. The main idea behind these matching decompilations is to "split" the binary by section and then map those segments into a code file. So for example, 'Object.cpp' will contain `.text` for its actual code, `.data` for vtables, etc, `.got` for the global offset table, etc. I want to use objdiff in unison with a Visual Studio Code session to make decompiling easier and to streamline. I don't plan on ever making this public, just as my own personal endeavour. 

## The Goal
The goal is 1:1 decompilation, reconstructing the code and data in each source file and ultimately rebuilding a byte-exact NSO. The translation-unit manifest, `config/1.0.0/splits.txt`, records the section ranges owned by each unit. The current entry is:

```
src/nw/lms/lms_message.c:
    .text   start:0x006B2540 end:0x006B256C
```

Supported split sections are `.text`, `.rodata`, `.data`, and `.bss`. The splitter carves reference ELF objects directly from `build/fury.elf` and reconstructs relocations. The compiler builds corresponding objects from source, and objdiff compares the two.

Adding `done` after a unit's header marks it complete in objdiff metadata. It does not currently change the build or select objects for linking. A real link step that combines matched source objects with the remaining original code and data, followed by NSO generation and hash verification, is planned but not implemented.

## The Compiler
The current toolchain uses Clang for NX 1.8.14 (LLVM 8.0.1), targeting `aarch64-nintendo-nx-elf`, with the Windows compiler executables under `tools/nnsdk/`. Compiler paths and flags live in `config/1.0.0/config.json`. The default flags follow the NintendoSDK 10.4.0 manual and still need validation against a broader set of matches.

## objdiff
[objdiff](https://github.com/encounter/objdiff) visualizes function matching. `configure.py` generates the root `objdiff.json` from the split manifest; point the GUI or VS Code extension at that file for interactive comparison. `ref/objdiff.json` is a reference example, not the active configuration. This project targets Switch on Windows; broader platform support is not a goal.

