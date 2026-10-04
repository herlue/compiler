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
  semantic_type_t type
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
  scope_t* scope = context->current_scope;
  symbol_t* symbol;

  while (scope) {
    symbol = scope->symbols;
    while (symbol) {
      if (
        symbol->name_length == name_length &&
        memcmp(symbol->name, name, name_length) == 0
      )
        return symbol;

      symbol = symbol->next;
    }
    scope = scope->parent;
  }

  return NULL;
}

static semantic_type_t analyze_expr(semantic_context_t*, ast_node_t*);

static semantic_type_t analyze_unary(semantic_context_t* context, ast_node_t* expr) {
  semantic_type_t l_type, r_type;
  l_type = analyze_expr(context, expr->op_binary.l_expr);
  r_type = analyze_expr(context, expr->op_binary.r_expr);

  switch (expr->op_binary.op) {
    case TOK_EQ:
      // if (l_type != ) check assignable
  }
} 

static semantic_type_t analyze_binary(semantic_context_t* context, ast_node_t* expr) {

}

static semantic_type_t analyze_expr(semantic_context_t* context, ast_node_t* expr) {
  symbol_t* symbol;
  switch (expr->kind) {
    case AST_BINARY: return analyze_unary(context, expr);
    case AST_UNARY: return analyze_unary(context, expr);
    case AST_ID:
      symbol = scope_lookup(context, expr->id.name, expr->id.length);
      if (!symbol) return TYPE_ERROR;
      return symbol->type;
    case AST_CALL:
      // ...
    default: return TYPE_ERROR;
  }
  return TYPE_ERROR;
}

static bool analyze_stmt(semantic_context_t* context, ast_node_t* stmt) {

}

static bool analyze_decl(semantic_context_t* context, ast_node_t* decl) {
  switch (decl->kind) {
    case AST_VAR_DECL:
      
  }
}

bool semantic_analysis(semantic_context_t* context, ast_node_t* program) {
  size_t i;

  // analyze decls
  for (i = 0; i < program->program.decls.count; i++) {
    ast_node_t* decl = program->program.decls.items[i];
    if (!analyze_decl(context, decl))
      return false;
  }

  return true;
}
