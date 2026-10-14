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
*Status: Current Focus*

The primary objective of Phase 1 is to establish a rock-solid, zero-dependency compiler pipeline, formalize the grammar specification, and provide production-grade developer primitives across the core specification files.

### Key Deliverables
- [x] **Formal Language Specifications**: Complete foundational specs for Lexer, Pratt Parser, Type System, Memory Management, and I/O.
- [x] **Lexer & Pratt Parser (`aethc lexer` / `aethc parser`)**: Robust UTF-8 scanner and expression parser supporting the tilde (`~`) statement boundary.
- [x] **Type Checker & Inferrer**: Bidirectional type checking engine with compile-time monomorphization for generic bounds `[T]`.
- [x] **Zero-Overhead C99 Backend**: Codegen target producing clean, standards-compliant C99/C11 code compatible with GCC, Clang, and MSVC.
- [x] **Interactive REPL (`aethi`)**: AST tree-walk interpreter and bytecode VM engine for immediate evaluation and debugging.
- [x] **Package & Toolchain CLI (`aeth`)**: Unified CLI binary managing project initialization, compilation targets, and testing routines.

---

## Phase 2: Architecture Adapters & Domain Engines (v1.1 – v1.5)
*Target Timeline: Q1 – Q3 2027*

Phase 2 expands Aetheris across target domains—giving first-class code generation capabilities to microcontrollers, reactive web targets, and GPU tensor accelerators.

### Key Deliverables
- [ ] **Dual-Width Target Compiler (`--target=native-32` vs `--target=native-64`)**:
  - Automatic `uptr`/`iptr` pointer alignment verification.
  - Aggressive tail-call optimizations for 32-bit embedded targets with minimal RAM footprints.
  - Native SIMD vectorization (AVX-512, ARM NEON, RISC-V Vector Extensions).
- [ ] **Web Engine & WASM Transpiler (`--target=web`)**:
  - Direct conversion of `component` UI nodes into lightweight Virtual DOM and WebAssembly canvas renders.
  - Hot Reloading Server (`aeth dev`) integrated into the web pipeline.
- [ ] **High-Throughput Async Backend Framework**:
  - Zero-copy TCP/UDP networking runtime using Linux `io_uring` and epoll multiplexing.
  - Native `route` and `reply ... with status` abstractions with zero external web dependencies.
- [ ] **AI & CUDA Kernel Generator (`--target=ai`)**:
  - Translation of `neural` architectures into specialized C/CUDA kernels for NVIDIA GPU acceleration.
  - Automatic float precision tuning (`f32` default with optional `f16`/`bf16` target lowering).

---

## Phase 3: Ecosystem Expansion & Developer Experience (v1.6 – v2.0)
*Target Timeline: Q4 2027 – Q2 2028*

Phase 3 focuses on developer tooling, IDE integrations, automated package resolution, and enterprise verification tools.

### Key Deliverables
- [ ] **Aetheris Language Server Protocol (LSP)**:
  - Real-time syntax highlighting, type inference hover cards, auto-completion, and diagnostics.
  - Extensions for VS Code, Neovim, and JetBrains IDEs.
- [ ] **Aetheris Package Registry (`aeth PM`)**:
  - Secure, decentralized package registry for community-driven standard library extensions.
  - Cryptographic checksum validation and lockfile-free reproducible builds.
- [ ] **Formally Verified Standard Library (`std`)**:
  - Complete memory-safety verification of standard memory pools, hash maps (`map[K, V]`), and system string handlers.
- [ ] **Embedded Hardware Hardware Abstraction Layer (HAL)**:
  - Ready-to-use standard modules for STM32, ESP32, and Raspberry Pi Pico microcontrollers.

---

## Phase 4: Autonomous Compilation & Self-Hosting (v2.1 – v3.0)
*Target Timeline: 2028+*

The ultimate milestone for Aetheris is achieving complete self-hosting and integrating intelligent compile-time code optimization routines.

### Key Deliverables
- [ ] **Self-Hosting Compiler (`aethc-in-aeth`)**:
  - Re-writing the core compiler (`aethc`) entirely in Aetheris code, eliminating initial C bootstrapping dependencies.
- [ ] **Profile-Guided Optimization (PGO) Engine**:
  - Dynamic execution profiling that feeds runtime telemetry back into secondary compilation builds for ultra-low latency execution.
- [ ] **Formal Proof Integration**:
  - Static safety verifier guaranteeing zero out-of-bounds pointer reads or null-dereference faults at compile-time.