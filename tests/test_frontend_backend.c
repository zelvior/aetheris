#include <assert.h>
#include <stdio.h>
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/typechecker.h"
#include "../src/codegen.h"
#include "../src/interpreter.h"

void test_frontend_component() {
    const char *source =
        "component Counter ~\n"
        "    render ~\n"
        "        show Heading(\"Counter App\") ~\n"
        "        show Button(\"Increment\") ~\n"
        "    end\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_fe.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);
    assert(program->stmt_count == 1);

    TypeChecker tc;
    typechecker_init(&tc, TARGET_WEB);
    assert(typecheck_program(&tc, program));
    typechecker_free(&tc);

    Interpreter interp;
    interpreter_init(&interp);
    eval_program(&interp, program);
    interpreter_free(&interp);

    free_program(program);
    printf("test_frontend_component passed!\n");
}

void test_backend_route() {
    const char *source =
        "route GET \"/api/v1/health\" ~\n"
        "    let statusMsg: text = \"Server OK\" ~\n"
        "    reply statusMsg with status 200 ~\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_be.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);
    assert(program->stmt_count == 1);

    TypeChecker tc;
    typechecker_init(&tc, TARGET_NATIVE_64);
    assert(typecheck_program(&tc, program));
    typechecker_free(&tc);

    Interpreter interp;
    interpreter_init(&interp);
    eval_program(&interp, program);
    interpreter_free(&interp);

    free_program(program);
    printf("test_backend_route passed!\n");
}

int main() {
    printf("Running Frontend & Backend Integration Tests...\n");
    test_frontend_component();
    test_backend_route();
    printf("All Frontend & Backend Integration Tests Passed!\n");
    return 0;
}
