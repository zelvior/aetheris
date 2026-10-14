# Aetheris Linter & Formatter Specification (v1.0)

`aeth lint` performs static analysis beyond what the compiler enforces; `aeth fmt` provides a single canonical code style with no configuration debates.

---

## 1. Formatter (`aeth fmt`)

```bash
aeth fmt                 # format the workspace
aeth fmt --check         # exit non-zero if changes are needed (CI)
aeth fmt src/main.aeth   # single file
```

### Canonical Style

| Rule | Standard |
| :--- | :--- |
| Indentation | 4 spaces; tabs are converted |
| Line width | 100 columns; wrapping at argument and operator boundaries |
| Terminator | Exactly one space before `~`, never on its own line |
| Block keywords | `end` aligned with the opening keyword (`fn`, `if`, `match`, ...) |
| Blank lines | One between top-level items; none at block starts or ends |
| Imports | Grouped (std, external, local), alphabetized |
| Trailing commas | Required in multi-line lists and struct literals |
| Operators | Spaces around binary operators; none around `.` and unary operators |
| Casing | `camelCase` values/functions, `PascalCase` types/components, `SCREAMING_CASE` constants |

Before:

```aeth
fn add(a:i32,b:i32)->i32 ~ return a+b~ end
```

After:

```aeth
fn add(a: i32, b: i32) -> i32 ~
    return a + b ~
end
```

Formatting is **idempotent** and **semantics-preserving**; comments are preserved in place. Disable regions with `// aeth-fmt: off` and `// aeth-fmt: on`.

The only configurable options in `aeth.toml` are `line_width` (80–120) and `import_grouping`.

---

## 2. Linter (`aeth lint`)

```bash
aeth lint                    # run default rule set
aeth lint --fix              # apply safe automatic fixes
aeth lint --target=native-32 # include target-specific rules
aeth lint --format=json      # machine-readable output
```

### Severity Levels

`error` (fails CI), `warn`, `info`, `off`. Profiles: `minimal`, `recommended` (default), `strict`, `embedded`, `web`.

---

## 3. Rule Catalog

### Correctness

| Code | Rule | Default |
| :--- | :--- | :--- |
| `L001` | Unused variable, parameter, or import | warn |
| `L002` | Unreachable code after `return`/`fail`/`break` | warn |
| `L003` | Ignored `Result` value | error |
| `L004` | `or` fallback masks a more specific error | info |
| `L005` | Shadowed binding | warn |
| `L006` | `match` missing cases without `default` | error |
| `L007` | Comparison of floats with `==` | warn |
| `L008` | Integer arithmetic that can overflow in a constant context | warn |

### Safety & Security

| Code | Rule | Default |
| :--- | :--- | :--- |
| `L101` | Raw pointer dereference outside `unsafe`/systems block | error |
| `L102` | Casting `uptr` to a smaller integer (truncation) | error |
| `L103` | Hard-coded credential patterns | error |
| `L104` | Missing bounds check on external input index | warn |
| `L105` | Use of deprecated crypto (`std.crypto.legacy`) | warn |

### Portability (32/64-bit)

| Code | Rule | Default |
| :--- | :--- | :--- |
| `L201` | Assumption that `uptr` is a fixed size | warn |
| `L202` | `f64` used on a target without FPU | warn (`embedded`: error) |
| `L203` | Heap allocation on `native-32` | warn (`embedded`: error) |
| `L204` | Serializing `uptr`/`iptr` directly | error |
| `L205` | Recursion that is not tail-call eligible on `native-32` | warn |

### Style & Maintainability

| Code | Rule | Default |
| :--- | :--- | :--- |
| `L301` | Naming convention violation | warn |
| `L302` | Function longer than 80 lines | info |
| `L303` | Cyclomatic complexity above 15 | info |
| `L304` | Public item without doc comment | info (`strict`: warn) |
| `L305` | Magic number (outside constants and 0/1) | info |
| `L306` | Redundant type annotation | info |
| `L307` | Deeply nested blocks (more than 4 levels) | warn |

### Domain-Specific

| Code | Rule | Applies To |
| :--- | :--- | :--- |
| `A11Y-001`…`A11Y-0xx` | Accessibility (see `ACCESSIBILITY_AND_A11Y.md`) | `component` |
| `L401` | `route` without explicit status on all paths | `route` |
| `L402` | Unvalidated request body used directly | `route` |
| `L403` | `neural` layer shape mismatch | `neural` |
| `L404` | State mutated outside event handler | `component` |

---

## 4. Configuration

```toml
[lint]
profile = "recommended"

[lint.rules]
L302 = "off"
L203 = "error"

[lint.paths]
"tests/**" = { L305 = "off", L304 = "off" }
```

Inline suppression requires a reason:

```aeth
// aeth-lint: allow(L305, reason = "register offset from datasheet")
let reg = base + 0x14 ~
```

Suppressions without a reason emit `L900`.

---

## 5. Autofixes

Safe fixes (applied by `--fix`): unused import removal, adding `_` prefixes, adding missing `~`/`end`, `or`→`match` conversion suggestions, constant folding of literals. Unsafe fixes require `--fix --unsafe` and show a diff.

## 6. CI Integration

```bash
aeth fmt --check && aeth lint --profile=strict --format=sarif > lint.sarif
```

SARIF output integrates with GitHub code scanning. Pre-commit hook: `aeth hooks install`.
