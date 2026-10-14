#include <assert.h>
#include <stdio.h>
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/typechecker.h"
#include "../src/interpreter.h"

void test_async_spawn() {
    const char *source =
        "async fn fetchData() -> i32 ~\n"
        "    return 100 ~\n"
        "end ~\n"
        "fn main() -> i32 ~\n"
        "    let val: i32 = await fetchData() ~\n"
        "    spawn fetchData() ~\n"
        "    return val ~\n"
        "end ~\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_async.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);
    assert(program->stmt_count == 2);

    TypeChecker tc;
    typechecker_init(&tc, TARGET_NATIVE_64);
    assert(typecheck_program(&tc, program));
    typechecker_free(&tc);

    Interpreter interp;
    interpreter_init(&interp);
    Value res = eval_program(&interp, program);
    assert(res.kind == VAL_INT);
    assert(res.int_val == 100);

    interpreter_free(&interp);
    free_program(program);

    printf("test_async_spawn passed!\n");
}

void test_neural_ai() {
    const char *source =
        "neural SentimentModel ~\n"
        "end ~\n"
        "fn main() -> i32 ~\n"
        "    let score: f32 = infer SentimentModel with 42 ~\n"
        "    return 0 ~\n"
        "end ~\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_ai.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);
    assert(program->stmt_count == 2);

    TypeChecker tc;
    typechecker_init(&tc, TARGET_AI);
    assert(typecheck_program(&tc, program));
    typechecker_free(&tc);

    Interpreter interp;
    interpreter_init(&interp);
    Value res = eval_program(&interp, program);
    assert(res.kind == VAL_INT);
    assert(res.int_val == 0);

    interpreter_free(&interp);
    free_program(program);

    printf("test_neural_ai passed!\n");
}

int main() {
    printf("Running Async & AI Unit Tests...\n");
    test_async_spawn();
    test_neural_ai();
    printf("All Async & AI Unit Tests Passed!\n");
    return 0;
}
