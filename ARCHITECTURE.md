# Aetheris Compiler Architecture

```
Aetheris Source (.aeth) ─► Lexer ─► Pratt Parser ─► Type Checker ─► SSA IR ─► Backends (C99 / WASM / CUDA)
```

## Subsystems
- `src/lexer.c`: Handcrafted UTF-8 character scanner.
- `src/parser.c`: Top-down operator precedence (Pratt) parser.
- `src/typechecker.c`: Scope symbol-table bidirectional type analyzer.
- `src/ir.c`: SSA-like intermediate representation generator.
- `src/codegen.c`: Zero-overhead C99 source transpiler.
- `src/interpreter.c`: In-memory AST tree-walk execution engine.
- `src/webgen.c`: Native Aetheris web generator.
- `src/stdlib.c`: Standard library module preloader.
