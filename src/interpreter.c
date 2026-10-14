#define _POSIX_C_SOURCE 200809L
#include "interpreter.h"

static Environment* env_create(Environment *parent) {
    Environment *env = (Environment*)malloc(sizeof(Environment));
    env->count = 0;
    env->capacity = 8;
    env->vars = (EnvVar*)malloc(sizeof(EnvVar) * env->capacity);
    env->parent = parent;
    return env;
}

static void env_free(Environment *env) {
    if (!env) return;
    for (size_t i = 0; i < env->count; i++) {
        free(env->vars[i].name);
        if (env->vars[i].value.kind == VAL_STRING) {
            free(env->vars[i].value.str_val);
        }
    }
    free(env->vars);
    free(env);
}

static void env_assign(Environment *env, const char *name, Value val) {
    Environment *curr = env;
    while (curr) {
        for (size_t i = 0; i < curr->count; i++) {
            if (strcmp(curr->vars[i].name, name) == 0) {
                if (curr->vars[i].value.kind == VAL_STRING) free(curr->vars[i].value.str_val);
                curr->vars[i].value = val;
                if (val.kind == VAL_STRING) curr->vars[i].value.str_val = strdup(val.str_val);
                return;
            }
        }
        curr = curr->parent;
    }

    if (env->count >= env->capacity) {
        env->capacity *= 2;
        env->vars = (EnvVar*)realloc(env->vars, sizeof(EnvVar) * env->capacity);
    }
    env->vars[env->count].name = strdup(name);
    env->vars[env->count].value = val;
    if (val.kind == VAL_STRING) {
        env->vars[env->count].value.str_val = strdup(val.str_val);
    }
    env->count++;
}

static Value* env_get(Environment *env, const char *name) {
    Environment *curr = env;
    while (curr) {
        for (size_t i = 0; i < curr->count; i++) {
            if (strcmp(curr->vars[i].name, name) == 0) {
                return &curr->vars[i].value;
            }
        }
        curr = curr->parent;
    }
    return NULL;
}

void interpreter_init(Interpreter *interp) {
    interp->global_env = env_create(NULL);
    interp->current_env = interp->global_env;
    interp->program = NULL;
    interp->has_returned = false;
    interp->return_value.kind = VAL_UNIT;
}

void interpreter_free(Interpreter *interp) {
    while (interp->current_env) {
        Environment *parent = interp->current_env->parent;
        env_free(interp->current_env);
        interp->current_env = parent;
    }
}

