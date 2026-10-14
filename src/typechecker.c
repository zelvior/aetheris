#define _POSIX_C_SOURCE 200809L
#include "typechecker.h"

static SymbolTable* symbol_table_create(SymbolTable *parent) {
    SymbolTable *st = (SymbolTable*)malloc(sizeof(SymbolTable));
    st->count = 0;
    st->capacity = 8;
    st->symbols = (Symbol*)malloc(sizeof(Symbol) * st->capacity);
    st->parent = parent;
    return st;
}

static void symbol_table_free(SymbolTable *st) {
    if (!st) return;
    for (size_t i = 0; i < st->count; i++) {
        free(st->symbols[i].name);
        if (st->symbols[i].type.base) {
            free(st->symbols[i].type.base);
        }
    }
    free(st->symbols);
    free(st);
}

static void symbol_table_insert(SymbolTable *st, const char *name, Type type, bool is_mut, bool is_const) {
    if (st->count >= st->capacity) {
        st->capacity *= 2;
        st->symbols = (Symbol*)realloc(st->symbols, sizeof(Symbol) * st->capacity);
    }
    st->symbols[st->count].name = strdup(name);
    st->symbols[st->count].type = type;
    st->symbols[st->count].is_mutable = is_mut;
    st->symbols[st->count].is_const = is_const;
    st->count++;
}

static Symbol* symbol_table_lookup(SymbolTable *st, const char *name) {
    SymbolTable *curr = st;
    while (curr) {
        for (size_t i = 0; i < curr->count; i++) {
            if (strcmp(curr->symbols[i].name, name) == 0) {
                return &curr->symbols[i];
            }
        }
        curr = curr->parent;
    }
    return NULL;
}

void typechecker_init(TypeChecker *tc, TargetArch target) {
    tc->target = target;
    tc->current_scope = symbol_table_create(NULL);
    tc->had_error = false;
}

void typechecker_free(TypeChecker *tc) {
    while (tc->current_scope) {
        SymbolTable *parent = tc->current_scope->parent;
        symbol_table_free(tc->current_scope);
        tc->current_scope = parent;
    }
}

Type string_to_type(const char *name, TargetArch target) {
    Type t = {TYPE_UNKNOWN, NULL};
    if (!name) return t;

    if (strcmp(name, "i8") == 0) t.kind = TYPE_I8;
    else if (strcmp(name, "i16") == 0) t.kind = TYPE_I16;
    else if (strcmp(name, "i32") == 0) t.kind = TYPE_I32;
    else if (strcmp(name, "i64") == 0) t.kind = TYPE_I64;
    else if (strcmp(name, "u8") == 0) t.kind = TYPE_U8;
    else if (strcmp(name, "u16") == 0) t.kind = TYPE_U16;
    else if (strcmp(name, "u32") == 0) t.kind = TYPE_U32;
    else if (strcmp(name, "u64") == 0) t.kind = TYPE_U64;
    else if (strcmp(name, "iptr") == 0) t.kind = TYPE_IPTR;
    else if (strcmp(name, "uptr") == 0) t.kind = TYPE_UPTR;
    else if (strcmp(name, "f32") == 0) t.kind = TYPE_F32;
    else if (strcmp(name, "f64") == 0) t.kind = TYPE_F64;
    else if (strcmp(name, "bool") == 0) t.kind = TYPE_BOOL;
    else if (strcmp(name, "text") == 0) t.kind = TYPE_TEXT;
    else if (strcmp(name, "unit") == 0) t.kind = TYPE_UNIT;
    else t.kind = TYPE_UNKNOWN;

    if (target == TARGET_NATIVE_32) {
        if (t.kind == TYPE_IPTR) t.kind = TYPE_I32;
        if (t.kind == TYPE_UPTR) t.kind = TYPE_U32;
    } else if (target == TARGET_NATIVE_64) {
        if (t.kind == TYPE_IPTR) t.kind = TYPE_I64;
        if (t.kind == TYPE_UPTR) t.kind = TYPE_U64;
    }

    return t;
}

bool types_equal(Type t1, Type t2) {
    if (t1.kind != t2.kind) return false;
    if (t1.kind == TYPE_POINTER) {
        if (!t1.base || !t2.base) return false;
        return types_equal(*t1.base, *t2.base);
    }
    return true;
}

const char* type_to_string(Type t) {
    switch (t.kind) {
        case TYPE_UNIT: return "unit";
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_IPTR: return "iptr";
        case TYPE_UPTR: return "uptr";
        case TYPE_F32: return "f32";
        case TYPE_F64: return "f64";
        case TYPE_BOOL: return "bool";
        case TYPE_TEXT: return "text";
        case TYPE_POINTER: return "pointer";
        default: return "unknown";
    }
}

