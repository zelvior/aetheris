#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/lexer.h"

void test_basic_tokens() {
    const char *source = "let count: i32 = 42 ~\nconst name: text = \"Aetheris\" ~";
    Lexer lexer;
    lexer_init(&lexer, source, "test_basic.aeth");

    Token t1 = lexer_next_token(&lexer);
    assert(t1.type == TOKEN_LET);

    Token t2 = lexer_next_token(&lexer);
    assert(t2.type == TOKEN_IDENTIFIER);
    assert(t2.length == 5);
    assert(memcmp(t2.start, "count", 5) == 0);

    Token t3 = lexer_next_token(&lexer);
    assert(t3.type == TOKEN_COLON);

    Token t4 = lexer_next_token(&lexer);
    assert(t4.type == TOKEN_TYPE_I32);

    Token t5 = lexer_next_token(&lexer);
    assert(t5.type == TOKEN_ASSIGN);

    Token t6 = lexer_next_token(&lexer);
    assert(t6.type == TOKEN_INT_LITERAL);
    assert(t6.value.int_val == 42);

    Token t7 = lexer_next_token(&lexer);
    assert(t7.type == TOKEN_TERMINATOR);

    Token t8 = lexer_next_token(&lexer);
    assert(t8.type == TOKEN_CONST);

    Token t9 = lexer_next_token(&lexer);
    assert(t9.type == TOKEN_IDENTIFIER);

    Token t10 = lexer_next_token(&lexer);
    assert(t10.type == TOKEN_COLON);

    Token t11 = lexer_next_token(&lexer);
    assert(t11.type == TOKEN_TYPE_TEXT);

    Token t12 = lexer_next_token(&lexer);
    assert(t12.type == TOKEN_ASSIGN);

    Token t13 = lexer_next_token(&lexer);
    assert(t13.type == TOKEN_TEXT_LITERAL);
    assert(strcmp(t13.value.string_val, "Aetheris") == 0);

    Token t14 = lexer_next_token(&lexer);
    assert(t14.type == TOKEN_TERMINATOR);

    Token t15 = lexer_next_token(&lexer);
    assert(t15.type == TOKEN_EOF);

    printf("test_basic_tokens passed!\n");
}

void test_tilde_disambiguation() {
    // Expression statement terminator vs Bitwise NOT
    const char *source = "let x = ~mask ~\nreturn ok(x) ~";
    Lexer lexer;
    lexer_init(&lexer, source, "test_tilde.aeth");

    assert(lexer_next_token(&lexer).type == TOKEN_LET);
    assert(lexer_next_token(&lexer).type == TOKEN_IDENTIFIER);
    assert(lexer_next_token(&lexer).type == TOKEN_ASSIGN);

    Token t_not = lexer_next_token(&lexer);
    assert(t_not.type == TOKEN_BITWISE_NOT);

    assert(lexer_next_token(&lexer).type == TOKEN_IDENTIFIER);

    Token t_term = lexer_next_token(&lexer);
    assert(t_term.type == TOKEN_TERMINATOR);

    assert(lexer_next_token(&lexer).type == TOKEN_RETURN);

    printf("test_tilde_disambiguation passed!\n");
}

void test_hex_and_operators() {
    const char *source = "let ptr: uptr = 0x80000000 ~\nif a == b and c != d ~";
    Lexer lexer;
    lexer_init(&lexer, source, "test_hex.aeth");

    assert(lexer_next_token(&lexer).type == TOKEN_LET);
    assert(lexer_next_token(&lexer).type == TOKEN_IDENTIFIER);
    assert(lexer_next_token(&lexer).type == TOKEN_COLON);
    assert(lexer_next_token(&lexer).type == TOKEN_TYPE_UPTR);
    assert(lexer_next_token(&lexer).type == TOKEN_ASSIGN);

    Token t_hex = lexer_next_token(&lexer);
    assert(t_hex.type == TOKEN_INT_LITERAL);
    assert((uint64_t)t_hex.value.int_val == 0x80000000ULL);

    assert(lexer_next_token(&lexer).type == TOKEN_TERMINATOR);
    assert(lexer_next_token(&lexer).type == TOKEN_IF);
    assert(lexer_next_token(&lexer).type == TOKEN_IDENTIFIER);
    assert(lexer_next_token(&lexer).type == TOKEN_EQ);
    assert(lexer_next_token(&lexer).type == TOKEN_IDENTIFIER);
    assert(lexer_next_token(&lexer).type == TOKEN_LOGICAL_AND);
    assert(lexer_next_token(&lexer).type == TOKEN_IDENTIFIER);
    assert(lexer_next_token(&lexer).type == TOKEN_NEQ);
    assert(lexer_next_token(&lexer).type == TOKEN_IDENTIFIER);
    assert(lexer_next_token(&lexer).type == TOKEN_TERMINATOR);

    printf("test_hex_and_operators passed!\n");
}

int main() {
    printf("Running Lexer Unit Tests...\n");
    test_basic_tokens();
    test_tilde_disambiguation();
    test_hex_and_operators();
    printf("All Lexer Unit Tests Passed!\n");
    return 0;
}
