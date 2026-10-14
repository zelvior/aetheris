#define _POSIX_C_SOURCE 200809L
#include "ir.h"

static IrInstruction* create_inst(IrOpcode opcode) {
    IrInstruction *inst = (IrInstruction*)calloc(1, sizeof(IrInstruction));
    inst->opcode = opcode;
    return inst;
}

static void append_inst(IrProgram *prog, IrInstruction *inst) {
    if (!prog->head) {
        prog->head = inst;
        prog->tail = inst;
    } else {
        prog->tail->next = inst;
        prog->tail = inst;
    }
}

static char* new_temp(IrProgram *prog) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%%r%zu", prog->temp_count++);
    return strdup(buf);
}

static char* new_label(IrProgram *prog) {
    char buf[32];
    snprintf(buf, sizeof(buf), "L%zu", prog->label_count++);
    return strdup(buf);
}

static char* ir_gen_expr(IrProgram *prog, Expr *expr) {
    if (!expr) return NULL;

    switch (expr->type) {
        case EXPR_INT_LITERAL: {
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_CONST_INT);
            inst->dest = t;
            inst->int_val = expr->int_val;
            append_inst(prog, inst);
            return t;
        }
        case EXPR_FLOAT_LITERAL: {
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_CONST_FLOAT);
            inst->dest = t;
            inst->float_val = expr->float_val;
            append_inst(prog, inst);
            return t;
        }
        case EXPR_TEXT_LITERAL: {
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_CONST_STRING);
            inst->dest = t;
            inst->src1 = strdup(expr->text_val ? expr->text_val : "");
            append_inst(prog, inst);
            return t;
        }
        case EXPR_BOOL_LITERAL: {
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_CONST_BOOL);
            inst->dest = t;
            inst->int_val = expr->bool_val ? 1 : 0;
            append_inst(prog, inst);
            return t;
        }
        case EXPR_IDENTIFIER: {
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_LOAD);
            inst->dest = t;
            inst->src1 = strdup(expr->identifier_name);
            append_inst(prog, inst);
            return t;
        }
        case EXPR_UNARY: {
            char *operand = ir_gen_expr(prog, expr->unary.operand);
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_UNOP);
            inst->dest = t;
            inst->src1 = strdup(operand ? operand : "");
            inst->binary_op = expr->unary.op;
            append_inst(prog, inst);
            return t;
        }
        case EXPR_BINARY: {
            char *left = ir_gen_expr(prog, expr->binary.left);
            char *right = ir_gen_expr(prog, expr->binary.right);
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_BINOP);
            inst->dest = t;
            inst->src1 = strdup(left ? left : "");
            inst->src2 = strdup(right ? right : "");
            inst->binary_op = expr->binary.op;
            append_inst(prog, inst);
            return t;
        }
        case EXPR_CALL: {
            char *t = new_temp(prog);
            IrInstruction *inst = create_inst(IR_CALL);
            inst->dest = t;
            inst->src1 = strdup(expr->call.callee);
            append_inst(prog, inst);
            return t;
        }
        case EXPR_ASSIGNMENT: {
            char *val = ir_gen_expr(prog, expr->assignment.value);
            IrInstruction *inst = create_inst(IR_STORE);
            inst->src1 = strdup(expr->assignment.target_name);
            inst->src2 = strdup(val ? val : "");
            append_inst(prog, inst);
            return val;
        }
    }
    return NULL;
}

