#pragma once

#include "scanner.h"
#include "arena.h"
#include "ast.h"

#include <stdbool.h>

typedef struct {
  scanner_t* scanner;
  arena_t* arena;
  token_t current;
  token_t next;
} parser_t;

parser_t parser_init(scanner_t*, arena_t*);
bool parser_check(parser_t*, tokentype_t);
bool parser_consume(parser_t*, tokentype_t);
ast_node_t* parser_parse(parser_t*);
void parser_advance(parser_t*);
