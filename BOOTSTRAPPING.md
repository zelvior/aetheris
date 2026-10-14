# Aetheris Bootstrapping Architecture

## Bootstrap Compiler (`aethc`)
The bootstrap compiler toolchain is implemented in **ISO C11 (`gcc -std=c11`)** without third-party libraries.

## Build Steps
```bash
# Compile local bootstrap binary
make bootstrap

# Verify toolchain version
./build/bin/aeth --version
```
