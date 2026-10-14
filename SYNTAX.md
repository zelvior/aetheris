# Language Syntax Specification (v1.0)

This specification defines the syntax, type system, and structural grammar for a universal, multi-target programming language designed for Web, Backend, Systems, and AI workloads. 

The language is architecture-agnostic, providing native support for both **32-bit** and **64-bit** target architectures through explicit primitive types and platform-aware pointer sizes (`uint_ptr`, `int_ptr`).

---

## 1. Lexical Conventions & Line Ending

* **Statement Terminator (`~`)**: Every statement or instruction ends with a tilde (`~`). Newlines are treated as standard whitespace, enabling both compact single-line and clean multi-line expressions.
* **Architecture Agnosticism**: Target architecture (32-bit vs. 64-bit) is selected at compile/transpile time (`--target=32` or `--target=64`). Memory alignment, pointer sizes, and standard integer aliases adapt dynamically without changing code syntax.
* **Identifiers**: Must start with a lowercase letter or underscore, followed by alphanumeric characters or underscores: `[a-z_][a-zA-Z0-9_]*`.
* **Comments**:
  * Single-line: `// comment`
  * Multi-line: `/* comment block */`

---

## 2. Primitive Types & Architecture Support

To seamlessly bridge 32-bit embedded/legacy systems and 64-bit high-performance servers, types are categorized into **Explicit Bit-Widths** and **Pointer-Sized Adapters**.

| Category | Type Keyword | Description | Architecture Behavior | Example |
| :--- | :--- | :--- | :--- | :--- |
| **Integers** | `i8`, `i16`, `i32`, `i64` | Signed fixed-width integers | Exact bit size on all architectures | `let count: i32 = 42 ~` |
| | `u8`, `u16`, `u32`, `u64` | Unsigned fixed-width integers | Exact bit size on all architectures | `let id: u64 = 10001 ~` |
| **Pointer Ints** | `iptr`, `uptr` | Pointer-sized integers | 32-bit on x86/ARM32; 64-bit on x86_64/AArch64 | `let ptrVal: uptr = 0x7FFF ~` |
| **Floats** | `f32`, `f64` | IEEE 754 floating-point numbers | Exact bit size on all target systems | `let pi: f32 = 3.14159 ~` |
| **Booleans** | `bool` | Logical truth value (`true` / `false`) | 8-bit byte aligned | `let active = true ~` |
| **Text** | `text` | UTF-8 encoded string buffer | Dynamic pointer allocation (`uptr`) | `let label = "Nova" ~` |
| **Collections**| `list[T]`, `map[K, V]` | Dynamic list and key-value dictionary | Dynamically sized based on target `uptr` | `let items = [1, 2, 3] ~` |

---

## 3. Core Keywords Master List

The v1.0 vocabulary is organized by functional domain:

* **Variable & Memory**: `let`, `const`, `mut`, `addr`, `deref`
* **Control Flow**: `if`, `else`, `match`, `case`, `default`, `loop`, `while`, `for`, `in`, `break`, `continue`
* **Functions & Scope**: `fn`, `return`, `end`, `async`, `await`
* **Types & Structs**: `type`, `struct`, `enum`
* **UI & Frontend**: `component`, `render`, `bind`, `on`, `emit`, `style`
* **Backend & API**: `route`, `request`, `reply`, `status`, `db`
* **AI & Neural Ops**: `tensor`, `neural`, `layer`, `fit`, `infer`, `weights`
* **Modules**: `module`, `import`, `export`, `from`

---

## 4. Operators & Symbols Reference

* **Assignment**: `=`
* **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `**` (exponentiation)
* **Bitwise Operations** *(Essential for 32-bit low-level ops)*: `&` (AND), `|` (OR), `^` (XOR), `~` (NOT), `<<` (left shift), `>>` (right shift)
* **Comparison**: `==`, `!=`, `<`, `>`, `<=`, `>=`
* **Logical**: `and`, `or`, `not`
* **Pointers & References**: `&` (address-of), `*` (dereference)
* **Event & Null Check**: `?` (optional / trigger)
* **Delimiters**: `()` (expressions), `[]` (indexing/generics), `{}` (objects/struct literals), `,` (separators), `.` (member access)

---

## 5. Formal Grammar Constructs & Code Examples

### A. Memory Management & Multi-Architecture Types

```
// Explicit bit-width for cross-platform safety
const maxCount: u32 = 1024 ~
let pointerAddress: uptr = 0x80000000 ~

// Architecture-agnostic function returning memory offset
fn getOffset(base: uptr, index: u32) -> uptr ~
    return base + (index * 4) ~
end
```

### B. Control Flow & Match Statements

```
fn evaluateStatus(code: i32) ~
    match code ~
        case 200 => print("Success") ~
        case 404 => print("Not Found") ~
        default  => print("Unknown Error") ~
    end
end
```

### C. Frontend UI Declaration

```
component Counter ~
    mut state count: i32 = 0 ~

    render ~
        show Button(text: "Increment") with style(bgColor: "#007ACC") ~
        on click ? trigger count = count + 1 ~
        show Text("Current Count: " + count) ~
    end
end
```

### D. Backend API Service

```
route POST "/api/v1/user/create" ~
    let payload = request.body ~
    if payload.age < 18 ~
        reply "User must be an adult" with status 400 ~
    else ~
        let newUser = db.users.insert(payload) ~
        reply newUser with status 201 ~
    end
end
```

### E. AI Tensor Math & Neural Architecture

```
neural SentimentModel ~
    // 32-bit floats for GPU acceleration and memory efficiency
    layer Dense(inputs: 512, outputs: 128, activation: "relu") ~
    layer Dropout(rate: 0.15) ~
    layer Dense(inputs: 128, outputs: 2, activation: "softmax") ~

    fit(dataset: "data/reviews.csv", epochs: 20, batchSize: 64) ~
end

fn runInference(inputData: tensor[f32]) -> f32 ~
    let prediction = infer SentimentModel with inputData ~
    return prediction ~
end
```

---

## 6. Architecture Compilation Strategy (32-bit vs. 64-bit)

1. **Targeting 32-Bit (ARM32 / x86 / Embedded):**
   * Pointers (`iptr`, `uptr`) resolution: 32 bits (4 bytes).
   * Registers aligned to 4-byte boundaries.
   * `tensor[f32]` operations vectorized using SIMD/NEON instructions where available.

2. **Targeting 64-Bit (x86_64 / AArch64 / WebAssembly):**
   * Pointers (`iptr`, `uptr`) resolution: 64 bits (8 bytes).
   * Memory alignment defaulted to 8-byte boundaries.
   * Large integer operations (`i64`, `u64`) executed natively in single register cycles.