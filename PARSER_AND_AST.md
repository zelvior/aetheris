# Aetheris Parser & Abstract Syntax Tree (AST) Specification (v1.0)

The **Aetheris Parser (`aethc parser`)** translates the flat token stream produced by the lexer into a hierarchical, strongly typed Abstract Syntax Tree (AST). It employs a robust Pratt parsing algorithm combined with deterministic tilde terminator (`~`) boundary checks to eliminate syntax ambiguity across all multi-target domains.

---

## 1. Parsing Architecture & Pipeline

```
       [ Token Stream ]
              │
              ▼
    ┌───────────────────┐
    │  Pratt Parser     │ ──► Handles operator precedence & expressions
    └─────────┬─────────┘
              │
              ▼
    ┌───────────────────┐
    │  AST Builder      │ ──► Constructs structured nodes (`fn`, `struct`, `route`, `component`)
    └─────────┬─────────┘
              │
              ▼
    [ Abstract Syntax Tree (AST) ]
```

1. **Lexical Token Input**: Consumes tokens tagged with line and column markers.
2. **Pratt Expression Parsing**: Resolves binary and unary operator precedence (ranging from rank 1 to 11 as defined in keywords/operators specs).
3. **Statement Boundary Enforcement**: Verifies that every statement closes with the mandatory tilde terminator (`~`).

---

## 2. Core AST Node Structures

Every syntactic construct in Aetheris maps to a standardized node structure in memory. Below is the structural representation of primary AST node categories:

### A. Variable Declaration Node (`VarDeclNode`)
```json
{
  "type": "VarDeclNode",
  "isMutable": false,
  "identifier": "maxCount",
  "dataType": "u32",
  "initializer": {
    "type": "LiteralNode",
    "value": 1024,
    "literalType": "u32"
  }
}
```

### B. Function Declaration Node (`FnDeclNode`)
```json
{
  "type": "FnDeclNode",
  "identifier": "getOffset",
  "parameters": [
    { "name": "base", "type": "uptr" },
    { "name": "index", "type": "u32" }
  ],
  "returnType": "uptr",
  "body": [
    {
      "type": "ReturnNode",
      "expression": {
        "type": "BinaryOpNode",
        "operator": "+",
        "left": { "type": "IdentifierNode", "name": "base" },
        "right": {
          "type": "BinaryOpNode",
          "operator": "*",
          "left": { "type": "IdentifierNode", "name": "index" },
          "right": { "type": "LiteralNode", "value": 4 }
        }
      }
    }
  ]
}
```

### C. Domain-Specific Nodes (Web, Backend, AI)

* **`RouteNode`**: Encapsulates API endpoints (`route GET "/path" ~`).
* **`ComponentNode`**: Encapsulates reactive UI nodes (`component Name ~ ... end`).
* **`NeuralNode`**: Encapsulates AI model layers and training loops (`neural Name ~ ... end`).

---

## 3. Pratt Parser Implementation Rules

* **Prefix Parsers**: Handle literals, identifiers, unary operators (`-`, `not`, `~`, `&`, `*`, `check`).
* **Infix Parsers**: Handle binary operations (`+`, `-`, `*`, `/`, `==`, `and`, `or`) based on their binding power (precedence rank).
* **Terminator Sync**: After parsing any expression statement, the parser asserts the presence of `TOKEN_TERMINATOR` (`~`), raising a syntax diagnostic if omitted.