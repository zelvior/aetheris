# Contributing to Aetheris (`.aeth`)

Thank you for your interest in contributing to **Aetheris**! 

Aetheris is an open-source, universal programming language engineered to deliver zero-overhead safety and maximum developer efficiency across 32-bit microcontrollers, 64-bit servers, WebAssembly, and AI tensor accelerators.

This document provides guidelines, architectural overviews, and code quality standards to help you contribute effectively to the core toolchain (`aeth`), compiler (`aethc`), interpreter (`aethi`), standard library (`std`), and language documentation.

---

## 1. Code of Conduct

We are committed to maintaining a welcoming, inclusive, and constructive community. All contributors and maintainers are expected to adhere to our community principles:
* Respect differing viewpoints and constructive feedback.
* Focus on what is best for the project and community ergonomics.
* Show empathy toward other community members.

---

## 2. Ways to Contribute

There are many ways to contribute to the Aetheris ecosystem beyond writing compiler logic:

1. **Bug Reports**: Open an issue detailing unexpected compiler behavior, broken syntax edge cases, or platform crashes.
2. **Feature Requests**: Propose enhancements to language ergonomism, standard library utilities, or target support.
3. **Documentation**: Clarify specification docs (`docs/*.md`), fix typos, or write real-world examples.
4. **Toolchain & Compiler Development**: Implement lexer/parser rules, AST nodes, type-checker rules, or target code generators (C99, JS/WASM, CUDA).
5. **Standard Library**: Add core modules (`std.math`, `std.collections`, `std.fs`, `std.json`) or extend target compatibility.

---

## 3. Local Development Setup

### Prerequisites

To build and test the Aetheris core compiler and runtime toolchain locally, ensure you have:
* A C99/C11 compliant C compiler (`gcc`, `clang`, or `zig cc`).
* `make` or `cmake` (v3.15+).
* `git` for version control.

### Setup Instructions

1. **Fork and Clone the Repository**:
   ```bash
   git clone https://github.com/your-username/aetheris.git
   cd aetheris
   ```

2. **Build the Toolchain from Source**:
   ```bash
   make bootstrap
   ```
   This compiles the bootstrap compiler binary (`aethc`) and CLI interface (`aeth`) into the local `build/bin/` directory.

3. **Verify the Build**:
   ```bash
   ./build/bin/aeth --version
   ```

---

## 4. Testing & Verification Workflows

We require all code contributions—especially core language parser, type inference, or code generation changes—to include automated unit and integration test coverage.

### Running the Test Suite

Execute the entire test runner via `make`:
```bash
make test
```

### Writing Tests for Language Features

Test cases are located under the `tests/` folder and organized by pipeline stage:
* `tests/lexer/`: Lexical analysis unit tests and token stream verification.
* `tests/parser/`: AST creation and syntax terminator (`~`) validation tests.
* `tests/typechecker/`: Bidirectional type inference and architecture width tests (`--target=32` vs `--target=64`).
* `tests/codegen/`: Target code generation output tests (C99, WASM, CUDA).

Every test file should assert correct behavior for valid input and verify actionable error reporting for invalid syntax.

---

## 5. Architectural Conventions & Style Guide

### Core Language Philosophy
When designing features or writing code for Aetheris, keep the core principles in mind:
1. **Explicit Widening & Architecture Awareness**: Never introduce silent bit-width coercion. Respect `uptr` and hardware pointer boundaries.
2. **Tilde Boundary Enforcement**: The statement terminator (`~`) must be strictly validated across all AST nodes.
3. **Deterministic Error Handling**: Prefer returning explicit `Result[T, E]` types over throwing unhandled runtime panics.

### Codebase Formatting
* **C / Native Backend Code**: Follow standard C11 formatting with 4-space indentation, explicit return checking, and strict memory allocation tracking.
* **Aetheris Standard Library (`std`)**: Follow the standard syntax formatting as specified in `docs/syntax_v1.0.md`. Use clear identifier names and mandate `~` terminators.

---

## 6. Submitting a Pull Request (PR)

Before submitting a PR, ensure your branch is up to date with `main` and all verification steps pass:

1. **Create a Feature Branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Format and Commit Your Changes**:
   Use clear, descriptive commit messages:
   ```bash
   git commit -m "feat(parser): add support for neural layer AST nodes"
   ```

3. **Run All Tests**:
   Ensure no regressions were introduced:
   ```bash
   make test
   ```

4. **Open a Pull Request**:
   * Link relevant issue numbers (e.g., `Fixes #42`).
   * Provide a concise summary of the change, the rationale, and any specification docs updated.
   * Attach relevant output snippets or test logs demonstrating correctness.

---

## 7. Community & Discussion

Have questions about the architecture or want to discuss a new feature before implementing it?
* **GitHub Discussions**: Best for feature proposals and architectural feedback.
* **Issue Tracker**: Best for confirmed bugs and specification task tracking.

Thank you for helping build the future of **Aetheris**!