#define _POSIX_C_SOURCE 200809L
#include "codegen.h"

static void write_indent(Codegen *cg) {
    for (int i = 0; i < cg->indent_level; i++) {
        fprintf(cg->out, "    ");
    }
}

void codegen_init(Codegen *cg, FILE *out, TargetArch target) {
    cg->out = out;
    cg->target = target;
    cg->indent_level = 0;
}

static void codegen_expr(Codegen *cg, Expr *expr) {
    if (!expr) return;
    switch (expr->type) {
        case EXPR_INT_LITERAL:
            fprintf(cg->out, "%ld", expr->int_val);
            break;
        case EXPR_FLOAT_LITERAL:
            fprintf(cg->out, "%f", expr->float_val);
            break;
        case EXPR_TEXT_LITERAL:
            fprintf(cg->out, "\"%s\"", expr->text_val);
            break;
        case EXPR_BOOL_LITERAL:
            fprintf(cg->out, "%s", expr->bool_val ? "true" : "false");
            break;
        case EXPR_IDENTIFIER:
            fprintf(cg->out, "%s", expr->identifier_name);
            break;
        case EXPR_UNARY:
            if (expr->unary.op == TOKEN_MINUS) fprintf(cg->out, "-");
            else if (expr->unary.op == TOKEN_LOGICAL_NOT) fprintf(cg->out, "!");
            else if (expr->unary.op == TOKEN_BITWISE_NOT) fprintf(cg->out, "~");
            fprintf(cg->out, "(");
            codegen_expr(cg, expr->unary.operand);
            fprintf(cg->out, ")");
            break;
        case EXPR_BINARY:
            fprintf(cg->out, "(");
            codegen_expr(cg, expr->binary.left);
            switch (expr->binary.op) {
                case TOKEN_PLUS: fprintf(cg->out, " + "); break;
                case TOKEN_MINUS: fprintf(cg->out, " - "); break;
                case TOKEN_STAR: fprintf(cg->out, " * "); break;
                case TOKEN_SLASH: fprintf(cg->out, " / "); break;
                case TOKEN_PERCENT: fprintf(cg->out, " %% "); break;
                case TOKEN_EQ: fprintf(cg->out, " == "); break;
                case TOKEN_NEQ: fprintf(cg->out, " != "); break;
                case TOKEN_LT: fprintf(cg->out, " < "); break;
                case TOKEN_GT: fprintf(cg->out, " > "); break;
                case TOKEN_LTE: fprintf(cg->out, " <= "); break;
                case TOKEN_GTE: fprintf(cg->out, " >= "); break;
                case TOKEN_LOGICAL_AND: fprintf(cg->out, " && "); break;
                case TOKEN_LOGICAL_OR: fprintf(cg->out, " || "); break;
                case TOKEN_BITWISE_AND: fprintf(cg->out, " & "); break;
                case TOKEN_BITWISE_OR: fprintf(cg->out, " | "); break;
                case TOKEN_BITWISE_XOR: fprintf(cg->out, " ^ "); break;
                case TOKEN_LSHIFT: fprintf(cg->out, " << "); break;
                case TOKEN_RSHIFT: fprintf(cg->out, " >> "); break;
                default: fprintf(cg->out, " /* op */ "); break;
            }
            codegen_expr(cg, expr->binary.right);
            fprintf(cg->out, ")");
            break;
        case EXPR_CALL:
            fprintf(cg->out, "%s(", expr->call.callee);
            for (size_t i = 0; i < expr->call.arg_count; i++) {
                if (i > 0) fprintf(cg->out, ", ");
                codegen_expr(cg, expr->call.args[i]);
            }
            fprintf(cg->out, ")");
            break;
        case EXPR_ASSIGNMENT:
            fprintf(cg->out, "%s = ", expr->assignment.target_name);
            codegen_expr(cg, expr->assignment.value);
            break;
    }
}

static const char* c_type_map(const char *aeth_type, TargetArch target) {
    if (!aeth_type) return "int32_t";
    if (strcmp(aeth_type, "i8") == 0) return "int8_t";
    if (strcmp(aeth_type, "i16") == 0) return "int16_t";
    if (strcmp(aeth_type, "i32") == 0) return "int32_t";
    if (strcmp(aeth_type, "i64") == 0) return "int64_t";
    if (strcmp(aeth_type, "u8") == 0) return "uint8_t";
    if (strcmp(aeth_type, "u16") == 0) return "uint16_t";
    if (strcmp(aeth_type, "u32") == 0) return "uint32_t";
    if (strcmp(aeth_type, "u64") == 0) return "uint64_t";
    if (strcmp(aeth_type, "f32") == 0) return "float";
    if (strcmp(aeth_type, "f64") == 0) return "double";
    if (strcmp(aeth_type, "bool") == 0) return "bool";
    if (strcmp(aeth_type, "text") == 0) return "const char*";
    if (strcmp(aeth_type, "unit") == 0) return "void";
    if (strcmp(aeth_type, "iptr") == 0) return target == TARGET_NATIVE_32 ? "int32_t" : "int64_t";
    if (strcmp(aeth_type, "uptr") == 0) return target == TARGET_NATIVE_32 ? "uint32_t" : "uint64_t";
    return "int32_t";
}

