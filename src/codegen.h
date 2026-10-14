#ifndef AETH_CODEGEN_H
#define AETH_CODEGEN_H

#include "ast.h"
#include "typechecker.h"

typedef struct {
    FILE *out;
    TargetArch target;
    int indent_level;
} Codegen;

void codegen_init(Codegen *cg, FILE *out, TargetArch target);
bool codegen_generate_c(Codegen *cg, Program *program);

#endif
