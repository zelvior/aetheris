#ifndef AETH_LEXER_H
#define AETH_LEXER_H

#include "common.h"

typedef enum {
    // Special
    TOKEN_EOF = 0,
    TOKEN_ERROR,

    // Keywords
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_CONST,
    TOKEN_FN,
    TOKEN_RETURN,
    TOKEN_END,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_MATCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_LOOP,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_ASYNC,
    TOKEN_AWAIT,
    TOKEN_TYPE,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_COMPONENT,
    TOKEN_RENDER,
    TOKEN_BIND,
    TOKEN_ON,
    TOKEN_EMIT,
    TOKEN_STYLE,
    TOKEN_ROUTE,
    TOKEN_REQUEST,
    TOKEN_REPLY,
    TOKEN_STATUS,
    TOKEN_DB,
    TOKEN_NEURAL,
    TOKEN_LAYER,
    TOKEN_FIT,
    TOKEN_INFER,
    TOKEN_WEIGHTS,
    TOKEN_MODULE,
    TOKEN_IMPORT,
    TOKEN_EXPORT,
    TOKEN_FROM,

    // Identifiers & Literals
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_TEXT_LITERAL,
    TOKEN_BOOL_LITERAL,

    // Primitive Type Names
    TOKEN_TYPE_I8,
    TOKEN_TYPE_I16,
    TOKEN_TYPE_I32,
    TOKEN_TYPE_I64,
    TOKEN_TYPE_U8,
    TOKEN_TYPE_U16,
    TOKEN_TYPE_U32,
    TOKEN_TYPE_U64,
    TOKEN_TYPE_IPTR,
    TOKEN_TYPE_UPTR,
    TOKEN_TYPE_F32,
    TOKEN_TYPE_F64,
    TOKEN_TYPE_BOOL,
    TOKEN_TYPE_TEXT,

    // Operators
    TOKEN_PLUS,             // +
    TOKEN_MINUS,            // -
    TOKEN_STAR,             // *
    TOKEN_SLASH,            // /
    TOKEN_PERCENT,          // %
    TOKEN_POWER,            // **
    TOKEN_ASSIGN,           // =
    TOKEN_EQ,               // ==
    TOKEN_NEQ,              // !=
    TOKEN_LT,               // <
    TOKEN_GT,               // >
    TOKEN_LTE,              // <=
    TOKEN_GTE,              // >=
    TOKEN_LOGICAL_AND,      // and
    TOKEN_LOGICAL_OR,       // or
    TOKEN_LOGICAL_NOT,      // not
    TOKEN_BITWISE_AND,      // &
    TOKEN_BITWISE_OR,       // |
    TOKEN_BITWISE_XOR,      // ^
    TOKEN_BITWISE_NOT,      // ~ (when unary before operand)
    TOKEN_LSHIFT,           // <<
    TOKEN_RSHIFT,           // >>
    TOKEN_ARROW,            // ->
    TOKEN_FAT_ARROW,        // =>
    TOKEN_QUESTION,         // ?

    // Delimiters
    TOKEN_LPAREN,           // (
    TOKEN_RPAREN,           // )
    TOKEN_LBRACKET,         // [
    TOKEN_RBRACKET,         // ]
    TOKEN_LBRACE,           // {
    TOKEN_RBRACE,           // }
    TOKEN_COMMA,            // ,
    TOKEN_COLON,            // :
    TOKEN_DOT,              // .

    // Statement Terminator
    TOKEN_TERMINATOR        // ~
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
    size_t line;
    size_t column;
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        char *string_val;
    } value;
} Token;

typedef struct {
    const char *source;
    const char *filename;
    size_t cursor;
    size_t line;
    size_t column;
    Token last_token;
} Lexer;

void lexer_init(Lexer *lexer, const char *source, const char *filename);
Token lexer_next_token(Lexer *lexer);
const char* token_type_to_string(TokenType type);

#endif
