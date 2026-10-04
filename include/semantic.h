#pragma once

#include "ast.h"
#include "arena.h"
#include <stddef.h>

typedef enum {
  SYMBOL_VAR,
  SYMBOL_RECORD,
  SYMBOL_FUNC,
  SYMBOL_PARAM
} symbol_kind_t;

typedef enum {
  TYPE_ERROR,
  TYPE_BOOL,
  TYPE_BYTE,
  TYPE_FLOAT,
  TYPE_INT,
  TYPE_STRING
} semantic_type_t;

typedef struct symbol symbol_t;

struct symbol {
  const char* name;
  size_t name_length;
  symbol_kind_t kind;
  ast_node_t* declaration;
  semantic_type_t type;

  symbol_t* next;
};

typedef struct scope scope_t;

struct scope {
  scope_t* parent;
  symbol_t* symbols;
};

typedef struct {
  arena_t* arena;
  scope_t* global_scope;
  scope_t* current_scope;
} semantic_context_t;

semantic_context_t semantic_context_init(arena_t*);
scope_t* scope_create(semantic_context_t*, scope_t*);

symbol_t* symbol_create(semantic_context_t*, const char*, size_t, symbol_kind_t, ast_node_t*, semantic_type_t);

symbol_t* scope_lookup(semantic_context_t*, const char*, size_t);

bool semantic_analysis(semantic_context_t*, ast_node_t*);
