# Aetheris Language Formal Specification (v1.0)

## 1. Syntax Fundamentals
- **Statement Terminator (`~`)**: Every statement ends with a tilde `~`.
- **Block End (`end`)**: Scope blocks close with `end`.

## 2. Type System
- Primitive bit-widths: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`, `text`.
- Target pointer integers: `iptr`, `uptr`.
- Explicit coercions only; no silent bit-width widening.

## 3. Domain Declarations
- `component` & `render` for UI views.
- `route` & `reply` for HTTP web services.
- `async fn` & `await` for asynchronous operations.
- `neural` & `infer` for AI models.
