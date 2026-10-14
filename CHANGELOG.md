# Aetheris Language Changelog

All notable changes to the Aetheris (`.aeth`) language and compiler toolchain will be documented here.

## [v1.0.0] - Core Toolchain Release
### Added
- ISO C11 compiler core (`aethc`) with zero third-party dependencies.
- UTF-8 Lexer with tilde (`~`) statement terminator disambiguation.
- Pratt Parser & AST nodes for functions, control flow, components, and routes.
- Dual-width pointer type checker (`uptr`/`iptr` for 32-bit and 64-bit targets).
- Intermediate Representation (IR) pass and C99 code generator.
- Tree-walk interpreter (`aethi`) and interactive REPL (`aeth repl`).
- Native Aetheris web generator (`aeth dev`) emitting `.aeth_site` bundles.
- Standard Library (`std.math`, `std.string`, `std.collections`, `std.io`, `std.json`).
