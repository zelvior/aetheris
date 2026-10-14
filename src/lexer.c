#define _POSIX_C_SOURCE 200809L
#include "lexer.h"

void lexer_init(Lexer *lexer, const char *source, const char *filename) {
    lexer->source = source;
    lexer->filename = filename ? filename : "<unknown>";
    lexer->cursor = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->last_token.type = TOKEN_EOF;
}

static bool is_at_end(Lexer *lexer) {
    return lexer->source[lexer->cursor] == '\0';
}

static char peek(Lexer *lexer) {
    return lexer->source[lexer->cursor];
}

static char peek_next(Lexer *lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->cursor + 1];
}

static char advance(Lexer *lexer) {
    char c = lexer->source[lexer->cursor++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

static bool match(Lexer *lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->cursor] != expected) return false;
    advance(lexer);
    return true;
}

static Token make_token(Lexer *lexer, TokenType type, const char *start, size_t length, size_t line, size_t col) {
    Token token;
    token.type = type;
    token.start = start;
    token.length = length;
    token.line = line;
    token.column = col;
    token.value.int_val = 0;
    lexer->last_token = token;
    return token;
}

static Token error_token(Lexer *lexer, const char *message, size_t line, size_t col) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = strlen(message);
    token.line = line;
    token.column = col;
    token.value.string_val = (char*)message;
    lexer->last_token = token;
    return token;
}

