#ifndef AETH_TYPECHECKER_H
#define AETH_TYPECHECKER_H

#include "ast.h"

typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_UNIT,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_IPTR,
    TYPE_UPTR,
    TYPE_F32,
    TYPE_F64,
    TYPE_BOOL,
    TYPE_TEXT,
    TYPE_POINTER
} TypeKind;

typedef struct Type {
    TypeKind kind;
    struct Type *base; // for pointers
} Type;

typedef struct Symbol {
    char *name;
    Type type;
    bool is_mutable;
    bool is_const;
} Symbol;

typedef struct SymbolTable {
    Symbol *symbols;
    size_t count;
    size_t capacity;
    struct SymbolTable *parent;
} SymbolTable;

typedef struct {
    TargetArch target;
    SymbolTable *current_scope;
    bool had_error;
} TypeChecker;

void typechecker_init(TypeChecker *tc, TargetArch target);
void typechecker_free(TypeChecker *tc);

Type string_to_type(const char *name, TargetArch target);
bool types_equal(Type t1, Type t2);
const char* type_to_string(Type t);

Type typecheck_expression(TypeChecker *tc, Expr *expr);
bool typecheck_statement(TypeChecker *tc, Stmt *stmt);
bool typecheck_program(TypeChecker *tc, Program *program);

#endif
