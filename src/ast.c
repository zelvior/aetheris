#define _POSIX_C_SOURCE 200809L
#include "ast.h"

Expr* ast_create_int_literal(int64_t val, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_INT_LITERAL;
    e->location = loc;
    e->int_val = val;
    return e;
}

Expr* ast_create_float_literal(double val, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_FLOAT_LITERAL;
    e->location = loc;
    e->float_val = val;
    return e;
}

Expr* ast_create_text_literal(const char *val, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_TEXT_LITERAL;
    e->location = loc;
    e->text_val = strdup(val ? val : "");
    return e;
}

Expr* ast_create_bool_literal(bool val, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_BOOL_LITERAL;
    e->location = loc;
    e->bool_val = val;
    return e;
}

Expr* ast_create_identifier(const char *name, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_IDENTIFIER;
    e->location = loc;
    e->identifier_name = strdup(name ? name : "");
    return e;
}

Expr* ast_create_unary(TokenType op, Expr *operand, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_UNARY;
    e->location = loc;
    e->unary.op = op;
    e->unary.operand = operand;
    return e;
}

Expr* ast_create_binary(TokenType op, Expr *left, Expr *right, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_BINARY;
    e->location = loc;
    e->binary.op = op;
    e->binary.left = left;
    e->binary.right = right;
    return e;
}

Expr* ast_create_call(const char *callee, Expr **args, size_t arg_count, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_CALL;
    e->location = loc;
    e->call.callee = strdup(callee ? callee : "");
    e->call.args = args;
    e->call.arg_count = arg_count;
    return e;
}

Expr* ast_create_assignment(const char *target, Expr *value, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_ASSIGNMENT;
    e->location = loc;
    e->assignment.target_name = strdup(target ? target : "");
    e->assignment.value = value;
    return e;
}

Expr* ast_create_show(const char *widget_name, Expr **props, size_t prop_count, SourceLocation loc) {
    Expr *e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_SHOW;
    e->location = loc;
    e->show.widget_name = strdup(widget_name ? widget_name : "");
    e->show.props = props;
    e->show.prop_count = prop_count;
    return e;
}

Stmt* ast_create_expr_stmt(Expr *expr, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_EXPR;
    s->location = loc;
    s->expr = expr;
    return s;
}

Stmt* ast_create_var_decl(const char *name, bool is_mut, bool is_const, TypeAnnotation type_ann, Expr *init, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_VAR_DECL;
    s->location = loc;
    s->var_decl.name = strdup(name ? name : "");
    s->var_decl.is_mutable = is_mut;
    s->var_decl.is_const = is_const;
    s->var_decl.type_annotation.name = type_ann.name ? strdup(type_ann.name) : strdup("unknown");
    s->var_decl.type_annotation.is_pointer = type_ann.is_pointer;
    s->var_decl.initializer = init;
    return s;
}

Stmt* ast_create_return_stmt(Expr *value, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_RETURN;
    s->location = loc;
    s->return_stmt.value = value;
    return s;
}

Stmt* ast_create_if_stmt(Expr *cond, Stmt *then_b, Stmt *else_b, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_IF;
    s->location = loc;
    s->if_stmt.condition = cond;
    s->if_stmt.then_branch = then_b;
    s->if_stmt.else_branch = else_b;
    return s;
}

Stmt* ast_create_while_stmt(Expr *cond, Stmt *body, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_WHILE;
    s->location = loc;
    s->while_stmt.condition = cond;
    s->while_stmt.body = body;
    return s;
}

Stmt* ast_create_block_stmt(Stmt **stmts, size_t count, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_BLOCK;
    s->location = loc;
    s->block.stmts = stmts;
    s->block.stmt_count = count;
    return s;
}

Stmt* ast_create_func_decl(const char *name, Param *params, size_t param_count, TypeAnnotation ret_type, Stmt *body, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_FUNCTION_DECL;
    s->location = loc;
    s->func_decl.name = strdup(name ? name : "");
    s->func_decl.params = params;
    s->func_decl.param_count = param_count;
    s->func_decl.return_type.name = ret_type.name ? strdup(ret_type.name) : strdup("unit");
    s->func_decl.return_type.is_pointer = ret_type.is_pointer;
    s->func_decl.body = body;
    return s;
}

