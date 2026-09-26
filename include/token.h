#pragma once

#include <stddef.h>

typedef struct {
  size_t line;
  size_t column;
  size_t offset;
} src_pos_t;

typedef struct {
  src_pos_t start;
  src_pos_t end;
} src_span_t;

typedef enum {
  TOK_AS,
  TOK_ASTERISK,
  TOK_BAND,
  TOK_BNOT,
  TOK_BOOL,
  TOK_BOOLLIT,
  TOK_BOR,
  TOK_BXOR,
  TOK_BYTE,
  TOK_BYTELIT,
  TOK_COL,
  TOK_COLCOL,
  TOK_COMMA,
  TOK_DOT,
  TOK_ELSE,
  TOK_ERR,
  TOK_EOF,
  TOK_EQ,
  TOK_EQEQ,
  TOK_F32,
  TOK_F64,
  TOK_FLOATLIT,
  TOK_FOR,
  TOK_GT,
  TOK_GTEQ,
  TOK_I8,
  TOK_I16,
  TOK_I32,
  TOK_I64,
  TOK_ID,
  TOK_IF,
  TOK_INTLIT,
  TOK_LAND,
  TOK_LBRACK,
  TOK_LCURL,
  TOK_LNOT,
  TOK_LOR,
  TOK_LPAREN,
  TOK_LSHIFT,
  TOK_LT,
  TOK_LTEQ,
  TOK_LXOR,
  TOK_MINUS,
  TOK_MINUSGT,
  TOK_MINUSMINUS,
  TOK_MOD,
  TOK_NOTEQ,
  TOK_NS,
  TOK_PLUS,
  TOK_PLUSPLUS,
  TOK_RBRACK,
  TOK_RCURL,
  TOK_RECORD,
  TOK_RET,
  TOK_RPAREN,
  TOK_RSHIFT,
  TOK_SEMICOLON,
  TOK_SLASH,
  TOK_STRLIT,
  TOK_U8,
  TOK_U16,
  TOK_U32,
  TOK_U64,
  TOK_USE,
  TOK_WHILE,
} tokentype_t;

typedef struct {
  tokentype_t type;
  const char* lexeme;
  size_t length;
  src_span_t src_span;
} token_t;

tokentype_t token_identifier_type(const char*, size_t);
tokentype_t token_operator_type(char);
