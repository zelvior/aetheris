# Aetheris Language Development Roadmap (v1.0 – v3.0)

The **Aetheris Development Roadmap** outlines the strategic engineering milestones, feature deliverables, and timeline for bringing the language, compiler, runtime, and ecosystem from core architecture to widespread production adoption.

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                               AETHERIS ROADMAP                                   │
├───────────────────┬───────────────────┬───────────────────┬──────────────────────┤
│     Phase 1       │      Phase 2      │      Phase 3      │       Phase 4        │
│  Core Toolchain   │ Multi-Target Engine│ Ecosystem Expansion│ Autonomous & Native  │
│    (v0.1 – v1.0)  │   (v1.1 – v1.5)   │   (v1.6 – v2.0)   │    (v2.1 – v3.0)     │
└───────────────────┴───────────────────┴───────────────────┴──────────────────────┘
```

---

## Phase 1: Core Foundation & Toolchain Stabilization (v0.1 – v1.0)
*Status: Completed*

The primary objective of Phase 1 was to establish a rock-solid, zero-dependency C11 compiler pipeline, formalize the grammar specification, and provide production-grade developer primitives across the core specification files.

### Key Deliverables
- [x] **Formal Language Specifications**: Complete foundational specs for Lexer, Pratt Parser, Type System, Memory Management, and I/O.
- [x] **Lexer & Pratt Parser (`aethc lexer` / `aethc parser`)**: Robust UTF-8 scanner and expression parser supporting the tilde (`~`) statement boundary.
- [x] **Type Checker & Inferrer**: Bidirectional type checking engine with dual-width pointer integer resolution (`uptr`/`iptr`) for 32-bit and 64-bit target architectures.
- [x] **SSA-like Intermediate Representation**: Linear IR instruction pass (`ALLOC`, `LOAD`, `STORE`, `BINOP`, `BRANCH`).
- [x] **Zero-Overhead C99 Backend**: Codegen target producing clean, standards-compliant C99/C11 code compatible with GCC, Clang, and MSVC.
- [x] **Interactive REPL (`aethi`)**: AST tree-walk interpreter evaluating expressions and statements directly in memory.
- [x] **Frontend Components & Backend Routes**: Syntax and execution engine for `component`, `render`, `show`, `route`, and `reply ... with status`.
- [x] **Native Web Engine (`aeth dev`)**: Pure Aetheris site bundle generator (`.aeth_site`, `.aeth_ui`).
- [x] **Standard Library (`std`)**: Modules for `std.math`, `std.string`, `std.collections`, `std.io`, `std.json`.
- [x] **Unified CLI Toolchain (`aeth`)**: Managed CLI binary handling `run`, `build`, `dev`, and `repl`.

---

## Phase 2: Architecture Adapters & Domain Engines (v1.1 – v1.5)
*Target Timeline: Q1 – Q3 2027*

Phase 2 expands Aetheris across target domains—giving first-class code generation capabilities to microcontrollers, reactive web targets, and GPU tensor accelerators.

### Key Deliverables
- [ ] **Dual-Width Target Compiler (`--target=native-32` vs `--target=native-64`)**:
  - Automatic `uptr`/`iptr` pointer alignment verification.
  - Aggressive tail-call optimizations for 32-bit embedded targets with minimal RAM footprints.
  - Native SIMD vectorization (AVX-512, ARM NEON, RISC-V Vector Extensions).
- [ ] **High-Throughput Async Backend Framework**:
  - Zero-copy TCP/UDP networking runtime using Linux `io_uring` and epoll multiplexing.
- [ ] **AI & CUDA Kernel Generator (`--target=ai`)**:
  - Translation of `neural` architectures into specialized C/CUDA kernels for NVIDIA GPU acceleration.
