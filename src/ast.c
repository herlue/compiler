#include "ast.h"

ast_node_t* ast_alloc(arena_t* arena, ast_kind_t kind) {
  ast_node_t* node = arena_alloc(arena, sizeof(ast_node_t), _Alignof(ast_node_t)); // _Alignof deprecated, use alignof instead?
  if (!node)
    return NULL;

  node->kind = kind;

  return node;
}
