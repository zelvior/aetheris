# Aetheris Keywords & Operators Specification (v1.0)

The **Aetheris Language Specification** defines a compact, highly expressive token set engineered for maximum human readability, zero syntactic ambiguity, and seamless dual-architecture execution (32-bit and 64-bit systems).

---

## 1. Lexical Grammar & Token Classification

Aetheris tokens are categorized into four primary groups:
1. **Reserved Keywords**: Plain-English control terms used to declare scope, domain logic, and control flow.
2. **Operators**: High-performance mathematical, bitwise, logic, and reference symbols.
3. **Delimiters & Terminators**: Structural boundaries, including the mandatory tilde (`~`) statement terminator.
4. **Literals & Identifiers**: User-defined variable names and strict constant values.

---

## 2. Master Reserved Keywords Table

Reserved keywords cannot be used as variable identifiers. They are organized by functional domain below:

### A. Core Language & Variable Management

| Keyword | Description & Usage | Target / Scope Constraints |
| :--- | :--- | :--- |
| `let` | Declares an immutable local variable with automatic type inference or explicit type annotation. | All targets |
| `mut` | Declares a mutable variable whose value can be updated during execution. | All targets |
| `const` | Declares a compile-time constant scalar or inline literal value. | All targets |
| `fn` | Defines a re-usable function or system procedure block. | All targets |
| `return` | Exits the current function, returning a value or empty unit payload. | All targets |
| `end` | Closes a multi-line block scope (`fn`, `if`, `struct`, `component`, `match`, etc.). | All targets |

### B. Memory, Pointers & Native Control

| Keyword | Description & Usage | Target / Scope Constraints |
| :--- | :--- | :--- |
| `addr` | Returns the memory address pointer (`uptr`) of a variable. | Systems / Native targets |
| `deref` | Explicitly reads or writes to the memory address pointed to by a pointer. | Systems / Native targets |
| `struct` | Declares a user-defined memory layout container for typed properties. | All targets |
| `enum` | Defines a custom tagged union or discrete value enumeration type. | All targets |
| `type` | Defines an alias for existing primitive or complex architecture types. | All targets |

### C. Control Flow & Pattern Matching

| Keyword | Description & Usage | Target / Scope Constraints |
| :--- | :--- | :--- |
| `if` | Begins a conditional execution block based on a boolean evaluation. | All targets |
| `else` | Alternative execution pathway when an `if` expression evaluates to `false`. | All targets |
| `match` | Begins a pattern matching structural comparison against values or `Result` types. | All targets |
| `case` | Defines a single match pattern arm. | All targets |
| `default` | Fallback arm in a `match` expression if no preceding `case` statements match. | All targets |
| `loop` | Initiates an infinite execution loop (requires explicit `break` to exit). | All targets |
| `while` | Executes a loop continuously while a boolean expression evaluates to `true`. | All targets |
| `for` | Iterates over items in an iterable collection (`list`, range, or stream). | All targets |
| `in` | Membership and iteration keyword used inside `for` loops. | All targets |
| `break` | Terminate loop execution immediately and jump to statement after loop block. | All targets |
| `continue` | Skip remaining statements in current loop iteration and move to next pass. | All targets |

### D. Error Handling & Async Execution

| Keyword | Description & Usage | Target / Scope Constraints |
| :--- | :--- | :--- |
| `check` | Unwraps a `Result[T, E]`. If `err`, short-circuits and returns error from host function. | All targets |
| `fail` | Explicitly throws or returns an error payload from a fallible block. | All targets |
| `async` | Marks a function or execution block as asynchronous and non-blocking. | All targets |
| `await` | Pauses execution until an asynchronous future/promise completes. | All targets |

### E. Domain Keywords: Web, API, and AI

| Keyword | Description & Usage | Target / Scope Constraints |
| :--- | :--- | :--- |
| `component` | Declares a reactive UI component node for Web and WASM compilation. | Frontend / Web targets |
| `render` | Defines the view DOM/Canvas structure emitted by a `component`. | Frontend / Web targets |
| `show` | Displays a child component or primitive visual element in UI rendering. | Frontend / Web targets |
| `with` | Applies styling or configuration attributes to a `show` or `reply` block. | All targets |
| `on` | Listens for reactive user events (`click`, `input`, `hover`) in UI code. | Frontend / Web targets |
| `route` | Defines an HTTP network API endpoint binding (`GET`, `POST`, `PUT`, `DELETE`). | Backend / Network targets |
| `reply` | Returns HTTP payload response data along with status codes. | Backend / Network targets |
| `status` | Configures network status codes (e.g., `status 200`, `status 404`). | Backend / Network targets |
| `neural` | Declares a deep learning neural network structure. | AI / Tensor targets |
| `layer` | Defines a single neural layer within a `neural` block (`Dense`, `Dropout`). | AI / Tensor targets |
| `fit` | Initiates model training over a training dataset buffer. | AI / Tensor targets |
| `infer` | Executes forward pass inference on a trained model using input tensor data. | AI / Tensor targets |

---

## 3. Comprehensive Operators Reference

### A. Arithmetic Operators

