#pragma once

#include "token.h"
#include "arena.h"

typedef enum {
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

struct ast_node {
  ast_kind_t kind;
  src_span_t src_span;

  union {
    struct {
      token_t token;
    } literal;

    struct {
      token_t token;
    } identifier;

    struct {
      token_t* parts;
      size_t count;
    } qualified_identifier;

    struct {
      tokentype_t op;
      ast_node_t* left;
      ast_node_t* right;
    } binary;

    struct {
      tokentype_t op;
      ast_node_t* operand;
    } unary;
  };
};

ast_node_t* ast_alloc(arena_t*, ast_kind_t);
