# Aetheris Debugging Symbols & DWARF Specification (v1.0)

This document defines how `aethc` emits debug information, how Aetheris constructs map to DWARF, and how debuggers (GDB, LLDB, VS Code) interact with Aetheris programs.

---

## 1. Debug Info Levels

| Flag | Contents | Use |
| :--- | :--- | :--- |
| `--debug=0` (default in release) | None | Smallest binaries |
| `--debug=line` | Line tables only | Backtraces and profiling |
| `--debug=1` | Line tables + types + variables | Normal debugging |
| `--debug=2` | Adds macro/derive expansion and inlined frames | Deep debugging |

```bash
aethc build main.aeth --target=native-64 --debug=1 -o server
aethc build main.aeth --debug=1 --split-debug   # writes server.debug beside the binary
```

---

## 2. DWARF Version & Sections

DWARF 5 is the default (`--dwarf=4` for older tooling). Emitted sections:

| Section | Purpose |
| :--- | :--- |
| `.debug_info` | Types, functions, variables |
| `.debug_abbrev` | Abbreviations for `.debug_info` |
| `.debug_line` | Source line to address mapping |
| `.debug_str`, `.debug_line_str` | String tables |
| `.debug_frame` / `.eh_frame` | Call frame information for unwinding |
| `.debug_loclists`, `.debug_rnglists` | Variable locations, ranges |
| `.debug_aranges`, `.debug_names` | Fast lookup |
| `.aethdbg` | Aetheris-specific extensions (see §5) |

On Windows, PDB files are emitted; on macOS, `.dSYM` bundles.

---

## 3. Type Mapping

| Aetheris | DWARF |
| :--- | :--- |
| `i8`…`i64` | `DW_TAG_base_type` (`DW_ATE_signed`) |
| `u8`…`u64` | `DW_ATE_unsigned` (`DW_ATE_unsigned_char` for `u8` in byte contexts) |
| `f32`, `f64` | `DW_ATE_float` |
| `bool` | `DW_ATE_boolean` |
| `iptr`, `uptr` | Base type sized 4 or 8 by target, named `iptr`/`uptr` |
| `text` | Structure `{ ptr: *u8, len: uptr }` with UTF-8 string visualizer |
| `list[T]` | Structure `{ ptr: *T, len: uptr, cap: uptr }` |
| `map[K, V]` | Opaque structure with a pretty-printer |
| `struct` | `DW_TAG_structure_type` with member offsets |
| `enum` with payload | `DW_TAG_variant_part` |
| `Result[T, E]` | Variant: `ok(T)` / `err(E)` discriminated by tag |
| Generics | Each monomorphized instance is a distinct type named `identity[i32]` |
| `fn(T) -> U` | `DW_TAG_subroutine_type` |

Type sizes and alignments reflect the **selected target** (`native-32` vs `native-64`), so struct layouts shown in the debugger match the machine.

---

## 4. Source Mapping

* **Line tables** map machine addresses to `file:line:column`, with `is_stmt` set at `~` boundaries so single-stepping advances one Aetheris statement at a time.
* **Inlined functions** use `DW_TAG_inlined_subroutine`, preserving call-site info.
* **`check` operator**: the implicit early-return branch is attributed to the `check` expression and marked with a synthetic frame note `[check: propagating err]`.
* **Constant-folded and DCE'd code** keeps the original line mapping where possible; optimized builds may show `<optimized out>`.
* **Tail-call optimization** removes frames; use `--debug=2 --keep-tco-frames` to emit synthetic frames for debugging.

---

## 5. Aetheris-Specific Extensions (`.aethdbg`)

| Extension | Description |
| :--- | :--- |
| Component map | Maps `component` signals and `render` nodes to generated DOM or UI nodes |
| Async task table | Names and states of suspended `async` tasks, with logical await chains |
| Route map | Maps `route` declarations to handler symbols |
| Tensor shapes | Shape and dtype metadata for `tensor[T]` values |
| Log map | Message IDs to format strings (see `LOGGING_AND_TELEMETRY.md`) |
| Macro/derive origins | Links generated code back to `@derive` sites |

Debuggers without extension support still function using standard DWARF.

---

## 6. Debugger Support

| Debugger | Support |
| :--- | :--- |
| GDB / LLDB | Python/LLDB pretty-printers shipped in `share/aeth/debug/` for `text`, `list`, `map`, `Result` |
| VS Code | Debug Adapter Protocol (`aeth dap`) with breakpoints, watch, async task view |
| Browser DevTools | Source maps (`.wasm.map`, `.js.map`) with DWARF-in-WASM (`--wasm-dwarf`) |
| Embedded | OpenOCD / probe-rs over SWD/JTAG; RTT or semihosting for console |

### Example GDB Session

```text
(gdb) break parsePort
(gdb) run
Breakpoint 1, parsePort (rawText="8080") at src/net.aeth:12
12     let portNum = convertToU16(rawText) ~
(gdb) print rawText
$1 = "8080"
(gdb) finish
Run till exit from parsePort
Value returned is ok(8080)
```

---

## 7. Expression Evaluation & Conditions

The debugger evaluates a subset of Aetheris expressions: arithmetic, member access, indexing, casts (`x as u32`), and `deref`. Conditional breakpoints use the same syntax:

```text
(gdb) break handleRequest if req.body.len() > 1024
```

## 8. Backtraces & Panics

Debug-built panics print a symbolized backtrace using `.debug_line` and `.eh_frame`. Release builds with `--debug=line` and `--strip-names=off` produce usable traces; fully stripped builds print raw addresses that `aeth symbolize --debug server.debug <addrs>` resolves offline.

## 9. Reproducibility & Privacy

Absolute build paths are remapped with `--remap-path-prefix /home/me/proj=.`, and `.debug` files are keyed by a build ID (SHA-1 of the code) so symbol servers can match binaries exactly.
