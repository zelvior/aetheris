#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "typechecker.h"
#include "codegen.h"
#include "interpreter.h"

void print_version() {
    printf("Aetheris Compiler & Toolchain (aeth) v1.0.0\n");
    printf("One Syntax. Any Target. Absolute Clarity.\n");
}

void print_usage() {
    printf("Usage:\n");
    printf("  aeth --version                         Display toolchain version\n");
    printf("  aeth run <file.aeth>                  Run file in interpreter\n");
    printf("  aeth build <file.aeth> [options]      Compile file to C99 source/binary\n");
    printf("    --target=native-32                  Target 32-bit architecture\n");
    printf("    --target=native-64                  Target 64-bit architecture (default)\n");
    printf("  aeth repl                              Launch interactive REPL\n");
}

void run_repl() {
    print_version();
    printf("Type 'exit~' or CTRL+C to quit.\n\n");

    Interpreter interp;
    interpreter_init(&interp);

    char line[1024];
    while (1) {
        printf("aeth> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        if (strncmp(line, "exit", 4) == 0) break;

        Lexer lexer;
        lexer_init(&lexer, line, "<repl>");

        Parser parser;
        parser_init(&parser, &lexer);

        Program *program = parse_program(&parser);
        if (!parser.had_error) {
            Value val = eval_program(&interp, program);
            if (val.kind == VAL_INT) printf("=> %ld\n", val.int_val);
            else if (val.kind == VAL_FLOAT) printf("=> %f\n", val.float_val);
            else if (val.kind == VAL_BOOL) printf("=> %s\n", val.bool_val ? "true" : "false");
            else if (val.kind == VAL_STRING) printf("=> \"%s\"\n", val.str_val);
        }
        free_program(program);
    }

    interpreter_free(&interp);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        print_version();
        return 0;
    }

    if (strcmp(argv[1], "repl") == 0) {
        run_repl();
        return 0;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing source file for 'run'\n");
            return 1;
        }
        const char *filepath = argv[2];
        char *source = read_file_contents(filepath);
        if (!source) return 1;

        Lexer lexer;
        lexer_init(&lexer, source, filepath);

        Parser parser;
        parser_init(&parser, &lexer);

        Program *program = parse_program(&parser);
        if (parser.had_error) {
            free(source);
            return 1;
        }

        TypeChecker tc;
        typechecker_init(&tc, TARGET_NATIVE_64);
        if (!typecheck_program(&tc, program)) {
            typechecker_free(&tc);
            free_program(program);
            free(source);
            return 1;
        }
        typechecker_free(&tc);

        Interpreter interp;
        interpreter_init(&interp);
        eval_program(&interp, program);

        interpreter_free(&interp);
        free_program(program);
        free(source);
        return 0;
    }

    if (strcmp(argv[1], "build") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing source file for 'build'\n");
            return 1;
        }
        const char *filepath = argv[2];
        TargetArch target = TARGET_NATIVE_64;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "--target=native-32") == 0) {
                target = TARGET_NATIVE_32;
            } else if (strcmp(argv[i], "--target=native-64") == 0) {
                target = TARGET_NATIVE_64;
            }
        }

        char *source = read_file_contents(filepath);
        if (!source) return 1;

        Lexer lexer;
        lexer_init(&lexer, source, filepath);

        Parser parser;
        parser_init(&parser, &lexer);

        Program *program = parse_program(&parser);
        if (parser.had_error) {
            free(source);
            return 1;
        }

        TypeChecker tc;
        typechecker_init(&tc, target);
        if (!typecheck_program(&tc, program)) {
            typechecker_free(&tc);
            free_program(program);
            free(source);
            return 1;
        }
        typechecker_free(&tc);

        const char *out_c_path = "build/output.c";
        FILE *out = fopen(out_c_path, "w");
        if (!out) {
            fprintf(stderr, "Error: could not create output file '%s'\n", out_c_path);
            free_program(program);
            free(source);
            return 1;
        }

        Codegen cg;
        codegen_init(&cg, out, target);
        codegen_generate_c(&cg, program);
        fclose(out);

        printf("Build succeeded! Generated C99 code in '%s'\n", out_c_path);

        free_program(program);
        free(source);
        return 0;
    }

    print_usage();
    return 0;
}
