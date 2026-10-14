# Aetheris Asset Pipeline & Fonts Specification (v1.0)

The asset pipeline processes images, fonts, audio, shaders, and data files at build time and embeds or packages them for each target.

---

## 1. Pipeline Overview

```
  assets/  ──►  Import  ──►  Transform  ──►  Hash & Pack  ──►  Embed / Bundle
                  │            │                 │
              type detect   resize, compress,   content hash
                            subset, convert     manifest.json
```

* **Reproducible**: Outputs are content-addressed; identical inputs yield identical hashes.
* **Incremental**: Only changed assets are re-processed (cache in `.aeth/cache/assets`).

---

## 2. Referencing Assets in Code

```aeth
const logo = asset("images/logo.png") ~              // embedded at compile time
const config = asset("data/defaults.json") ~

show Image(src: logo) ~
let bytes: list[u8] = logo.bytes() ~
```

Asset paths are validated at compile time; a missing file is error `AS001`.

### Delivery Modes

| Mode | Behavior | Typical Use |
| :--- | :--- | :--- |
| `embed` | Bytes placed in `.rodata` | Embedded firmware, small icons |
| `bundle` | Packed into `assets.pak` loaded on demand | Desktop apps and games |
| `url` | Emitted as hashed file with URL | Web (`/assets/logo.3f9a1c.png`) |

---

## 3. Images

| Source | Transforms | Output |
| :--- | :--- | :--- |
| PNG, JPEG, SVG | Resize, density variants (`@1x`, `@2x`, `@3x`) | WebP, AVIF, PNG |
| SVG | Optimize, rasterize at build time on embedded | Path data or bitmap |
| Embedded displays | Convert to RGB565 / 1-bit | Raw bitmap in flash |

```toml
[assets.images]
formats = ["webp", "avif"]
densities = [1, 2, 3]
embedded_format = "rgb565"
```

---

## 4. Fonts

### Supported Formats

TTF, OTF, WOFF2 inputs. Output depends on target.

| Target | Output |
| :--- | :--- |
| Web | WOFF2, subsetted, with `font-display: swap` |
| Desktop | Glyph atlas (SDF or bitmap) + shaping data |
| Embedded | Pre-rasterized bitmap font at fixed sizes |

### Declaring Fonts

```aeth
font Body = asset("fonts/Inter.ttf") with subset("latin") ~
font Mono = asset("fonts/JetBrainsMono.ttf") with weights(400, 700) ~

show Text("Hello") with style(font: Body, fontSize: 16) ~
```

### Subsetting

`subset("latin")`, `subset("latin-ext")`, `subset("cjk")`, or `subset(chars: "0123456789.:")`. The compiler also detects literal text in the source and can auto-subset (`--font-autosubset`).

### Text Shaping & Internationalization

* UTF-8 throughout; shaping via a built-in HarfBuzz-compatible engine for complex scripts.
* Right-to-left and bidirectional text supported through the Unicode Bidirectional Algorithm.
* Fallback stacks: `font: [Body, Emoji, SystemUI]`.
* Embedded builds list the exact codepoints included; missing glyphs render as U+FFFD.

---

## 5. Audio & Other Assets

| Type | Input | Output |
| :--- | :--- | :--- |
| Audio | WAV, FLAC, MP3, OGG | Opus/OGG, or raw PCM for embedded |
| Shaders | `.aeth` `shader` blocks | SPIR-V, MSL, HLSL, WGSL |
| Data | JSON, CSV, TOML | Parsed constants or binary blobs (see `JSON_AND_BINARY_SERIALIZATION.md`) |
| Models | ONNX, safetensors | Packed weights (see `MODEL_CONVERSION_AND_ONNX.md`) |

---

## 6. Compression & Manifest

Bundled assets can be compressed with Zstd or Brotli (see `COMPRESSION_ALGORITHMS.md`). The pipeline emits `manifest.json`:

```json
{
  "images/logo.png": { "hash": "3f9a1c", "size": 4821, "mode": "url", "url": "/assets/logo.3f9a1c.png" },
  "fonts/Inter.ttf": { "hash": "a7b2e0", "size": 38211, "mode": "bundle", "subset": "latin" }
}
```

## 7. CLI

```bash
aeth assets build        # process all assets
aeth assets analyze      # size report and unused asset detection
aeth assets clean        # clear cache
```
