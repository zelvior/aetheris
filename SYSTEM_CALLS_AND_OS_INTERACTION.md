# Aetheris System Calls & OS Interaction Specification (v1.0)

Aetheris programs reach the operating system through a thin, typed abstraction (`std.os`) that lowers directly to system calls on each platform. No libc is required on native targets.

---

## 1. Principles

1. **Typed Syscalls**: Every call returns `Result[T, OsError]`; raw `errno` values are never exposed as plain integers.
2. **Zero-libc Option**: Linux builds can be fully static with direct `syscall` instructions.
3. **Portable by Default, Raw by Choice**: High-level modules (`std.fs`, `std.net`, `std.process`) work everywhere; `std.os.raw` exposes platform-specific calls.

---

## 2. The `OsError` Type

```aeth
enum OsError ~
    NotFound ~
    PermissionDenied ~
    AlreadyExists ~
    Interrupted ~
    WouldBlock ~
    InvalidInput ~
    OutOfMemory ~
    Other(code: i32) ~
end
```

---

## 3. Raw Syscall Interface

```aeth
import { syscall3 } from "std.os.raw" ~

fn rawWrite(fd: i32, buf: uptr, len: uptr) -> Result[uptr, OsError] ~
    let n = check syscall3(SYS_WRITE, fd as uptr, buf, len) ~
    return ok(n) ~
end
```

### Calling Conventions

| Target | Instruction | Syscall Number | Arguments |
| :--- | :--- | :--- | :--- |
| Linux x86_64 | `syscall` | `rax` | `rdi, rsi, rdx, r10, r8, r9` |
| Linux AArch64 | `svc #0` | `x8` | `x0`–`x5` |
| Linux x86 (32) | `int 0x80` | `eax` | `ebx, ecx, edx, esi, edi, ebp` |
| Linux ARM32 | `svc #0` | `r7` | `r0`–`r5` |
| macOS | via `libSystem` shim | n/a | Platform ABI |
| Windows | via `ntdll`/Win32 shim | n/a | Platform ABI |
| Bare-metal | none | n/a | Board support package (BSP) functions |

---

## 4. Core OS Modules

| Module | Responsibilities |
| :--- | :--- |
| `std.fs` | Files, directories, metadata, memory-mapped files |
| `std.net` | Sockets, DNS, TLS handoff |
| `std.process` | Spawn, wait, pipes, environment, exit codes |
| `std.time` | Monotonic and wall clocks, sleep, timers |
| `std.thread` | Threads, mutexes, atomics (native-64) |
| `std.os.signal` | Signal registration (see `PROCESS_SIGNAL_HANDLING.md`) |

### Spawning a Process

```aeth
fn runBuild() -> Result[i32, text] ~
    let child = check process.spawn("make", ["all"]) or err("Cannot start make") ~
    let status = check await child.wait() or err("Wait failed") ~
    return ok(status.code) ~
end
```

---

## 5. Asynchronous I/O Backends

| OS | Backend |
| :--- | :--- |
| Linux | `io_uring` (fallback: `epoll`) |
| macOS / BSD | `kqueue` |
| Windows | IOCP |
| Bare-metal | Polling loops or interrupt-driven ring buffers |

---

## 6. Bare-Metal & RTOS Interaction

On targets without an OS, `std.os` is replaced by a **Board Support Package**:

```aeth
bsp fn uartWrite(byte: u8) ~
    let status: uptr = UART_BASE + 0x00 ~
    while (*(status as *const u32) & TX_READY) == 0 ~ end
    *((UART_BASE + 0x04) as *mut u32) = byte as u32 ~
end
```

RTOS bridges (FreeRTOS, Zephyr) are provided as optional packages that map `std.thread` and `std.time` to the RTOS primitives.

---

## 7. Sandboxing & Capabilities

Hosted builds may opt into a capability model:

```toml
[capabilities]
fs_read = ["/etc/app", "/var/data"]
fs_write = ["/var/data"]
net_connect = ["api.example.com:443"]
```

Violations return `err(OsError.PermissionDenied)` rather than terminating the process. On Linux, the compiler can emit a matching `seccomp` filter.
