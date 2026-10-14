# Aetheris Browser Compilation & WebAssembly Specification (v1.0)

This document defines how `aethc --target=web` produces WebAssembly modules and JavaScript glue, and how Aetheris code interacts with the browser.

---

## 1. Output Modes

| Mode | Flag | Output | Use Case |
| :--- | :--- | :--- | :--- |
| WASM + JS glue | `--target=web` | `app.wasm`, `app.js` | Default for production |
| Pure ES6 JS | `--target=web --emit=js` | `bundle.js` | Tiny widgets, no WASM support |
| WASI | `--target=wasi` | `app.wasm` | Server-side and edge runtimes |
| Component Model | `--target=web --wasm-component` | `.wasm` component | Cross-language embedding |

```bash
aethc build web_app.aeth --target=web -o dist/
aeth dev --target=web          # hot reload server (see HOT_RELOAD_AND_REPL.md)
```

---

## 2. Memory Model

* WASM linear memory is a single `ArrayBuffer`; `uptr`/`iptr` map to `u32`/`i32` under `wasm32` (default) or `u64`/`i64` under `--wasm-memory64`.
* The Aetheris allocator manages linear memory and calls `memory.grow` when arenas are exhausted.
* Strings (`text`) are UTF-8 in linear memory; the glue layer converts with `TextDecoder`/`TextEncoder` only at boundary crossings.

---

## 3. JavaScript Interop

### Exporting to JS

```aeth
export js fn greet(name: text) -> text ~
    return "Hello, " + name ~
end
```

```js
import init, { greet } from "./app.js";
await init();
console.log(greet("World"));
```

### Importing from JS

```aeth
extern js fn alert(msg: text) ~
extern js fn fetchJson(url: text) -> Result[text, text] ~
```

### Type Mapping

| Aetheris | WASM | JavaScript |
| :--- | :--- | :--- |
| `i8`–`i32`, `u8`–`u32` | `i32` | `Number` |
| `i64`, `u64` | `i64` | `BigInt` |
| `f32`, `f64` | `f32`, `f64` | `Number` |
| `bool` | `i32` | `Boolean` |
| `text` | pointer + length | `String` |
| `list[u8]` | pointer + length | `Uint8Array` view |
| `Result[T, E]` | tagged struct | `{ ok }` or `{ err }` object |
| `tensor[f32]` | pointer + shape | `Float32Array` / `GPUBuffer` |

---

## 4. DOM & Component Lowering

`component` trees compile into direct DOM operations. Each `state` becomes a **signal cell** with a list of subscribers; only the affected text nodes or attributes update.

```aeth
component Greeting ~
    mut state name: text = "World" ~
    render ~
        show Text("Hello, " + name) ~
    end
end
```

lowers (conceptually) to:

```js
const name = signal("World");
const t = document.createTextNode("");
effect(() => { t.data = "Hello, " + name.get(); });
```

No virtual DOM is generated unless `--vdom` is requested for compatibility testing.

---

## 5. Async & Event Loop

`async fn` compiles to promise-returning exports. `await` on `http.get` lowers to `fetch`. Long-running CPU tasks can be moved to Web Workers:

```aeth
let result = await worker.run(heavyCompute, input) ~
```

---

## 6. Size & Performance

| Optimization | Flag | Effect |
| :--- | :--- | :--- |
| Dead code elimination | default | Removes unused std functions |
| `wasm-opt` pass | `-O3` | Binary size and speed |
| Streaming compile | default | Uses `WebAssembly.instantiateStreaming` |
| Brotli precompression | `--compress=br` | Emits `.wasm.br` |
| SIMD | `--wasm-simd` | Enables 128-bit SIMD (see `MATH_INTRINSICS_AND_SIMD.md`) |

## 7. Security

Compiled modules run inside the browser sandbox. Aetheris emits no `eval`, supports strict CSP (`script-src 'self' 'wasm-unsafe-eval'`), and generates Subresource Integrity hashes in `manifest.json`.
