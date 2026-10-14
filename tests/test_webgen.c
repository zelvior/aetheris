#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/webgen.h"
#include "../src/parser.h"

void test_webgen_site() {
    const char *source =
        "component NavigationBar ~\n"
        "    render ~\n"
        "        show Heading(\"Aetheris Web\") ~\n"
        "    end\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_web.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);

    bool ok = webgen_generate_site(program, "build/test_dist");
    assert(ok);

    FILE *f_site = fopen("build/test_dist/index.aeth_site", "r");
    assert(f_site != NULL);
    char buf[1024];
    size_t len = fread(buf, 1, sizeof(buf) - 1, f_site);
    buf[len] = '\0';
    fclose(f_site);

    assert(strstr(buf, "Aetheris Native Web Engine") != NULL);
    assert(strstr(buf, "NavigationBar") != NULL);
    assert(strstr(buf, "Heading") != NULL);

    FILE *f_manifest = fopen("build/test_dist/manifest.aeth_ui", "r");
    assert(f_manifest != NULL);
    len = fread(buf, 1, sizeof(buf) - 1, f_manifest);
    buf[len] = '\0';
    fclose(f_manifest);

    assert(strstr(buf, "manifest AetherisWebsite") != NULL);

    free_program(program);
    printf("test_webgen_site passed!\n");
}

int main() {
    printf("Running WebGen Unit Tests...\n");
    test_webgen_site();
    printf("All WebGen Unit Tests Passed!\n");
    return 0;
}
