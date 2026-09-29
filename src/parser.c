#include "parser.h"
#include "parse.h"

parser_t parser_init(scanner_t* scanner, arena_t* arena, ast_node_stack_t* stack) {
  parser_t parser;
  parser.scanner = scanner;
  parser.arena = arena;
  parser.stack = stack;
  parser.current = scanner_next(scanner);
  parser.next = scanner_next(scanner);
  return parser;
}

void parser_advance(parser_t* parser) {
  parser->current = parser->next;
  parser->next = scanner_next(parser->scanner);
}

// needed?
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

ast_node_t* parser_parse(parser_t* parser) {
  return parse_program(parser);
}

bool parser_lookahead(parser_t* parser, bool (*parse)(parser_t*)) {
  parser_t tmp = *parser;
  scanner_t scanner = *parser->scanner;
  tmp.scanner = &scanner;
  tmp.arena = NULL;

  return parse(&tmp);
}
