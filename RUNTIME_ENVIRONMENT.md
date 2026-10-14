# Aetheris Runtime Environment Specification (v1.0)

The **Aetheris Runtime (`aethrt`)** is the minimal support layer linked into every compiled `.aeth` program. It contains no garbage collector, no background threads by default, and no reflection tables. Everything not used is tree-shaken away by `aethc`.

---

## 1. Runtime Design Goals

1. **Tiny Footprint**: Bare-metal `native-32` builds can ship with a runtime under 2 KB (startup stub + panic handler).
2. **Deterministic Startup**: Initialization order is fixed and documented; no hidden global constructors.
3. **Profile-Based Composition**: The runtime is assembled from profiles selected by `--target`.

| Profile | Targets | Contents |
| :--- | :--- | :--- |
| `rt-core` | All | Startup, panic handler, `Result` helpers, bounds-check stubs |
| `rt-alloc` | native-32/64 | Arena allocator, refcount helpers (see `CUSTOM_MEMORY_ALLOCATOR.md`) |
| `rt-async` | native-64, web | Event loop, task scheduler, futures |
| `rt-os` | native-64 | System call shims, signal hooks, thread pool |
| `rt-web` | web | JS glue, WASM memory bridge |
| `rt-gpu` | ai | Device discovery, kernel launch, buffer management |

---

## 2. Startup Sequence

```
 [ OS loader / reset vector ]
            │
            ▼
   1. _aeth_start        (arch-specific entry, sets stack pointer)
            │
   2. rt_init_memory     (arenas, static buffers)
            │
   3. rt_init_platform   (stdio streams, clock, signal handlers)
            │
   4. module init        (top-level `const` and `let` in import order)
            │
   5. main()             (user entrypoint)
            │
   6. rt_shutdown        (flush streams, release arenas, exit code)
```

The entrypoint may be declared in either form:

```aeth
fn main() ~
    print("Hello, Aetheris") ~
end

fn main() -> Result[unit, text] ~
    check startServices() ~
    return ok(()) ~
end
```

If `main` returns `err(msg)`, the runtime prints the message to `stderr` (when available) and exits with code `1`.

---

## 3. Panics

Aetheris prefers `Result[T, E]`, but some faults are unrecoverable: out-of-bounds access in checked builds, arena exhaustion with no fallback, and explicit `fail` outside a fallible function.

| Target | Panic Behavior |
| :--- | :--- |
| `native-64` | Print message and source location to `stderr`, exit with code `101` |
| `native-32` | Invoke the user-installable hook `rt_panic_hook(code: u32)`; default is halt loop |
| `web` | Throw a `AethPanic` JS error and log to console |
| `ai` | Abort the kernel stream and surface a `Result` error to the host |

Panics never unwind the stack. Destructors are not run; arenas are reclaimed by the OS or hardware reset.

---

## 4. Threads and Concurrency

* **native-32**: Single-threaded by default. Interrupt handlers are declared with `interrupt fn` and may not allocate.
* **native-64**: A fixed-size worker pool (default: logical CPU count) executes `async` tasks. Tasks are cooperatively scheduled at `await` points.
* **web**: Single event loop; optional `SharedArrayBuffer` workers when the host permits.

---

## 5. Runtime Configuration

Configuration is baked at compile time via `aeth.toml` and may be overridden by environment variables on hosted targets.

```toml
[runtime]
stack_size = "512KB"
heap_arena = "64MB"
worker_threads = 8
panic = "abort"
```

| Environment Variable | Effect |
| :--- | :--- |
| `AETH_THREADS` | Override worker pool size |
| `AETH_HEAP` | Override initial arena size |
| `AETH_LOG` | Set default log level (see `LOGGING_AND_TELEMETRY.md`) |
