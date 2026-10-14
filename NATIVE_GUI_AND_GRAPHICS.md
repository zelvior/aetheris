# Aetheris Native GUI & Graphics Specification (v1.0)

Aetheris `component` declarations compile not only to the web but also to native windows. This document covers the native GUI toolkit (`std.ui`) and low-level 2D/3D graphics (`std.gfx`).

---

## 1. Architecture

```
   component tree  ──►  Layout Engine  ──►  Display List  ──►  Renderer
                            │                                     │
                      Style & Theme                       Vulkan / Metal / D3D12
                                                          OpenGL ES / Software
```

1. **Retained Component Tree**: `component` nodes hold reactive `state` and are diffed at the signal level, not by full tree comparison.
2. **Layout Engine**: Flexbox-style layout (`row`, `column`, `grid`) resolved in a single pass.
3. **Display List**: Layout output is flattened to draw commands (rect, glyph run, image, path).
4. **Renderer**: GPU-accelerated back end selected per platform, with a CPU software fallback.

---

## 2. Window Backends

| Platform | Windowing | Renderer |
| :--- | :--- | :--- |
| Linux | Wayland / X11 | Vulkan, OpenGL ES |
| macOS | Cocoa | Metal |
| Windows | Win32 | Direct3D 12 |
| Embedded | Framebuffer / SPI display | Software rasterizer |
| Web | Canvas / DOM | WebGPU, Canvas2D |

---

## 3. Native Component Example

```aeth
import { Window, Column, Row, Button, Text } from "std.ui" ~

component CounterApp ~
    mut state count: i32 = 0 ~

    render ~
        show Column() with style(padding: 16, gap: 8) ~
            show Text("Count: " + count) with style(fontSize: 24) ~
            show Row(gap: 8) ~
                show Button("-") ~
                on click ? trigger count = count - 1 ~
                show Button("+") ~
                on click ? trigger count = count + 1 ~
            end
        end
    end
end

fn main() ~
    Window.open(title: "Counter", width: 320, height: 160, root: CounterApp) ~
end
```

---

## 4. Layout & Styling

| Property | Values |
| :--- | :--- |
| `direction` | `row`, `column` |
| `align` / `justify` | `start`, `center`, `end`, `stretch`, `space-between` |
| `padding`, `margin`, `gap` | Pixels (`i32`) or density-independent units (`dp`) |
| `width`, `height` | Fixed, `fill`, or `auto` |
| `color`, `bgColor` | `#RRGGBB`, `#RRGGBBAA`, or theme token |

Themes are defined with `theme` blocks and applied via `with theme(Dark)`. High-DPI scaling is automatic; `1dp = 1/160 inch`.

---

## 5. Low-Level Graphics (`std.gfx`)

```aeth
import { Canvas, Color, Path } from "std.gfx" ~

fn drawBadge(c: *mut Canvas) ~
    c.fillRect(0, 0, 64, 64, Color.rgb(43, 108, 176)) ~
    mut p = Path.new() ~
    p.moveTo(8.0, 32.0) ~
    p.lineTo(32.0, 56.0) ~
    p.lineTo(56.0, 8.0) ~
    c.strokePath(p, Color.white, 4.0) ~
end
```

### 3D & Shaders

`shader` blocks compile to SPIR-V, MSL, HLSL, or WGSL from a single source:

```aeth
shader fragment tint(uv: vec2[f32], color: vec4[f32]) -> vec4[f32] ~
    return color * vec4(uv.x, uv.y, 1.0, 1.0) ~
end
```

---

## 6. Input & Events

| Event | Payload |
| :--- | :--- |
| `click`, `doubleClick` | `x, y, button, modifiers` |
| `key` | `code, text, modifiers, repeat` |
| `scroll` | `dx, dy` |
| `touch` | `id, x, y, phase` |
| `resize`, `focus`, `close` | Window state |

Events run on the UI thread; heavy work should be dispatched with `async` tasks that update `state` on completion.

---

## 7. Embedded Display Mode

On `native-32` microcontrollers, `std.ui` uses a partial-redraw dirty-rectangle renderer with a fixed line buffer (default 20 lines) to minimize RAM. Fonts are bitmap or pre-rasterized (see `ASSET_PIPELINE_AND_FONTS.md`). Accessibility hooks are described in `ACCESSIBILITY_AND_A11Y.md`.
