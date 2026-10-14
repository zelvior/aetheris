# Aetheris Tooling & Ecosystem

The unified `aeth` CLI provides developer tooling in a single binary:

## Subcommands
- `aeth run <file.aeth>`: Evaluates Aetheris scripts in the tree-walk interpreter.
- `aeth build <file.aeth> [--target=native-32|native-64|web]`: Transpiles source to target outputs.
- `aeth dev [file.aeth]`: Starts the dev server and generates web assets in `dist/`.
- `aeth repl`: Launches an interactive Aetheris evaluation shell.
- `aeth --version`: Displays toolchain version and target build information.
