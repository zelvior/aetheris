# Aetheris Process & Signal Handling Specification (v1.0)

This document specifies process lifecycle management, exit codes, and signal handling through `std.process` and `std.os.signal`.

---

## 1. Process Lifecycle

```
  start ──► main() ──► [ running ] ──► shutdown hooks ──► rt_shutdown ──► exit(code)
                            │
                  signal / panic / exit()
```

### Exit Codes

| Code | Meaning |
| :--- | :--- |
| `0` | Success |
| `1` | `main` returned `err(...)` |
| `2` | Command-line usage error |
| `101` | Panic |
| `128 + n` | Terminated by signal `n` (POSIX convention) |

```aeth
import { process } from "std.process" ~

fn main() ~
    process.onExit(fn() ~ flushLogs() ~ end) ~
    process.exit(0) ~
end
```

Shutdown hooks run in reverse registration order with a default 5-second budget.

---

## 2. Signals

Signals are exposed as an async stream, so no code runs in a raw interrupt context.

| Signal | Name in Aetheris | Default Action |
| :--- | :--- | :--- |
| SIGINT | `Signal.Interrupt` | Terminate |
| SIGTERM | `Signal.Terminate` | Terminate |
| SIGHUP | `Signal.Hangup` | Terminate |
| SIGUSR1 / SIGUSR2 | `Signal.User1` / `User2` | Terminate |
| SIGPIPE | `Signal.Pipe` | Ignored (writes return `err`) |
| SIGCHLD | `Signal.Child` | Ignored |
| SIGKILL / SIGSTOP | n/a | Cannot be caught |

On Windows, `CTRL_C_EVENT`, `CTRL_BREAK_EVENT`, and `CTRL_CLOSE_EVENT` map to `Interrupt`, `Interrupt`, and `Terminate`.

### Graceful Shutdown Pattern

```aeth
import { signal } from "std.os.signal" ~

async fn main() -> Result[unit, text] ~
    let sig = signal.listen([Signal.Interrupt, Signal.Terminate]) ~
    let server = check net.listen("0.0.0.0", 8080) or err("bind failed") ~

    match await select(server.run(), sig.next()) ~
        case Selected.First(r)  => return r ~
        case Selected.Second(s) ~
            log.info("received signal, draining") ~
            await server.drain(timeout: 10s) ~
            return ok(()) ~
        end
    end
end
```

### Restrictions

* No signal handler callbacks that interrupt arbitrary code; use `signal.listen` streams.
* Synchronous signals (SIGSEGV, SIGBUS, SIGFPE, SIGILL) are converted into panics with a diagnostic and backtrace in debug builds.
* Embedded targets map "signals" to interrupt vectors via `interrupt fn` handlers, which may only touch `volatile` state and lock-free queues.

---

## 3. Child Processes

```aeth
let child = check process.spawn("convert", ["in.png", "out.webp"]) ~
    .stdout(Pipe) .stderr(Pipe) .env("LANG", "C") ~
let output = check await child.waitOutput() ~
if output.status.code != 0 ~
    return err(output.stderr) ~
end
```

| Method | Description |
| :--- | :--- |
| `spawn(cmd, args)` | Start a process without a shell |
| `.stdin/.stdout/.stderr(mode)` | `Inherit`, `Pipe`, `Null`, or file |
| `.cwd(path)`, `.env(k, v)` | Working directory and environment |
| `wait()`, `waitOutput()` | Await completion |
| `kill(signal)` | Send a signal (`Terminate` by default) |

Commands are executed without shell interpretation unless `process.shell(...)` is used explicitly, avoiding injection.

---

## 4. Process Groups & Daemons

* `process.group()` creates a group so `kill` terminates the whole tree.
* `daemonize()` (Unix) forks, detaches, and redirects standard streams.
* Systemd integration: `sdNotify("READY=1")` and socket activation are supported.

## 5. Resource Limits

`process.limits.set(Memory, 512.mb)`, `OpenFiles`, and `CpuTime` map to `setrlimit` / Job Objects. Exceeding a limit yields `err(OsError.OutOfMemory)` or termination per policy.
