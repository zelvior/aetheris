# Aetheris Compiler Architecture Specification (v1.0)

The **Aetheris Compiler (`aethc`)** is a lightweight, zero-dependency, multi-target translation and compilation system. It transforms `.aeth` source code into deterministic targets—including **C99/C11 (for systems and native builds)**, **JavaScript / WebAssembly (for Web and UI)**, and **SIMD/CUDA C (for AI and tensor workloads)**.

---

## 1. Pipeline Stages

The compilation pipeline follows a clean four-stage design:

```
                          ┌──────────────────────────┐
                          │   Aetheris Source (.aeth)│
                          └────────────┬─────────────┘
                                       │
                         [ Stage 1: Lexical Analysis ]
                                       │
                         [ Stage 2: AST Parsing     ]
                                       │
                         [ Stage 3: Type Checking   ]
                                       │
                         [ Stage 4: Code Generation ]
                                       │
               ┌───────────────────────┼───────────────────────┐
               │                       │                       │
               ▼                       ▼                       ▼
     ┌──────────────────┐    ┌──────────────────┐    ┌──────────────────┐
     │ C99 Target       │    │ JS / WASM Target │    │ CUDA Target      │
     │ (Embedded/32-64) │    │ (Web/Frontend)   │    │ (AI Tensors)     │
     └──────────────────┘    └──────────────────┘    └──────────────────┘
```

---

## 2. Compiler Stages Detailed

### Stage 1: Lexical Analysis (Lexer)
* **Input**: Raw UTF-8 source text.
* **Output**: Token stream with explicit positional information (line, column).
* **Key Features**:
  * Scans tilde (`~`) as `TOKEN_TERMINATOR`.
  * Categorizes numerical literals into explicit bit-widths (`i32`, `uptr`, `f32`).
  * Handles multi-line statements delimited by `~`.

### Stage 2: Abstract Syntax Tree Parsing (AST Parser)
* **Input**: Token stream.
* **Output**: Abstract Syntax Tree (AST).
* **Key Features**:
  * Implements top-down operator precedence (Pratt parsing) for expressions.
  * Validates domain constructs (`component`, `route`, `neural`, `match`).
  * Generates actionable syntax error reporting with exact line markers.

### Stage 3: Semantic Analysis & Architecture Specialization
* **Input**: AST + Target Configuration (`--target=32` or `--target=64`).
* **Output**: Type-checked, architecture-lowered AST.
* **Key Features**:
  * **Pointer Resolution**: Resolves `iptr` and `uptr` sizes (4 bytes for 32-bit, 8 bytes for 64-bit).
  * **Result Control Flow Safety**: Validates that fallible operations with `Result[T, E]` use `check`, `or`, or `match`.
  * **Inference Resolution**: Infers primitive variable types without requiring explicit type annotations.

### Stage 4: Target Code Generation (Codegen)
* **Input**: Lowered AST.
* **Output**: Target-specific source or machine code.
  * **Native Target (`--target=native`)**: Emits optimized C99/C11 code, compiled via GCC/Clang/Zig-CC.
  * **Web Target (`--target=web`)**: Emits modern ES6+ JS or WebAssembly modules.
  * **AI Target (`--target=ai`)**: Emits optimized C with BLAS/OpenMP or NVIDIA CUDA kernels.

---

## 3. Architecture Target Lowering Matrix

| Keyword / Primitive | Native 32-Bit Target | Native 64-Bit Target | Web/JS Target |
| :--- | :--- | :--- | :--- |
| `uptr` / `iptr` | `uint32_t` / `int32_t` | `uint64_t` / `int64_t` | `BigInt` / `Number` |
| `check parsePort()` | `if (res.is_err) return res;` | `if (res.is_err) return res;` | `if (res.err) return res;` |
| `component UI` | Native Canvas/WebAssembly | Native Window / Raylib | Virtual DOM / React-like nodes |
| `tensor[f32]` | ARM NEON / x86 SSE float array | AVX-512 / CUDA float buffer | WebGL / WebGPU Float32Array |

---

## 4. CLI Usage & Flags

```bash
# Build for 64-bit native server systems
aethc build main.aeth --target=native-64 -o server

# Build for 32-bit embedded microcontrollers
aethc build embedded.aeth --target=native-32 -o firmware.bin

# Build web frontend package
aethc build web_app.aeth --target=web -o dist/bundle.js

# Train and infer AI models with GPU acceleration
aethc run model.aeth --target=ai --cuda
```