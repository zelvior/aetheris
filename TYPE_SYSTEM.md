# Aetheris Type System & Inference Specification (v1.0)

The **Aetheris Type System** is engineered to provide uncompromising compile-time safety, zero-overhead execution, and absolute predictability across diverse execution domains—from resource-constrained 32-bit microcontrollers to high-throughput 64-bit server nodes and GPU-accelerated AI tensor pipelines.

## 1. Core Principles of the Type System

1. **Statically Checked, Zero Runtime Tax**: All types are validated, inferred, and monomorphized at compile time by `aethc`. There is no runtime type inspection penalty in production builds.

2. **Explicit Architecture Boundaries**: Primitive integer and pointer widths (`i32`, `u64`, `uptr`) are explicitly declared or resolved to match hardware word sizes precisely, preventing platform-dependent silent integer overflows.

3. **Zero Implicit Widening**: Aetheris prohibits silent casting between numeric types of differing bit-widths or signs. Conversions must be explicit via casting operators (e.g., `value as u32`), ensuring complete developer intent and safety.

## 2. Type Inference Engine (`let` vs Explicit Annotations)

Aetheris utilizes a bidirectional type inference algorithm. Local variables declared with `let` have their types deduced instantly from their initializers, while public APIs, function signatures, and struct fields require explicit declarations.

```
// Type inference deduces 'score' as i32
let score = 95 ~

// Explicit type annotation for boundary or public clarity
let threshold: u32 = 1024 ~

```

### Inference Resolution Rules:
* Integer literals default to `i32` unless constrained by context or suffix.
* Floating-point literals default to `f64` (or `f32` when targeted specifically at GPU tensor blocks).
* String literals resolve to heap-backed or stack-view `text` buffers.

## 3. Subtyping, Generics, and Monomorphization

Aetheris supports powerful generic programming via square brackets (`[T]`), allowing functions, structs, and custom collections to operate over any data type without sacrificing performance.

### A. Generic Function Declaration
```
fn identity[T](val: T) -> T ~
    return val ~
end

```

### B. Compile-Time Monomorphization
The Aetheris compiler does not use runtime type erasure (unlike Java or Python). When a generic function or structure is invoked with concrete types (e.g., `identity[i32]` and `identity[text]`), the compiler generates specialized machine code for each type at compile time, ensuring maximum optimization and zero boxing overhead.

## 4. Result and Error Type Checking

Fallible operations in Aetheris are governed by the built-in generic type `Result[T, E]`. The type checker enforces strict usage rules:

1. A function returning a `Result` must be handled by the caller.
2. Unchecked ignoring of a `Result` triggers a compile-time diagnostic error.
3. The `check` operator automatically extracts type `T` on success or short-circuits the enclosing function with error type `E`.

```
fn fetchValue() -> Result[i32, text] ~
    return ok(42) ~
end

fn process() -> Result[u32, text] ~
    let val = check fetchValue() ~
    return ok(val as u32) ~
end

```

## 5. Pointer and Memory Safety Typing

For systems programming and low-level hardware manipulation, Aetheris provides specialized pointer types enforced by strict borrow and lifetime checker rules:

* **`uptr` / `iptr`**: Universal architecture-adaptive pointer types (4 bytes on 32-bit targets; 8 bytes on 64-bit targets).
* **`*mut T` / `*const T`**: Raw pointer types used exclusively in explicit unsafe or systems blocks when interacting with hardware registers or memory-mapped I/O.

```
fn writeRegister(addr: uptr, val: u32) ~
    *(addr as *mut u32) = val ~
end

```

## 6. Target-Specific Type Mapping

The type checker dynamically adapts primitive sizing based on compilation flags (`--target=32` vs `--target=64`):

| Aetheris Type | 32-bit Target (`--target=native-32`) | 64-bit Target (`--target=native-64`) |
| :--- | :--- | :--- |
| `uptr` | `uint32_t` (4 bytes) | `uint64_t` (8 bytes) |
| `iptr` | `int32_t` (4 bytes) | `int64_t` (8 bytes) |
| `tensor[f32]` | ARM NEON / SIMD vector buffer | AVX-512 / CUDA float array |