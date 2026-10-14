# Aetheris Installation & User Guide

Welcome to **Aetheris (`.aeth`)**—the universal, architecture-aware programming language engineered for zero-overhead safety across microcontrollers, servers, web applications, and AI pipelines.

---

## Direct Start (Zero Configuration Required)

You do **not** need to manually install dependencies or configure build environments. Simply clone the repository and start coding directly using the included `./aeth` executable wrapper:

```bash
# Clone the repository
git clone https://github.com/aetheris-lang/aetheris.git
cd aetheris

# Check toolchain version
./aeth --version

# Run any Aetheris program immediately!
./aeth run examples/demo.aeth
```

The `./aeth` wrapper automatically builds and compiles the ISO C11 toolchain on its first run.

---

## Toolchain Subcommands Reference

| Command | Action | Example |
| :--- | :--- | :--- |
| `./aeth run <file.aeth>` | Executes script directly in the interpreter | `./aeth run src/main.aeth` |
| `./aeth dev [file.aeth]` | Builds web assets in `dist/` and launches local dev server | `./aeth dev src/app.aeth` |
| `./aeth build <file> [options]` | Transpiles `.aeth` to C99 native target code | `./aeth build src/main.aeth --target=native-64` |
| `./aeth repl` | Opens interactive REPL shell | `./aeth repl` |
| `./aeth --version` | Displays toolchain version | `./aeth --version` |

---

## Target Flags

* `--target=native-64`: Targets 64-bit desktop/server systems (default).
* `--target=native-32`: Targets 32-bit microcontroller systems.
* `--target=web`: Transpiles frontend components and site bundles into `dist/`.
* `--target=ai`: Targets AI tensor accelerators.
