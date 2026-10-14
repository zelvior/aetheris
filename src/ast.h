#ifndef AETH_AST_H
#define AETH_AST_H

#include "lexer.h"

typedef enum {
    EXPR_INT_LITERAL,
    EXPR_FLOAT_LITERAL,
    EXPR_TEXT_LITERAL,
    EXPR_BOOL_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_CALL,
    EXPR_ASSIGNMENT
} ExprType;

typedef enum {
    STMT_EXPR,
    STMT_VAR_DECL,
    STMT_RETURN,
    STMT_IF,
    STMT_WHILE,
    STMT_BLOCK,
    STMT_FUNCTION_DECL
} StmtType;

typedef struct Expr Expr;
typedef struct Stmt Stmt;

typedef struct {
    char *name;
    bool is_pointer;
} TypeAnnotation;

struct Expr {
    ExprType type;
    SourceLocation location;
    union {
        int64_t int_val;
        double float_val;
        char *text_val;
        bool bool_val;
        char *identifier_name;
        struct {
            TokenType op;
            Expr *operand;
        } unary;
        struct {
            TokenType op;
            Expr *left;
            Expr *right;
        } binary;
        struct {
            char *callee;
            Expr **args;
            size_t arg_count;
        } call;
        struct {
            char *target_name;
            Expr *value;
        } assignment;
    };
};

typedef struct {
    char *name;
    TypeAnnotation type_annotation;
} Param;

struct Stmt {
    StmtType type;
    SourceLocation location;
    union {
        Expr *expr;
        struct {
            char *name;
            bool is_mutable;
            bool is_const;
            TypeAnnotation type_annotation;
            Expr *initializer;
        } var_decl;
        struct {
            Expr *value;
        } return_stmt;
        struct {
            Expr *condition;
            Stmt *then_branch;
            Stmt *else_branch;
        } if_stmt;
        struct {
            Expr *condition;
            Stmt *body;
        } while_stmt;
        struct {
            Stmt **stmts;
            size_t stmt_count;
        } block;
        struct {
            char *name;
            Param *params;
            size_t param_count;
            TypeAnnotation return_type;
            Stmt *body;
        } func_decl;
    };
};

typedef struct {
    Stmt **stmts;
    size_t stmt_count;
} Program;

// AST constructors
Expr* ast_create_int_literal(int64_t val, SourceLocation loc);
Expr* ast_create_float_literal(double val, SourceLocation loc);
Expr* ast_create_text_literal(const char *val, SourceLocation loc);
Expr* ast_create_bool_literal(bool val, SourceLocation loc);
Expr* ast_create_identifier(const char *name, SourceLocation loc);
Expr* ast_create_unary(TokenType op, Expr *operand, SourceLocation loc);
Expr* ast_create_binary(TokenType op, Expr *left, Expr *right, SourceLocation loc);
Expr* ast_create_call(const char *callee, Expr **args, size_t arg_count, SourceLocation loc);
Expr* ast_create_assignment(const char *target, Expr *value, SourceLocation loc);

Stmt* ast_create_expr_stmt(Expr *expr, SourceLocation loc);
Stmt* ast_create_var_decl(const char *name, bool is_mut, bool is_const, TypeAnnotation type_ann, Expr *init, SourceLocation loc);
Stmt* ast_create_return_stmt(Expr *value, SourceLocation loc);
Stmt* ast_create_if_stmt(Expr *cond, Stmt *then_b, Stmt *else_b, SourceLocation loc);
Stmt* ast_create_while_stmt(Expr *cond, Stmt *body, SourceLocation loc);
Stmt* ast_create_block_stmt(Stmt **stmts, size_t count, SourceLocation loc);
Stmt* ast_create_func_decl(const char *name, Param *params, size_t param_count, TypeAnnotation ret_type, Stmt *body, SourceLocation loc);

Program* ast_create_program(Stmt **stmts, size_t count);

void free_expr(Expr *expr);
void free_stmt(Stmt *stmt);
void free_program(Program *program);

#endif