Type typecheck_expression(TypeChecker *tc, Expr *expr) {
    Type t = {TYPE_UNKNOWN, NULL};
    if (!expr) return t;

    switch (expr->type) {
        case EXPR_INT_LITERAL:
            t.kind = TYPE_I32; // Default integer literal
            return t;
        case EXPR_FLOAT_LITERAL:
            t.kind = TYPE_F64;
            return t;
        case EXPR_TEXT_LITERAL:
            t.kind = TYPE_TEXT;
            return t;
        case EXPR_BOOL_LITERAL:
            t.kind = TYPE_BOOL;
            return t;
        case EXPR_IDENTIFIER: {
            Symbol *s = symbol_table_lookup(tc->current_scope, expr->identifier_name);
            if (!s) {
                fprintf(stderr, "[TypeError] Undefined variable '%s' at line %zu\n",
                        expr->identifier_name, expr->location.line);
                tc->had_error = true;
                return t;
            }
            return s->type;
        }
        case EXPR_UNARY: {
            Type operand_type = typecheck_expression(tc, expr->unary.operand);
            return operand_type;
        }
        case EXPR_BINARY: {
            Type left_t = typecheck_expression(tc, expr->binary.left);
            Type right_t = typecheck_expression(tc, expr->binary.right);

            if (expr->binary.op == TOKEN_EQ || expr->binary.op == TOKEN_NEQ ||
                expr->binary.op == TOKEN_LT || expr->binary.op == TOKEN_GT ||
                expr->binary.op == TOKEN_LTE || expr->binary.op == TOKEN_GTE) {
                t.kind = TYPE_BOOL;
                return t;
            }

            // Strict no silent widening: left and right types must match exactly
            if (!types_equal(left_t, right_t)) {
                fprintf(stderr, "[TypeError] Type mismatch in binary expression at line %zu: '%s' vs '%s'. Explicit widening required.\n",
                        expr->location.line, type_to_string(left_t), type_to_string(right_t));
                tc->had_error = true;
                return t;
            }
            return left_t;
        }
        case EXPR_CALL: {
            Symbol *s = symbol_table_lookup(tc->current_scope, expr->call.callee);
            if (s) {
                return s->type;
            }
            t.kind = TYPE_I32; // Default assume function return
            return t;
        }
        case EXPR_ASSIGNMENT: {
            Symbol *s = symbol_table_lookup(tc->current_scope, expr->assignment.target_name);
            if (!s) {
                fprintf(stderr, "[TypeError] Cannot assign to undeclared variable '%s'\n", expr->assignment.target_name);
                tc->had_error = true;
                return t;
            }
            Type val_t = typecheck_expression(tc, expr->assignment.value);
            if (!types_equal(s->type, val_t)) {
                fprintf(stderr, "[TypeError] Type mismatch in assignment to '%s'\n", expr->assignment.target_name);
                tc->had_error = true;
            }
            return s->type;
        }
    }

    return t;
}

bool typecheck_statement(TypeChecker *tc, Stmt *stmt) {
    if (!stmt) return true;

    switch (stmt->type) {
        case STMT_EXPR:
            typecheck_expression(tc, stmt->expr);
            return !tc->had_error;

        case STMT_VAR_DECL: {
            Type declared_t = string_to_type(stmt->var_decl.type_annotation.name, tc->target);
            if (stmt->var_decl.type_annotation.is_pointer) {
                Type *base = (Type*)malloc(sizeof(Type));
                *base = declared_t;
                declared_t.kind = TYPE_POINTER;
                declared_t.base = base;
            }

            if (stmt->var_decl.initializer) {
                Type init_t = typecheck_expression(tc, stmt->var_decl.initializer);
                if (declared_t.kind != TYPE_UNKNOWN && !types_equal(declared_t, init_t)) {
                    fprintf(stderr, "[TypeError] Variable '%s' declared as '%s' but initialized with '%s'\n",
                            stmt->var_decl.name, type_to_string(declared_t), type_to_string(init_t));
                    tc->had_error = true;
                }
            }

            symbol_table_insert(tc->current_scope, stmt->var_decl.name, declared_t, stmt->var_decl.is_mutable, stmt->var_decl.is_const);
            return !tc->had_error;
        }

        case STMT_RETURN:
            if (stmt->return_stmt.value) {
                typecheck_expression(tc, stmt->return_stmt.value);
            }
            return !tc->had_error;

        case STMT_IF:
            typecheck_expression(tc, stmt->if_stmt.condition);
            typecheck_statement(tc, stmt->if_stmt.then_branch);
            if (stmt->if_stmt.else_branch) {
                typecheck_statement(tc, stmt->if_stmt.else_branch);
            }
            return !tc->had_error;

        case STMT_WHILE:
            typecheck_expression(tc, stmt->while_stmt.condition);
            typecheck_statement(tc, stmt->while_stmt.body);
            return !tc->had_error;

        case STMT_BLOCK: {
            tc->current_scope = symbol_table_create(tc->current_scope);
            for (size_t i = 0; i < stmt->block.stmt_count; i++) {
                typecheck_statement(tc, stmt->block.stmts[i]);
            }
            SymbolTable *old = tc->current_scope;
            tc->current_scope = old->parent;
            symbol_table_free(old);
            return !tc->had_error;
        }

        case STMT_FUNCTION_DECL: {
            Type ret_t = string_to_type(stmt->func_decl.return_type.name, tc->target);
            symbol_table_insert(tc->current_scope, stmt->func_decl.name, ret_t, false, true);

            tc->current_scope = symbol_table_create(tc->current_scope);
            for (size_t i = 0; i < stmt->func_decl.param_count; i++) {
                Type pt = string_to_type(stmt->func_decl.params[i].type_annotation.name, tc->target);
                symbol_table_insert(tc->current_scope, stmt->func_decl.params[i].name, pt, false, false);
            }

            typecheck_statement(tc, stmt->func_decl.body);

            SymbolTable *old = tc->current_scope;
            tc->current_scope = old->parent;
            symbol_table_free(old);
            return !tc->had_error;
        }
    }

    return true;
}

bool typecheck_program(TypeChecker *tc, Program *program) {
    if (!program) return false;
    for (size_t i = 0; i < program->stmt_count; i++) {
        typecheck_statement(tc, program->stmts[i]);
    }
    return !tc->had_error;
}
