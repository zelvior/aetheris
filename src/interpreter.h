#ifndef AETH_INTERPRETER_H
#define AETH_INTERPRETER_H

#include "ast.h"
#include "typechecker.h"

typedef enum {
    VAL_UNIT,
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_STRING
} ValueKind;

typedef struct {
    ValueKind kind;
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        char *str_val;
    };
} Value;

typedef struct EnvVar {
    char *name;
    Value value;
} EnvVar;

typedef struct Environment {
    EnvVar *vars;
    size_t count;
    size_t capacity;
    struct Environment *parent;
} Environment;

typedef struct {
    Environment *global_env;
    Environment *current_env;
    Program *program;
    bool has_returned;
    Value return_value;
} Interpreter;

void interpreter_init(Interpreter *interp);
void interpreter_free(Interpreter *interp);

Value eval_expression(Interpreter *interp, Expr *expr);
bool eval_statement(Interpreter *interp, Stmt *stmt);
Value eval_program(Interpreter *interp, Program *program);

#endif