static void skip_whitespace_and_comments(Lexer *lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                        advance(lexer);
                    }
                } else if (peek_next(lexer) == '*') {
                    advance(lexer); // '/'
                    advance(lexer); // '*'
                    while (!is_at_end(lexer)) {
                        if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                            advance(lexer); // '*'
                            advance(lexer); // '/'
                            break;
                        }
                        advance(lexer);
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static TokenType check_keyword(const char *start, size_t length) {
    switch (length) {
        case 2:
            if (memcmp(start, "fn", 2) == 0) return TOKEN_FN;
            if (memcmp(start, "if", 2) == 0) return TOKEN_IF;
            if (memcmp(start, "in", 2) == 0) return TOKEN_IN;
            if (memcmp(start, "or", 2) == 0) return TOKEN_LOGICAL_OR;
            if (memcmp(start, "on", 2) == 0) return TOKEN_ON;
            if (memcmp(start, "db", 2) == 0) return TOKEN_DB;
            if (memcmp(start, "i8", 2) == 0) return TOKEN_TYPE_I8;
            if (memcmp(start, "u8", 2) == 0) return TOKEN_TYPE_U8;
            break;
        case 3:
            if (memcmp(start, "let", 3) == 0) return TOKEN_LET;
            if (memcmp(start, "mut", 3) == 0) return TOKEN_MUT;
            if (memcmp(start, "end", 3) == 0) return TOKEN_END;
            if (memcmp(start, "for", 3) == 0) return TOKEN_FOR;
            if (memcmp(start, "and", 3) == 0) return TOKEN_LOGICAL_AND;
            if (memcmp(start, "not", 3) == 0) return TOKEN_LOGICAL_NOT;
            if (memcmp(start, "fit", 3) == 0) return TOKEN_FIT;
            if (memcmp(start, "GET", 3) == 0) return TOKEN_HTTP_GET;
            if (memcmp(start, "PUT", 3) == 0) return TOKEN_HTTP_PUT;
            if (memcmp(start, "i16", 3) == 0) return TOKEN_TYPE_I16;
            if (memcmp(start, "i32", 3) == 0) return TOKEN_TYPE_I32;
            if (memcmp(start, "i64", 3) == 0) return TOKEN_TYPE_I64;
            if (memcmp(start, "u16", 3) == 0) return TOKEN_TYPE_U16;
            if (memcmp(start, "u32", 3) == 0) return TOKEN_TYPE_U32;
            if (memcmp(start, "u64", 3) == 0) return TOKEN_TYPE_U64;
            if (memcmp(start, "f32", 3) == 0) return TOKEN_TYPE_F32;
            if (memcmp(start, "f64", 3) == 0) return TOKEN_TYPE_F64;
            break;
        case 4:
            if (memcmp(start, "else", 4) == 0) return TOKEN_ELSE;
            if (memcmp(start, "case", 4) == 0) return TOKEN_CASE;
            if (memcmp(start, "loop", 4) == 0) return TOKEN_LOOP;
            if (memcmp(start, "type", 4) == 0) return TOKEN_TYPE;
            if (memcmp(start, "enum", 4) == 0) return TOKEN_ENUM;
            if (memcmp(start, "show", 4) == 0) return TOKEN_SHOW;
            if (memcmp(start, "with", 4) == 0) return TOKEN_WITH;
            if (memcmp(start, "bind", 4) == 0) return TOKEN_BIND;
            if (memcmp(start, "emit", 4) == 0) return TOKEN_EMIT;
            if (memcmp(start, "from", 4) == 0) return TOKEN_FROM;
            if (memcmp(start, "POST", 4) == 0) return TOKEN_HTTP_POST;
            if (memcmp(start, "iptr", 4) == 0) return TOKEN_TYPE_IPTR;
            if (memcmp(start, "uptr", 4) == 0) return TOKEN_TYPE_UPTR;
            if (memcmp(start, "bool", 4) == 0) return TOKEN_TYPE_BOOL;
            if (memcmp(start, "text", 4) == 0) return TOKEN_TYPE_TEXT;
            if (memcmp(start, "true", 4) == 0) return TOKEN_BOOL_LITERAL;
            break;
        case 5:
            if (memcmp(start, "const", 5) == 0) return TOKEN_CONST;
            if (memcmp(start, "match", 5) == 0) return TOKEN_MATCH;
            if (memcmp(start, "while", 5) == 0) return TOKEN_WHILE;
            if (memcmp(start, "break", 5) == 0) return TOKEN_BREAK;
            if (memcmp(start, "async", 5) == 0) return TOKEN_ASYNC;
            if (memcmp(start, "await", 5) == 0) return TOKEN_AWAIT;
            if (memcmp(start, "style", 5) == 0) return TOKEN_STYLE;
            if (memcmp(start, "route", 5) == 0) return TOKEN_ROUTE;
            if (memcmp(start, "reply", 5) == 0) return TOKEN_REPLY;
            if (memcmp(start, "layer", 5) == 0) return TOKEN_LAYER;
            if (memcmp(start, "infer", 5) == 0) return TOKEN_INFER;
            if (memcmp(start, "false", 5) == 0) return TOKEN_BOOL_LITERAL;
            break;
        case 6:
            if (memcmp(start, "return", 6) == 0) return TOKEN_RETURN;
            if (memcmp(start, "struct", 6) == 0) return TOKEN_STRUCT;
            if (memcmp(start, "render", 6) == 0) return TOKEN_RENDER;
            if (memcmp(start, "status", 6) == 0) return TOKEN_STATUS;
            if (memcmp(start, "neural", 6) == 0) return TOKEN_NEURAL;
            if (memcmp(start, "export", 6) == 0) return TOKEN_EXPORT;
            if (memcmp(start, "import", 6) == 0) return TOKEN_IMPORT;
            if (memcmp(start, "module", 6) == 0) return TOKEN_MODULE;
            if (memcmp(start, "DELETE", 6) == 0) return TOKEN_HTTP_DELETE;
            break;
        case 7:
            if (memcmp(start, "default", 7) == 0) return TOKEN_DEFAULT;
            if (memcmp(start, "request", 7) == 0) return TOKEN_REQUEST;
            if (memcmp(start, "weights", 7) == 0) return TOKEN_WEIGHTS;
            break;
        case 8:
            if (memcmp(start, "continue", 8) == 0) return TOKEN_CONTINUE;
            break;
        case 9:
            if (memcmp(start, "component", 9) == 0) return TOKEN_COMPONENT;
            break;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer *lexer, const char *start, size_t line, size_t col) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    size_t length = (size_t)(lexer->source + lexer->cursor - start);
    TokenType type = check_keyword(start, length);
    Token token = make_token(lexer, type, start, length, line, col);
    if (type == TOKEN_BOOL_LITERAL) {
        token.value.bool_val = (start[0] == 't');
    }
    return token;
}

static Token number(Lexer *lexer, const char *start, size_t line, size_t col) {
    bool is_float = false;

    if (start[0] == '0' && (peek(lexer) == 'x' || peek(lexer) == 'X')) {
        advance(lexer); // 'x'
        while (isxdigit(peek(lexer))) {
            advance(lexer);
        }
        size_t length = (size_t)(lexer->source + lexer->cursor - start);
        Token token = make_token(lexer, TOKEN_INT_LITERAL, start, length, line, col);
        token.value.int_val = strtoll(start, NULL, 16);
        return token;
    }

    while (isdigit(peek(lexer))) {
        advance(lexer);
    }

    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        is_float = true;
        advance(lexer); // '.'
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
    }

    size_t length = (size_t)(lexer->source + lexer->cursor - start);
    if (is_float) {
        Token token = make_token(lexer, TOKEN_FLOAT_LITERAL, start, length, line, col);
        token.value.float_val = strtod(start, NULL);
        return token;
    } else {
        Token token = make_token(lexer, TOKEN_INT_LITERAL, start, length, line, col);
        token.value.int_val = strtoll(start, NULL, 10);
        return token;
    }
}

static Token string_literal(Lexer *lexer, size_t line, size_t col) {
    const char *start = lexer->source + lexer->cursor;
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\\' && peek_next(lexer) != '\0') {
            advance(lexer);
        }
        advance(lexer);
    }

    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string literal", line, col);
    }

    size_t length = (size_t)(lexer->source + lexer->cursor - start);
    advance(lexer); // closing "

    Token token = make_token(lexer, TOKEN_TEXT_LITERAL, start, length, line, col);
    char *str_buf = (char*)malloc(length + 1);
    memcpy(str_buf, start, length);
    str_buf[length] = '\0';
    token.value.string_val = str_buf;
    return token;
}

