# Aetheris Accessibility (A11Y) Specification (v1.0)

Accessibility is a compile-time concern in Aetheris. UI components emit semantic metadata for assistive technology on every target, and the compiler warns when required metadata is missing.

---

## 1. Principles

1. **Accessible by Default**: Built-in components (`Button`, `Input`, `Heading`) carry correct roles without extra code.
2. **Fail at Compile Time**: Missing labels on interactive elements are lint errors (`A11Y-001`), not runtime surprises.
3. **Same Semantics Everywhere**: One accessibility tree is generated and mapped to each platform API.

---

## 2. The Accessibility Tree

Each `show` node produces an accessibility node:

| Field | Description |
| :--- | :--- |
| `role` | `button`, `link`, `heading`, `textbox`, `image`, `list`, `dialog`, etc. |
| `label` | Accessible name |
| `description` | Extra help text |
| `value` | Current value (inputs, sliders) |
| `state` | `disabled`, `checked`, `expanded`, `selected`, `busy` |
| `level` | Heading level (1–6) |

### Platform Mapping

| Target | API |
| :--- | :--- |
| Web | ARIA attributes + semantic HTML elements |
| Windows | UI Automation |
| macOS | NSAccessibility |
| Linux | AT-SPI |
| Embedded displays | Optional serial/audio output of focus text |

---

## 3. Declaring Accessibility Metadata

```aeth
component SaveButton ~
    render ~
        show Button("Save") with a11y(label: "Save document", hint: "Saves your changes") ~
        on click ? trigger save() ~
    end
end

show Image(src: "chart.png") with a11y(label: "Sales chart: up 12% in Q3") ~
show Image(src: "divider.png") with a11y(decorative: true) ~
```

Icon-only buttons must supply `label`; otherwise the compiler reports `A11Y-001`.

---

## 4. Keyboard & Focus

* Every interactive element is reachable by `Tab` in DOM/layout order; override with `tabIndex`.
* `Enter`/`Space` activate buttons; arrow keys navigate lists, menus, and tabs.
* Focus is always visible; the default focus ring meets a 3:1 contrast ratio.
* Modal dialogs trap focus and restore it on close.

```aeth
show Dialog(title: "Confirm") with a11y(modal: true) ~
    show Button("Cancel") with a11y(focusFirst: true) ~
end
```

---

## 5. Dynamic Content

Live regions announce changes to screen readers:

```aeth
show Text(statusMessage) with a11y(live: "polite") ~      // non-interrupting
show Text(errorMessage)  with a11y(live: "assertive") ~   // interrupting
```

---

## 6. Visual Requirements

| Rule | Check |
| :--- | :--- |
| Text contrast | At least 4.5:1 (3:1 for large text) against background |
| Touch targets | At least 44x44 dp |
| Motion | Honors `prefers-reduced-motion`; animations expose `reducedMotion` fallback |
| Scaling | Layout tolerates 200% text scale without clipping |
| Color alone | State is never conveyed by color only |

The compiler computes contrast for static colors and warns (`A11Y-014`). Themes may provide `highContrast` variants selected automatically when the OS requests them.

---

## 7. Testing & Tooling

```bash
aeth lint --a11y            # static checks
aeth test --a11y-audit      # runs rendered-tree audit in tests
```

```aeth
test "login form is accessible" ~
    let tree = render(LoginForm) ~
    assert tree.allInteractiveHaveLabels() ~
    assert tree.tabOrderIsLogical() ~
end
```

Targets conform to **WCAG 2.2 AA** by default. See `LINTER_AND_FORMATTER.md` for rule configuration.
