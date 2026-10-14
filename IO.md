# Aetheris I/O Subsystem Specification (v1.0)

The **Aetheris I/O Subsystem** provides a unified, zero-overhead, asynchronous input/output interface designed to operate seamlessly across microcontrollers (32-bit), high-throughput servers (64-bit), and browser environments (WebAssembly).

---

## 1. Core I/O Design Principles

1. **Non-Blocking by Default**: All I/O channels (files, network sockets, standard streams) operate asynchronously without blocking the main execution thread.
2. **Zero-Copy Buffering**: Input/output streams utilize underlying raw memory buffer views (`uptr`), minimizing memory allocation overhead during heavy data transfers.
3. **Cross-Platform Abstraction**: A single syntax seamlessly compiles down to POSIX/Win32 file descriptors, Linux `io_uring`, or browser Readable/Writable streams depending on the target.

---

## 2. Standard Streams & Terminal Operations

Aetheris provides built-in stream objects for standard console input and output, terminating expressions with the standard tilde (`~`).

### A. Output (`print` & `write`)
```rust
// Standard formatted print to stdout
print("System booting on target architecture...") ~

// Low-level buffer write
let payload: text = "HTTP/1.1 200 OK\r\n" ~
stdout.write(payload) ~
```

### B. Input (`input`)
```rust
print("Enter your configuration key: ") ~
let configKey = input() ~
print("Loaded key: " + configKey) ~
```

---

## 3. File System Operations (`fs`)

File system interactions are managed through the built-in `fs` module, returning `Result[T, E]` to enforce deterministic error handling.

### Reading Files
```rust
fn loadConfig(path: text) -> Result[text, text] ~
    let fileContent = check fs.readText(path) or err("Failed to open configuration file") ~
    return ok(fileContent) ~
end
```

### Writing Files
```rust
fn saveLog(logData: text) ~
    let result = fs.writeText("/var/log/aetheris.log", logData) ~
    match result ~
        case ok(_) => print("Log saved successfully.") ~
        case err(msg) => print("Warning: Could not save log: " + msg) ~
    end
end
```

---

## 4. Network and Socket I/O (`net`)

For backend services, Aetheris provides high-performance TCP/UDP socket abstractions integrated with asynchronous event loops.

```rust
async fn startTcpServer(port: u16) -> Result[bool, text] ~
    let listener = check net.listen("0.0.0.0", port) or err("Port binding failed") ~
    print("Listening for incoming connections on port " + port) ~
    
    loop ~
        let client = check listener.accept() ~
        async handleClientConnection(client) ~
    end
    
    return ok(true) ~
end
```

---

## 5. Architecture-Specific I/O Targets

| Target Environment | Low-Level Mechanism | Memory Handling |
| :--- | :--- | :--- |
| **Native 32-Bit** | Direct system calls (`sys_read`, `sys_write`), polling loops | Static fixed-size byte arrays (`u8`) to prevent heap fragmentation |
| **Native 64-Bit** | `io_uring` (Linux) / IOCP (Windows), epoll/kqueue | Dynamic pointer buffers (`uptr`) with zero-copy reference mapping |
| **Web / WASM** | JavaScript Streams API, Fetch API, WebSockets | TypedArrays (`Uint8Array`, `ArrayBuffer`) mapped to JS memory |