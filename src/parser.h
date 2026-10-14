#ifndef AETH_PARSER_H
#define AETH_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,     // == !=
    PREC_COMPARISON,   // < > <= >=
    PREC_BITWISE,      // & | ^ << >>
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / %
    PREC_POWER,       // **
    PREC_UNARY,       // not - ~
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

void parser_init(Parser *parser, Lexer *lexer);
Expr* parse_expression(Parser *parser);
Expr* parse_precedence(Parser *parser, Precedence precedence);
Stmt* parse_statement(Parser *parser);
Program* parse_program(Parser *parser);

#endif