| Operator | Symbol | Example | Description |
| :--- | :--- | :--- | :--- |
| Addition | `+` | `a + b` | Calculates sum of numeric types or concatenates `text`. |
| Subtraction | `-` | `a - b` | Calculates difference of numeric types or pointer offset distance. |
| Multiplication | `*` | `a * b` | Computes scalar multiplication or matrix dot product. |
| Division | `/` | `a / b` | Computes quotient (truncates on integer types, floating-point on floats). |
| Modulo | `%` | `a % b` | Calculates remainder of division on integer types (`i8` through `u64`). |
| Power | `**` | `a ** b` | Raises base `a` to power `b`. |

### B. Bitwise Operators (Crucial for Embedded 32-bit Systems)

| Operator | Symbol | Example | Description |
| :--- | :--- | :--- | :--- |
| Bitwise AND | `&` | `flags & 0x0F` | Bitwise AND mask across target byte registers. |
| Bitwise OR | `\|` | `flags \| 0x80` | Bitwise OR flag setting across target byte registers. |
| Bitwise XOR | `^` | `mask ^ key` | Bitwise Exclusive OR operation. |
| Bitwise NOT | `~` (prefix) | `~mask` | Inverts bits of an integer register value. |
| Left Shift | `<<` | `val << 2` | Shifts bits left, padding with zeros. |
| Right Shift | `>>` | `val >> 2` | Logical or arithmetic right shift depending on signed/unsigned type. |

### C. Logical & Comparison Operators

| Operator | Symbol / Keyword | Example | Description |
| :--- | :--- | :--- | :--- |
| Equal | `==` | `a == b` | Returns `true` if both values or pointers match. |
| Not Equal | `!=` | `a != b` | Returns `true` if values or pointers do not match. |
| Less Than | `<` | `a < b` | Strictly less than comparison. |
| Greater Than | `>` | `a > b` | Strictly greater than comparison. |
| Less or Equal | `<=` | `a <= b` | Less than or equal to comparison. |
| Greater or Equal | `>=` | `a >= b` | Greater than or equal to comparison. |
| Logical AND | `and` | `x and y` | Evaluates to `true` if both expressions are truthy. Short-circuits. |
| Logical OR | `or` | `x or y` | Evaluates to `true` if either expression is truthy. Serves as fallback operator. |
| Logical NOT | `not` | `not active` | Inverts boolean truth value. |

### D. Memory, Pointers, and Unwrapping

| Operator | Symbol / Keyword | Example | Description |
| :--- | :--- | :--- | :--- |
| Address-Of | `&` | `&val` | Retrieves memory address as `uptr`. |
| Dereference | `*` | `*ptr` | Dereferences memory location to access or mutate value. |
| Event Trigger | `?` | `on click ?` | Listens for event triggers or uncurries reactive state emitters. |
| Statement Terminator | `~` | `let x = 10 ~` | Standard line/statement boundary marker for all Aetheris expressions. |

---

## 4. Operator Precedence & Associativity

The table below outlines operator evaluation order from **highest (1)** to **lowest (11)** precedence:

| Rank | Operator Group | Symbols / Keywords | Associativity |
| :--- | :--- | :--- | :--- |
| **1** | Member Access & Primary | `.`, `()`, `[]`, `?` | Left-to-Right |
| **2** | Unary, Pointers & Inversion | `not`, `~` (prefix), `&`, `*`, `check` | Right-to-Left |
| **3** | Exponentiation | `**` | Right-to-Left |
| **4** | Multiplicative | `*`, `/`, `%` | Left-to-Right |
| **5** | Additive & Concatenation | `+`, `-` | Left-to-Right |
| **6** | Bitwise Shifts | `<<`, `>>` | Left-to-Right |
| **7** | Bitwise Masking | `&`, `^`, `\|` | Left-to-Right |
| **8** | Relational Comparison | `<`, `<=`, `>`, `>=` | Left-to-Right |
| **9** | Equality Comparison | `==`, `!=` | Left-to-Right |
| **10** | Logical Conjunction & Disjunction | `and`, `or` | Left-to-Right |
| **11** | Assignment & Pipeline Operations | `=`, `=>` | Right-to-Left |

---

## 5. Architectural Keyword Adaptations (32-bit vs. 64-bit)

Aetheris keywords and operators adjust underlying byte offsets based on target platform specifications:

```
[ Aetheris Source Code ]
       │
       ├─► --target=native-32: 'uptr' resolves to uint32_t (4-byte alignment)
       │                       'deref' uses 32-bit register reads/writes
       │
       └─► --target=native-64: 'uptr' resolves to uint64_t (8-byte alignment)
                               'deref' uses native 64-bit register reads/writes
```

1. **`addr` & `deref`**:
   * Under `--target=native-32`, pointer arithmetic advances by 4-byte boundaries.
   * Under `--target=native-64`, pointer arithmetic advances by 8-byte boundaries.

2. **`check` & `Result` Unwrapping**:
   * On embedded targets, `check` reads a 1-byte status bit at the head of the Result struct before jumping.
   * On 64-bit enterprise servers, `check` evaluates register flags for immediate short-circuiting.