Stmt* ast_create_component_decl(const char *name, Param *props, size_t prop_count, Stmt *render_body, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_COMPONENT_DECL;
    s->location = loc;
    s->component_decl.name = strdup(name ? name : "");
    s->component_decl.props = props;
    s->component_decl.prop_count = prop_count;
    s->component_decl.render_body = render_body;
    return s;
}

Stmt* ast_create_route_decl(const char *method, const char *path, Stmt *handler_body, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_ROUTE_DECL;
    s->location = loc;
    s->route_decl.method = strdup(method ? method : "GET");
    s->route_decl.path = strdup(path ? path : "/");
    s->route_decl.handler_body = handler_body;
    return s;
}

Stmt* ast_create_reply_stmt(Expr *response_expr, int status_code, SourceLocation loc) {
    Stmt *s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_REPLY;
    s->location = loc;
    s->reply_stmt.response_expr = response_expr;
    s->reply_stmt.status_code = status_code;
    return s;
}

Program* ast_create_program(Stmt **stmts, size_t count) {
    Program *p = (Program*)malloc(sizeof(Program));
    p->stmts = stmts;
    p->stmt_count = count;
    return p;
}

void free_expr(Expr *expr) {
    if (!expr) return;
    switch (expr->type) {
        case EXPR_TEXT_LITERAL:
            free(expr->text_val);
            break;
        case EXPR_IDENTIFIER:
            free(expr->identifier_name);
            break;
        case EXPR_UNARY:
            free_expr(expr->unary.operand);
            break;
        case EXPR_BINARY:
            free_expr(expr->binary.left);
            free_expr(expr->binary.right);
            break;
        case EXPR_CALL:
            free(expr->call.callee);
            for (size_t i = 0; i < expr->call.arg_count; i++) {
                free_expr(expr->call.args[i]);
            }
            free(expr->call.args);
            break;
        case EXPR_ASSIGNMENT:
            free(expr->assignment.target_name);
            free_expr(expr->assignment.value);
            break;
        case EXPR_SHOW:
            free(expr->show.widget_name);
            for (size_t i = 0; i < expr->show.prop_count; i++) {
                free_expr(expr->show.props[i]);
            }
            free(expr->show.props);
            break;
        default:
            break;
    }
    free(expr);
}

void free_stmt(Stmt *stmt) {
    if (!stmt) return;
    switch (stmt->type) {
        case STMT_EXPR:
            free_expr(stmt->expr);
            break;
        case STMT_VAR_DECL:
            free(stmt->var_decl.name);
            free(stmt->var_decl.type_annotation.name);
            free_expr(stmt->var_decl.initializer);
            break;
        case STMT_RETURN:
            free_expr(stmt->return_stmt.value);
            break;
        case STMT_IF:
            free_expr(stmt->if_stmt.condition);
            free_stmt(stmt->if_stmt.then_branch);
            free_stmt(stmt->if_stmt.else_branch);
            break;
        case STMT_WHILE:
            free_expr(stmt->while_stmt.condition);
            free_stmt(stmt->while_stmt.body);
            break;
        case STMT_BLOCK:
            for (size_t i = 0; i < stmt->block.stmt_count; i++) {
                free_stmt(stmt->block.stmts[i]);
            }
            free(stmt->block.stmts);
            break;
        case STMT_FUNCTION_DECL:
            free(stmt->func_decl.name);
            for (size_t i = 0; i < stmt->func_decl.param_count; i++) {
                free(stmt->func_decl.params[i].name);
                free(stmt->func_decl.params[i].type_annotation.name);
            }
            free(stmt->func_decl.params);
            free(stmt->func_decl.return_type.name);
            free_stmt(stmt->func_decl.body);
            break;
        case STMT_COMPONENT_DECL:
            free(stmt->component_decl.name);
            for (size_t i = 0; i < stmt->component_decl.prop_count; i++) {
                free(stmt->component_decl.props[i].name);
                free(stmt->component_decl.props[i].type_annotation.name);
            }
            free(stmt->component_decl.props);
            free_stmt(stmt->component_decl.render_body);
            break;
        case STMT_ROUTE_DECL:
            free(stmt->route_decl.method);
            free(stmt->route_decl.path);
            free_stmt(stmt->route_decl.handler_body);
            break;
        case STMT_REPLY:
            free_expr(stmt->reply_stmt.response_expr);
            break;
    }
    free(stmt);
}

void free_program(Program *program) {
    if (!program) return;
    for (size_t i = 0; i < program->stmt_count; i++) {
        free_stmt(program->stmts[i]);
    }
    free(program->stmts);
    free(program);
}
