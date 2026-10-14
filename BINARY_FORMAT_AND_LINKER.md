# Aetheris Binary Format & Linker Specification (v1.0)

This document defines the object file layout (`.aobj`), the package archive format (`.alib`), the executable output formats, and the behavior of the Aetheris linker (`aethl`).

---

## 1. Output Formats

| Target | Executable Format | Notes |
| :--- | :--- | :--- |
| Linux | ELF64 / ELF32 | Static by default; optional dynamic linking |
| macOS | Mach-O | Universal (x86_64 + arm64) supported |
| Windows | PE/COFF | `.exe` and `.dll` |
| Bare-metal | Raw `.bin`, Intel `.hex`, or ELF | Controlled by linker script |
| Web | `.wasm` + JS glue | See `BROWSER_COMPILATION_OR_WASM.md` |

---

## 2. Aetheris Object File (`.aobj`)

An `.aobj` file is produced per module and carries machine code plus metadata that standard object formats cannot represent (generics, type signatures, target width).

```
┌──────────────────────────────┐
│ Header (magic "AETH", ver)   │
├──────────────────────────────┤
│ Target descriptor            │  arch, pointer width, endianness
├──────────────────────────────┤
│ Section table                │
├──────────────────────────────┤
│ .text   (machine code)       │
│ .rodata (constants, text)    │
│ .data   (initialized data)   │
│ .bss    (zero-initialized)   │
├──────────────────────────────┤
│ Symbol table                 │
│ Relocation table             │
├──────────────────────────────┤
│ Type metadata (.aethtypes)   │  signatures for cross-module checking
│ Debug info (.aethdbg/DWARF)  │
└──────────────────────────────┘
```

### Header Layout

| Offset | Size | Field |
| :--- | :--- | :--- |
| 0x00 | 4 | Magic `0x48544541` ("AETH", little-endian) |
| 0x04 | 2 | Format version (major, minor) |
| 0x06 | 1 | Pointer width (`4` or `8`) |
| 0x07 | 1 | Endianness (`0` little, `1` big) |
| 0x08 | 4 | Architecture ID |
| 0x0C | 4 | Section table offset |
| 0x10 | 4 | Section count |

Linking `.aobj` files of different pointer widths is a hard error (`L001`).

---

## 3. Symbol Mangling

Symbols are mangled as `_A<len><module>_<len><name>_<signature-hash>`.

Example: `fn add(a: i32, b: i32) -> i32` in module `math` becomes `_A4math_3add_9f3c21`.

Functions marked `export c` use the plain, unmangled name for C interoperability:

```aeth
export c fn aetheris_add(a: i32, b: i32) -> i32 ~
    return a + b ~
end
```

---

## 4. Linking Process

1. **Resolve**: Collect `.aobj` and `.alib` inputs, resolve imported symbols.
2. **Type Verify**: Compare `.aethtypes` signatures across module boundaries.
3. **Tree-Shake**: Remove unreachable functions and data (section-level garbage collection).
4. **Layout**: Assign addresses based on the linker script.
5. **Relocate**: Patch addresses using the relocation table.
6. **Emit**: Write the final executable, image, or library.

### Linker Flags

| Flag | Description |
| :--- | :--- |
| `--static` / `--dynamic` | Link mode |
| `--lto` | Link-time optimization across modules |
| `--strip` | Remove symbols and debug info |
| `--map=<file>` | Write a memory map |
| `--script=<file>` | Use a custom linker script |
| `--gc-sections` | Enabled by default |

---

## 5. Linker Scripts for Embedded Targets

```ld
MEMORY ~
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K ~
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K ~
end

SECTIONS ~
    .vectors : { KEEP(*(.vectors)) } > FLASH ~
    .text    : { *(.text*) *(.rodata*) } > FLASH ~
    .data    : { *(.data*) } > RAM AT > FLASH ~
    .bss     : { *(.bss*) *(COMMON) } > RAM ~
    _stack_top = ORIGIN(RAM) + LENGTH(RAM) ~
end
```

The `.data` section is copied from flash to RAM by `_aeth_start` before `main`.

---

## 6. Static Libraries (`.alib`)

An `.alib` is an archive of `.aobj` files plus an index and a manifest (`name`, `version`, `target`, `checksum`). Archives are content-hashed (SHA-256) so reproducible builds produce identical bytes.

## 7. Foreign Function Interface

```aeth
extern c fn puts(s: *const u8) -> i32 ~
```

C calling conventions are used for `extern c` and `export c`. Struct layouts marked `@repr(c)` match the platform C ABI.
