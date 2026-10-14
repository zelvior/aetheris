# Aetheris JSON & Binary Serialization Specification (v1.0)

`std.json` and `std.codec` provide typed, zero-copy-friendly serialization. Encoders and decoders are generated at compile time from type definitions, with no reflection at runtime.

---

## 1. Deriving Codecs

```aeth
@derive(Json, Binary)
struct User ~
    id: u64 ~
    name: text ~
    email: text ~
    age: u16 ~
    tags: list[text] ~
end
```

`@derive` generates `encode` and `decode` functions specialized to the struct (monomorphized, inlined).

---

## 2. JSON

```aeth
import { json } from "std.json" ~

fn load(raw: text) -> Result[User, JsonError] ~
    return json.decode[User](raw) ~
end

fn dump(u: User) -> text ~
    return json.encode(u) ~
end

let pretty = json.encodePretty(user, indent: 2) ~
```

### Type Mapping

| Aetheris | JSON |
| :--- | :--- |
| `i8`–`i32`, `u8`–`u32`, `f32`, `f64` | number |
| `i64`, `u64` | number; encoded as string if outside ±2^53 (`@json(bigAsString)`) |
| `bool` | `true` / `false` |
| `text` | string (UTF-8, escaped) |
| `list[T]` | array |
| `map[text, V]` | object |
| `Option[T]` | value or `null` |
| `enum` | string, or `{ "type": ..., "value": ... }` for payload variants |

### Field Attributes

```aeth
struct Config ~
    @json(rename: "maxConn") max_conn: u32 ~
    @json(default: 30) timeoutSecs: u32 ~
    @json(skipIfNone) note: Option[text] ~
    @json(skip) cache: list[u8] ~
end
```

### Error Reporting

`JsonError` includes `kind`, `line`, `column`, and a JSON-pointer `path` such as `/tags/2`:

```text
JsonError: expected u16 at /age (line 4, col 12): value 70000 out of range
```

### Parsing Modes

| Mode | Description |
| :--- | :--- |
| `decode[T]` | Typed, validating; rejects unknown fields if `@json(strict)` |
| `parse(raw)` | Dynamic `JsonValue` tree |
| `Stream` | Incremental parser for large inputs (see `DATA_STREAMING_AND_PIPELINES.md`) |
| Zero-copy | `text` fields borrow from the input buffer when `decodeBorrowed` is used |

Limits: default max depth 128 and max size configurable, protecting against resource exhaustion. Duplicate keys are an error unless `@json(lastWins)`.

---

## 3. Binary Format (`ABIN`)

A compact, schema-driven format for storage and network transfer.

| Type | Encoding |
| :--- | :--- |
| Unsigned ints | Little-endian fixed width, or LEB128 with `@bin(varint)` |
| Signed ints | Little-endian fixed width, or ZigZag + LEB128 |
| `f32`/`f64` | IEEE 754 little-endian |
| `bool` | 1 byte (0 or 1; other values rejected) |
| `text` | LEB128 length + UTF-8 bytes |
| `list[T]` | LEB128 count + elements |
| `map[K, V]` | LEB128 count + sorted key/value pairs (deterministic) |
| `struct` | Fields in declaration order, no padding |
| `enum` | LEB128 tag + payload |

### Header

```
┌─────────┬─────────┬──────────────┬───────────────┐
│ "ABIN"  │ version │ schema hash  │ payload ...   │
│ 4 bytes │ 1 byte  │ 8 bytes      │               │
└─────────┴─────────┴──────────────┴───────────────┘
```

The schema hash lets decoders detect mismatches (`err(SchemaMismatch)`). Fields added with `@bin(since: 2)` remain readable by older writers when defaults exist.

```aeth
let bytes = codec.encode(user) ~
let back  = check codec.decode[User](bytes) ~
```

---

## 4. Endianness & Width

* Wire formats are fixed-width and endian-defined, never `uptr`-dependent; `uptr`/`iptr` cannot be serialized directly (compile error `C300`).
* Convert explicitly: `let idx: u64 = ptrIndex as u64 ~`.
* Identical bytes are produced on `native-32` and `native-64`.

## 5. Other Formats

`std.codec` also offers CBOR, MessagePack, TOML, and CSV adapters implementing the same `Encode`/`Decode` interfaces, so one `@derive` can serve multiple formats.

## 6. Security

Decoders validate lengths before allocating, bound recursion depth, reject invalid UTF-8, and never execute data. Use `Limits` to cap sizes when handling untrusted input.
