# Aetheris Custom Memory Allocator Specification (v1.0)

Aetheris ships with a pluggable allocator interface. The default allocators are deterministic and GC-free; developers can replace or compose them for embedded, server, and GPU-host workloads.

---

## 1. Allocator Interface

Every allocator implements the `Allocator` interface:

```aeth
interface Allocator ~
    fn alloc(size: uptr, align: u32) -> Result[uptr, AllocError] ~
    fn realloc(ptr: uptr, oldSize: uptr, newSize: uptr, align: u32) -> Result[uptr, AllocError] ~
    fn free(ptr: uptr, size: uptr, align: u32) ~
end

enum AllocError ~
    OutOfMemory ~
    InvalidAlignment ~
    Oversize ~
end
```

`free` receives the original size and alignment, so allocators need no per-block headers.

---

## 2. Built-In Allocators

| Allocator | Best For | Behavior |
| :--- | :--- | :--- |
| `BumpArena` | Request scopes, parsing, frames | O(1) alloc; frees everything at once via `reset()` |
| `FixedPool[T]` | Embedded objects of equal size | O(1) alloc/free from a static slab |
| `StackAlloc` | Temporary scratch data | LIFO allocation and release |
| `FreeList` | General purpose on native-64 | Size-class bins with coalescing |
| `RcAlloc` | Shared `list`, `map`, `text` | Wraps another allocator with reference counts |
| `PageAlloc` | Large buffers, GPU-pinned memory | Direct `mmap`/`VirtualAlloc` pages |

---

## 3. Using Arenas

```aeth
fn handleRequest(req: Request) -> Result[Response, text] ~
    mut arena = BumpArena.new(64 * 1024) ~
    with allocator arena ~
        let parts = req.body.split("&") ~     // allocated in arena
        let reply = buildReply(parts) ~
        return ok(reply.copyOut()) ~          // explicit copy to caller's allocator
    end
end
```

When the `with allocator` block ends, the arena is reset in a single pointer write. Values allocated in the arena cannot escape the block without an explicit copy; the lifetime checker enforces this.

---

## 4. Target Allocation Strategy

| Target | Default Allocator | Notes |
| :--- | :--- | :--- |
| `native-32` | `FixedPool` + static `BumpArena` | No general heap unless `--heap=on`; all sizes known at link time |
| `native-64` | `FreeList` + per-thread `BumpArena` | Thread-local caches avoid lock contention |
| `web` | WASM linear memory (`memory.grow`) | Growth policy configurable |
| `ai` | `PageAlloc` + device pools | Host and device pools tracked separately |

### Alignment Rules

* `native-32`: minimum 4-byte alignment; `f64` and SIMD types require 8/16 bytes.
* `native-64`: minimum 8-byte alignment; SIMD vectors align to 16/32/64 bytes.
* Cache-line padding can be requested with `@align(64)` on a `struct`.

---

## 5. Writing a Custom Allocator

```aeth
struct CountingAlloc ~
    inner: *mut FreeList ~
    mut liveBytes: uptr ~
end

impl Allocator for CountingAlloc ~
    fn alloc(size: uptr, align: u32) -> Result[uptr, AllocError] ~
        let p = check self.inner.alloc(size, align) ~
        self.liveBytes = self.liveBytes + size ~
        return ok(p) ~
    end

    fn free(ptr: uptr, size: uptr, align: u32) ~
        self.inner.free(ptr, size, align) ~
        self.liveBytes = self.liveBytes - size ~
    end
end
```

Install globally with `set_global_allocator(myAlloc) ~` in `main`, or scope it with `with allocator`.

---

## 6. Diagnostics

* `--alloc-trace`: logs every allocation with source location.
* `--alloc-stats`: prints peak usage, fragmentation, and allocation counts at shutdown.
* `--poison`: fills freed memory with `0xDD` to expose use-after-free in debug builds.
* Leak report (debug builds): unreleased `RcAlloc` blocks are listed at exit.
