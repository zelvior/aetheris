#define _POSIX_C_SOURCE 200809L
#include "parser.h"

typedef Expr* (*PrefixFn)(Parser *parser, Token token);
typedef Expr* (*InfixFn)(Parser *parser, Expr *left, Token token);

typedef struct {
    PrefixFn prefix;
    InfixFn infix;
    Precedence precedence;
} ParseRule;

static ParseRule* get_rule(TokenType type);

static void advance_parser(Parser *parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = lexer_next_token(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;

        fprintf(stderr, "[Error] Lexer error at line %zu, col %zu: %s\n",
                parser->current.line, parser->current.column, parser->current.start);
        parser->had_error = true;
    }
}

static void consume(Parser *parser, TokenType type, const char *message) {
    if (parser->current.type == type) {
        advance_parser(parser);
        return;
    }

    fprintf(stderr, "[Error] at '%s' (line %zu, col %zu): %s\n",
            token_type_to_string(parser->current.type),
            parser->current.line, parser->current.column, message);
    parser->had_error = true;
}

static bool check(Parser *parser, TokenType type) {
    return parser->current.type == type;
}

static bool match_token(Parser *parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance_parser(parser);
    return true;
}

void parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    advance_parser(parser); // prime parser
}

Expr* parse_precedence(Parser *parser, Precedence precedence) {
    Token token = parser->current;
    advance_parser(parser);

    PrefixFn prefix = get_rule(token.type)->prefix;
    if (prefix == NULL) {
        fprintf(stderr, "[Error] Expected expression at line %zu, col %zu, got %s\n",
                token.line, token.column, token_type_to_string(token.type));
        parser->had_error = true;
        return NULL;
    }

    Expr *expr = prefix(parser, token);

    while (precedence <= get_rule(parser->current.type)->precedence) {
        Token infix_token = parser->current;
        advance_parser(parser);
        InfixFn infix = get_rule(infix_token.type)->infix;
        if (infix == NULL) break;
        expr = infix(parser, expr, infix_token);
    }

    return expr;
}

Expr* parse_expression(Parser *parser) {
    return parse_precedence(parser, PREC_ASSIGNMENT);
}

static Expr* parse_number(Parser *parser, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    if (token.type == TOKEN_FLOAT_LITERAL) {
        return ast_create_float_literal(token.value.float_val, loc);
    }
    return ast_create_int_literal(token.value.int_val, loc);
}

static Expr* parse_string(Parser *parser, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    return ast_create_text_literal(token.value.string_val ? token.value.string_val : "", loc);
}

static Expr* parse_bool(Parser *parser, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    return ast_create_bool_literal(token.value.bool_val, loc);
}

static Expr* parse_variable(Parser *parser, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    char name_buf[256];
    size_t len = token.length < 255 ? token.length : 255;
    memcpy(name_buf, token.start, len);
    name_buf[len] = '\0';
    return ast_create_identifier(name_buf, loc);
}

static Expr* parse_show_expression(Parser *parser, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    consume(parser, TOKEN_IDENTIFIER, "Expect widget or component name after 'show'.");
    Token widget_tok = parser->previous;
    char widget_name[256];
    size_t len = widget_tok.length < 255 ? widget_tok.length : 255;
    memcpy(widget_name, widget_tok.start, len);
    widget_name[len] = '\0';

    size_t cap = 8;
    size_t count = 0;
    Expr **props = (Expr**)malloc(sizeof(Expr*) * cap);

    if (match_token(parser, TOKEN_LPAREN)) {
        if (!check(parser, TOKEN_RPAREN)) {
            do {
                if (count >= cap) {
                    cap *= 2;
                    props = (Expr**)realloc(props, sizeof(Expr*) * cap);
                }
                props[count++] = parse_expression(parser);
            } while (match_token(parser, TOKEN_COMMA));
        }
        consume(parser, TOKEN_RPAREN, "Expect ')' after widget properties.");
    }

    return ast_create_show(widget_name, props, count, loc);
}

static Expr* parse_grouping(Parser *parser, Token token) {
    (void)token;
    Expr *expr = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
    return expr;
}

