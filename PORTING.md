# Porting Aetheris to New Architecture Targets

Aetheris (`.aeth`) is architecture-aware and designed for straightforward porting to new hardware targets.

## Porting Checklist
1. **Pointer Width**: Define `uptr` and `iptr` word boundaries (32-bit vs 64-bit) in `src/typechecker.c`.
2. **C99 Mapping**: Configure hardware primitives and alignment rules in `src/codegen.c`.
3. **IR Lowering**: Implement assembly/machine emitter passes for the target ISA in `src/ir.c`.
