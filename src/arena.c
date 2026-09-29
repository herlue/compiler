#include "arena.h"

#include <stdlib.h>

arena_t arena_init(size_t capacity) {
  arena_t arena;
  arena.buffer = malloc(capacity);
  arena.capacity = arena.buffer ? capacity : 0;
  arena.offset = 0;
  return arena;
}

void* arena_alloc(arena_t* arena, size_t size, size_t alignment) {
  if (alignment == 0)
    return NULL;

  size_t next_offset = arena->offset;
  size_t remainder = next_offset % alignment;

  if (remainder)
    next_offset += (alignment - remainder);

  if (next_offset + size > arena->capacity)
    return NULL; // alloc more?

  void* ptr = arena->buffer + next_offset;
  arena->offset = next_offset + size;

  return ptr;
}

void arena_free(arena_t* arena) {
  free(arena->buffer);
  arena->capacity = 0;
  arena->offset = 0;
}

void arena_rewind(arena_t* arena, size_t offset) {
  if (arena->offset < offset) return;
  arena->offset = offset;
}

