# Aetheris Math Intrinsics & SIMD Specification (v1.0)

This document defines compiler-known math intrinsics and the portable SIMD vector types of `std.simd`.

---

## 1. Intrinsic Functions

Intrinsics are recognized by `aethc` and lowered to single instructions when hardware supports them, or to software routines otherwise.

| Intrinsic | Description | x86_64 | AArch64 |
| :--- | :--- | :--- | :--- |
| `sqrt(x)` | Square root | `sqrtss`/`sqrtsd` | `fsqrt` |
| `fma(a, b, c)` | Fused multiply-add | `vfmadd` | `fmadd` |
| `abs(x)`, `min`, `max` | Absolute value, min/max | `andps`, `minps` | `fabs`, `fmin` |
| `floor`, `ceil`, `round` | Rounding | `roundss` | `frintm`/`frintp`/`frinta` |
| `popcount(x)` | Count set bits | `popcnt` | `cnt` |
| `clz(x)`, `ctz(x)` | Leading/trailing zeros | `lzcnt`, `tzcnt` | `clz`, `rbit+clz` |
| `rotl`, `rotr` | Bit rotation | `rol`, `ror` | `ror` |
| `bswap(x)` | Byte swap | `bswap` | `rev` |

```aeth
import { fma, popcount } from "std.math" ~

fn dot3(a: f32, b: f32, c: f32, d: f32) -> f32 ~
    return fma(a, b, c * d) ~
end
```

On `native-32` targets without an FPU, `f32` and `f64` operations lower to soft-float routines. `f64` intrinsics emit a warning (`W114`) unless `--allow-soft-f64` is given.

---

## 2. Integer Arithmetic Variants

Because Aetheris forbids silent overflow, explicit variants are provided:

| Form | Behavior |
| :--- | :--- |
| `a + b` | Debug: panic on overflow. Release: wraps only if `--overflow=wrap` |
| `addWrap(a, b)` | Two's-complement wrapping |
| `addSat(a, b)` | Saturates at type bounds |
| `addChecked(a, b)` | Returns `Result[T, text]` |

The same suffixes exist for `sub`, `mul`, and `shl`.

---

## 3. SIMD Vector Types

```aeth
vec4[f32]    // 128-bit: 4 lanes of f32
vec8[f32]    // 256-bit: 8 lanes
vec16[f32]   // 512-bit: 16 lanes
vec16[u8]    // 128-bit: 16 lanes of bytes
mask8        // lane predicate for 8 lanes
```

Lane counts are fixed at the type level. The compiler lowers each type to the widest available instruction set and **splits or emulates** if the target is narrower.

| Instruction Set | Targets | Max Width |
| :--- | :--- | :--- |
| SSE2 / SSE4.2 | x86, x86_64 | 128-bit |
| AVX2 | x86_64 | 256-bit |
| AVX-512 | x86_64 | 512-bit |
| NEON | ARM32 (v7), AArch64 | 128-bit |
| SVE | AArch64 | 128–2048-bit |
| RVV | RISC-V | Scalable |
| WASM SIMD | web | 128-bit |

---

## 4. SIMD Operations

```aeth
fn addArrays(a: *const f32, b: *const f32, out: *mut f32, n: u32) ~
    mut i: u32 = 0 ~
    while i + 8 <= n ~
        let va = vec8[f32].load(a + i) ~
        let vb = vec8[f32].load(b + i) ~
        (va + vb).store(out + i) ~
        i = i + 8 ~
    end
    while i < n ~                 // scalar tail
        out[i] = a[i] + b[i] ~
        i = i + 1 ~
    end
end
```

| Operation | Description |
| :--- | :--- |
| `load`, `store`, `loadAligned` | Memory transfer |
| `+ - * /`, `fma` | Lane-wise arithmetic |
| `select(mask, a, b)` | Per-lane blend |
| `shuffle(v, [..])` | Lane permutation with compile-time indices |
| `reduceAdd`, `reduceMax` | Horizontal reductions |
| `gather`, `scatter` | Indexed memory access |
| `cmpLt`, `cmpEq` | Produce masks |

---

## 5. Auto-Vectorization

Loops over contiguous data are vectorized automatically when the compiler can prove no aliasing. Hints:

* `@vectorize` forces vectorization or errors with the reason.
* `@novector` disables it.
* `@noalias` on pointer parameters asserts non-overlap.

Run with `--report-vec` to see why a loop was or was not vectorized.

---

## 6. Runtime Feature Detection

```aeth
if cpu.has("avx2") ~
    sumAvx2(data) ~
else ~
    sumScalar(data) ~
end
```

`@multiversion` generates function clones per instruction set and dispatches at load time with negligible overhead.

## 7. Floating-Point Determinism

* Default: IEEE 754 compliant, no fast-math.
* `--fast-math` allows reassociation and ignores NaN/Inf rules; it must be opted into per module via `@fastmath`.
* Results of `fma` are identical across all targets because software fallbacks implement exact fusing.
