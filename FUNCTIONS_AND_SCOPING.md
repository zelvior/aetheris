# Aetheris Functions & Scoping Specification (v1.0)

The **Aetheris Functions and Scoping Subsystem** provides a clean, predictable, and deterministic model for procedural logic, closures, lexical boundaries, and asynchronous execution—tailored to operate seamlessly from resource-constrained 32-bit microcontrollers to high-performance 64-bit server nodes.

## 1. Core Scoping Principles

1. **Strict Lexical Scoping**: Variables are bound to the block scope in which they are declared, enclosed by statement terminators (`~`) and closed explicitly with the `end` keyword.

2. **Zero Hidden Closures**: Closures capture environment variables explicitly or via read-only stack references, preventing unpredictable heap allocations on embedded targets.

3. **Explicit Return Paths**: Functions that return data must declare their return type using the `->` operator. Functions returning unit payloads omit the return declaration.

## 2. Function Declaration & Basic Syntax

Functions are declared using the `fn` keyword, followed by an identifier, parameter list enclosed in parentheses, return type (optional), statement terminator (`~`), block body, and closed with `end`.

```
// Standard function with explicit types and return signature
fn calculateTax(subtotal: f32, rate: f32) -> f32 ~
    let rawTax = subtotal * rate ~
    return rawTax ~
end

// Procedure returning no value (Unit payload)
fn logBootMessage(msg: text) ~
    print("[SYSTEM BOOT]: " + msg) ~
end
```

## 3. Parameter Passing & Mutability

* **Immutable by Default**: All function parameters are treated as immutable values within the function body to prevent accidental side effects.

* **Mutable Parameters (`mut`)**: If a parameter requires local modification, it must be explicitly declared with the `mut` keyword.

```
fn applyDiscount(mut price: f32, discountRate: f32) -> f32 ~
    price = price - (price * discountRate) ~
    return price ~
end
```

## 4. Asynchronous Functions (`async` / `await`)

For network servers, I/O streams, and non-blocking tasks, functions can be prefixed with `async`. Execution pauses using the `await` keyword without blocking host threads.

```
async fn fetchUserProfile(userId: u64) -> Result[text, text] ~
    let endpoint = "/api/v1/users/" + userId ~
    let response = check await http.get(endpoint) or err("Network timeout") ~
    return ok(response) ~
end
```

## 5. Scope Lifetime Rules (Stack vs. Heap)

* **Local Variables**: Declared via `let` or `mut` inside a function body, local primitives are allocated on the current stack frame. When the function block hits `end`, the stack pointer unwinds instantly.

* **Block Scoping (`if`, `loop`, `match`)**: Inner blocks create subordinate lexical child scopes. Variables declared inside an inner block cannot escape past its corresponding `end` statement.

```
fn processBatch(limit: i32) ~
    if limit > 0 ~
        let batchId = 42 ~
        print("Processing batch: " + batchId) ~
    end
    
    // Error: 'batchId' is out of scope here and cannot be accessed!
end
```

## 6. Architecture Specialization

* **32-Bit Targets**: Function call frames maintain rigid 4-byte stack alignment. Tail-call optimization is enforced aggressively by the compiler to prevent stack overflow on microcontrollers with tiny RAM footprints.

* **64-Bit Targets**: Call frames utilize 8-byte register alignment, mapping arguments directly to CPU registers (`RDI`, `RSI`, `RDX`, etc. on x86_64) for maximum throughput.