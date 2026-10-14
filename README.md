# Aetheris (`.aeth`)

**One Syntax. Any Target. Absolute Clarity.**

Aetheris is a universal, architecture-aware programming language engineered to deliver zero-overhead safety and maximum developer efficiency across 32-bit microcontrollers, 64-bit servers, WebAssembly front ends, and GPU-accelerated AI pipelines, with no garbage collector and no hidden runtime cost.

```aeth
fn parsePort(rawText: text) -> Result[u16, text] ~
    if rawText == "" ~
        return err("Port string cannot be empty") ~
    end
    let portNum = convertToU16(rawText) ~
    return ok(portNum) ~
end

fn main() -> Result[unit, text] ~
    let port = check parsePort("8080") ~
    print("Server initialized on port: " + port) ~
    return ok(()) ~
end
```

---

## Highlights

* **Implementation**: The Aetheris core toolchain (`aeth`) is written in **ISO C11 (`gcc -std=c11`)** with zero external third-party framework or runtime dependencies.
* **Tilde terminator (`~`)**: Statements end with `~`, blocks close with `end`. Readable, and independent of whitespace.
* **Architecture-aware types**: `i32`, `u64`, and pointer-sized `uptr`/`iptr` that adapt to `--target=native-32` or `native-64`.
* **No silent widening**: Numeric conversions are explicit (`value as u32`).
* **Deterministic errors**: `Result[T, E]` with `check`, `or`, and `match`; no exceptions or stack unwinding.
* **Domain keywords built in**: `component`, `render`, `show`, `route`, `reply`, `async`, `await`, `spawn`, `neural`, `infer`.
* **Zero-overhead lowering**: C99, WebAssembly/JS, SSA IR, CUDA/SIMD, or direct assembly output.
* **One toolchain**: compiler (`aethc`), interpreter (`aethi`), linker, webgen, standard library (`std`), and REPL in a single `aeth` binary.

---

## Quick Start

```bash
# Build toolchain from source
make bootstrap
./build/bin/aeth --version

# Run a project script in the interpreter
./build/bin/aeth run src/main.aeth

# Build for different targets
./build/bin/aeth build src/main.aeth --target=native-64      # servers and desktops
./build/bin/aeth build src/main.aeth --target=native-32      # microcontrollers
./build/bin/aeth dev src/main.aeth                           # web frontend server & dist/ package
```

---

## Documentation Map

### Language & Core Specifications
| Document | Topic |
| :--- | :--- |
| `PHILOSOPHY.md` | Design goals and tenets |
| `SYNTAX.md`, `KEYWORDS_AND_OPERATORS.md` | Grammar, keywords, operators, precedence |
| `DATA_TYPES.md`, `TYPE_SYSTEM.md` | Primitives, generics, inference |
| `FUNCTIONS_AND_SCOPING.md` | Functions, closures, async/await |
| `ERROR_HANDLING.md` | `Result`, `check`, `or`, `match` |
| `MEMORY_MANAGEMENT.md` | Memory allocation and allocators |

### Toolchain & Architecture
| Document | Topic |
| :--- | :--- |
| `LEXER_OR_TOKENIZER.md`, `PARSER_AND_AST.md` | Scanner, Pratt parser, and AST |
| `COMPILER.md`, `COMPILER_TARGETS_AND_ASM.md` | IR, C99 backend, assembly emission |
| `INTERPRETER.md`, `VIRTUAL_MACHINE_SPEC.md` | Tree-walk interpreter and REPL |
| `FRONTEND.md`, `BACKEND.md` | Reactive UI components & HTTP REST routes |
| `STANDARD_LIBRARY.md` | Built-in `std` overview (`std.math`, `std.string`, `std.io`, `std.json`) |
| `ROADMAP.md` | Timeline and completed Phase 1 deliverables |
