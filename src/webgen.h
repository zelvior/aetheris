#ifndef AETH_WEBGEN_H
#define AETH_WEBGEN_H

#include "ast.h"

bool webgen_generate_site(Program *ast, const char *output_dir);

#endif
