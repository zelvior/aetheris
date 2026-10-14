# Aetheris Error Handling Specification (v1.0)

**Aetheris** (`.aeth`) rejects heavy runtime exception handling (such as traditional `try/catch/finally` blocks and call-stack unwinding overhead). Instead, it implements a **Zero-Overhead Deterministic Result Pattern** combining lightweight explicit result structures, compile-time error checking, and beginner-friendly safety keywords.

---

## 1. Core Principles of Error Handling in Aetheris

1. **Explicit, Deterministic Control Flow**: Functions that can fail must explicitly declare their potential failure modes in their type signatures.
2. **Zero Stack Unwinding**: Errors are passed as plain stack values (`Result[T, E]`), preventing the massive performance penalties of traditional runtime exceptions on both 32-bit embedded systems and 64-bit high-throughput backend servers.
3. **Beginner Ergo-First**: Simple keyword operators like `check`, `fail`, and `?` allow engineers to propagate or recover from errors without deeply nested blocks.

---

## 2. The Result Primitive: `Result[T, E]`

Aetheris provides a built-in generic result type for fallible operations:

* **`ok(value)`**: Wraps a successful value of type `T`.
* **`err(reason)`**: Wraps an error payload or fault indicator of type `E`.

```
fn parsePort(rawText: text) -> Result[u16, text] ~
    if rawText == "" ~
        return err("Port string cannot be empty") ~
    end
    
    let portNum = convertToU16(rawText) ~
    if portNum < 1024 ~
        return err("Port must be unprivileged (> 1023)") ~
    end
    
    return ok(portNum) ~
end
```

---

## 3. Propagation & Recovery Keywords

### A. The `check` Operator (Short-Circuit Propagation)
Instead of deep nesting, the `check` keyword unwraps a `Result[T, E]`. If the operation is `ok`, it assigns the value. If it is `err`, the current function immediately returns the error.

```
fn initializeServer(configPath: text) -> Result[bool, text] ~
    // If parsePort fails, initializeServer automatically short-circuits and returns the error
    let port = check parsePort("8080") ~
    
    print("Server initialized on port: " + port) ~
    return ok(true) ~
end
```

### B. Safe Fallbacks using `or`
Beginners and low-boilerplate workflows can provide safe fallback values directly inline:

```
// If parsePort fails, fallbackPort defaults to 8080
let fallbackPort = parsePort("invalid") or 8080 ~
```

### C. Matching Result Outcomes
Explicit pattern matching using `match` guarantees that both success and failure pathways are handled cleanly:

```
fn startApp() ~
    match parsePort("9000") ~
        case ok(port) => print("Binding to port: " + port) ~
        case err(msg)  => print("Failed to start: " + msg) ~
    end
end
```

---

## 4. Multi-Target Error Strategies (32-bit vs. 64-bit)

Aetheris aligns error structures to target hardware constraints during compile-time specialization:

* **32-Bit Systems / Embedded Targets**:
  * Error payloads are restricted to fixed 32-bit codes (`u32`) or small byte pointers (`uptr`) to prevent memory allocation during error handling.
  * Zero heap usage when bubbling errors up the call stack.

* **64-Bit Server / Web Targets**:
  * Error payloads can include extended UTF-8 context strings, stack trace references, and diagnostic JSON structures.

---

## 5. Domain-Specific Error Interceptors

### A. Web UI Components
UI components automatically isolate render errors to prevent full DOM crashes:

```
component UserDashboard ~
    render ~
        show UserList() or show Text("Unable to load users. Please refresh.") ~
    end
end
```

### B. Backend API Routes
Uncaught HTTP route errors map directly to HTTP status codes:

```
route GET "/api/v1/profile" ~
    let user = check db.findUser(request.id) or reply "User not found" with status 404 ~
    reply user with status 200 ~
end
```

### C. AI & Tensor Operations
Hardware/CUDA memory allocation faults are captured safely without crashing the main thread:

```
fn loadWeights(path: text) -> Result[tensor[f32], text] ~
    let weights = check loadTensorFromDisk(path) or err("GPU Memory Allocation Failed") ~
    return ok(weights) ~
end
```