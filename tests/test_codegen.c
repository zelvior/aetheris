#include <assert.h>
#include <stdio.h>
#include "../src/codegen.h"
#include "../src/parser.h"

void test_codegen_basic() {
    const char *source =
        "fn main() -> i32 ~\n"
        "    let x: i32 = 10 ~\n"
        "    let y: i32 = 20 ~\n"
        "    return x + y ~\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_cg.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);

    FILE *out = fopen("build/test_out.c", "w");
    assert(out != NULL);

    Codegen cg;
    codegen_init(&cg, out, TARGET_NATIVE_64);
    bool ok = codegen_generate_c(&cg, program);
    assert(ok);

    fclose(out);
    free_program(program);

    FILE *read_back = fopen("build/test_out.c", "r");
    assert(read_back != NULL);
    char buf[1024];
    size_t len = fread(buf, 1, sizeof(buf) - 1, read_back);
    buf[len] = '\0';
    fclose(read_back);

    assert(strstr(buf, "int32_t main()") != NULL);
    assert(strstr(buf, "int32_t x = 10;") != NULL);
    assert(strstr(buf, "return (x + y);") != NULL);

    printf("test_codegen_basic passed!\n");
}

int main() {
    printf("Running Codegen Unit Tests...\n");
    test_codegen_basic();
    printf("All Codegen Unit Tests Passed!\n");
    return 0;
}