Value eval_expression(Interpreter *interp, Expr *expr) {
    Value v;
    v.kind = VAL_UNIT;
    if (!expr) return v;

    switch (expr->type) {
        case EXPR_INT_LITERAL:
            v.kind = VAL_INT;
            v.int_val = expr->int_val;
            return v;
        case EXPR_FLOAT_LITERAL:
            v.kind = VAL_FLOAT;
            v.float_val = expr->float_val;
            return v;
        case EXPR_TEXT_LITERAL:
            v.kind = VAL_STRING;
            v.str_val = expr->text_val;
            return v;
        case EXPR_BOOL_LITERAL:
            v.kind = VAL_BOOL;
            v.bool_val = expr->bool_val;
            return v;
        case EXPR_IDENTIFIER: {
            Value *val = env_get(interp->current_env, expr->identifier_name);
            if (val) return *val;
            fprintf(stderr, "[RuntimeError] Undefined variable '%s'\n", expr->identifier_name);
            return v;
        }
        case EXPR_SHOW: {
            printf("[UI Component Render] <%s>", expr->show.widget_name);
            for (size_t i = 0; i < expr->show.prop_count; i++) {
                Value prop_val = eval_expression(interp, expr->show.props[i]);
                if (prop_val.kind == VAL_STRING) printf(" prop=\"%s\"", prop_val.str_val);
                else if (prop_val.kind == VAL_INT) printf(" prop=%ld", prop_val.int_val);
            }
            printf(" </%s>\n", expr->show.widget_name);
            v.kind = VAL_UNIT;
            return v;
        }
        case EXPR_UNARY: {
            Value op = eval_expression(interp, expr->unary.operand);
            if (expr->unary.op == TOKEN_MINUS && op.kind == VAL_INT) {
                v.kind = VAL_INT;
                v.int_val = -op.int_val;
                return v;
            }
            if (expr->unary.op == TOKEN_LOGICAL_NOT && op.kind == VAL_BOOL) {
                v.kind = VAL_BOOL;
                v.bool_val = !op.bool_val;
                return v;
            }
            return op;
        }
        case EXPR_BINARY: {
            Value left = eval_expression(interp, expr->binary.left);
            Value right = eval_expression(interp, expr->binary.right);

            if (left.kind == VAL_INT && right.kind == VAL_INT) {
                v.kind = VAL_INT;
                switch (expr->binary.op) {
                    case TOKEN_PLUS: v.int_val = left.int_val + right.int_val; return v;
                    case TOKEN_MINUS: v.int_val = left.int_val - right.int_val; return v;
                    case TOKEN_STAR: v.int_val = left.int_val * right.int_val; return v;
                    case TOKEN_SLASH: v.int_val = right.int_val != 0 ? left.int_val / right.int_val : 0; return v;
                    case TOKEN_EQ: v.kind = VAL_BOOL; v.bool_val = (left.int_val == right.int_val); return v;
                    case TOKEN_NEQ: v.kind = VAL_BOOL; v.bool_val = (left.int_val != right.int_val); return v;
                    case TOKEN_LT: v.kind = VAL_BOOL; v.bool_val = (left.int_val < right.int_val); return v;
                    case TOKEN_GT: v.kind = VAL_BOOL; v.bool_val = (left.int_val > right.int_val); return v;
                    case TOKEN_LTE: v.kind = VAL_BOOL; v.bool_val = (left.int_val <= right.int_val); return v;
                    case TOKEN_GTE: v.kind = VAL_BOOL; v.bool_val = (left.int_val >= right.int_val); return v;
                    default: break;
                }
            }
            return v;
        }
        case EXPR_CALL: {
            if (strcmp(expr->call.callee, "print") == 0) {
                for (size_t i = 0; i < expr->call.arg_count; i++) {
                    Value arg = eval_expression(interp, expr->call.args[i]);
                    if (arg.kind == VAL_INT) printf("%ld", arg.int_val);
                    else if (arg.kind == VAL_FLOAT) printf("%f", arg.float_val);
                    else if (arg.kind == VAL_STRING) printf("%s", arg.str_val);
                    else if (arg.kind == VAL_BOOL) printf("%s", arg.bool_val ? "true" : "false");
                }
                printf("\n");
                v.kind = VAL_UNIT;
                return v;
            }

            if (interp->program) {
                for (size_t i = 0; i < interp->program->stmt_count; i++) {
                    Stmt *s = interp->program->stmts[i];
                    if (s->type == STMT_FUNCTION_DECL && strcmp(s->func_decl.name, expr->call.callee) == 0) {
                        Environment *call_env = env_create(interp->global_env);
                        for (size_t p = 0; p < s->func_decl.param_count && p < expr->call.arg_count; p++) {
                            Value arg_val = eval_expression(interp, expr->call.args[p]);
                            env_assign(call_env, s->func_decl.params[p].name, arg_val);
                        }

                        Environment *prev_env = interp->current_env;
                        interp->current_env = call_env;
                        bool prev_ret = interp->has_returned;
                        interp->has_returned = false;

                        eval_statement(interp, s->func_decl.body);

                        Value ret = interp->return_value;
                        interp->current_env = prev_env;
                        interp->has_returned = prev_ret;
                        env_free(call_env);
                        return ret;
                    }
                }
            }
            return v;
        }
        case EXPR_ASSIGNMENT: {
            Value val = eval_expression(interp, expr->assignment.value);
            env_assign(interp->current_env, expr->assignment.target_name, val);
            return val;
        }
    }
    return v;
}

