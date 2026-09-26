#include "token.h"

#include <string.h>

tokentype_t token_identifier_type(const char* lexeme, size_t length) {
  switch (length) {
    case 2:
      if (memcmp(lexeme, "as", 2) == 0) return TOK_AS;
      if (memcmp(lexeme, "i8", 2) == 0) return TOK_I8;
      if (memcmp(lexeme, "if", 2) == 0) return TOK_IF;
      if (memcmp(lexeme, "or", 2) == 0) return TOK_LOR;
      if (memcmp(lexeme, "u8", 2) == 0) return TOK_U8;
      break;
    case 3:
      if (memcmp(lexeme, "and", 3) == 0) return TOK_LAND;
      if (memcmp(lexeme, "f32", 3) == 0) return TOK_F32;
      if (memcmp(lexeme, "f64", 3) == 0) return TOK_F64;
      if (memcmp(lexeme, "for", 3) == 0) return TOK_FOR;
      if (memcmp(lexeme, "i16", 3) == 0) return TOK_I16;
      if (memcmp(lexeme, "i32", 3) == 0) return TOK_I32;
      if (memcmp(lexeme, "i64", 3) == 0) return TOK_I64;
      if (memcmp(lexeme, "not", 3) == 0) return TOK_LNOT;
      if (memcmp(lexeme, "u16", 3) == 0) return TOK_U16;
      if (memcmp(lexeme, "u32", 3) == 0) return TOK_U32;
      if (memcmp(lexeme, "u64", 3) == 0) return TOK_U64;
      if (memcmp(lexeme, "use", 3) == 0) return TOK_USE;
      if (memcmp(lexeme, "xor", 3) == 0) return TOK_LXOR;
      break;
    case 4:
      if (memcmp(lexeme, "bool", 4) == 0) return TOK_BOOL;
      if (memcmp(lexeme, "byte", 4) == 0) return TOK_BYTE;
      if (memcmp(lexeme, "else", 4) == 0) return TOK_ELSE;
      if (memcmp(lexeme, "true", 4) == 0) return TOK_BOOLLIT;
      break;
    case 5:
      if (memcmp(lexeme, "false", 5) == 0) return TOK_BOOLLIT;
      if (memcmp(lexeme, "while", 5) == 0) return TOK_WHILE;
      break;
    case 6:
      if (memcmp(lexeme, "record", 6) == 0) return TOK_RECORD;
      if (memcmp(lexeme, "return", 6) == 0) return TOK_RET;
      break;
    case 9:
      if (memcmp(lexeme, "namespace", 9) == 0) return TOK_NS;
      break;
  }

  return TOK_ID;
}

tokentype_t token_operator_type(char c) {
  switch (c) {
    case '*': return TOK_ASTERISK;
    case '&': return TOK_BAND;
    case '!': return TOK_BNOT;
    case '|': return TOK_BOR;
    case '^': return TOK_BXOR;
    case ':': return TOK_COL; // maybe ::
    case ',': return TOK_COMMA;
    case '.': return TOK_DOT;
    case '=': return TOK_EQ; // maybe ==
    case '>': return TOK_GT; // maybe >= or >>
    case '[': return TOK_LBRACK;
    case ']': return TOK_RBRACK;
    case '{': return TOK_LCURL;
    case '}': return TOK_RCURL;
    case '(': return TOK_LPAREN;
    case ')': return TOK_RPAREN;
    case '<': return TOK_LT; // maybe <= or <<
    case '-': return TOK_MINUS; // maybe --
    case '%': return TOK_MOD;
    case '+': return TOK_PLUS; // maybe ++
    case ';': return TOK_SEMICOLON;
    case '/': return TOK_SLASH;
    default: return TOK_ERR;
  }
}
