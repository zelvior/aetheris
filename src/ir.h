#ifndef AETH_IR_H
#define AETH_IR_H

#include "ast.h"
#include "typechecker.h"

typedef enum {
    IR_CONST_INT,
    IR_CONST_FLOAT,
    IR_CONST_STRING,
    IR_CONST_BOOL,
    IR_ALLOC,
    IR_LOAD,
    IR_STORE,
    IR_BINOP,
    IR_UNOP,
    IR_LABEL,
    IR_BRANCH,
    IR_JUMP,
    IR_CALL,
    IR_RETURN
} IrOpcode;

typedef struct IrInstruction IrInstruction;

struct IrInstruction {
    IrOpcode opcode;
    char *dest;
    char *src1;
    char *src2;
    int64_t int_val;
    double float_val;
    TokenType binary_op;
    IrInstruction *next;
};

typedef struct {
    IrInstruction *head;
    IrInstruction *tail;
    size_t temp_count;
    size_t label_count;
} IrProgram;

IrProgram* ir_generate(Program *ast, TargetArch target);
void ir_free(IrProgram *ir);
void ir_print(IrProgram *ir, FILE *out);

#endif
