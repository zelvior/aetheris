# Aetheris GPU Native Emission Specification (v1.0)

This document defines how `aethc --target=ai` and `--target=gpu` lower `kernel`, `tensor`, and `neural` constructs into device code for NVIDIA, AMD, Apple, and WebGPU hardware.

---

## 1. GPU Backends

| Backend | Flag | Output | Hardware |
| :--- | :--- | :--- | :--- |
| CUDA | `--gpu=cuda` | PTX / `.cubin` / `.cu` | NVIDIA |
| ROCm / HIP | `--gpu=rocm` | HSACO / `.hip` | AMD |
| Metal | `--gpu=metal` | MSL / `.metallib` | Apple Silicon |
| Vulkan Compute | `--gpu=vulkan` | SPIR-V | Cross-vendor |
| WebGPU | `--gpu=webgpu` | WGSL | Browsers |
| CPU fallback | `--gpu=cpu` | SIMD C | Any |

Multiple backends can be built into one binary (`--gpu=cuda,vulkan`); the runtime selects at startup.

---

## 2. Kernels

A `kernel` is a function executed in parallel across a grid of threads.

```aeth
kernel fn saxpy(a: f32, x: *const f32, y: *mut f32, n: u32) ~
    let i = thread.x + block.x * block.dim ~
    if i < n ~
        y[i] = a * x[i] + y[i] ~
    end
end

fn run(x: tensor[f32], y: tensor[f32]) -> Result[unit, text] ~
    let n = x.len() ~
    check launch saxpy(2.0, x.ptr(), y.ptr(), n) grid(n / 256 + 1) block(256) ~
    return ok(()) ~
end
```

### Built-In Thread Variables

| Name | Meaning |
| :--- | :--- |
| `thread.x/y/z` | Thread index within block |
| `block.x/y/z` | Block index within grid |
| `block.dim` | Threads per block |
| `grid.dim` | Blocks per grid |
| `warp.lane` | Lane within warp/wave |

Kernels cannot allocate, recurse, or use `async`. The type checker rejects these with `G201`.

---

## 3. Memory Spaces

| Qualifier | Space | Lifetime |
| :--- | :--- | :--- |
| `global` | Device main memory | Program |
| `shared` | On-chip per-block memory | Block |
| `local` | Per-thread registers/spill | Thread |
| `const` | Cached read-only memory | Program |
| `host` | CPU memory | Program |
| `pinned` | Page-locked host memory for fast transfer | Program |

```aeth
kernel fn blockSum(data: *const f32, out: *mut f32) ~
    shared let tile: [f32; 256] ~
    tile[thread.x] = data[thread.x + block.x * block.dim] ~
    sync block ~
    // tree reduction ...
end
```

Pointer types are tagged with a space; passing a `host` pointer to a kernel is a compile-time error.

---

## 4. Tensor Lowering

`tensor[f32]` is lowered to a device buffer with metadata `{ptr, shape, strides, device}`.

| Aetheris Operation | Lowered To |
| :--- | :--- |
| `a @ b` (matmul) | cuBLAS `sgemm` / rocBLAS / MPS / tiled custom kernel |
| `a + b`, `a * b` | Fused elementwise kernel |
| `conv2d(x, w)` | cuDNN / MIOpen / custom implicit-GEMM |
| `softmax`, `layerNorm` | Fused reduction kernels |
| `x.to(device)` | Async host-device copy |

**Operator Fusion**: Chains of elementwise operations are fused into one kernel to avoid memory round trips. `--report-fusion` lists fused groups.

---

## 5. Neural Layer Emission

`layer Dense(inputs: 512, outputs: 256, activation: "relu")` emits:

1. Weight and bias buffers (`f32`, optionally `f16`/`bf16`).
2. A GEMM call with bias add and activation fused into the epilogue.
3. A backward kernel when `fit` is present.

### Precision Controls

```aeth
neural Classifier ~
    precision(compute: "bf16", accumulate: "f32") ~
    layer Dense(inputs: 512, outputs: 128, activation: "relu") ~
end
```

---

## 6. Streams, Events & Errors

* `stream s = gpu.stream() ~` creates an ordered work queue.
* `launch ... on s` enqueues asynchronously; `await s.sync()` waits.
* All device calls return `Result`. Out-of-memory returns `err("GPU Memory Allocation Failed")`; device-lost errors surface as `err(GpuError.DeviceLost)`.

## 7. Tooling

| Flag | Effect |
| :--- | :--- |
| `--emit-ptx`, `--emit-spirv`, `--emit-msl` | Dump intermediate device code |
| `--gpu-arch=sm_90` | Select architecture generation |
| `--gpu-profile` | Per-kernel timing and occupancy report |
| `--gpu-debug` | Bounds checks and race detection (slow) |
