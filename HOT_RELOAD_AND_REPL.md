# Aetheris Hot Reload & REPL Specification (v1.0)

This document extends the interpreter spec with live-development workflows: the enhanced REPL (`aethi`) and the hot-reload server (`aeth dev`).

---

## 1. Development Modes

| Command | Description |
| :--- | :--- |
| `aethi` | Interactive REPL (tree-walk or VM) |
| `aeth dev` | Watches files, rebuilds, and live-updates a running program |
| `aeth dev --target=web` | Browser hot reload with state preservation |
| `aeth dev --attach <pid>` | Patch a running native process (debug builds) |

---

## 2. Hot Reload Pipeline

```
  file change ──► incremental parse ──► type check changed modules
                                              │
                                   compute patch (changed fns / components)
                                              │
                    ┌─────────────────────────┴───────────────┐
                    ▼                                         ▼
          VM: swap bytecode in place               Native: load patch .so/.dll,
          Web: replace module via HMR              redirect function table
```

* **Incremental**: Only modules whose content hash changed (and their dependents) are rechecked.
* **Sub-100 ms** target for typical single-file edits.
* On a type error, the old version keeps running and the diagnostic is shown inline.

---

## 3. State Preservation Rules

| Change | Result |
| :--- | :--- |
| Function body edited | Swapped live; next call uses new code |
| `component` render edited | Re-rendered; all `state` values kept |
| `state` added | Initialized with default; others kept |
| `state` type changed or removed | That state resets; a notice is printed |
| `struct` layout changed | Live instances migrated if a `migrate` function exists, else full restart |
| Function signature changed | Callers rechecked; restart if ABI-incompatible |

```aeth
migrate Point3D from v1 to v2 ~
    return Point3D { x: old.x, y: old.y, z: 0.0 } ~
end
```

Hot reload is disabled in release builds.

---

## 4. REPL Features

```bash
$ aethi
Aetheris REPL v1.0 (64-bit VM Mode)
>>> let xs = [1, 2, 3] ~
>>> xs.push(4) ~
>>> xs ~
[1, 2, 3, 4]
>>> :type xs
list[i32]
```

### Multi-line Input

When a block is open (`fn`, `if`, `match` without `end`), the prompt changes to `...` until the block closes.

### REPL Commands

| Command | Description |
| :--- | :--- |
| `:type <expr>` | Show inferred type |
| `:ast <expr>` | Print AST |
| `:ir <expr>` | Print SSA IR |
| `:asm <fn>` | Show generated assembly |
| `:load <file>` | Load a module |
| `:reload` | Reload loaded modules |
| `:target 32\|64` | Switch simulated pointer width |
| `:time <expr>` | Measure execution time |
| `:help`, `:quit` | Help and exit |

Results of `Result` expressions are printed as `ok(...)` or `err(...)`; a bare `check` at the REPL top level prints the error instead of returning.

---

## 5. Remote & Notebook Use

* `aethi --listen 127.0.0.1:7777` exposes a JSON-RPC session for editor integrations (see `LSP_SPECIFICATION.md`).
* A Jupyter-compatible kernel (`aeth-kernel`) allows inline evaluation of `.aeth` cells, including tensor visualization.

## 6. Configuration

```toml
[dev]
port = 3000
watch = ["src", "assets"]
ignore = ["target", "*.tmp"]
overlay_errors = true
```
