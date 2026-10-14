# Aetheris Compiler Targets & Low-Level Assembly Specification (v1.0)

The **Aetheris Compiler Target and Code Generation Subsystem** (`aethc`) is responsible for translating the type-checked Abstract Syntax Tree (AST) into highly optimized, deterministic target outputs. Designed to operate without heavy runtimes or garbage collection, Aetheris supports multi-target lowering spanning resource-constrained 32-bit microcontrollers, high-throughput 64-bit servers, WebAssembly runtimes, and GPU tensor accelerators.

## 1. Core Target Architecture Overview

```
                      ┌────────────────────────────────────────┐
                      │ Type-Checked Abstract Syntax Tree (AST)│
                      └───────────────────┬────────────────────┘
                                          │
                         [ Intermediate Representation (IR) ]
                                          │
         ┌───────────────────┬────────────┴────────────┬───────────────────┐
         ▼                   ▼                         ▼                   ▼
┌──────────────────┐┌──────────────────┐      ┌──────────────────┐┌──────────────────┐
│ C99 Backend      ││ WASM / JS Target │      │ CUDA / SIMD Target││ Native Asm Emitter│
│ (Systems/32/64)  ││ (Web & DOM UI)   │      │ (AI Tensors)     ││ (x86_64/AArch64) │
└──────────────────┘└──────────────────┘      └──────────────────┘└──────────────────┘
```

The Aetheris compilation pipeline lowers AST nodes through an architecture-neutral Intermediate Representation (IR) before invoking specialized backend emitters.

---

## 2. The Aetheris Intermediate Representation (IR)

Before targeting specific machine code or high-level source languages, `aethc` flattens expressions into a strongly-typed SSA (Static Single Assignment) form IR.

### A. Core IR Instructions

| IR Opcode | Operand Format | Description |
| :--- | :--- | :--- |
| `ALLOC` | `dest, type, size` | Allocates stack or heap memory block. |
| `LOAD` | `dest, ptr` | Dereferences pointer address into register. |
| `STORE` | `ptr, src` | Writes register value into memory address. |
| `BINOP` | `dest, op, lhs, rhs` | Performs arithmetic or bitwise operation. |
| `BRANCH` | `cond, label_true, label_false` | Conditional jump based on boolean evaluation. |
| `CALL` | `dest, func, args...` | Invokes function frame with arguments. |
| `CHECK_TAG` | `dest, result_ptr` | Inspects `Result[T, E]` status flag. |

### B. Sample IR Representation for `let sum = 10 + 32 ~`

```ir
%r1 = CONST_INT 10, i32
%r2 = CONST_INT 32, i32
%r0 = BINOP ADD, %r1, %r2
STORE stack_offset_0, %r0
```

---

## 3. Backend Emitters & Target Specifications

### A. The C99/C11 Emitter (`--target=native-32` / `--target=native-64`)

The primary systems backend emits clean, standards-compliant C99 code designed to be compiled via `gcc`, `clang`, or `zig cc`.

* **Pointer Mapping**:
  * `--target=native-32`: `uptr` maps to `uint32_t`, `iptr` maps to `int32_t`. Structures are padded to strict 4-byte boundaries.
  * `--target=native-64`: `uptr` maps to `uint64_t`, `iptr` maps to `int64_t`. Structures align to 8-byte cache line boundaries.
* **Error Handling Lowering**: `check` statements are transformed into fast inline conditional return checks:
  ```c
  Result_i32 res = parsePort("8080");
  if (res.is_err) { return res; }
  uint16_t port = res.value;
  ```

### B. The WebAssembly & JS Emitter (`--target=web`)

When compiling frontend components (`component`) and reactive scripts, `aethc` emits lightweight WebAssembly binary modules or ES6 JavaScript bindings.

* **DOM Interfacing**: `show Button("Click")` lowers directly into zero-reflection `document.createElement('button')` calls or cached WASM virtual node templates.
* **State Signals**: `mut state count = i32 = 0` creates a reactive signal cell bound directly to text node updates without virtual DOM diffing trees.

### C. The CUDA & SIMD Emitter (`--target=ai`)

For deep learning and tensor operations (`neural`, `tensor`), the backend generates optimized CUDA kernels (`.cu`) or SIMD-vectorized C loops (AVX-512 / ARM NEON).

* **Tensor Lowering**: `tensor[f32]` maps to contiguous GPU device memory buffers (`float*`) with automated stride calculation.
* **Layer Compilation**: `layer Dense(inputs: 512, outputs: 256)` compiles into optimized matrix multiplication kernels (`cublasSgemm` or custom fused GPU instructions).

---

## 4. Direct Assembly Emitter (`--emit-asm`)

For advanced systems debugging, low-level kernel development, and bare-metal compilation, Aetheris supports direct native assembly emission (`.s`) for target architectures via `--emit-asm`.

### Example: Generated x86_64 Assembly for an Addition Function

Source Aetheris:
```aeth
fn add(a: i32, b: i32) -> i32 ~
    return a + b ~
end
```

Emitted x86_64 Assembly (`--emit-asm --target=native-64`):
```assembly
.global aetheris_add
.type aetheris_add, @function
aetheris_add:
    .cfi_startproc
    pushq   %rbp
    movq    %rsp, %rbp
    movl    %edi, -4(%rbp)      ; Store parameter 'a'
    movl    %esi, -8(%rbp)      ; Store parameter 'b'
    movl    -4(%rbp), %eax      ; Move 'a' into accumulator
    addl    -8(%rbp), %eax      ; Add 'b' to accumulator
    popq    %rbp
    ret
    .cfi_endproc
```

### Example: Generated ARM32 (Thumb-2) Assembly for Microcontrollers

Emitted ARM32 Assembly (`--emit-asm --target=native-32`):
```assembly
.syntax unified
.global aetheris_toggle_pin
.type aetheris_toggle_pin, %function
aetheris_toggle_pin:
    push    {r4, lr}
    ldr     r0, =0x40020000     ; Load GPIO base address
    ldr     r1, [r0, #0x14]     ; Load ODR register offset
    orr     r1, r1, #1          ; Set pin bit high
    str     r1, [r0, #0x14]     ; Store back to register
    pop     {r4, pc}
```

---

## 5. Optimization Passes

Before final emission, the Aetheris compiler applies three mandatory optimization tiers:

1. **Dead Code Elimination (DCE)**: Removes unreachable branches and unused local variables.
2. **Constant Folding**: Evaluates constant mathematical expressions at compile time (e.g., `60 * 60 * 24` $\rightarrow$ `86400`).
3. **Tail-Call Optimization (TCO)**: Converts recursive function tails into iterative jumps to prevent stack overflow on resource-constrained 32-bit embedded targets.