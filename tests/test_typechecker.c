#include <assert.h>
#include <stdio.h>
#include "../src/typechecker.h"
#include "../src/parser.h"

void test_typechecker_target_pointer_width() {
    Type uptr_32 = string_to_type("uptr", TARGET_NATIVE_32);
    Type uptr_64 = string_to_type("uptr", TARGET_NATIVE_64);

    assert(uptr_32.kind == TYPE_U32);
    assert(uptr_64.kind == TYPE_U64);

    printf("test_typechecker_target_pointer_width passed!\n");
}

void test_typechecker_valid_program() {
    const char *source =
        "fn calculate(val: i32) -> i32 ~\n"
        "    let factor: i32 = 10 ~\n"
        "    let result: i32 = val * factor ~\n"
        "    return result ~\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_tc.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);

    TypeChecker tc;
    typechecker_init(&tc, TARGET_NATIVE_64);

    bool ok = typecheck_program(&tc, program);
    assert(ok);
    assert(!tc.had_error);

    typechecker_free(&tc);
    free_program(program);

    printf("test_typechecker_valid_program passed!\n");
}

void test_typechecker_strict_no_silent_widening() {
    const char *source =
        "fn failMismatch() ~\n"
        "    let a: i32 = 10 ~\n"
        "    let b: i64 = 20 ~\n"
        "    let c: i32 = a + b ~\n" // Mismatch i32 + i64 without explicit cast
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_tc_err.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);

    TypeChecker tc;
    typechecker_init(&tc, TARGET_NATIVE_64);

    bool ok = typecheck_program(&tc, program);
    assert(!ok);
    assert(tc.had_error);

    typechecker_free(&tc);
    free_program(program);

    printf("test_typechecker_strict_no_silent_widening passed!\n");
}

int main() {
    printf("Running TypeChecker Unit Tests...\n");
    test_typechecker_target_pointer_width();
    test_typechecker_valid_program();
    test_typechecker_strict_no_silent_widening();
    printf("All TypeChecker Unit Tests Passed!\n");
    return 0;
}