static void ir_gen_stmt(IrProgram *prog, Stmt *stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_EXPR:
            ir_gen_expr(prog, stmt->expr);
            break;
        case STMT_VAR_DECL: {
            IrInstruction *alloc = create_inst(IR_ALLOC);
            alloc->dest = strdup(stmt->var_decl.name);
            append_inst(prog, alloc);

            if (stmt->var_decl.initializer) {
                char *init_val = ir_gen_expr(prog, stmt->var_decl.initializer);
                IrInstruction *store = create_inst(IR_STORE);
                store->src1 = strdup(stmt->var_decl.name);
                store->src2 = strdup(init_val ? init_val : "");
                append_inst(prog, store);
            }
            break;
        }
        case STMT_RETURN: {
            char *ret_val = NULL;
            if (stmt->return_stmt.value) {
                ret_val = ir_gen_expr(prog, stmt->return_stmt.value);
            }
            IrInstruction *ret = create_inst(IR_RETURN);
            ret->src1 = ret_val ? strdup(ret_val) : NULL;
            append_inst(prog, ret);
            break;
        }
        case STMT_IF: {
            char *cond = ir_gen_expr(prog, stmt->if_stmt.condition);
            char *lbl_else = new_label(prog);
            char *lbl_end = new_label(prog);

            IrInstruction *br = create_inst(IR_BRANCH);
            br->src1 = strdup(cond ? cond : "");
            br->src2 = strdup(lbl_else);
            append_inst(prog, br);

            if (stmt->if_stmt.then_branch->type == STMT_BLOCK) {
                for (size_t i = 0; i < stmt->if_stmt.then_branch->block.stmt_count; i++) {
                    ir_gen_stmt(prog, stmt->if_stmt.then_branch->block.stmts[i]);
                }
            } else {
                ir_gen_stmt(prog, stmt->if_stmt.then_branch);
            }

            IrInstruction *jmp = create_inst(IR_JUMP);
            jmp->dest = strdup(lbl_end);
            append_inst(prog, jmp);

            IrInstruction *lbl_e = create_inst(IR_LABEL);
            lbl_e->dest = strdup(lbl_else);
            append_inst(prog, lbl_e);

            if (stmt->if_stmt.else_branch) {
                if (stmt->if_stmt.else_branch->type == STMT_BLOCK) {
                    for (size_t i = 0; i < stmt->if_stmt.else_branch->block.stmt_count; i++) {
                        ir_gen_stmt(prog, stmt->if_stmt.else_branch->block.stmts[i]);
                    }
                } else {
                    ir_gen_stmt(prog, stmt->if_stmt.else_branch);
                }
            }

            IrInstruction *lbl_end_inst = create_inst(IR_LABEL);
            lbl_end_inst->dest = strdup(lbl_end);
            append_inst(prog, lbl_end_inst);

            free(lbl_else);
            free(lbl_end);
            break;
        }
        case STMT_WHILE: {
            char *lbl_start = new_label(prog);
            char *lbl_end = new_label(prog);

            IrInstruction *lbl_s = create_inst(IR_LABEL);
            lbl_s->dest = strdup(lbl_start);
            append_inst(prog, lbl_s);

            char *cond = ir_gen_expr(prog, stmt->while_stmt.condition);
            IrInstruction *br = create_inst(IR_BRANCH);
            br->src1 = strdup(cond ? cond : "");
            br->src2 = strdup(lbl_end);
            append_inst(prog, br);

            if (stmt->while_stmt.body->type == STMT_BLOCK) {
                for (size_t i = 0; i < stmt->while_stmt.body->block.stmt_count; i++) {
                    ir_gen_stmt(prog, stmt->while_stmt.body->block.stmts[i]);
                }
            } else {
                ir_gen_stmt(prog, stmt->while_stmt.body);
            }

            IrInstruction *jmp = create_inst(IR_JUMP);
            jmp->dest = strdup(lbl_start);
            append_inst(prog, jmp);

            IrInstruction *lbl_e = create_inst(IR_LABEL);
            lbl_e->dest = strdup(lbl_end);
            append_inst(prog, lbl_e);

            free(lbl_start);
            free(lbl_end);
            break;
        }
        case STMT_BLOCK:
            for (size_t i = 0; i < stmt->block.stmt_count; i++) {
                ir_gen_stmt(prog, stmt->block.stmts[i]);
            }
            break;
        case STMT_FUNCTION_DECL: {
            IrInstruction *lbl = create_inst(IR_LABEL);
            lbl->dest = strdup(stmt->func_decl.name);
            append_inst(prog, lbl);

            if (stmt->func_decl.body->type == STMT_BLOCK) {
                for (size_t i = 0; i < stmt->func_decl.body->block.stmt_count; i++) {
                    ir_gen_stmt(prog, stmt->func_decl.body->block.stmts[i]);
                }
            } else {
                ir_gen_stmt(prog, stmt->func_decl.body);
            }
            break;
        }
    }
}

IrProgram* ir_generate(Program *ast, TargetArch target) {
    (void)target;
    IrProgram *prog = (IrProgram*)calloc(1, sizeof(IrProgram));
    if (!ast) return prog;

    for (size_t i = 0; i < ast->stmt_count; i++) {
        ir_gen_stmt(prog, ast->stmts[i]);
    }
    return prog;
}

void ir_free(IrProgram *ir) {
    if (!ir) return;
    IrInstruction *curr = ir->head;
    while (curr) {
        IrInstruction *next = curr->next;
        free(curr->dest);
        free(curr->src1);
        free(curr->src2);
        free(curr);
        curr = next;
    }
    free(ir);
}

void ir_print(IrProgram *ir, FILE *out) {
    if (!ir || !out) return;
    IrInstruction *curr = ir->head;
    while (curr) {
        switch (curr->opcode) {
            case IR_CONST_INT:
                fprintf(out, "  %s = CONST_INT %ld\n", curr->dest, curr->int_val);
                break;
            case IR_CONST_FLOAT:
                fprintf(out, "  %s = CONST_FLOAT %f\n", curr->dest, curr->float_val);
                break;
            case IR_CONST_STRING:
                fprintf(out, "  %s = CONST_STRING \"%s\"\n", curr->dest, curr->src1);
                break;
            case IR_CONST_BOOL:
                fprintf(out, "  %s = CONST_BOOL %ld\n", curr->dest, curr->int_val);
                break;
            case IR_ALLOC:
                fprintf(out, "  ALLOC %s\n", curr->dest);
                break;
            case IR_LOAD:
                fprintf(out, "  %s = LOAD %s\n", curr->dest, curr->src1);
                break;
            case IR_STORE:
                fprintf(out, "  STORE %s, %s\n", curr->src1, curr->src2);
                break;
            case IR_BINOP:
                fprintf(out, "  %s = BINOP %d, %s, %s\n", curr->dest, curr->binary_op, curr->src1, curr->src2);
                break;
            case IR_UNOP:
                fprintf(out, "  %s = UNOP %d, %s\n", curr->dest, curr->binary_op, curr->src1);
                break;
            case IR_LABEL:
                fprintf(out, "%s:\n", curr->dest);
                break;
            case IR_BRANCH:
                fprintf(out, "  BRANCH %s, %s\n", curr->src1, curr->src2);
                break;
            case IR_JUMP:
                fprintf(out, "  JUMP %s\n", curr->dest);
                break;
            case IR_CALL:
                fprintf(out, "  %s = CALL %s\n", curr->dest, curr->src1);
                break;
            case IR_RETURN:
                fprintf(out, "  RETURN %s\n", curr->src1 ? curr->src1 : "void");
                break;
        }
        curr = curr->next;
    }
}
