# Aetheris Data Types & Type System Specification (v1.0)

The **Aetheris Type System** is engineered to provide absolute safety, predictable memory footprints, and seamless scaling across both resource-constrained 32-bit microcontrollers and high-throughput 64-bit server nodes.

## 1. Type System Philosophy

1. **Explicit Architecture Boundaries**: Types maintain strict bit-width guarantees (`i32`, `u64`), while pointer types (`uptr`, `iptr`) dynamically adapt to match the host hardware word size.
2. **Zero Implicit Widening**: Aetheris prevents silent overflow bugs by enforcing explicit casts when moving between different integer or float bit-widths.
3. **Zero-Overhead Inference**: Local variables support automatic type inference at compile time via `let`, while public function signatures require explicit typing for absolute clarity.

---

## 2. Primitive Types Master Table

| Type Keyword | Category | Bit-Width / Size | Description & Usage | Example |
| :--- | :--- | :--- | :--- | :--- |
| `i8` | Signed Integer | 8-bit (1 byte) | Small signed integers, byte streams | `let flag: i8 = -12 ~` |
| `i16` | Signed Integer | 16-bit (2 bytes) | Medium range signed values, audio samples | `let sample: i16 = -32768 ~` |
| `i32` | Signed Integer | 32-bit (4 bytes) | Standard signed integer for general math | `let count: i32 = 100000 ~` |
| `i64` | Signed Integer | 64-bit (8 bytes) | Large scale counters, high-precision timestamps | `let epoch: i64 = 1711900000 ~` |
| `u8` | Unsigned Integer | 8-bit (1 byte) | Raw binary data, ASCII character bytes | `let byteVal: u8 = 255 ~` |
| `u16` | Unsigned Integer | 16-bit (2 bytes) | Port numbers, network identifiers | `let port: u16 = 8080 ~` |
| `u32` | Unsigned Integer | 32-bit (4 bytes) | Positive counts, memory offsets | `let offset: u32 = 4096 ~` |
| `u64` | Unsigned Integer | 64-bit (8 bytes) | Huge indexing, 64-bit identifiers | `let uid: u64 = 9876543210 ~` |
| `iptr` | Pointer Integer | 4 bytes (32-bit) / 8 bytes (64-bit) | Signed pointer offset arithmetic | `let diff: iptr = ptr2 - ptr1 ~` |
| `uptr` | Pointer Integer | 4 bytes (32-bit) / 8 bytes (64-bit) | Raw memory addresses and pointers | `let base: uptr = 0x7FFF0000 ~` |
| `f32` | Floating Point | 32-bit (4 bytes) | Single-precision float (ideal for GPU/SIMD/AI) | `let weight: f32 = 0.5432 ~` |
| `f64` | Floating Point | 64-bit (8 bytes) | Double-precision float for high-precision math | `let pi: f64 = 3.141592653589793 ~` |
| `bool` | Boolean | 8-bit (1 byte aligned) | Logical truth values (`true` or `false`) | `let isValid = true ~` |
| `text` | String Buffer | Dynamic (`uptr` header + UTF-8 bytes) | UTF-8 encoded immutable or mutable text strings | `let title = "Aetheris Engine" ~` |

---

## 3. Complex and Structured Types

### A. Dynamic Lists (`list[T]`)
Ordered sequences of items backed by contiguous memory buffers. Capacities scale automatically through arena or regional allocators.

```
let scores: list[i32] = [95, 88, 76, 92] ~
scores.push(100) ~
```

### B. Key-Value Dictionaries (`map[K, V]`)
Hash-indexed maps for lightning-fast lookup operations. Keys must be comparable primitives or text.

```
let userConfig: map[text, text] = {
    theme: "dark",
    region: "us-west"
} ~
```

### C. Custom Structs (`struct`)
User-defined composite layouts combining fixed-width primitives or pointers with zero runtime padding overhead.

```
struct Point3D ~
    x: f32 ~
    y: f32 ~
    z: f32 ~
end
```

### D. Result Enums (`Result[T, E]`)
The core built-in type for deterministic error handling, holding either a success value `ok(T)` or failure payload `err(E)`.

```
fn lookupUser(id: u64) -> Result[text, text] ~
    if id == 0 ~ return err("Invalid ID") ~
    return ok("AdminUser") ~
end
```

---

## 4. Architecture Memory Layout & Alignment

Aetheris aligns data types strictly based on compilation targets to guarantee hardware compatibility:

* **32-Bit Target (`--target=native-32`)**:
  * Pointers (`uptr`, `iptr`) occupy 4 bytes.
  * Struct members align to 4-byte boundaries.
  * `f64` operations are software-emulated or double-pumped if hardware FPU is absent.

* **64-Bit Target (`--target=native-64`)**:
  * Pointers (`uptr`, `iptr`) occupy 8 bytes.
  * Struct members align to 8-byte boundaries for optimal CPU cache line fetches.
  * Native 64-bit register arithmetic utilized for all `i64`, `u64`, and `uptr` operations.