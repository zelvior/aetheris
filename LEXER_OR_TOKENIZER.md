# Aetheris Lexer & Tokenizer Specification (v1.0)

The **Aetheris Lexer (`aethc lexer`)** is the foundational front-end component of the compiler pipeline. It ingests raw UTF-8 source text stream (.aeth), strips out whitespace and comments, tracks exact line and column coordinates for precise diagnostic reporting, and converts the character stream into a dense, strongly typed token stream for the Pratt parser.

## 1. Lexical Architecture & Processing Pipeline

```
       [ Raw UTF-8 Source Text ]
                  │
                  ▼
        ┌───────────────────┐
        │ Character Stream  │ ──► Tracks cursor, line, & column position
        └─────────┬─────────┘
                  │
                  ▼
        ┌───────────────────┐
        │  Lexer Engine     │ ──► Matches keywords, literals, operators, & terminator `~`
        └─────────┬─────────┘
                  │
                  ▼
        [ Typed Token Stream ]
```

1. **UTF-8 Character Input**: Consumes source files character by character while managing multibyte code points safely.
2. **Whitespace & Comment Stripping**: Discards spaces, tabs, carriage returns, and comment blocks (`//` and `/* ... */`) while updating coordinate offsets.
3. **Greedy Token Matching**: Recognizes multi-character operators (`==`, `=>`, `**`, `<<`, `>>`, `<=`, `>=`, `!=`) and reserves keywords before falling back to identifiers and literals.
4. **Statement Boundary Generation**: Converts the mandatory tilde character (`~`) into the explicit `TOKEN_TERMINATOR` token.

---

## 2. Token Classification & Structure

Every token produced by the lexer is represented as a structured data packet containing its category, lexeme string value, and precise source file location.

### A. Token Struct Definition

```json
{
  "type": "TOKEN_KEYWORD",
  "lexeme": "let",
  "line": 14,
  "column": 4
}
```

### B. Core Token Categories

| Category Group | Associated Token Types | Description |
| :--- | :--- | :--- |
| **Keywords** | `TOKEN_LET`, `TOKEN_MUT`, `TOKEN_CONST`, `TOKEN_FN`, `TOKEN_RETURN`, `TOKEN_END`, `TOKEN_COMPONENT`, `TOKEN_ROUTE`, `TOKEN_NEURAL`, etc. | Reserved words defining control flow, scope, and domains. |
| **Identifiers** | `TOKEN_IDENTIFIER` | User-defined variable, function, or type names (`[a-z_][a-zA-Z0-9_]*`). |
| **Literals** | `TOKEN_INT_LITERAL`, `TOKEN_FLOAT_LITERAL`, `TOKEN_TEXT_LITERAL`, `TOKEN_BOOL_LITERAL` | Concrete scalar values with explicit or inferred types. |
| **Operators** | `TOKEN_PLUS`, `TOKEN_MINUS`, `TOKEN_STAR`, `TOKEN_SLASH`, `TOKEN_ASSIGN`, `TOKEN_EQ`, `TOKEN_ARROW`, etc. | Mathematical, logical, and relational calculation symbols. |
| **Delimiters** | `TOKEN_LPAREN`, `TOKEN_RPAREN`, `TOKEN_LBRACKET`, `TOKEN_RBRACKET`, `TOKEN_COMMA`, `TOKEN_COLON`, `TOKEN_DOT` | Grouping, parameter list, and member access boundaries. |
| **Terminator** | `TOKEN_TERMINATOR` (`~`) | Mandatory statement boundary marker. |

---

## 3. Lexical Rules & Edge Cases

### A. The Tilde Terminator (`~`)
The tilde character has a dual personality in Aetheris:
1. As an operator: **Bitwise NOT** (`~mask`).
2. As a terminator: **Statement End** (`let x = 10 ~`). 
The lexer disambiguates these contexts based on preceding tokens (e.g., if `~` follows an expression or literal value, it is tagged as `TOKEN_TERMINATOR`; if it precedes an identifier or expression operand, it is tagged as `TOKEN_BITWISE_NOT`).

### B. Number Literal Tagging
Numerical constants support explicit bit-width suffixes or standard decimal parsing:
* `1024` $\rightarrow$ `TOKEN_INT_LITERAL` (value: `1024`, default `i32`)
* `3.14159` $\rightarrow$ `TOKEN_FLOAT_LITERAL` (value: `3.14159`, default `f64`)
* `0x40020000` $\rightarrow$ `TOKEN_INT_LITERAL` (Hexadecimal parsing for systems/pointer addresses)

### C. Text String Buffers
Text literals are enclosed in double quotes (`"`), supporting UTF-8 character encoding and escape sequences (`\n`, `\t`, `\"`):
```text
"Server initialized successfully on port 8080"
```
The lexer captures the internal string content and wraps it into `TOKEN_TEXT_LITERAL`.

---

## 4. Error Diagnostics & Recovery

When the lexer encounters an unexpected or illegal character (such as an unclosed string literal or invalid symbol like `@` or `#`), it immediately halts local tokenization and issues a precise diagnostic error:

```text
Error[E001]: Unexpected character '@' encountered.
  --> main.aeth:12:24
   |
12 |     let userScore = 95 @ 5 ~
   |                        ^ Invalid token symbol in Aetheris syntax.