#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/ir.h"
#include "../src/parser.h"

void test_ir_generation() {
    const char *source =
        "fn calculate(x: i32) -> i32 ~\n"
        "    let y: i32 = 10 ~\n"
        "    return x + y ~\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_ir.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);

    IrProgram *ir = ir_generate(program, TARGET_NATIVE_64);
    assert(ir != NULL);
    assert(ir->head != NULL);

    FILE *out = fopen("build/test_ir.txt", "w");
    assert(out != NULL);
    ir_print(ir, out);
    fclose(out);

    ir_free(ir);
    free_program(program);

    FILE *read_back = fopen("build/test_ir.txt", "r");
    assert(read_back != NULL);
    char buf[1024];
    size_t len = fread(buf, 1, sizeof(buf) - 1, read_back);
    buf[len] = '\0';
    fclose(read_back);

    assert(strstr(buf, "calculate:") != NULL);
    assert(strstr(buf, "ALLOC y") != NULL);
    assert(strstr(buf, "STORE y, %r0") != NULL);

    printf("test_ir_generation passed!\n");
}

int main() {
    printf("Running IR Unit Tests...\n");
    test_ir_generation();
    printf("All IR Unit Tests Passed!\n");
    return 0;
}
