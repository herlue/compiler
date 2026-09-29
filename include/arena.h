#pragma once

#include <stddef.h>

typedef struct {
  unsigned char* buffer;
  size_t capacity;
  size_t offset;
} arena_t;

arena_t arena_init(size_t);
void* arena_alloc(arena_t*, size_t, size_t);
void arena_free(arena_t*);
void arena_rewind(arena_t*, size_t);
