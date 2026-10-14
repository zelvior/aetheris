# Aetheris Language Server Protocol Specification (v1.0)

`aeth-lsp` is the language server for Aetheris. It implements LSP 3.17 and reuses the compiler front end (lexer, parser, type checker) for accurate, incremental analysis.

---

## 1. Architecture

```
   Editor (VS Code / Neovim / JetBrains / Helix)
                │   JSON-RPC over stdio or TCP
                ▼
         ┌────────────┐
         │  aeth-lsp  │
         ├────────────┤
         │ Document store (rope, versioned)
         │ Incremental lexer + parser (error-tolerant)
         │ Salsa-style query cache: parse → resolve → typecheck
         │ Target-aware analysis (32 / 64 / web / ai)
         └────────────┘
```

* **Error-Tolerant Parsing**: A missing `~` or `end` produces an AST with error nodes so features keep working while typing.
* **Incremental**: Only affected queries are recomputed; typical edit-to-diagnostic latency is under 50 ms.
* **Target-Aware**: The active target from `aeth.toml` (or a selected profile) determines `uptr` size, available domain keywords, and diagnostics.

```bash
aeth lsp --stdio
```

---

## 2. Supported Capabilities

| LSP Feature | Behavior |
| :--- | :--- |
| `textDocument/publishDiagnostics` | Syntax, type, `Result`-handling, A11Y, and lint diagnostics with codes |
| `completion` | Keywords, locals, members, imports, struct fields, enum cases, snippets |
| `hover` | Inferred type, doc comments, size and alignment per target |
| `signatureHelp` | Parameter hints, including generics |
| `definition`, `typeDefinition`, `implementation` | Navigate across modules and imports |
| `references`, `rename` | Workspace-wide, scope-aware |
| `documentSymbol`, `workspaceSymbol` | Outline and global search |
| `formatting`, `rangeFormatting` | Delegates to `aeth fmt` |
| `codeAction` | Quick fixes and refactors |
| `inlayHint` | Inferred types, parameter names, `?` propagation points |
| `semanticTokens` | Accurate highlighting (disambiguates `~` terminator vs bitwise NOT) |
| `foldingRange`, `selectionRange` | Block-aware (`fn ... end`) |
| `callHierarchy` | Incoming and outgoing calls |
| `codeLens` | "Run test", "Run main", reference counts |
| `diagnostic` pull model | Supported alongside push |

---

## 3. Diagnostics Example

```json
{
  "range": { "start": { "line": 11, "character": 28 }, "end": { "line": 11, "character": 29 } },
  "severity": 1,
  "code": "E014",
  "source": "aethc",
  "message": "Expected statement terminator '~'",
  "relatedInformation": []
}
```

### Quick Fixes

| Diagnostic | Action |
| :--- | :--- |
| Missing `~` | Insert terminator |
| Missing `end` | Insert `end` at the matching indentation |
| Unhandled `Result` | Wrap with `check`, `or <fallback>`, or `match` |
| Implicit widening | Insert `as <type>` cast |
| Unused variable | Remove or prefix with `_` |
| Missing import | Add `import { ... } from ...` |

---

## 4. Custom Requests & Notifications

| Method | Purpose |
| :--- | :--- |
| `aeth/expandMacro` | Show macro or `@derive` expansion |
| `aeth/viewIr` | SSA IR for a function |
| `aeth/viewAsm` | Assembly for current target |
| `aeth/targetInfo` | Pointer width, alignment, layout of a type |
| `aeth/switchTarget` | Change analysis target at runtime |
| `aeth/runTest` | Execute test under cursor |
| `aeth/memoryReport` | Static stack/arena usage for embedded builds |
| `$/aeth/progress` | Indexing and build progress |

---

## 5. Configuration

Settings are read from `aeth.toml` (`[lsp]`) and editor `initializationOptions`:

```toml
[lsp]
target = "native-64"
diagnostics_delay_ms = 100
inlay_hints = { types = true, params = true, propagation = true }
format_on_save = true
lint = "recommended"
```

## 6. Editor Packages

| Editor | Package |
| :--- | :--- |
| VS Code | `aetheris-lang` extension (grammar, snippets, debug adapter, LSP client) |
| Neovim | `nvim-lspconfig` entry + Tree-sitter grammar `tree-sitter-aeth` |
| JetBrains | `Aetheris` plugin using LSP4IJ |
| Helix / Zed / Emacs | Config snippets in the repository |

## 7. Performance & Limits

* Workspace index is persisted in `.aeth/index` and revalidated by content hash.
* Large projects (>10k files) use lazy module loading; memory target under 500 MB.
* Requests are cancellable; stale results are dropped on newer document versions.
