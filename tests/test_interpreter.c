#include <assert.h>
#include <stdio.h>
#include "../src/interpreter.h"
#include "../src/parser.h"

void test_interpreter_execution() {
    const char *source =
        "fn multiply(a: i32, b: i32) -> i32 ~\n"
        "    return a * b ~\n"
        "end\n"
        "fn main() -> i32 ~\n"
        "    let x: i32 = 6 ~\n"
        "    let y: i32 = 7 ~\n"
        "    let res: i32 = multiply(x, y) ~\n"
        "    print(\"Result: \", res) ~\n"
        "    return res ~\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_interp.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);

    Interpreter interp;
    interpreter_init(&interp);

    Value res = eval_program(&interp, program);
    assert(res.kind == VAL_INT);
    assert(res.int_val == 42);

    interpreter_free(&interp);
    free_program(program);

    printf("test_interpreter_execution passed!\n");
}

int main() {
    printf("Running Interpreter Unit Tests...\n");
    test_interpreter_execution();
    printf("All Interpreter Unit Tests Passed!\n");
    return 0;
}
