# Aetheris Intermediate Representation (IR) Specification

The Aetheris IR is a strongly-typed, linear SSA-like instruction representation used prior to backend lowering.

## Instructions
- `ALLOC <var>`: Stack frame allocation.
- `LOAD <dest>, <src>`: Variable load into register.
- `STORE <target>, <val>`: Variable store from register.
- `BINOP <dest>, <op>, <lhs>, <rhs>`: Arithmetic/logical evaluation.
- `UNOP <dest>, <op>, <src>`: Unary evaluation.
- `LABEL <name>`: Branch destination marker.
- `BRANCH <cond>, <target>`: Conditional jump.
- `JUMP <target>`: Unconditional jump.
- `CALL <dest>, <func>`: Function frame invocation.
- `RETURN <val>`: Stack frame return.
