# Aetheris (`.aeth`)

**One Syntax. Any Target. Absolute Clarity.**

Aetheris is a universal, architecture-aware programming language. A single readable grammar builds 32-bit microcontroller firmware, 64-bit servers, WebAssembly front ends, and GPU-accelerated AI pipelines, with no garbage collector and no hidden runtime cost.

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

* **Tilde terminator (`~`)**: Statements end with `~`, blocks close with `end`. Readable, and independent of whitespace.
* **Architecture-aware types**: `i32`, `u64`, and pointer-sized `uptr`/`iptr` that adapt to `--target=native-32` or `native-64`.
* **No silent widening**: Numeric conversions are explicit (`value as u32`).
* **Deterministic errors**: `Result[T, E]` with `check`, `or`, and `match`; no exceptions or stack unwinding.
* **Domain keywords built in**: `component`, `route`, `neural`, `kernel`, with no extra frameworks required.
* **Zero-overhead lowering**: C99, WebAssembly/JS, CUDA/SIMD, or direct assembly output.
* **One toolchain**: compiler, interpreter, linker, formatter, linter, language server, and package manager in a single `aeth` binary.

---

## Quick Start

```bash
# Install (Linux / macOS)
curl -sSL https://install.aetheris.lang | sh
aeth --version

# Create and run a project
aeth new my_app
cd my_app
aeth run src/main.aeth

# Build for different targets
aeth build src/main.aeth --target=native-64      # servers and desktops
aeth build src/main.aeth --target=native-32      # microcontrollers
aeth build src/main.aeth --target=web            # WebAssembly / JS
aeth build src/main.aeth --target=ai --gpu=cuda  # GPU
```

Windows install instructions and build-from-source steps are in `INSTALLATION.md`.

---

## A Taste of Each Domain

**Backend**
```aeth
route GET "/api/v1/users/:id" ~
    let user = check await db.users.findById(request.params.id) or reply "User not found" with status 404 ~
    reply user with status 200 ~
end
```

**Frontend**
```aeth
component Counter ~
    mut state count: i32 = 0 ~
    render ~
        show Button("Increment") ~
        on click ? trigger count = count + 1 ~
        show Text("Count: " + count) ~
    end
end
```

**Embedded**
```aeth
fn setPin(pin: u8) ~
    let odr: uptr = GPIO_BASE_ADDR + ODR_OFFSET ~
    *(odr as *mut u32) = *(odr as *mut u32) | (1 << pin) ~
end
```

**AI**
```aeth
neural SentimentClassifier ~
    layer Dense(inputs: 512, outputs: 256, activation: "relu") ~
    layer Dense(inputs: 256, outputs: 2, activation: "softmax") ~
end
```

More in `EXAMPLES.md`.

---

## Documentation Map

### Language
| Document | Topic |
| :--- | :--- |
| `PHILOSOPHY.md` | Design goals and tenets |
| `SYNTAX.md`, `KEYWORDS_AND_OPERATORS.md` | Grammar, keywords, operators, precedence |
| `DATA_TYPES.md`, `TYPE_SYSTEM.md` | Primitives, generics, inference |
| `FUNCTIONS_AND_SCOPING.md` | Functions, closures, async |
| `ERROR_HANDLING.md` | `Result`, `check`, `or`, `match` |
| `MEMORY_MANAGEMENT.md`, `CUSTOM_MEMORY_ALLOCATOR.md` | Allocation and allocators |
| `EXAMPLES.md` | Real-world patterns |

### Toolchain
| Document | Topic |
| :--- | :--- |
| `LEXER_OR_TOKENIZER.md`, `PARSER_AND_AST.md` | Front end |
| `COMPILER.md`, `COMPILER_TARGETS_AND_ASM.md` | Pipeline, IR, backends, assembly |
| `BINARY_FORMAT_AND_LINKER.md` | Object format, linking, linker scripts |
| `INTERPRETER.md`, `VIRTUAL_MACHINE_SPEC.md` | `aethi` and bytecode VM |
| `HOT_RELOAD_AND_REPL.md` | Live development |
| `LSP_SPECIFICATION.md` | Editor support |
| `LINTER_AND_FORMATTER.md` | `aeth lint`, `aeth fmt` |
| `DEBUGGING_SYMBOLS_AND_DWARF.md` | Debug info |
| `INSTALLATION.md` | Setup and CLI reference |

### Platform & Runtime
| Document | Topic |
| :--- | :--- |
| `RUNTIME_ENVIRONMENT.md` | Startup, panics, runtime profiles |
| `SYSTEM_CALLS_AND_OS_INTERACTION.md` | OS interface |
| `PROCESS_SIGNAL_HANDLING.md` | Processes and signals |
| `IO.md` | Streams, files, sockets |
| `LOGGING_AND_TELEMETRY.md` | Logs, metrics, traces |

### Domains
| Document | Topic |
| :--- | :--- |
| `FRONTEND.md`, `BROWSER_COMPILATION_OR_WASM.md` | Web |
| `NATIVE_GUI_AND_GRAPHICS.md`, `ACCESSIBILITY_AND_A11Y.md`, `ASSET_PIPELINE_AND_FONTS.md` | Desktop UI and assets |
| `BACKEND.md` | HTTP services |
| `MATH_INTRINSICS_AND_SIMD.md`, `GPU_NATIVE_EMISSION.md` | Numerics and GPU |
| `MODEL_CONVERSION_AND_ONNX.md` | ML model import/export |
| `DATA_STREAMING_AND_PIPELINES.md` | Streaming |
| `JSON_AND_BINARY_SERIALIZATION.md`, `COMPRESSION_ALGORITHMS.md` | Data formats |
| `CRYPTOGRAPHY_AND_SECURITY.md` | Security |
| `STANDARD_LIBRARY.md` | `std` overview |

### Project
| Document | Topic |
| :--- | :--- |
| `ROADMAP.md` | Plans through v3.0 |
| `CONTRIBUTING.md` | How to contribute |
| `LICENSE.md` | Aetheris Open License v1.0 |

---

## Architecture at a Glance

```
 .aeth source ─► Lexer ─► Pratt Parser ─► Type Checker ─► SSA IR ─► Backends
                                                                      ├─ C99 (native-32 / native-64)
                                                                      ├─ WASM / JS (web)
                                                                      ├─ CUDA / SIMD (ai)
                                                                      └─ x86_64 / AArch64 / ARM32 assembly
```

---

## Status

Aetheris is under active development. See `ROADMAP.md` for which features are complete and which are planned. Specifications are versioned (currently v1.0); expect refinements before the language is declared stable.

## Contributing

Contributions of all kinds are welcome: bug reports, documentation, compiler work, standard library modules, and examples. Read `CONTRIBUTING.md` for setup, testing, and style guidelines.

```bash
git clone https://github.com/aetheris-lang/aetheris.git
cd aetheris && make bootstrap && make test
```

## Security

Report vulnerabilities privately rather than through public issues. See `CRYPTOGRAPHY_AND_SECURITY.md` for the reporting process and supply-chain features.

## License

Released under the **Aetheris Open License v1.0**. See `LICENSE.md`. Code you write and artifacts you generate with Aetheris belong to you.
