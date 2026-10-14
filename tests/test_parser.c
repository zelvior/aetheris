#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/parser.h"

void test_parse_function_and_let() {
    const char *source =
        "fn add(a: i32, b: i32) -> i32 ~\n"
        "    let sum: i32 = a + b ~\n"
        "    return sum ~\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_fn.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);
    assert(program->stmt_count == 1);

    Stmt *fn_stmt = program->stmts[0];
    assert(fn_stmt->type == STMT_FUNCTION_DECL);
    assert(strcmp(fn_stmt->func_decl.name, "add") == 0);
    assert(fn_stmt->func_decl.param_count == 2);
    assert(strcmp(fn_stmt->func_decl.params[0].name, "a") == 0);
    assert(strcmp(fn_stmt->func_decl.params[1].name, "b") == 0);

    Stmt *body_block = fn_stmt->func_decl.body;
    assert(body_block->type == STMT_BLOCK);
    assert(body_block->block.stmt_count == 2);

    Stmt *let_stmt = body_block->block.stmts[0];
    assert(let_stmt->type == STMT_VAR_DECL);
    assert(strcmp(let_stmt->var_decl.name, "sum") == 0);
    assert(let_stmt->var_decl.initializer->type == EXPR_BINARY);

    Stmt *ret_stmt = body_block->block.stmts[1];
    assert(ret_stmt->type == STMT_RETURN);
    assert(ret_stmt->return_stmt.value->type == EXPR_IDENTIFIER);

    free_program(program);
    printf("test_parse_function_and_let passed!\n");
}

void test_parse_if_else() {
    const char *source =
        "fn checkVal(val: i32) ~\n"
        "    if val > 10 ~\n"
        "        let res = 1 ~\n"
        "    else ~\n"
        "        let res = 0 ~\n"
        "    end\n"
        "end\n";

    Lexer lexer;
    lexer_init(&lexer, source, "test_if.aeth");

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse_program(&parser);
    assert(!parser.had_error);
    assert(program->stmt_count == 1);

    Stmt *fn_stmt = program->stmts[0];
    Stmt *if_stmt = fn_stmt->func_decl.body->block.stmts[0];
    assert(if_stmt->type == STMT_IF);
    assert(if_stmt->if_stmt.condition->type == EXPR_BINARY);
    assert(if_stmt->if_stmt.then_branch->type == STMT_BLOCK);
    assert(if_stmt->if_stmt.else_branch->type == STMT_BLOCK);

    free_program(program);
    printf("test_parse_if_else passed!\n");
}

int main() {
    printf("Running Parser Unit Tests...\n");
    test_parse_function_and_let();
    test_parse_if_else();
    printf("All Parser Unit Tests Passed!\n");
    return 0;
}
