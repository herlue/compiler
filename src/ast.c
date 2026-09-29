#include "ast.h"

#include <stdlib.h>

ast_node_t* ast_alloc(arena_t* arena, ast_kind_t kind) {
  ast_node_t* node = arena_alloc(arena, sizeof(ast_node_t), _Alignof(ast_node_t)); // _Alignof deprecated, use alignof instead?
  if (!node)
    return NULL;

  node->kind = kind;

  return node;
}

ast_node_stack_t ast_node_stack_init(size_t capacity) {
  ast_node_stack_t stack;
  stack.items = malloc(capacity * sizeof(ast_node_t*));
  stack.capacity = stack.items ? capacity : 0;
  stack.count = 0;
  return stack;
}

bool ast_node_stack_push(ast_node_stack_t* stack, ast_node_t* node) {
  if (!stack || stack->count >= stack->capacity || !node) return false;

  stack->items[stack->count++] = node;

  return true;
} 

ast_node_t* ast_node_stack_pop(ast_node_stack_t* stack) {
  if (stack->count == 0) return NULL;

  return stack->items[--stack->count];
}

ast_node_t* ast_node_stack_top(ast_node_stack_t* stack) {
  if (stack->count == 0) return NULL;
  return stack->items[stack->count - 1];
}
