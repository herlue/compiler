#include "scanner.h"
#include "types.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static token_t scanner_make_token(const scanner_t* scanner, tokentype_t type, src_pos_t start) {
  token_t token;
  token.type = type;
  token.lexeme = type == TOK_EOF ? NULL : scanner->source + start.offset;
  token.length = scanner->current.offset - start.offset;
  token.src_span.start = start;
  token.src_span.end = scanner->current;
  return token;
}

scanner_t scanner_init(const char* source, size_t length) {
  return (scanner_t) {
    .source = source,
    .length = length,
    .current = (src_pos_t) {
      .line = 1,
      .column = 0,
      .offset = 0
    }
  };
}

static bool scanner_at_end(const scanner_t* scanner) {
  return scanner->current.offset >= scanner->length;
}

static uchar scanner_next_char(scanner_t* scanner) {
  uchar c = scanner->source[scanner->current.offset++];
  if (c == '\n') {
    scanner->current.line++;
    scanner->current.column = 0;
  } else {
    scanner->current.column++;
  }
  return c;
}

static uchar scanner_peek_char(const scanner_t* scanner) {
  return scanner->source[scanner->current.offset];
}

token_t scanner_next(scanner_t* scanner) {
  while (!scanner_at_end(scanner) && isspace(scanner_peek_char(scanner))) {
    scanner_next_char(scanner);
  }

  src_pos_t start = scanner->current;

  if (scanner_at_end(scanner)) {
    return scanner_make_token(scanner, TOK_EOF, start);
  }

  uchar c = scanner_next_char(scanner);

  // check for keyword or identifier
  if (isalpha(c) || c == '_') {  
    while (!scanner_at_end(scanner)) {
      c = scanner_peek_char(scanner);
      if (!isalnum(c) && c != '_') {
        break;
      }
      scanner_next_char(scanner);
    }

    token_t token = scanner_make_token(scanner, TOK_ID, start);
    token.type = token_identifier_type(token.lexeme, token.length);
    return token;
  }

  // by now, we only support numeric literals in decimal format without size suffixes
  if (isdigit(c)) {
    tokentype_t type = TOK_INTLIT;

    while (!scanner_at_end(scanner)) {
      c = scanner_peek_char(scanner);

      if (!isdigit(c)) {
        if (isalpha(c)) {
          // alpha characters following digits without whitespace in between are not allowed -> syntax error!
          return scanner_make_token(scanner, TOK_ERR, start);
        }
        if (c == '.') {
          if (type == TOK_FLOATLIT) {
            return scanner_make_token(scanner, TOK_ERR, start); // two dots found
          }
          type = TOK_FLOATLIT;
        } else {
          break;
        }
      }
      scanner_next_char(scanner);
    }
    
    return scanner_make_token(scanner, type, start);
  }

  // check for string literal
  if (c == '"') {
    while (!scanner_at_end(scanner)) {
      c = scanner_peek_char(scanner);
      if (c == '"') {
        scanner_next_char(scanner);
        return scanner_make_token(scanner, TOK_STRLIT, start);
      }
      if (c == '\n') {
        break; // string literals must be one line
      }
      scanner_next_char(scanner);
    }
    return scanner_make_token(scanner, TOK_ERR, start);
  }

  tokentype_t type;

  switch (c) {
    case '*':
      type = TOK_ASTERISK;
      break;
    case ':':
      if (!scanner_at_end(scanner) && scanner_peek_char(scanner) == ':') {
        scanner_next_char(scanner);
        type = TOK_COLCOL;
      } else {
        type = TOK_COL;
      }
      break;
    case ',':
      type = TOK_COMMA;
      break;
    case '.':
      type = TOK_DOT;
      break;
    case '=':
      if (!scanner_at_end(scanner) && scanner_peek_char(scanner) == '=') {
        scanner_next_char(scanner);
        type = TOK_EQEQ;
      } else {
        type = TOK_EQ;
      }
      break;
    case '>':
      if (!scanner_at_end(scanner)) {
        c = scanner_peek_char(scanner);
        if (c == '=') {
          type = TOK_GTEQ;
          scanner_next_char(scanner);
          break;
        }
        if (c == '>') {
          type = TOK_RSHIFT;
          scanner_next_char(scanner);
          break;
        }
      }
      type = TOK_GT;
      break;
    case '[':
      type = TOK_LBRACK;
      break;
    case ']':
      type = TOK_RBRACK;
      break;
    case '{':
      type = TOK_LCURL;
      break;
    case '}':
      type = TOK_RCURL;
      break;
    case '(':
      type = TOK_LPAREN;
      break;
    case ')':
      type = TOK_RPAREN;
      break;
    case '<':
      if (!scanner_at_end(scanner)) {
        c = scanner_peek_char(scanner);
        if (c == '=') {
          type = TOK_LTEQ;
          scanner_next_char(scanner);
          break;
        }
        if (c == '<') {
          type = TOK_LSHIFT;
          scanner_next_char(scanner);
          break;
        }
      }
      type = TOK_LT;
      break;
    case '-':
      if (!scanner_at_end(scanner) && scanner_peek_char(scanner) == '-') {
        scanner_next_char(scanner);
        type = TOK_MINUSMINUS;
      } else {
        type = TOK_MINUS;
      }
      break;
    case '+':
      if (!scanner_at_end(scanner) && scanner_peek_char(scanner) == '+') {
        scanner_next_char(scanner);
        type = TOK_PLUSPLUS;
      } else {
        type = TOK_PLUS;
      }
      break;
    case ';':
      type = TOK_SEMICOLON;
      break;
    case '/':
      type = TOK_SLASH;
      break;
    default:
      type = TOK_ERR;
  }

  return scanner_make_token(scanner, type, start);
}