static Expr* parse_unary(Parser *parser, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    Expr *operand = parse_precedence(parser, PREC_UNARY);
    return ast_create_unary(token.type, operand, loc);
}

static Expr* parse_binary(Parser *parser, Expr *left, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    ParseRule *rule = get_rule(token.type);
    Expr *right = parse_precedence(parser, (Precedence)(rule->precedence + 1));
    return ast_create_binary(token.type, left, right, loc);
}

static Expr* parse_call_infix(Parser *parser, Expr *left, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    (void)token;

    if (left->type != EXPR_IDENTIFIER) {
        fprintf(stderr, "[Error] Expected function name before '(' at line %zu\n", token.line);
        parser->had_error = true;
        return left;
    }

    size_t capacity = 8;
    size_t count = 0;
    Expr **args = (Expr**)malloc(sizeof(Expr*) * capacity);

    if (!check(parser, TOKEN_RPAREN)) {
        do {
            if (count >= capacity) {
                capacity *= 2;
                args = (Expr**)realloc(args, sizeof(Expr*) * capacity);
            }
            args[count++] = parse_expression(parser);
        } while (match_token(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");
    Expr *call_expr = ast_create_call(left->identifier_name, args, count, loc);
    free_expr(left); // consume identifier expr
    return call_expr;
}

static Expr* parse_assign_infix(Parser *parser, Expr *left, Token token) {
    SourceLocation loc = {parser->lexer->filename, token.line, token.column};
    (void)token;

    if (left->type != EXPR_IDENTIFIER) {
        fprintf(stderr, "[Error] Invalid assignment target at line %zu\n", token.line);
        parser->had_error = true;
        return left;
    }

    Expr *value = parse_expression(parser);
    Expr *assign_expr = ast_create_assignment(left->identifier_name, value, loc);
    free_expr(left);
    return assign_expr;
}

static ParseRule rules[256] = {
    [TOKEN_INT_LITERAL]    = { parse_number, NULL, PREC_NONE },
    [TOKEN_FLOAT_LITERAL]  = { parse_number, NULL, PREC_NONE },
    [TOKEN_TEXT_LITERAL]   = { parse_string, NULL, PREC_NONE },
    [TOKEN_BOOL_LITERAL]   = { parse_bool, NULL, PREC_NONE },
    [TOKEN_IDENTIFIER]     = { parse_variable, NULL, PREC_NONE },
    [TOKEN_SHOW]           = { parse_show_expression, NULL, PREC_NONE },
    [TOKEN_LPAREN]         = { parse_grouping, parse_call_infix, PREC_CALL },
    [TOKEN_PLUS]           = { NULL, parse_binary, PREC_TERM },
    [TOKEN_MINUS]          = { parse_unary, parse_binary, PREC_TERM },
    [TOKEN_STAR]           = { NULL, parse_binary, PREC_FACTOR },
    [TOKEN_SLASH]          = { NULL, parse_binary, PREC_FACTOR },
    [TOKEN_PERCENT]        = { NULL, parse_binary, PREC_FACTOR },
    [TOKEN_POWER]          = { NULL, parse_binary, PREC_POWER },
    [TOKEN_ASSIGN]         = { NULL, parse_assign_infix, PREC_ASSIGNMENT },
    [TOKEN_EQ]             = { NULL, parse_binary, PREC_EQUALITY },
    [TOKEN_NEQ]            = { NULL, parse_binary, PREC_EQUALITY },
    [TOKEN_LT]             = { NULL, parse_binary, PREC_COMPARISON },
    [TOKEN_GT]             = { NULL, parse_binary, PREC_COMPARISON },
    [TOKEN_LTE]            = { NULL, parse_binary, PREC_COMPARISON },
    [TOKEN_GTE]            = { NULL, parse_binary, PREC_COMPARISON },
    [TOKEN_LOGICAL_AND]    = { NULL, parse_binary, PREC_AND },
    [TOKEN_LOGICAL_OR]     = { NULL, parse_binary, PREC_OR },
    [TOKEN_LOGICAL_NOT]    = { parse_unary, NULL, PREC_UNARY },
    [TOKEN_BITWISE_NOT]    = { parse_unary, NULL, PREC_UNARY },
    [TOKEN_BITWISE_AND]    = { NULL, parse_binary, PREC_BITWISE },
    [TOKEN_BITWISE_OR]     = { NULL, parse_binary, PREC_BITWISE },
    [TOKEN_BITWISE_XOR]    = { NULL, parse_binary, PREC_BITWISE },
    [TOKEN_LSHIFT]         = { NULL, parse_binary, PREC_BITWISE },
    [TOKEN_RSHIFT]         = { NULL, parse_binary, PREC_BITWISE },
};

static ParseRule* get_rule(TokenType type) {
    if ((size_t)type >= (sizeof(rules)/sizeof(rules[0]))) {
        static ParseRule empty = { NULL, NULL, PREC_NONE };
        return &empty;
    }
    return &rules[type];
}

static TypeAnnotation parse_raw_type_specifier(Parser *parser) {
    TypeAnnotation ann = {strdup("i32"), false};
    if (check(parser, TOKEN_STAR)) {
        advance_parser(parser);
        ann.is_pointer = true;
    }
    Token t = parser->current;
    if (t.type == TOKEN_IDENTIFIER ||
        t.type == TOKEN_TYPE_I8 || t.type == TOKEN_TYPE_I16 ||
        t.type == TOKEN_TYPE_I32 || t.type == TOKEN_TYPE_I64 ||
        t.type == TOKEN_TYPE_U8 || t.type == TOKEN_TYPE_U16 ||
        t.type == TOKEN_TYPE_U32 || t.type == TOKEN_TYPE_U64 ||
        t.type == TOKEN_TYPE_IPTR || t.type == TOKEN_TYPE_UPTR ||
        t.type == TOKEN_TYPE_F32 || t.type == TOKEN_TYPE_F64 ||
        t.type == TOKEN_TYPE_BOOL || t.type == TOKEN_TYPE_TEXT) {

        char name_buf[256];
        size_t len = t.length < 255 ? t.length : 255;
        memcpy(name_buf, t.start, len);
        name_buf[len] = '\0';
        free(ann.name);
        ann.name = strdup(name_buf);
        advance_parser(parser);
    }
    return ann;
}

static TypeAnnotation parse_type_annotation(Parser *parser) {
    TypeAnnotation ann = {strdup("i32"), false};
    if (check(parser, TOKEN_COLON)) {
        advance_parser(parser);
        free(ann.name);
        return parse_raw_type_specifier(parser);
    }
    return ann;
}

static Stmt* parse_var_declaration(Parser *parser, bool is_const) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    bool is_mut = match_token(parser, TOKEN_MUT);

    consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    Token name_token = parser->previous;
    char var_name[256];
    size_t len = name_token.length < 255 ? name_token.length : 255;
    memcpy(var_name, name_token.start, len);
    var_name[len] = '\0';

    TypeAnnotation ann = parse_type_annotation(parser);

    Expr *initializer = NULL;
    if (match_token(parser, TOKEN_ASSIGN)) {
        initializer = parse_expression(parser);
    }

    consume(parser, TOKEN_TERMINATOR, "Expect '~' statement terminator after variable declaration.");
    Stmt *stmt = ast_create_var_decl(var_name, is_mut, is_const, ann, initializer, loc);
    free(ann.name);
    return stmt;
}

static Stmt* parse_return_statement(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    Expr *value = NULL;
    if (!check(parser, TOKEN_TERMINATOR)) {
        value = parse_expression(parser);
    }
    consume(parser, TOKEN_TERMINATOR, "Expect '~' statement terminator after return.");
    return ast_create_return_stmt(value, loc);
}

static Stmt* parse_block_statement(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->current.line, parser->current.column};
    size_t capacity = 8;
    size_t count = 0;
    Stmt **stmts = (Stmt**)malloc(sizeof(Stmt*) * capacity);

    while (!check(parser, TOKEN_END) && !check(parser, TOKEN_EOF)) {
        if (check(parser, TOKEN_TERMINATOR)) {
            advance_parser(parser);
            continue;
        }
        if (count >= capacity) {
            capacity *= 2;
            stmts = (Stmt**)realloc(stmts, sizeof(Stmt*) * capacity);
        }
        stmts[count++] = parse_statement(parser);
    }

    consume(parser, TOKEN_END, "Expect 'end' to close block.");
    return ast_create_block_stmt(stmts, count, loc);
}

static Stmt* parse_if_statement(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    Expr *condition = parse_expression(parser);

    match_token(parser, TOKEN_TERMINATOR);

    size_t capacity = 8;
    size_t count = 0;
    Stmt **then_stmts = (Stmt**)malloc(sizeof(Stmt*) * capacity);

    Stmt *else_branch = NULL;

    while (!check(parser, TOKEN_END) && !check(parser, TOKEN_ELSE) && !check(parser, TOKEN_EOF)) {
        if (check(parser, TOKEN_TERMINATOR)) {
            advance_parser(parser);
            continue;
        }
        if (count >= capacity) {
            capacity *= 2;
            then_stmts = (Stmt**)realloc(then_stmts, sizeof(Stmt*) * capacity);
        }
        then_stmts[count++] = parse_statement(parser);
    }

    Stmt *then_branch = ast_create_block_stmt(then_stmts, count, loc);

    if (match_token(parser, TOKEN_ELSE)) {
        match_token(parser, TOKEN_TERMINATOR); // handle optional ~ after else
        if (check(parser, TOKEN_IF)) {
            advance_parser(parser); // consume if
            else_branch = parse_if_statement(parser);
        } else {
            else_branch = parse_block_statement(parser);
        }
    } else {
        consume(parser, TOKEN_END, "Expect 'end' after if block.");
    }

    return ast_create_if_stmt(condition, then_branch, else_branch, loc);
}

static Stmt* parse_while_statement(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    Expr *condition = parse_expression(parser);
    match_token(parser, TOKEN_TERMINATOR);

    Stmt *body = parse_block_statement(parser);
    return ast_create_while_stmt(condition, body, loc);
}

static Stmt* parse_function_declaration(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    Token name_token = parser->previous;
    char fn_name[256];
    size_t len = name_token.length < 255 ? name_token.length : 255;
    memcpy(fn_name, name_token.start, len);
    fn_name[len] = '\0';

    consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");

    size_t param_cap = 8;
    size_t param_count = 0;
    Param *params = (Param*)malloc(sizeof(Param) * param_cap);

    if (!check(parser, TOKEN_RPAREN)) {
        do {
            if (param_count >= param_cap) {
                param_cap *= 2;
                params = (Param*)realloc(params, sizeof(Param) * param_cap);
            }
            consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
            Token p_tok = parser->previous;
            char p_name[256];
            size_t plen = p_tok.length < 255 ? p_tok.length : 255;
            memcpy(p_name, p_tok.start, plen);
            p_name[plen] = '\0';

            TypeAnnotation p_ann = parse_type_annotation(parser);

            params[param_count].name = strdup(p_name);
            params[param_count].type_annotation = p_ann;
            param_count++;
        } while (match_token(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after parameter list.");

    TypeAnnotation ret_ann = {strdup("unit"), false};
    if (match_token(parser, TOKEN_ARROW)) {
        free(ret_ann.name);
        ret_ann = parse_raw_type_specifier(parser);
    }

    match_token(parser, TOKEN_TERMINATOR);

    Stmt *body = parse_block_statement(parser);

    Stmt *fn_stmt = ast_create_func_decl(fn_name, params, param_count, ret_ann, body, loc);
    free(ret_ann.name);
    return fn_stmt;
}

static Stmt* parse_component_declaration(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    consume(parser, TOKEN_IDENTIFIER, "Expect component name.");
    Token name_tok = parser->previous;
    char comp_name[256];
    size_t len = name_tok.length < 255 ? name_tok.length : 255;
    memcpy(comp_name, name_tok.start, len);
    comp_name[len] = '\0';

    match_token(parser, TOKEN_TERMINATOR);

    Stmt *render_body = parse_block_statement(parser);
    return ast_create_component_decl(comp_name, NULL, 0, render_body, loc);
}

static Stmt* parse_render_statement(Parser *parser) {
    match_token(parser, TOKEN_TERMINATOR);
    Stmt *render_body = parse_block_statement(parser);
    return render_body;
}

static Stmt* parse_route_declaration(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    char method[16] = "GET";
    if (parser->current.type == TOKEN_HTTP_GET) { advance_parser(parser); strcpy(method, "GET"); }
    else if (parser->current.type == TOKEN_HTTP_POST) { advance_parser(parser); strcpy(method, "POST"); }
    else if (parser->current.type == TOKEN_HTTP_PUT) { advance_parser(parser); strcpy(method, "PUT"); }
    else if (parser->current.type == TOKEN_HTTP_DELETE) { advance_parser(parser); strcpy(method, "DELETE"); }

    consume(parser, TOKEN_TEXT_LITERAL, "Expect route path string e.g. \"/api/v1/health\".");
    Token path_tok = parser->previous;
    char path_buf[256];
    size_t len = path_tok.length < 255 ? path_tok.length : 255;
    memcpy(path_buf, path_tok.start, len);
    path_buf[len] = '\0';

    match_token(parser, TOKEN_TERMINATOR);

    Stmt *handler_body = parse_block_statement(parser);
    return ast_create_route_decl(method, path_buf, handler_body, loc);
}

static Stmt* parse_reply_statement(Parser *parser) {
    SourceLocation loc = {parser->lexer->filename, parser->previous.line, parser->previous.column};
    Expr *resp_expr = parse_expression(parser);

    int status_code = 200;
    if (match_token(parser, TOKEN_WITH)) {
        if (match_token(parser, TOKEN_STATUS)) {
            consume(parser, TOKEN_INT_LITERAL, "Expect HTTP status integer e.g. 200.");
            status_code = (int)parser->previous.value.int_val;
        }
    }

    consume(parser, TOKEN_TERMINATOR, "Expect '~' statement terminator after reply.");
    return ast_create_reply_stmt(resp_expr, status_code, loc);
}

Stmt* parse_statement(Parser *parser) {
    if (match_token(parser, TOKEN_LET)) {
        return parse_var_declaration(parser, false);
    }
    if (match_token(parser, TOKEN_CONST)) {
        return parse_var_declaration(parser, true);
    }
    if (match_token(parser, TOKEN_RETURN)) {
        return parse_return_statement(parser);
    }
    if (match_token(parser, TOKEN_IF)) {
        return parse_if_statement(parser);
    }
    if (match_token(parser, TOKEN_WHILE) || match_token(parser, TOKEN_LOOP)) {
        return parse_while_statement(parser);
    }
    if (match_token(parser, TOKEN_FN)) {
        return parse_function_declaration(parser);
    }
    if (match_token(parser, TOKEN_COMPONENT)) {
        return parse_component_declaration(parser);
    }
    if (match_token(parser, TOKEN_RENDER)) {
        return parse_render_statement(parser);
    }
    if (match_token(parser, TOKEN_ROUTE)) {
        return parse_route_declaration(parser);
    }
    if (match_token(parser, TOKEN_REPLY)) {
        return parse_reply_statement(parser);
    }

    // Default: Expression statement ending with ~
    SourceLocation loc = {parser->lexer->filename, parser->current.line, parser->current.column};
    Expr *expr = parse_expression(parser);
    consume(parser, TOKEN_TERMINATOR, "Expect '~' statement terminator.");
    return ast_create_expr_stmt(expr, loc);
}

Program* parse_program(Parser *parser) {
    size_t capacity = 16;
    size_t count = 0;
    Stmt **stmts = (Stmt**)malloc(sizeof(Stmt*) * capacity);

    while (!check(parser, TOKEN_EOF)) {
        if (check(parser, TOKEN_TERMINATOR)) {
            advance_parser(parser);
            continue;
        }
        if (count >= capacity) {
            capacity *= 2;
            stmts = (Stmt**)realloc(stmts, sizeof(Stmt*) * capacity);
        }
        stmts[count++] = parse_statement(parser);
    }

    return ast_create_program(stmts, count);
}
