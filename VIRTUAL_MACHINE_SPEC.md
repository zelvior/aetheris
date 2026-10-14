# Aetheris Virtual Machine & Bytecode Specification (v1.0)

The **Aetheris Virtual Machine (`aethi` VM)** is a high-performance, register-based execution engine engineered to execute lowered Aetheris bytecode instructions. It provides a deterministic, zero-overhead execution environment capable of operating seamlessly across resource-constrained 32-bit microcontrollers and high-throughput 64-bit server nodes.

## 1. Architectural Principles of the Aetheris VM

1. **Register-Based Execution Model**: Unlike stack-based VMs (such as the JVM or CPython), the Aetheris VM operates on a fixed pool of virtual CPU registers (`R0` through `R255`), significantly reducing instruction dispatch count and memory overhead.

2. **Architecture-Adaptive Word Sizing**: The virtual machine dynamically aligns pointer and memory offsets (`uptr`, `iptr`) to match either 32-bit or 64-bit execution profiles based on invocation flags (`--target=32` vs `--target=64`).

3. **Zero-Exception Error Bubbling**: Fallible operations returning `Result[T, E]` are processed via direct tag inspection and status flag jumping without invoking slow runtime stack unwinding mechanisms.

## 2. Memory Layout & Virtual Registers

The VM manages execution state through four primary memory regions:

```
┌─────────────────────────────────────────────────────────────┐
│                    AETHERIS VM STATE                        │
├───────────────────┬─────────────────────────────────────────┤
│ Program Counter   │ 64-bit instruction pointer (PC)         │
├───────────────────┼─────────────────────────────────────────┤
│ Register File     │ 256 General-Purpose Registers (R0-R255) │
├───────────────────┼─────────────────────────────────────────┤
│ Constant Pool     │ Read-only literal strings and numbers   │
├───────────────────┼─────────────────────────────────────────┤
│ Call Stack        │ Frame pointers, local scopes, & returns │
└───────────────────┴─────────────────────────────────────────┘
```

* **`PC` (Program Counter)**: Tracks the index of the currently executing 32-bit bytecode instruction.
* **`FP` / `SP` (Frame & Stack Pointers)**: Manage local variable frames and nested function activations.
* **Architecture Scaling**: When running under `--target=32`, registers store 32-bit scalar values or 4-byte pointer offsets; under `--target=64`, registers scale to 64-bit boundaries.

## 3. Bytecode Instruction Set Architecture (ISA)

Aetheris bytecode instructions are packed into compact 32-bit words consisting of an 8-bit opcode followed by target and source register specifiers or constant pool indices.

### A. Core Opcodes Table

| Opcode Name | Hex / ID | Operand Format | Description |
| :--- | :--- | :--- | :--- |
| `LOAD_CONST` | `0x01` | `Rd, ConstIdx` | Loads a literal value from the constant pool into register `Rd`. |
| `MOV` | `0x02` | `Rd, Rs` | Copies the value from register `Rs` to register `Rd`. |
| `ADD` | `0x10` | `Rd, Ra, Rb` | Computes `Ra + Rb` and stores result in `Rd`. |
| `SUB` | `0x11` | `Rd, Ra, Rb` | Computes `Ra - Rb` and stores result in `Rd`. |
| `MUL` | `0x12` | `Rd, Ra, Rb` | Computes `Ra * Rb` and stores result in `Rd`. |
| `DIV` | `0x13` | `Rd, Ra, Rb` | Computes `Ra / Rb` and stores result in `Rd`. |
| `BIT_AND` | `0x20` | `Rd, Ra, Rb` | Performs bitwise AND (`Ra & Rb`) into `Rd`. |
| `BIT_OR` | `0x21` | `Rd, Ra, Rb` | Performs bitwise OR (`Ra | Rb`) into `Rd`. |
| `SHL` | `0x22` | `Rd, Ra, Rb` | Shifts `Ra` left by `Rb` bits into `Rd`. |
| `JMP` | `0x30` | `Offset` | Unconditional jump of the program counter (`PC`). |
| `JMP_FALSE` | `0x31` | `Rs, Offset` | Jumps to offset if register `Rs` evaluates to `false`. |
| `CALL` | `0x40` | `FuncAddr, ArgCount` | Invokes an Aetheris function frame. |
| `RET` | `0x41` | `Rs` | Returns control to caller, passing value in `Rs`. |
| `CHECK_RES` | `0x50` | `Rd, Rs` | Inspects `Result` tag in `Rs`. Unwraps value or triggers short-circuit. |

## 4. Execution Lifecycle & Bytecode Example

When executing a compiled `.aeth` script or evaluating expressions interactively via `aethi`, the VM decodes instructions sequentially:

```
// Source snippet: let sum = 10 + 32 ~
LOAD_CONST R1, 0     // Load 10 into R1 from constant pool
LOAD_CONST R2, 1     // Load 32 into R2 from constant pool
ADD        R0, R1, R2  // R0 = R1 + R2 (42)
```

## 5. VM Debugging & Introspection Flags

The `aethi` toolchain supports specialized flags for inspecting virtual machine execution at runtime:

* `--trace`: Dumps every executed opcode, register mutation, and stack frame state to standard output.
* `--profile`: Measures execution time, instruction dispatch count, and memory allocation frequency per AST node.
* `--target=32`: Forces the VM engine to simulate 32-bit pointer wrapping and strict 4-byte boundary limitations.