static void codegen_stmt(Codegen *cg, Stmt *stmt) {
    if (!stmt) return;
    write_indent(cg);

    switch (stmt->type) {
        case STMT_EXPR:
            codegen_expr(cg, stmt->expr);
            fprintf(cg->out, ";\n");
            break;
        case STMT_VAR_DECL: {
            const char *ctype = c_type_map(stmt->var_decl.type_annotation.name, cg->target);
            if (stmt->var_decl.is_const) fprintf(cg->out, "const ");
            fprintf(cg->out, "%s ", ctype);
            if (stmt->var_decl.type_annotation.is_pointer) fprintf(cg->out, "*");
            fprintf(cg->out, "%s", stmt->var_decl.name);
            if (stmt->var_decl.initializer) {
                fprintf(cg->out, " = ");
                codegen_expr(cg, stmt->var_decl.initializer);
            }
            fprintf(cg->out, ";\n");
            break;
        }
        case STMT_RETURN:
            fprintf(cg->out, "return");
            if (stmt->return_stmt.value) {
                fprintf(cg->out, " ");
                codegen_expr(cg, stmt->return_stmt.value);
            }
            fprintf(cg->out, ";\n");
            break;
        case STMT_IF:
            fprintf(cg->out, "if (");
            codegen_expr(cg, stmt->if_stmt.condition);
            fprintf(cg->out, ") {\n");
            cg->indent_level++;
            if (stmt->if_stmt.then_branch->type == STMT_BLOCK) {
                for (size_t i = 0; i < stmt->if_stmt.then_branch->block.stmt_count; i++) {
                    codegen_stmt(cg, stmt->if_stmt.then_branch->block.stmts[i]);
                }
            } else {
                codegen_stmt(cg, stmt->if_stmt.then_branch);
            }
            cg->indent_level--;
            write_indent(cg);
            fprintf(cg->out, "}");
            if (stmt->if_stmt.else_branch) {
                fprintf(cg->out, " else {\n");
                cg->indent_level++;
                if (stmt->if_stmt.else_branch->type == STMT_BLOCK) {
                    for (size_t i = 0; i < stmt->if_stmt.else_branch->block.stmt_count; i++) {
                        codegen_stmt(cg, stmt->if_stmt.else_branch->block.stmts[i]);
                    }
                } else {
                    codegen_stmt(cg, stmt->if_stmt.else_branch);
                }
                cg->indent_level--;
                write_indent(cg);
                fprintf(cg->out, "}\n");
            } else {
                fprintf(cg->out, "\n");
            }
            break;
        case STMT_WHILE:
            fprintf(cg->out, "while (");
            codegen_expr(cg, stmt->while_stmt.condition);
            fprintf(cg->out, ") {\n");
            cg->indent_level++;
            if (stmt->while_stmt.body->type == STMT_BLOCK) {
                for (size_t i = 0; i < stmt->while_stmt.body->block.stmt_count; i++) {
                    codegen_stmt(cg, stmt->while_stmt.body->block.stmts[i]);
                }
            } else {
                codegen_stmt(cg, stmt->while_stmt.body);
            }
            cg->indent_level--;
            write_indent(cg);
            fprintf(cg->out, "}\n");
            break;
        case STMT_BLOCK:
            fprintf(cg->out, "{\n");
            cg->indent_level++;
            for (size_t i = 0; i < stmt->block.stmt_count; i++) {
                codegen_stmt(cg, stmt->block.stmts[i]);
            }
            cg->indent_level--;
            write_indent(cg);
            fprintf(cg->out, "}\n");
            break;
        case STMT_FUNCTION_DECL: {
            const char *ret_type = c_type_map(stmt->func_decl.return_type.name, cg->target);
            fprintf(cg->out, "%s %s(", ret_type, stmt->func_decl.name);
            for (size_t i = 0; i < stmt->func_decl.param_count; i++) {
                if (i > 0) fprintf(cg->out, ", ");
                const char *ptype = c_type_map(stmt->func_decl.params[i].type_annotation.name, cg->target);
                fprintf(cg->out, "%s ", ptype);
                if (stmt->func_decl.params[i].type_annotation.is_pointer) fprintf(cg->out, "*");
                fprintf(cg->out, "%s", stmt->func_decl.params[i].name);
            }
            fprintf(cg->out, ") {\n");
            cg->indent_level++;
            if (stmt->func_decl.body->type == STMT_BLOCK) {
                for (size_t i = 0; i < stmt->func_decl.body->block.stmt_count; i++) {
                    codegen_stmt(cg, stmt->func_decl.body->block.stmts[i]);
                }
            } else {
                codegen_stmt(cg, stmt->func_decl.body);
            }
            cg->indent_level--;
            write_indent(cg);
            fprintf(cg->out, "}\n\n");
            break;
        }
    }
}

bool codegen_generate_c(Codegen *cg, Program *program) {
    if (!cg || !cg->out || !program) return false;

    // Headers
    fprintf(cg->out, "/* Generated by Aetheris Compiler (aethc) */\n");
    fprintf(cg->out, "#include <stdio.h>\n");
    fprintf(cg->out, "#include <stdint.h>\n");
    fprintf(cg->out, "#include <stdbool.h>\n");
    fprintf(cg->out, "#include <stdlib.h>\n");
    fprintf(cg->out, "#include <string.h>\n\n");

    for (size_t i = 0; i < program->stmt_count; i++) {
        codegen_stmt(cg, program->stmts[i]);
    }

    return true;
}
