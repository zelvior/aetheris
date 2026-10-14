# Aetheris Compression Algorithms Specification (v1.0)

`std.compress` offers streaming compression and decompression with deterministic memory usage and safe handling of untrusted input.

---

## 1. Supported Algorithms

| Algorithm | Module | Strength | Typical Use |
| :--- | :--- | :--- | :--- |
| **LZ4** | `compress.lz4` | Very fast, modest ratio | Real-time data, network frames, embedded |
| **Zstd** | `compress.zstd` | Excellent balance, dictionaries | General purpose (default) |
| **Deflate / gzip / zlib** | `compress.deflate` | Universal compatibility | HTTP, PNG, archives |
| **Brotli** | `compress.brotli` | Best ratio for text | Web assets |
| **LZMA / xz** | `compress.xz` | High ratio, slow | Distribution packages |
| **RLE / Delta** | `compress.simple` | Tiny code size | Sensor data, bitmaps on native-32 |

Only used algorithms are linked (tree-shaken). On `native-32` targets, `lz4` and `simple` are the recommended defaults (under 2 KB of code).

---

## 2. One-Shot API

```aeth
import { zstd } from "std.compress" ~

fn pack(data: list[u8]) -> Result[list[u8], CompressError] ~
    return zstd.compress(data, level: 5) ~
end

fn unpack(blob: list[u8]) -> Result[list[u8], CompressError] ~
    return zstd.decompress(blob, maxSize: 64.mb) ~
end
```

`maxSize` is **required** for decompression; omitting it is a compile error. This prevents decompression bombs.

---

## 3. Streaming API

```aeth
async fn compressFile(src: text, dst: text) -> Result[unit, text] ~
    let input  = check await fs.open(src) or err("cannot open input") ~
    let output = check await fs.create(dst) or err("cannot create output") ~
    let enc = zstd.Encoder.new(output, level: 3) ~

    mut buf: [u8; 65536] ~
    loop ~
        let n = check await input.read(buf) ~
        if n == 0 ~ break ~ end
        check await enc.write(buf[0..n]) ~
    end
    check await enc.finish() ~
    return ok(()) ~
end
```

Streams compose with the pipeline operators in `DATA_STREAMING_AND_PIPELINES.md`:

```aeth
source.file("logs.txt") |> compress.zstd(level: 3) |> sink.file("logs.txt.zst") ~
```

---

## 4. Memory Budgets

| Algorithm | Encoder Memory | Decoder Memory |
| :--- | :--- | :--- |
| LZ4 | 16 KB (default) | ~0 (in-place) |
| Deflate | ~256 KB | ~32 KB window |
| Zstd (level 3) | ~1 MB | window size (≤ 8 MB default) |
| Brotli (q5) | ~2 MB | window size |
| LZMA | 16+ MB | dictionary size |

Allocators can be supplied explicitly (`zstd.Encoder.new(out, alloc: arena)`), and fixed-buffer variants allocate nothing:

```aeth
let n = check lz4.compressInto(src, dstBuffer) or err("destination too small") ~
```

---

## 5. Dictionaries & Presets

Zstd dictionaries train on sample data (`aeth compress train samples/ -o app.dict`) and dramatically improve small-message compression such as JSON API payloads. Dictionaries are embedded using the asset pipeline: `asset("app.dict")`.

---

## 6. Container Formats

| Format | Purpose |
| :--- | :--- |
| `.gz`, `.zst`, `.br`, `.xz` | Single-stream files |
| `.zip` | Multi-file archive (`std.archive.zip`) |
| `.tar` | Concatenated files, combined with a stream compressor |
| `.alib`, `.pak` | Aetheris libraries and asset bundles (optional per-entry compression) |

All checksums are verified by default (CRC32 for gzip/zip, XXH64 for Zstd frames, Adler-32 for zlib).

---

## 7. Safety & Determinism

* **Bounded output**: `maxSize` and `maxRatio` limits (default 1000:1) stop decompression bombs.
* **No panics on bad data**: Corrupt input returns `err(CompressError.Corrupt)`.
* **Reproducible output**: Given identical input, level, and library version, compressed bytes are identical on every target.
* **Path safety**: Archive extraction rejects absolute paths and `..` traversal (`err(UnsafePath)`).

## 8. Hardware Acceleration

When available, the runtime uses SIMD routines (AVX2/NEON) and hardware inflate engines; results are bit-identical to the portable implementation.
