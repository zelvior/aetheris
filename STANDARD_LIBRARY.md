# Aetheris Standard Library Specification (v1.0)

The **Aetheris Standard Library** (`std`) provides a lean, zero-overhead, highly optimized collection of modules designed to support systems programming, high-throughput web servers, and AI/tensor computation without relying on heavy external runtime dependencies.

## 1. Design Principles

1. **Zero-Overhead & Tree-Shakable**: Unused standard library functions are stripped away during compilation (`aethc`), ensuring embedded 32-bit targets maintain tiny binary footprints.
2. **Architecture-Aware**: Core collections (`list`, `map`) and memory utilities dynamically adapt to 32-bit or 64-bit word boundaries (`uptr`).
3. **Deterministic Error Safety**: Functions that can fail return the standard `Result[T, E]` type, forcing clean, explicit error handling via `check` or `match`.

---

## 2. Core Modules Overview

| Module | Description | Primary Targets |
| :--- | :--- | :--- |
| **`std.io`** | Standard streams, terminal input/output, and console formatting | Native (32/64), Web, Interpreter |
| **`std.fs`** | File system reading, writing, path management, and directory traversal | Native (32/64), WASM |
| **`std.net`** | TCP/UDP sockets, HTTP helper abstractions, and network streams | Backend Servers, Web sockets |
| **`std.math`** | Trigonometric, exponential, floating-point utilities, and SIMD wrappers | All targets (Accelerated via GPU/SIMD where available) |
| **`std.json`** | Zero-copy JSON serialization, parsing, and dynamic type mapping | Web, Backend APIs |
| **`std.tensor`** | High-performance multi-dimensional array math and GPU tensor manipulation | AI / CUDA / WebGPU |

---

## 3. Detailed Module Specifications

### A. `std.math` (Mathematical Utilities)

The math module provides fundamental numeric operations optimized for hardware vector units (SSE, AVX, NEON).

```
// Absolute value calculation across arch types
fn abs[T](val: T) -> T ~
    if val < 0 ~ return -val ~
    return val ~
end

// Square root with hardware-level fallback
fn sqrt(value: f32) -> Result[f32, text] ~
    if value < 0.0 ~
        return err("Cannot calculate square root of negative number") ~
    end
    return ok(nativeSqrt(value)) ~
end
```

### B. `std.json` (Serialization & Parsing)

Provides lightweight parsing for API payloads and configuration files.

```
struct UserConfig ~
    username: text ~
    timeout: u32 ~
end

fn parseConfig(rawJson: text) -> Result[UserConfig, text] ~
    let config = check json.decode[UserConfig](rawJson) or err("Invalid JSON format") ~
    return ok(config) ~
end
```

### C. `std.collections` (`list` & `map`)

Dynamic collections built on top of raw memory buffer pointers (`uptr`) with automatic capacity scaling.

```
// Generic list mapping operation
fn mapList[T, U](inputList: list[T], transform: fn(T) -> U) -> list[U] ~
    mut result: list[U] = [] ~
    for item in inputList ~
        result.push(transform(item)) ~
    end
    return result ~
end
```

---

## 4. Platform Specialization in `std`

The standard library dynamically resolves underlying system calls based on target compilation flags:

* **When targeting `--target=native-32`**: 
  `std.fs` maps directly to direct fixed-buffer system calls without dynamic heap allocations. `std.math` avoids 64-bit double precision routines where single-precision `f32` is sufficient for microcontrollers.
* **When targeting `--target=native-64`**: 
  `std.net` utilizes epoll/io_uring event loops for non-blocking asynchronous socket multiplexing.
* **When targeting `--target=web`**: 
  `std.io` and `std.net` map cleanly to the browser's Fetch API, WebSockets, and standard Console streams.