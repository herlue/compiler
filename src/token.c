#include "token.h"

#include <string.h>

tokentype_t token_identifier_type(const char* lexeme, size_t length) {
  switch (length) {
    case 2:
      if (memcmp(lexeme, "if", 2) == 0) return TOK_IF;
      break;
    case 3:
      if (memcmp(lexeme, "f32", 3) == 0) return TOK_F32;
      if (memcmp(lexeme, "f64", 3) == 0) return TOK_F64;
      if (memcmp(lexeme, "for", 3) == 0) return TOK_FOR;
      if (memcmp(lexeme, "i16", 3) == 0) return TOK_I16;
      if (memcmp(lexeme, "i32", 3) == 0) return TOK_I32;
      if (memcmp(lexeme, "i64", 3) == 0) return TOK_I64;
      if (memcmp(lexeme, "use", 3) == 0) return TOK_USE;
      break;
    case 4:
      if (memcmp(lexeme, "byte", 4) == 0) return TOK_BYTE;
      if (memcmp(lexeme, "else", 4) == 0) return TOK_ELSE;
      break;
    case 5:
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
