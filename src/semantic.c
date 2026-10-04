#include "semantic.h"
#include "arena.h"
#include "ast.h"

#include <stddef.h>
#include <string.h>

static symbol_t* symbol_alloc(arena_t* arena, symbol_kind_t kind) {
  symbol_t* symbol = arena_alloc(arena, sizeof(symbol_t), _Alignof(symbol_t));
  if (!symbol)
    return NULL;

  symbol->kind = kind;

  return symbol;
}

static scope_t* scope_alloc(arena_t* arena) {
  return (scope_t*) arena_alloc(arena, sizeof(scope_t), _Alignof(scope_t));
}

semantic_context_t semantic_context_init(arena_t* arena) {
  semantic_context_t context;
  context.arena = arena;
  context.global_scope = NULL;
  context.current_scope = NULL;
  return context;
}

scope_t* scope_create(semantic_context_t* context, scope_t* parent) {
  scope_t* scope = scope_alloc(context->arena);
  if (!scope) return NULL;

  scope->symbols = NULL;

  if (parent)
    scope->parent = parent;
  else {
    scope->parent = NULL;
    context->global_scope = scope;
    context->current_scope = scope;
  }

  return scope;
}

symbol_t* symbol_create(
  semantic_context_t* context,
  const char* name,
  size_t name_length,
  symbol_kind_t kind,
  ast_node_t* declaration,
  ast_node_t* type
) {
  symbol_t symbol = {
    .name = name,
    .name_length = name_length,
    .kind = kind,
    .declaration = declaration,
    .type = type
  };

  symbol_t* ptr = context->current_scope->symbols;
  
  if (!ptr) {
    // first symbol in scope
    context->current_scope->symbols = symbol_alloc(context->arena, kind);
    if (!context->current_scope->symbols) return NULL;

    *context->current_scope->symbols = symbol;
    context->current_scope->symbols->next = NULL;
    return context->current_scope->symbols;
  }

  while (true) {
    if (ptr->name_length == name_length && memcmp(ptr->name, symbol.name, symbol.name_length) == 0) {
      return NULL;
      // multiple declaration
    }
    if (ptr->next == NULL) break;
    ptr = ptr->next;
  }

  ptr->next = symbol_alloc(context->arena, kind);
  if (!ptr->next) return NULL;

  *ptr->next = symbol;
  ptr->next->next = NULL;
  return ptr->next;
}

symbol_t* scope_lookup(semantic_context_t* context, const char* name, size_t name_length) {
  symbol_t* symbol = context->current_scope->symbols;
  if (!symbol)
    return NULL;

  scope_t* scope = context->current_scope;

  while (scope) {
    while (symbol) {
      if (
        symbol->name_length == name_length &&
        memcmp(symbol->name, name, name_length)
      )
        return symbol;

      symbol = symbol->next;
    }
    scope = scope->parent;
  }

  return NULL;
}
