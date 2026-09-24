#pragma once

#include "token.h"

typedef struct {
  const char* source;
  size_t length;
  src_pos_t current;
} scanner_t;

scanner_t scanner_init(const char*, size_t);
token_t scanner_next(scanner_t*);
