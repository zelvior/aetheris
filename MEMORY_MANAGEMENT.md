# Aetheris Memory Management Specification (v1.0)

The **Aetheris Memory Subsystem** is designed for extreme predictability, zero runtime garbage collection overhead, and seamless dual-architecture scaling (supporting 32-bit microcontrollers and 64-bit high-throughput servers alike).

## 1. Core Memory Design Principles

1. **Deterministic Lifecycle**: Memory is managed through strict lexical scoping and stack allocation by default. There is no background Garbage Collector (GC) pausing execution threads.

2. **Architecture-Native Pointers (`uptr` / `iptr`)**: Pointer sizes dynamically adapt to the compilation target (4 bytes on 32-bit targets; 8 bytes on 64-bit targets), ensuring zero-overhead interaction with raw hardware buffers.

3. **Explicit Control when Needed**: While high-level logic remains safe and automatically managed, systems programmers can drop down to raw pointer arithmetic (`&`, `*`, `addr`, `deref`) within explicit scopes.

---

## 2. Allocation Tiers & Strategies

| Target Environment | Allocation Strategy | Heap Behavior | Safety Enforcement |
| :--- | :--- | :--- | :--- |
| **Native 32-Bit** | Static arena or fixed-buffer pre-allocation | Avoided; dynamic heap restricted to prevent fragmentation | Compile-time bounds checking |
| **Native 64-Bit** | Regional memory arenas & smart reference counters | High-speed multithreaded bump allocators | Lifetime scope verification |
| **Web / WASM** | JavaScript TypedArray mapping (`ArrayBuffer`) | Managed by browser WASM memory boundary | Bounds-checked memory views |

---

## 3. Syntax & Pointer Operators

Aetheris provides direct memory manipulation operators for systems and performance-critical code blocks:

* **`&` (Address-of Operator)**: Retrieves the memory address of a variable.
* **`*` (Dereference Operator)**: Accesses or modifies the value stored at a pointer address.
* **`uptr`**: Universal pointer-sized integer type that adapts to the host architecture.

```text
// Example: Low-level memory mapping and pointer arithmetic
fn modifyBuffer(baseAddr: uptr, offset: u32, value: u32) ~
    let targetPtr: uptr = baseAddr + (offset * 4) ~
    
    // Dereference pointer to store value directly into memory
    *(targetPtr as *mut u32) = value ~
end
```

---

## 4. Stack vs. Heap Allocation Rules

* **Stack Allocation**: All primitives (`i32`, `f32`, `bool`) and fixed structs are allocated directly on the call stack. When a function scope exits (`end`), stack frames are instantly reclaimed with zero CPU cycles wasted.
* **Heap Allocation (`list`, `map`, dynamic `text`)**: Managed via lean reference counters or arena allocators embedded in the standard library core, releasing memory immediately when references drop to zero.