bool eval_statement(Interpreter *interp, Stmt *stmt) {
    if (!stmt || interp->has_returned) return true;

    switch (stmt->type) {
        case STMT_EXPR:
            eval_expression(interp, stmt->expr);
            return true;
        case STMT_VAR_DECL: {
            Value val = {VAL_UNIT, {0}};
            if (stmt->var_decl.initializer) {
                val = eval_expression(interp, stmt->var_decl.initializer);
            }
            env_assign(interp->current_env, stmt->var_decl.name, val);
            return true;
        }
        case STMT_RETURN:
            if (stmt->return_stmt.value) {
                interp->return_value = eval_expression(interp, stmt->return_stmt.value);
            } else {
                interp->return_value.kind = VAL_UNIT;
            }
            interp->has_returned = true;
            return true;
        case STMT_REPLY: {
            Value resp = eval_expression(interp, stmt->reply_stmt.response_expr);
            printf("[HTTP Reply %d] ", stmt->reply_stmt.status_code);
            if (resp.kind == VAL_STRING) printf("%s\n", resp.str_val);
            else if (resp.kind == VAL_INT) printf("%ld\n", resp.int_val);
            else printf("ok\n");

            interp->return_value = resp;
            interp->has_returned = true;
            return true;
        }
        case STMT_IF: {
            Value cond = eval_expression(interp, stmt->if_stmt.condition);
            bool cond_true = (cond.kind == VAL_BOOL && cond.bool_val) || (cond.kind == VAL_INT && cond.int_val != 0);
            if (cond_true) {
                eval_statement(interp, stmt->if_stmt.then_branch);
            } else if (stmt->if_stmt.else_branch) {
                eval_statement(interp, stmt->if_stmt.else_branch);
            }
            return true;
        }
        case STMT_WHILE: {
            while (!interp->has_returned) {
                Value cond = eval_expression(interp, stmt->while_stmt.condition);
                bool cond_true = (cond.kind == VAL_BOOL && cond.bool_val) || (cond.kind == VAL_INT && cond.int_val != 0);
                if (!cond_true) break;
                eval_statement(interp, stmt->while_stmt.body);
            }
            return true;
        }
        case STMT_BLOCK: {
            Environment *block_env = env_create(interp->current_env);
            interp->current_env = block_env;
            for (size_t i = 0; i < stmt->block.stmt_count; i++) {
                if (interp->has_returned) break;
                eval_statement(interp, stmt->block.stmts[i]);
            }
            interp->current_env = block_env->parent;
            env_free(block_env);
            return true;
        }
        case STMT_FUNCTION_DECL:
            return true;
        case STMT_COMPONENT_DECL:
            printf("[Registering Frontend Component: %s]\n", stmt->component_decl.name);
            eval_statement(interp, stmt->component_decl.render_body);
            return true;
        case STMT_ROUTE_DECL:
            printf("[Registering Server Route: %s %s]\n", stmt->route_decl.method, stmt->route_decl.path);
            eval_statement(interp, stmt->route_decl.handler_body);
            return true;
    }

    return true;
}

Value eval_program(Interpreter *interp, Program *program) {
    interp->program = program;
    Value result = {VAL_UNIT, {0}};

    for (size_t i = 0; i < program->stmt_count; i++) {
        eval_statement(interp, program->stmts[i]);
    }

    // Auto invoke main() if declared
    for (size_t i = 0; i < program->stmt_count; i++) {
        if (program->stmts[i]->type == STMT_FUNCTION_DECL && strcmp(program->stmts[i]->func_decl.name, "main") == 0) {
            Expr main_call;
            main_call.type = EXPR_CALL;
            main_call.call.callee = "main";
            main_call.call.args = NULL;
            main_call.call.arg_count = 0;
            return eval_expression(interp, &main_call);
        }
    }

    return result;
}
