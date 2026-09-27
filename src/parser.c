#include "parser.h"
#include "parse.h"

parser_t parser_init(scanner_t* scanner, arena_t* arena) {
  parser_t parser;
  parser.scanner = scanner;
  parser.arena = arena;
  parser.current = scanner_next(scanner);
  parser.next = scanner_next(scanner);
  return parser;
}

void parser_advance(parser_t* parser) {
  parser->current = parser->next;
  parser->next = scanner_next(parser->scanner);
}

bool parser_check(parser_t* parser, tokentype_t type) {
  return parser->current.type == type;
}

bool parser_consume(parser_t* parser, tokentype_t type) {
  if (parser->current.type != type) {
    return false;
  }
  parser_advance(parser);
  return true;
}

bool parser_parse(parser_t* parser) {
  if (parser->current.type == TOK_NS) {
    if (!parse_namespace_declaration(parser)) {
      return false;
    }
  }
  while (parser->current.type == TOK_USE) {
    if (!parse_use_declaration(parser)) {
      return false;
    }
  }
  while (parser->current.type != TOK_EOF) {
    if (!parse_top_level_declaration(parser)) {
      return false;
    }
  }
  return parser_consume(parser, TOK_EOF);
}
