#pragma once

#include "token.h"
#include "arena.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  AST_BOOL_LIT,
  AST_BYTE_LIT,
  AST_FLOAT_LIT,
  AST_INT_LIT,
  AST_STR_LIT,
  AST_REC_LIT,

  AST_REC_FIELD_INIT,
  AST_FUNC_PARAM,
  AST_FUNC_RECEIVER,
  AST_BLOCK,

  AST_TYPE,
  AST_BUILTIN_TYPE,
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
  AST_REC_DECL,
  AST_REC_FIELD_DECL,
  AST_NS_DECL,
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
      ast_node_list_t decls;
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

    struct {
      ast_node_t* name; // AST_ID
      ast_node_list_t fields;
    } rec_decl;

    struct {
      ast_node_t* name; // AST_ID
      ast_node_t* type; // AST_TYPE
    } rec_field_decl;

    struct {
      const char* type;
      const char* name;
    } field_decl;

    struct {
      ast_node_t* base; // AST_BUILTIN_TYPE or AST_QUAL_ID
      size_t ptr_depth;
      ast_node_list_t dimensions;
    } type;

    struct {
      tokentype_t type;

    } builtin_type;

    struct {
      union {
        // fixed types
        uint64_t int_value;
        double float_value;
        unsigned char byte_value;
        bool bool_value;
        struct {
          const char* data;
          size_t length;
        } string_value;
      } value;
    } lit;

    struct {
      ast_node_t* type_name; // AST_QUAL_ID
      ast_node_list_t fields;
    } rec_lit;

    struct {
      ast_node_t* name; // AST_ID
      ast_node_t* value; // expression
    } rec_field_init;

    struct {
      ast_node_t* type; // AST_TYPE
      ast_node_t* name; // AST_ID
      ast_node_t* expr; // AST_EXPR
      // expression

    } var_decl;

    struct {
      ast_node_t* return_type; // AST_TYPE
      ast_node_t* receiver; // AST_FUNC_RECEIVER
      ast_node_t* name; // AST_ID
      ast_node_list_t params;
      ast_node_t* body; // AST_BLOCK
    } func_decl;

    struct {
      ast_node_t* type_name; // AST_ID
      ast_node_t* name; // AST_ID 
    } func_receiver;

    struct {
      ast_node_t* type; // AST_TYPE
      ast_node_t* name; // AST_ID
    } func_param;

    struct {
      ast_node_list_t statements;
    } block;
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