Token lexer_next_token(Lexer *lexer) {
    skip_whitespace_and_comments(lexer);

    size_t line = lexer->line;
    size_t col = lexer->column;
    const char *start = lexer->source + lexer->cursor;

    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF, start, 0, line, col);
    }

    char c = advance(lexer);

    if (isalpha(c) || c == '_') {
        return identifier(lexer, start, line, col);
    }

    if (isdigit(c)) {
        return number(lexer, start, line, col);
    }

    switch (c) {
        case '~': {
            char next_c = peek(lexer);
            if (isalpha(next_c) || isdigit(next_c) || next_c == '_' || next_c == '(' || next_c == '~') {
                return make_token(lexer, TOKEN_BITWISE_NOT, start, 1, line, col);
            }
            return make_token(lexer, TOKEN_TERMINATOR, start, 1, line, col);
        }
        case '"':
            return string_literal(lexer, line, col);
        case '+':
            return make_token(lexer, TOKEN_PLUS, start, 1, line, col);
        case '-':
            if (match(lexer, '>')) return make_token(lexer, TOKEN_ARROW, start, 2, line, col);
            return make_token(lexer, TOKEN_MINUS, start, 1, line, col);
        case '*':
            if (match(lexer, '*')) return make_token(lexer, TOKEN_POWER, start, 2, line, col);
            return make_token(lexer, TOKEN_STAR, start, 1, line, col);
        case '/':
            return make_token(lexer, TOKEN_SLASH, start, 1, line, col);
        case '%':
            return make_token(lexer, TOKEN_PERCENT, start, 1, line, col);
        case '=':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_EQ, start, 2, line, col);
            if (match(lexer, '>')) return make_token(lexer, TOKEN_FAT_ARROW, start, 2, line, col);
            return make_token(lexer, TOKEN_ASSIGN, start, 1, line, col);
        case '!':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_NEQ, start, 2, line, col);
            return error_token(lexer, "Unexpected character '!'", line, col);
        case '<':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_LTE, start, 2, line, col);
            if (match(lexer, '<')) return make_token(lexer, TOKEN_LSHIFT, start, 2, line, col);
            return make_token(lexer, TOKEN_LT, start, 1, line, col);
        case '>':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_GTE, start, 2, line, col);
            if (match(lexer, '>')) return make_token(lexer, TOKEN_RSHIFT, start, 2, line, col);
            return make_token(lexer, TOKEN_GT, start, 1, line, col);
        case '&':
            return make_token(lexer, TOKEN_BITWISE_AND, start, 1, line, col);
        case '|':
            return make_token(lexer, TOKEN_BITWISE_OR, start, 1, line, col);
        case '^':
            return make_token(lexer, TOKEN_BITWISE_XOR, start, 1, line, col);
        case '?':
            return make_token(lexer, TOKEN_QUESTION, start, 1, line, col);
        case '(':
            return make_token(lexer, TOKEN_LPAREN, start, 1, line, col);
        case ')':
            return make_token(lexer, TOKEN_RPAREN, start, 1, line, col);
        case '[':
            return make_token(lexer, TOKEN_LBRACKET, start, 1, line, col);
        case ']':
            return make_token(lexer, TOKEN_RBRACKET, start, 1, line, col);
        case '{':
            return make_token(lexer, TOKEN_LBRACE, start, 1, line, col);
        case '}':
            return make_token(lexer, TOKEN_RBRACE, start, 1, line, col);
        case ',':
            return make_token(lexer, TOKEN_COMMA, start, 1, line, col);
        case ':':
            return make_token(lexer, TOKEN_COLON, start, 1, line, col);
        case '.':
            return make_token(lexer, TOKEN_DOT, start, 1, line, col);
        default:
            break;
    }

    char err_msg[64];
    snprintf(err_msg, sizeof(err_msg), "Unexpected character '%c'", c);
    return error_token(lexer, strdup(err_msg), line, col);
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        case TOKEN_LET: return "TOKEN_LET";
        case TOKEN_MUT: return "TOKEN_MUT";
        case TOKEN_CONST: return "TOKEN_CONST";
        case TOKEN_FN: return "TOKEN_FN";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_END: return "TOKEN_END";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_COMPONENT: return "TOKEN_COMPONENT";
        case TOKEN_RENDER: return "TOKEN_RENDER";
        case TOKEN_SHOW: return "TOKEN_SHOW";
        case TOKEN_WITH: return "TOKEN_WITH";
        case TOKEN_ROUTE: return "TOKEN_ROUTE";
        case TOKEN_REPLY: return "TOKEN_REPLY";
        case TOKEN_STATUS: return "TOKEN_STATUS";
        case TOKEN_HTTP_GET: return "TOKEN_HTTP_GET";
        case TOKEN_HTTP_POST: return "TOKEN_HTTP_POST";
        case TOKEN_TERMINATOR: return "TOKEN_TERMINATOR";
        default: return "TOKEN_UNKNOWN";
    }
}
