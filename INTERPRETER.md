# Aetheris Interpreter & Virtual Machine Specification (v1.0)

While the **Aetheris Compiler (`aethc`)** optimizes code down to native C99, WebAssembly, and CUDA binaries, the **Aetheris Interpreter (`aethi`)** provides a zero-setup, instant execution environment for rapid prototyping, REPL-driven development, and lightweight scripting.

---

## 1. Interpreter Execution Models

`aethi` supports two distinct execution modes depending on the workflow:

```
                      ┌──────────────────────────┐
                      │   Aetheris Source (.aeth)│
                      └────────────┬─────────────┘
                                   │
                           [ Aetheris Lexer / AST ]
                                   │
               ┌───────────────────┴───────────────────┐
               ▼                                       ▼
    ┌────────────────────┐                  ┌────────────────────┐
    │ Tree-Walk Engine   │                  │ Bytecode VM (JIT)  │
    │ (REPL / Debugging) │                  │ (High Performance) │
    └────────────────────┘                  └────────────────────┘
```

1. **Tree-Walk Interpreter (`--mode=treewalk`)**: Evaluates the Abstract Syntax Tree (AST) directly. Ideal for interactive REPL sessions, quick debugging, and educational inspection.
2. **Bytecode Virtual Machine (`--mode=vm`)**: Compiles the lowered AST into lean, architecture-agnostic bytecode instructions, executed on a register-based virtual machine with inline caching.

---

## 2. Bytecode Instruction Set Architecture (ISA)

The Aetheris VM uses a compact 32-bit instruction format. Each instruction consists of an 8-bit opcode followed by operand registers or literal pool indices.

### Core Instruction Categories
* **Stack & Register Management**: `LOAD`, `STORE`, `MOV`, `PUSH`, `POP`
* **Arithmetic & Bitwise**: `ADD`, `SUB`, `MUL`, `DIV`, `BIT_AND`, `BIT_OR`, `SHL`, `SHR`
* **Control Flow**: `JMP`, `JMP_IF_FALSE`, `CALL`, `RET`, `MATCH_CHECK`
* **Domain Operations**: `UI_RENDER`, `HTTP_ROUTE`, `TENSOR_INFER`

---

## 3. Architecture Specialization in the VM

Because Aetheris natively supports dual target constraints (32-bit embedded vs. 64-bit server/web), the interpreter adapts its primitive storage dynamically:

* **Pointer-Sized Integers (`iptr` / `uptr`)**: 
  * In 32-bit execution mode (`--target=native-32`), VM word size is locked to 4 bytes.
  * In 64-bit execution mode (`--target=native-64`), VM word size scales to 8 bytes, utilizing hardware-level 64-bit registers.
* **Memory Safety & Result Unwrapping (`Result[T, E]`)**: 
  * The VM handles `check` operations natively by inspecting the tag bit of the result structure, instantly jumping to the error handler or bubbling up the stack without runtime exception overhead.

---

## 4. Interactive REPL & Debugging

The `aethi` toolchain includes a built-in interactive shell supporting live code evaluation, instant error reporting with tilde terminator support, and environment inspection:

```bash
$ aethi
Aetheris REPL v1.0 (64-bit VM Mode)
Type expressions ending with ~ to evaluate.

>>> let score: i32 = 95 ~
>>> print("Score is: " + score) ~
Score is: 95
>>> 
```

### Key Debugging Flags
* `--trace`: Prints every executed bytecode instruction and register state.
* `--profile`: Measures execution time and memory allocation per AST node.
* `--target=32`: Forces the interpreter to simulate 32-bit pointer and integer boundary limits.