# Aetheris Installation & Toolchain Specification (v1.0)

The **Aetheris Toolchain (`aeth`)** is designed to be lightweight, fast, and self-contained. It packages the compiler (`aethc`), interpreter/REPL (`aethi`), package manager, and target toolchains into a single binary distribution with zero heavy external dependencies.

---

## 1. System Requirements

Aetheris requires minimal system resources to compile and run:

* **CPU**: 1 GHz or faster (x86_64, ARM64, RISC-V).
* **RAM**: Minimum 512 MB (2 GB+ recommended for large AI tensor model compilation).
* **Disk Space**: ~50 MB for the core toolchain and standard library.
* **Operating Systems**: Linux (kernel 4.x+), macOS (Catalina or newer), Windows 10/11 (via WSL2 or native PowerShell).

---

## 2. Quick Install Scripts

### A. Linux & macOS (Unix Shell)
Open your terminal and run the automated installation script to download and configure the latest stable Aetheris toolchain:

```bash
curl -sSL https://install.aetheris.lang | sh
```

The script will automatically detect your architecture (`native-32` or `native-64`), place the `aeth` binary in your local bin path, and configure your shell environment (`~/.bashrc` or `~/.zshrc`).

### B. Windows (PowerShell)
Open PowerShell as Administrator and execute the install command:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force; Invoke-WebRequest -Uri "https://install.aetheris.lang/install.ps1" -UseBasicParsing | Invoke-Expression
```

---

## 3. Verifying the Installation

Once installed, open a new terminal window and verify your Aetheris version and toolchain environment:

```bash
aeth --version
```

Expected output:
```text
Aetheris Toolchain v1.0.0 (Target: native-64, LLVM/C Backend)
Copyright (c) Aetheris Foundation.
```

---

## 4. Building from Source (For Contributors)

If you wish to build the Aetheris compiler and interpreter directly from source code:

### Prerequisites
* A C99-compliant compiler (`gcc`, `clang`, or `zig cc`).
* Make or CMake (v3.15+).

### Steps
1. Clone the official repository:
   ```bash
   git clone https://github.com/aetheris-lang/aetheris.git
   cd aetheris
   ```

2. Configure and build using the bootstrap script:
   ```bash
   make bootstrap
   ```

3. Run the test suite to ensure correctness:
   ```bash
   make test
   ```

4. Install globally to your system:
   ```bash
   sudo make install
   ```

---

## 5. Toolchain CLI Reference

The `aeth` command-line interface provides access to compiling, running, and managing projects:

| Command | Description | Example |
| :--- | :--- | :--- |
| `aeth new <name>` | Initializes a new Aetheris project template. | `aeth new my_app` |
| `aeth run <file>` | Runs an `.aeth` script instantly via the interpreter (`aethi`). | `aeth run main.aeth` |
| `aeth build <file>` | Compiles source code into a target binary or web bundle. | `aeth build main.aeth --target=native-64` |
| `aeth test` | Runs unit tests inside the current project workspace. | `aeth test` |

### Target Compilation Flags for `aeth build`:
* `--target=native-64`: Compiles for high-throughput 64-bit servers/desktops (default).
* `--target=native-32`: Compiles for resource-constrained 32-bit microcontrollers.
* `--target=web`: Compiles down to optimized ES6 JS / WebAssembly.
* `--target=ai`: Compiles tensor operations with CUDA/SIMD acceleration flags.