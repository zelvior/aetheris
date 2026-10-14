# Aetheris Verification & Testing Framework

## Running Test Suite
Execute the automated C test harness:
```bash
make test
```

## Test Coverage Modules
- `tests/test_lexer.c`: Token scanning and statement boundary disambiguation.
- `tests/test_parser.c`: Expression precedence, functions, and statements.
- `tests/test_typechecker.c`: Architecture-aware pointer resolution (`32-bit` vs `64-bit`).
- `tests/test_ir.c`: Linear IR generation.
- `tests/test_codegen.c`: Standard C99 code generation.
- `tests/test_interpreter.c`: In-memory AST evaluation.
- `tests/test_frontend_backend.c`: Components, routes, and reply statements.
- `tests/test_webgen.c`: Native web application bundle creation.
- `tests/test_async_ai.c`: Async/await routines, spawn, and neural model inference.
- `tests/test_stdlib.c`: Standard library math, string, and I/O modules.
