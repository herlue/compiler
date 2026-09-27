#pragma once

#include "scanner.h"

#include <stdbool.h>

typedef struct {
  scanner_t* scanner;
  token_t current;
  token_t next;
} parser_t;

parser_t parser_init(scanner_t*);
bool parser_check(parser_t*, tokentype_t);
bool parser_consume(parser_t*, tokentype_t);
bool parser_parse(parser_t*);
void parser_advance(parser_t*);
