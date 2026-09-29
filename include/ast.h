#pragma once

#include "token.h"
#include "arena.h"

#include <stdbool.h>

typedef enum {
  AST_ID,
  AST_QUAL_ID,
  AST_USE_DECL,
  AST_PROGRAM,
  AST_IDENTIFER,
  AST_LITERAL,
  AST_BINARY,
  AST_UNARY,
  AST_CALL,
  AST_INDEX,
  AST_MEMBER,
  AST_VAR_DECL,
  AST_FUNC_DECL,
  AST_RECORD_DECL,
  AST_NS_DECL,
  AST_BLOCK,
  AST_IF,
  AST_FOR,
  AST_WHILE,
  AST_RETURN
} ast_kind_t;

typedef struct ast_node ast_node_t;

typedef struct ast_node_list {
  ast_node_t** items;
  size_t count;
} ast_node_list_t;

struct ast_node {
  ast_kind_t kind;
  src_span_t src_span;

  union {
    struct {
      ast_node_t* namespace;
      ast_node_list_t uses;
    } program;

    struct {
      ast_node_t* name; // AST_QUAL_ID
    } ns_decl;

    struct {
      ast_node_list_t parts;
    } qual_id;

    // not really necessary since the token's lexeme is the identifier name
    struct {
      const char* name;
      size_t length;
    } id;

    struct {
      ast_node_t* name; // AST_QUAL_ID
      ast_node_t* alias; // AST_ID or NULL
    } use_decl;


    // struct {
    //   token_t token;
    // } literal;

    // struct {
    //   token_t token;
    // } identifier;

    // struct {
    //   tokentype_t op;
    //   ast_node_t* left;
    //   ast_node_t* right;
    // } binary;

    // struct {
    //   tokentype_t op;
    //   ast_node_t* operand;
    // } unary;
  };
};

typedef struct {
  ast_node_t** items;
  size_t count;
  size_t capacity;
} ast_node_stack_t;

ast_node_t* ast_alloc(arena_t*, ast_kind_t);

ast_node_stack_t ast_node_stack_init(size_t);
bool ast_node_stack_push(ast_node_stack_t*, ast_node_t*);
ast_node_t* ast_node_stack_pop(ast_node_stack_t*);
ast_node_t* ast_node_stack_top(ast_node_stack_t*);