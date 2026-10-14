# Aetheris Cryptography & Security Specification (v1.0)

This document defines `std.crypto`, secure-coding features in the language, and the security posture of the toolchain.

---

## 1. Principles

1. **Safe Defaults**: APIs make the secure choice the easy choice (authenticated encryption, constant-time comparison).
2. **Constant-Time by Construction**: Secret-dependent branches and indexes are rejected by the `secret` type system.
3. **No Roll-Your-Own**: Low-level primitives live under `std.crypto.raw` and require an explicit `unsafe crypto` block.
4. **Everything Returns `Result`**: Failures never silently produce weak output.

---

## 2. Algorithm Catalog

| Category | Algorithms |
| :--- | :--- |
| Hashing | SHA-256, SHA-512, SHA3-256, BLAKE3 |
| MAC | HMAC-SHA256, Poly1305 |
| Symmetric AEAD | ChaCha20-Poly1305 (default), AES-256-GCM |
| Key derivation | HKDF, Argon2id, scrypt, PBKDF2 |
| Signatures | Ed25519 (default), ECDSA P-256 |
| Key exchange | X25519, ECDH P-256 |
| Randomness | OS CSPRNG (`getrandom`, `BCryptGenRandom`, hardware TRNG on embedded) |

Deprecated algorithms (MD5, SHA-1, DES, RC4) are available only under `std.crypto.legacy` and emit warning `S100`.

---

## 3. Basic Usage

```aeth
import { hash, aead, random, kdf } from "std.crypto" ~

fn digest(data: list[u8]) -> list[u8] ~
    return hash.sha256(data) ~
end

fn seal(key: aead.Key, plaintext: list[u8]) -> Result[list[u8], text] ~
    let nonce = check random.bytes(12) or err("RNG unavailable") ~
    let box = check aead.chacha20poly1305.seal(key, nonce, plaintext) ~
    return ok(nonce + box) ~
end

fn hashPassword(pw: text) -> Result[text, text] ~
    return kdf.argon2id.hash(pw, memory: 64 * 1024, iterations: 3, lanes: 4) ~
end
```

Argon2id hashes are returned as self-describing PHC strings including parameters and salt.

---

## 4. The `secret` Type Qualifier

```aeth
fn verify(provided: secret[list[u8]], expected: secret[list[u8]]) -> bool ~
    return constantTimeEq(provided, expected) ~     // allowed
    // if provided[0] == 0 ~ ... end                // ERROR S201: branch on secret
end
```

| Rule | Enforcement |
| :--- | :--- |
| No branching on `secret` values | Compile-time error |
| No indexing with `secret` indices | Compile-time error |
| Zeroized on scope exit | Automatic (`volatile` writes, not optimized away) |
| No printing or logging | `print` and logger reject `secret` types |
| Declassification | Only via `declassify(x)` inside `unsafe crypto` |

---

## 5. Memory Safety Features

* Bounds checking is on by default; `--unchecked` is permitted only for modules marked `@trusted`.
* Stack canaries and control-flow integrity are enabled for `native-64` release builds.
* ASLR/PIE output by default; W^X enforced for generated code.
* `SecureBuffer` allocates locked, non-swappable pages (`mlock`) and zeroes on free.

---

## 6. TLS & Network Security

`std.net.tls` provides TLS 1.3 only. Certificate validation is mandatory; disabling it requires `tls.insecureForTesting()` which is a compile error in release builds. Embedded targets can use PSK mode or a minimal certificate store.

---

## 7. Supply-Chain Security

| Feature | Description |
| :--- | :--- |
| Lockfile & checksums | Every dependency is pinned by SHA-256 |
| Signed packages | Ed25519 publisher signatures verified by `aeth` |
| Reproducible builds | Same inputs produce identical binaries |
| SBOM | `aeth sbom` emits a CycloneDX bill of materials |
| Audit | `aeth audit` checks dependencies against advisory databases |

## 8. Vulnerability Reporting

Report security issues privately to the address listed in `SECURITY` contact info in the repository; do not open public issues. Maintainers acknowledge reports within 72 hours.

## 9. Disclaimer

Cryptographic code requires independent expert review before use in high-stakes systems.
