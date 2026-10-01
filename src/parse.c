#include "parser.h"
#include "parse.h"
#include "token.h"
#include "ast.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

// program = [ ns_decl ] { use_decl } { top_level_decl }
ast_node_t* parse_program(parser_t* parser) {
  ast_node_t* node = ast_alloc(parser->arena, AST_PROGRAM);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  node->program.namespace = NULL;
  node->program.uses = (ast_node_list_t) { 0 };

  size_t start, count;

  if (parser->current.type == TOK_NS) {
    node->program.namespace = parse_ns_decl(parser);
    if (!node->program.namespace) return NULL;
  }

  start = parser->stack->count;

  while (parser->current.type == TOK_USE)
    if (!ast_node_stack_push(parser->stack, parse_use_decl(parser))) return NULL;

  count = parser->stack->count - start;
  parser->stack->count = start;
  
  if (count > 0) {
    node->program.uses.items = arena_alloc(parser->arena, count * sizeof(ast_node_t*), _Alignof(ast_node_t*));
    if (!node->program.uses.items) return NULL;

    node->program.uses.count = count;
    memcpy(node->program.uses.items, parser->stack->items + start, count * sizeof(ast_node_t*));
  }

  start = parser->stack->count;

  // top_level_decl = { func_decl | rec_decl | var_decl }
  while (parser->current.type != TOK_EOF) {
    if (!ast_node_stack_push(parser->stack, parse_top_level_decl(parser))) return NULL;
  }

  count = parser->stack->count - start;
  parser->stack->count = start;

  if (count > 0) {
    node->program.decls.items = arena_alloc(parser->arena, count * sizeof(ast_node_t*), _Alignof(ast_node_t*));
    if (!node->program.decls.items) return NULL;

    memcpy(node->program.decls.items, parser->stack + start, count * sizeof(ast_node_t*));
    node->program.decls.count = count;
  }

  node->src_span.end = parser->current.src_span.end;

  return node;
}

ast_node_t* parse_top_level_decl(parser_t* parser) {
  if (parser->current.type == TOK_RECORD)
    return parse_rec_decl(parser);

  if (parser_lookahead(parser, is_var_decl_start))
    return parse_var_decl(parser);

  return parse_func_decl(parser);
}

ast_node_t* parse_id(parser_t* parser) {
  if (parser->current.type != TOK_ID) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_ID);
  if (!node) return NULL;

  node->src_span = parser->current.src_span;
  
  node->id.name = parser->current.lexeme;
  node->id.length = parser->current.length;

  parser_advance(parser);

  return node;
}

bool is_qual_id(parser_t* parser) {
  if (!parser_consume(parser, TOK_ID)) return false;

  while (parser_consume(parser, TOK_COLCOL))
    if (!parser_consume(parser, TOK_ID)) return false;

  return true;
}

// qual_id = id { "::" id }
ast_node_t* parse_qual_id(parser_t* parser) {
  if (parser->current.type != TOK_ID) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_QUAL_ID);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  size_t start = parser->stack->count;
  if (!ast_node_stack_push(parser->stack, parse_id(parser))) return NULL;

  while (parser_consume(parser, TOK_COLCOL))
    if (!ast_node_stack_push(parser->stack, parse_id(parser))) return NULL;

  node->src_span.end = ast_node_stack_top(parser->stack)->src_span.end;

  size_t count = parser->stack->count - start;

  node->qual_id.parts.items = arena_alloc(parser->arena, count * sizeof(ast_node_t*), _Alignof(ast_node_t*));
  if (!node->qual_id.parts.items) return NULL;
  node->qual_id.parts.count = count;

  memcpy(node->qual_id.parts.items, parser->stack->items + start, count * sizeof(ast_node_t*));

  parser->stack->count = start;

  return node;
}

// ns_decl = "namespace" qual_id ";"
ast_node_t* parse_ns_decl(parser_t* parser) {
  if (parser->current.type != TOK_NS) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_NS_DECL);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;
  parser_advance(parser);

  node->ns_decl.name = parse_qual_id(parser);
  if (!node->ns_decl.name) return NULL;

  if (parser->current.type != TOK_SEMICOLON) return NULL;

  node->src_span.end = parser->current.src_span.end;

  parser_advance(parser);

  return node;
}

// use_decl = "use" qual_id [ "as" id ]
ast_node_t* parse_use_decl(parser_t* parser) {
  if (parser->current.type != TOK_USE) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_USE_DECL);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;
  node->use_decl.alias = NULL;

  parser_advance(parser);

  node->use_decl.name = parse_qual_id(parser);
  if (!node->use_decl.name) return NULL;

  if (parser_consume(parser, TOK_AS)) {
    node->use_decl.alias = parse_id(parser);
    if (!node->use_decl.alias) return NULL;
  }

  if (parser->current.type != TOK_SEMICOLON) return NULL;

  node->src_span.end = parser->current.src_span.end;

  parser_advance(parser);

  return node;
}

// rec_decl = "record" id "=" "{" { rec_field_decl | rec_decl } "}" ";"
ast_node_t* parse_rec_decl(parser_t* parser) {
  if (parser->current.type != TOK_RECORD) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_REC_DECL);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  parser_advance(parser);

  node->rec_decl.name = parse_id(parser);
  if (!node->rec_decl.name) return NULL;

  if (!parser_consume(parser, TOK_LCURL)) return NULL;

  size_t start, count;
  start = parser->stack->count;

  while (!parser_consume(parser, TOK_RCURL)) {
    if (parser->current.type == TOK_RECORD) {
      if (!ast_node_stack_push(parser->stack, parse_rec_decl(parser))) return NULL;
    }
    else {
      if (!ast_node_stack_push(parser->stack, parse_rec_field_decl(parser))) return NULL;
    }
  }

  if (parser->current.type != TOK_SEMICOLON) return NULL;
  node->src_span.end = parser->current.src_span.end;
  parser_advance(parser);

  count = parser->stack->count - start;
  parser->stack->count = start;

  node->rec_decl.fields = (ast_node_list_t) { 0 };

  if (count > 0) {
    node->rec_decl.fields.items = arena_alloc(parser->arena, count * sizeof(ast_node_t*), _Alignof(ast_node_t*));
    if (!node->rec_decl.fields.items) return NULL;

    memcpy(node->rec_decl.fields.items, parser->stack->items + start, count * sizeof(ast_node_t*));

    node->rec_decl.fields.count = count;
  }

  return node;
}

// rec_field_decl = type id ";"
ast_node_t* parse_rec_field_decl(parser_t* parser) {
  ast_node_t* node = ast_alloc(parser->arena, AST_REC_FIELD_DECL);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  node->rec_field_decl.type = parse_type(parser);
  if (!node->rec_field_decl.type) return NULL;

  node->rec_field_decl.name = parse_id(parser);
  if (!node->rec_field_decl.name) return NULL;

  if (parser->current.type != TOK_SEMICOLON) return NULL;

  node->src_span.end = parser->current.src_span.end;

  parser_advance(parser);

  return node;
}

// type = ( builtin_type | qual_id ) { "*" } { "[" int_lit "]" }
ast_node_t* parse_type(parser_t* parser) {
  ast_node_t* node = ast_alloc(parser->arena, AST_TYPE);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;
  
  if (is_builtin_type(parser)) {
    node->type.base = parse_builtin_type(parser);
    if (!node->type.base) return NULL;
  } else {
    node->type.base = parse_qual_id(parser);
    if (!node->type.base) return NULL;
  }

  size_t ptr_depth = 0;
  while (parser_consume(parser, TOK_ASTERISK))
    ptr_depth++;

  node->type.ptr_depth = ptr_depth;

  size_t start, count;
  start = parser->stack->count;

  while (parser_consume(parser, TOK_LBRACK)) {
    if (parser_consume(parser, TOK_RBRACK)) {
      // this is a pretty freaky NULL ptr workaround - do not try this at home
      parser->stack->items[parser->stack->count++] = (ast_node_t*) NULL;
      continue;
    }

    if (!ast_node_stack_push(parser->stack, parse_int_lit(parser))) return NULL;
    
    if (!parser_consume(parser, TOK_RBRACK)) return NULL;
  }

  node->src_span.end = parser->current.src_span.end;
  node->type.dimensions = (ast_node_list_t) { 0 };

  count = parser->stack->count - start;
  parser->stack->count = start;

  if (count > 0) {
    node->type.dimensions.items = arena_alloc(parser->arena, count * sizeof(ast_node_t*), _Alignof(ast_node_t*));
    memcpy(node->type.dimensions.items, parser->stack->items + start, count * sizeof(ast_node_t*));
    if (!node->type.dimensions.items) return NULL;

    node->type.dimensions.count = count;
  }

  return node;
}

bool is_builtin_type(parser_t* parser) {
  switch (parser->current.type) {
    case TOK_BOOL:
    case TOK_BYTE:
    case TOK_I8: case TOK_I16: case TOK_I32: case TOK_I64:
    case TOK_U8: case TOK_U16: case TOK_U32: case TOK_U64:
    case TOK_F32: case TOK_F64:
    case TOK_STRING:
      return true;
    default: return false;
  }
}
ast_node_t* parse_builtin_type(parser_t* parser) {
  if (!is_builtin_type(parser)) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_BUILTIN_TYPE);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;
  node->builtin_type.type = parser->current.type;

  parser_advance(parser);

  node->src_span.end = parser->current.src_span.end;

  return node;
}

bool is_primitve_lit(parser_t* parser) {
  switch (parser->current.type) {
    case TOK_BOOLLIT:
    case TOK_BYTELIT:
    case TOK_FLOATLIT:
    case TOK_INTLIT:
    case TOK_STRLIT:
      return true;
    default: false;
  }
  return false;
}

ast_node_t* parse_primitive_lit(parser_t* parser) {
  switch (parser->current.type) {
    case TOK_BOOLLIT: return parse_bool_lit(parser);
    case TOK_BYTELIT: return parse_byte_lit(parser);
    case TOK_FLOATLIT: return parse_float_lit(parser);
    case TOK_INTLIT: return parse_int_lit(parser);
    case TOK_STRLIT: return parse_str_lit(parser);
    default: return NULL;
  }
}

// bool_lit = "true" | "false"
ast_node_t* parse_bool_lit(parser_t* parser) {
  if (parser->current.type != TOK_BOOLLIT) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_BOOL_LIT);
  if (!node) return NULL;

  node->src_span = parser->current.src_span;

  if (memcmp(parser->current.lexeme, "true", 4) == 0)
    node->lit.value.bool_value = true;
  else
    node->lit.value.bool_value = false;

  parser_advance(parser);

  return node;
}

ast_node_t* parse_byte_lit(parser_t* parser) {
  if (parser->current.type != TOK_BYTELIT) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_BYTE_LIT);
  if (!node) return NULL;

  node->src_span = parser->current.src_span;
  
  if (parser->current.length == 2)
    node->lit.value.byte_value = '\0';
  else
    node->lit.value.byte_value = parser->current.lexeme[1];

  parser_advance(parser);

  return node;
}

ast_node_t* parse_float_lit(parser_t* parser) {
  if (parser->current.type != TOK_FLOATLIT) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_FLOAT_LIT);
  if (!node) return NULL;

  node->src_span = parser->current.src_span;

  // temporary 0-teriminated buffer for float lit
  size_t tmplen = parser->current.length + 1;
  char tmp[tmplen];
  memcpy(tmp, parser->current.lexeme, tmplen - 1);
  tmp[tmplen - 1] = '\0';

  char* endptr;
  errno = 0;
  double value = strtod(tmp, &endptr);

  if (tmp + tmplen - 1 != endptr || tmp == endptr) return NULL;
  if (errno == ERANGE) return NULL;

  node->lit.value.float_value = value;

  parser_advance(parser);

  return node;
}

ast_node_t* parse_int_lit(parser_t* parser) {
  if (parser->current.type != TOK_INTLIT) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_INT_LIT);
  if (!node) return NULL;

  node->src_span = parser->current.src_span;

  // same as parse_float_lit
  size_t length = parser->current.length;
  char nptr[length + 1];
  char* endptr;
  errno = 0;

  memcpy(nptr, parser->current.lexeme, length);
  nptr[length + 1] = '\0';
  
  unsigned long long value = strtoull(nptr, &endptr, 10);
  if (nptr + length != endptr || nptr == endptr) return NULL;
  if (errno == ERANGE) return NULL;

  node->lit.value.int_value = value;

  parser_advance(parser);

  return node;
}

ast_node_t* parse_str_lit(parser_t* parser) {
  if (parser->current.type != TOK_STRLIT) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_STR_LIT);
  if (!node) return NULL;

  node->src_span = parser->current.src_span;

  size_t length = parser->current.length;
  if (length > 2) {
    node->lit.value.string_value.data = parser->current.lexeme + 1;
    node->lit.value.string_value.length = length - 2;
  } else {
    node->lit.value.string_value.data = NULL;
    node->lit.value.string_value.length = 0;
  }

  parser_advance(parser);

  return node;
}

// literal = primitive_literal | rec_lit
ast_node_t* parse_lit(parser_t* parser) {
  if (is_primitve_lit(parser))
    return parse_primitive_lit(parser);

  return parse_rec_lit(parser);
}

// rec_lit = qual_id "{" [ rec_field_init { "," rec_field_init } ] "}"
ast_node_t* parse_rec_lit(parser_t* parser) {
  if (parser->current.type != TOK_ID) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_REC_LIT);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  node->rec_lit.type_name = parse_qual_id(parser);
  if (!node->rec_lit.type_name) return NULL;

  if (!parser_consume(parser, TOK_LCURL)) return NULL;

  size_t start, count;
  start = parser->stack->count;

  // TODO: Improve logic with comma?
  while (parser->current.type != TOK_RCURL) {
    if (!ast_node_stack_push(parser->stack, parse_rec_field_init(parser)))
      return NULL;

    if (!parser_consume(parser, TOK_COMMA))
      break;
  }

  if (parser->current.type != TOK_RCURL) return NULL;

  count = parser->stack->count - start;
  parser->stack->count = start;

  node->src_span.end = parser->current.src_span.end;
  node->rec_lit.fields = (ast_node_list_t) { 0 };

  if (count > 0) {
    node->rec_lit.fields.items = arena_alloc(parser->arena, count * sizeof(ast_node_t*), _Alignof(ast_node_t*));
    if (!node->rec_lit.fields.items) return NULL;
    memcpy(node->rec_lit.fields.items, parser->stack + start, count * sizeof(ast_node_t*));
    node->rec_lit.fields.count = count;
  }

  parser_advance(parser);

  return node;
}

// rec_field_init = id ":" expression
ast_node_t* parse_rec_field_init(parser_t* parser) {
  if (parser->current.type != TOK_ID) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_REC_FIELD_INIT);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  node->rec_field_init.name = parse_id(parser);
  if (!node->rec_field_init.name) return NULL;

  if (!parser_consume(parser, TOK_COL)) return NULL;

  // WE ALLOW ONLY INTLITS HERE TO TEST IT

  if (parser->current.type != TOK_INTLIT) return NULL;

  node->src_span.end = parser->current.src_span.end;

  node->rec_field_init.value = parse_int_lit(parser);
  if (!node->rec_field_init.value) return NULL;

  return node;
}

bool is_var_decl_start(parser_t* parser) {
  if (is_builtin_type(parser))
    parser_advance(parser);
  else if (!is_qual_id(parser))
    return false;

  if (!parser_consume(parser, TOK_ID)) return false;

  if (parser->current.type == TOK_EQ || parser->current.type == TOK_SEMICOLON) return true;

  return false;
}

// var_decl = type id [ "=" expr ] ";" 
ast_node_t* parse_var_decl(parser_t* parser) {
  ast_node_t* node = ast_alloc(parser->arena, AST_VAR_DECL);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  node->var_decl.type = parse_type(parser);
  if (!node->var_decl.type) return NULL;

  node->var_decl.name = parse_id(parser);
  if (!node->var_decl.name) return NULL;

  if (parser_consume(parser, TOK_EQ)) {
    node->var_decl.expr = parse_expr(parser);
    if (!node->var_decl.expr) return NULL;
  }

  node->src_span.end = parser->current.src_span.end;

  if (!parser_consume(parser, TOK_SEMICOLON)) return NULL;

  return node;
}

// func_decl = type [ func_receiver ] id "(" [ param_list ] ")" block
ast_node_t* parse_func_decl(parser_t* parser) {
  ast_node_t* node = ast_alloc(parser->arena, AST_FUNC_DECL);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  node->func_decl.return_type = parse_type(parser);
  if (!node->func_decl.return_type) return NULL;

  node->func_decl.receiver = NULL;

  if (parser->current.type == TOK_LPAREN) {
    node->func_decl.receiver = parse_func_receiver(parser);
    if (!node->func_decl.receiver) return NULL;
  }

  node->func_decl.name = parse_id(parser);
  if (!node->func_decl.name) return NULL;

  if (!parser_consume(parser, TOK_LPAREN)) return NULL;

  size_t start, count;
  start = parser->stack->count;

  // TODO: maybe improve logic!
  while (!parser_consume(parser, TOK_RPAREN)) {
    if (!ast_node_stack_push(parser->stack, parse_func_param(parser))) return NULL;

    if (parser->current.type == TOK_COMMA)
      parser_advance(parser);
  }

  count = parser->stack->count - start;
  parser->stack->count = start;

  node->func_decl.params = (ast_node_list_t) { 0 };
  if (count > 0) {
    node->func_decl.params.items = arena_alloc(parser->arena, count * sizeof(ast_node_t*), _Alignof(ast_node_t*));
    if (!node->func_decl.params.items) return NULL;

    memcpy(node->func_decl.params.items, parser->stack->items + start, count * sizeof(ast_node_t*));
    node->func_decl.params.count = count;
  }

  node->func_decl.body = parse_block(parser);
  if (!node->func_decl.body) return NULL;

  node->src_span.end = node->func_decl.body->src_span.end;

  return node;
}

// func_receiver = "(" id id ")" "."
ast_node_t* parse_func_receiver(parser_t* parser) {
  if (parser->current.type != TOK_LPAREN) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_FUNC_RECEIVER);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  parser_advance(parser);

  node->func_receiver.type_name = parse_id(parser);
  if (!node->func_receiver.type_name) return NULL;

  node->func_receiver.name = parse_id(parser);
  if (!node->func_receiver.name) return NULL;

  if (!parser_consume(parser, TOK_RPAREN)) return NULL;

  node->src_span.end = parser->current.src_span.end;

  if (!parser_consume(parser, TOK_DOT)) return NULL;

  return node;
}

// func_param = type id
ast_node_t* parse_func_param(parser_t* parser) {
  ast_node_t* node = ast_alloc(parser->arena, AST_FUNC_PARAM);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  node->func_param.type = parse_type(parser);
  if (!node->func_param.type) return NULL;

  node->src_span.end = parser->current.src_span.end;

  node->func_param.name = parse_id(parser);
  if (!node->func_param.name) return NULL;

  return node;
}

ast_node_t* parse_block(parser_t* parser) {
  if (parser->current.type != TOK_LCURL) return NULL;

  ast_node_t* node = ast_alloc(parser->arena, AST_BLOCK);
  if (!node) return NULL;

  node->src_span.start = parser->current.src_span.start;

  parser_advance(parser);

  // statements ...

  node->src_span.end = parser->current.src_span.end;

  if (!parser_consume(parser, TOK_RCURL)) return NULL;

  return node;
}

ast_node_t* parse_expr(parser_t* parser) {
  return parse_int_lit(parser);
}




// bool parse_expression(parser_t* parser) {
//   return parse_assignment(parser);
// }

// //assignment = logical_or [ "=" assignment ]
// bool parse_assignment(parser_t* parser) {
//   if (!parse_logical_or(parser)) {
//     return false;
//   }
//   if (parser_consume(parser, TOK_EQ)) {
//     return parse_assignment(parser);
//   }
//   return true;
// }

// bool parse_logical_or(parser_t* parser) {
//   if (!parse_logical_xor(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_LOR)) {
//     if (!parse_logical_xor(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// bool parse_logical_xor(parser_t* parser) {
//   if (!parse_logical_and(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_LXOR)) {
//     if (!parse_logical_and(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// bool parse_logical_and(parser_t* parser) {
//   if (!parse_bitwise_or(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_LAND)) {
//     if (!parse_bitwise_or(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// bool parse_bitwise_or(parser_t* parser) {
//   if (!parse_bitwise_xor(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_BOR)) {
//     if (!parse_bitwise_xor(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// bool parse_bitwise_xor(parser_t* parser) {
//   if (!parse_bitwise_and(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_BXOR)) {
//     if (!parse_bitwise_and(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// // bitwise_and = equality { "&" equality }
// bool parse_bitwise_and(parser_t* parser) {
//   if (!parse_equality(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_BAND)) {
//     if (!parse_equality(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// // equality = comparison { ( "==" | "!=" ) comparison }
// bool parse_equality(parser_t* parser) {
//   if (!parse_comparison(parser)) {
//     return false;
//   }
//   while (true) {
//     switch (parser->current.type) {
//       case TOK_EQEQ:
//       case TOK_NOTEQ:
//         parser_advance(parser);
//         if (!parse_comparison(parser)) {
//           return false;
//         }
//         break;
//       default: return true;
//     }
//   }
// }

// // comparison = shift { ( "<" | "<=" | ">" | ">=" ) shift }
// bool parse_comparison(parser_t* parser) {
//   if (!parse_shift(parser)) {
//     return false;
//   }
//   while (true) {
//     switch (parser->current.type) {
//       case TOK_LT:
//       case TOK_LTEQ:
//       case TOK_GT:
//       case TOK_GTEQ:
//         parser_advance(parser);
//         if (!parse_shift(parser)) {
//           return false;
//         }
//         break;
//       default: return true;
//     }
//   }
// }

// // shift = additive { ( "<<" | ">>" ) additive }
// bool parse_shift(parser_t* parser) {
//   if (!parse_additive(parser)) {
//     return false;
//   }
//   while (true) {
//     switch (parser->current.type) {
//       case TOK_LSHIFT:
//       case TOK_RSHIFT:
//         parser_advance(parser);
//         if (!parse_additive(parser)) {
//           return false;
//         }
//         break;
//       default: return true;
//     }
//   }
// }

// // additive = multiplicative { ( "+" | "-" ) multiplicative }
// bool parse_additive(parser_t* parser) {
//   if (!parse_multiplicative(parser)) {
//     return false;
//   }
//   while (true) {
//     switch (parser->current.type) {
//       case TOK_PLUS:
//       case TOK_MINUS:
//         parser_advance(parser);
//         if (!parse_multiplicative(parser)) {
//           return false;
//         }
//         break;
//       default: return true;
//     }
//   }
// }

// bool parse_multiplicative(parser_t* parser) {
//   if (!parse_unary(parser)) {
//     return false;
//   }
//   while (true) {
//     switch (parser->current.type) {
//       case TOK_ASTERISK:
//       case TOK_SLASH:
//       case TOK_MOD:
//         parser_advance(parser);
//         if (!parse_unary(parser)) {
//           return false;
//         }
//         break;
//       default: return true;
//     }
//   }
// }

// bool parse_unary(parser_t* parser) {
//   while (true) {
//     switch (parser->current.type) {
//       case TOK_ASTERISK:
//       case TOK_BAND:
//       case TOK_BNOT:
//       case TOK_MINUS:
//       case TOK_LNOT:
//         parser_advance(parser);
//         break;
//       default: return parse_postfix(parser);
//     }
//   }
// }

// bool parse_argument_list(parser_t* parser) {
//   if (!parse_expression(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_COMMA)) {
//     if (!parse_expression(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// bool parse_postfix(parser_t* parser) {
//   if (!parse_primary(parser)) {
//     return false;
//   }
//   while (true) {
//     if (parser_consume(parser, TOK_LPAREN)) {
//       if (!parser_check(parser, TOK_RPAREN) && !parse_argument_list(parser)) {
//         return false;
//       } 
//       if (!parser_consume(parser, TOK_RPAREN)) {
//         return false;
//       }
//     } else if (parser_consume(parser, TOK_LBRACK)) {
//       if (!parse_expression(parser)) {
//         return false;
//       }
//       if (!parser_consume(parser, TOK_RBRACK)) {
//         return false;
//       }
//     } else if (parser_consume(parser, TOK_DOT)) {
//       if (!parser_consume(parser, TOK_ID)) {
//         return false;
//       }
//     } else if (parser_consume(parser, TOK_MINUSGT)) {
//       if (!parser_consume(parser, TOK_ID)) {
//         return false;
//       }
//     } else {
//       break;
//     }
//   }
//   if (parser_consume(parser, TOK_PLUSPLUS)); else parser_consume(parser, TOK_MINUSMINUS);

//   return true;
// }

// // bool parse_primary(parser_t* parser) {
// //   if (parse_primitive_literal(parser)) {
// //     return true;
// //   }

// //   if (parser->current.type == TOK_ID && (parser->next.type == TOK_LCURL || parser->next.type == TOK_COLCOL)) {
// //     return parse_record_literal(parser);
// //   }

// //   if (parser_check(parser, TOK_ID)) {
// //     return parse_qual_id(parser);
// //   }
// //   if (parser_consume(parser, TOK_LPAREN)) {
// //     if (!parse_expression(parser)) {
// //       return false;
// //     }
// //     return parser_consume(parser, TOK_RPAREN);
// //   }
// //   return false;
// // }

// bool parse_primary(parser_t* parser) {
//   if (parse_primitive_literal(parser)) {
//     return true;
//   }

//   if (parser_check(parser, TOK_ID)) {
//     scanner_t saved_scanner = *parser->scanner;
//     token_t saved_current = parser->current;
//     token_t saved_next = parser->next;

//     if (parse_qual_id(parser) &&
//         parser_check(parser, TOK_LCURL)) {
//       *parser->scanner = saved_scanner;
//       parser->current = saved_current;
//       parser->next = saved_next;

//       return parse_record_literal(parser);
//     }

//     *parser->scanner = saved_scanner;
//     parser->current = saved_current;
//     parser->next = saved_next;

//     return parse_qual_id(parser);
//   }

//   if (parser_consume(parser, TOK_LPAREN)) {
//     if (!parse_expression(parser)) {
//       return false;
//     }

//     return parser_consume(parser, TOK_RPAREN);
//   }

//   return false;
// }

// bool parse_primitive_literal(parser_t* parser) {
//   switch (parser->current.type) {
//     case TOK_BOOLLIT:
//     case TOK_BYTELIT:
//     case TOK_FLOATLIT:
//     case TOK_INTLIT:
//     case TOK_STRLIT:
//       parser_advance(parser);
//       return true;
//     default:
//       return false;
//   }
// }

// bool parse_record_literal(parser_t* parser) {
//   if (!parse_qual_id(parser)) {
//     return false;
//   }
//   if (!parser_consume(parser, TOK_LCURL)) {
//     return false;
//   }
//   if (!parser_check(parser, TOK_RCURL)) {
//     if (!parse_field_initializer(parser)) {
//       return false;
//     }
//     while (parser_consume(parser, TOK_COMMA)) {
//       if (!parse_field_initializer(parser)) {
//         return false;
//       }
//     }
//   }
//   return parser_consume(parser, TOK_RCURL);
// }

// bool parse_field_initializer(parser_t* parser) {
//   if (!parser_consume(parser, TOK_ID)) {
//     return false;
//   }
//   if (!parser_consume(parser, TOK_COL)) {
//     return false;
//   }
//   return parse_expression(parser);
// }

// bool parse_function_declaration(parser_t* parser) {
//   if (!(parser->current.type == TOK_ID &&
//         parser->next.type == TOK_LPAREN)) {
//     if (!parse_type(parser)) {
//       return false;
//     }
//   }

//   if (!parser_consume(parser, TOK_ID)) {
//     return false;
//   }

//   if (!parser_consume(parser, TOK_LPAREN)) {
//     return false;
//   }

//   if (!parser_check(parser, TOK_RPAREN)) {
//     if (!parse_parameter_list(parser)) {
//       return false;
//     }
//   }

//   if (!parser_consume(parser, TOK_RPAREN)) {
//     return false;
//   }

//   return parse_block(parser);
// }

// bool parse_parameter_list(parser_t* parser) {
//   if (!parse_parameter(parser)) {
//     return false;
//   }
//   while (parser_consume(parser, TOK_COMMA)) {
//     if (!parse_parameter(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// bool parse_parameter(parser_t* parser) {
//   if (!parse_type(parser)) {
//     return false;
//   }
//   return parser_consume(parser, TOK_ID);
// }

// bool parse_block(parser_t* parser) {
//   if (!parser_consume(parser, TOK_LCURL)) {
//     return false;
//   }
//   while (!parser_check(parser, TOK_RCURL)) {
//     if (!parse_statement(parser)) {
//       return false;
//     }
//   }

//   return parser_consume(parser, TOK_RCURL);
// }

// bool parse_statement(parser_t* parser) {
//   switch (parser->current.type) {
//     case TOK_RECORD: return parse_record_declaration(parser);
//     case TOK_IF: return parse_if_statement(parser);
//     case TOK_FOR: return parse_for_statement(parser);
//     case TOK_WHILE: return parse_while_statement(parser);
//     case TOK_RET: return parse_return_statement(parser);
//     case TOK_LCURL: return parse_block(parser);
//     default: break;
//   }

//   scanner_t saved_scanner = *parser->scanner;
//   token_t saved_current = parser->current;
//   token_t saved_next = parser->next;

//   bool result = parse_type(parser) && parser_check(parser, TOK_ID);

//   *parser->scanner = saved_scanner;
//   parser->current = saved_current;
//   parser->next = saved_next;

//   if (result) return parse_variable_declaration(parser);

//   return parse_expression_statement(parser);
// }

// bool parse_if_statement(parser_t* parser) {
//   if (!parser_consume(parser, TOK_IF)) {
//     return false;
//   }
//   if (!parse_expression(parser)) {
//     return false;
//   }
//   if (!parse_block(parser)) {
//     return false;
//   }
 
//   if (!parser_consume(parser, TOK_ELSE)) {
//     return true;
//   }
//   if (parser_check(parser, TOK_IF)) {
//     return parse_if_statement(parser);
//   }
//   if (parser_check(parser, TOK_LCURL)) {
//     return parse_block(parser);
//   }
//   return false;
// }

// bool parse_variable_definition(parser_t* parser) {
//   if (!parse_type(parser)) {
//     return false;
//   }
//   if (!parser_consume(parser, TOK_ID)) {
//     return false;
//   }
//   if (parser_consume(parser, TOK_EQ)) {
//     if (!parse_expression(parser)) {
//       return false;
//     }
//   }
//   return true;
// }

// bool parse_for_statement(parser_t* parser) {
//   if (!parser_consume(parser, TOK_FOR)) {
//     return false;
//   }
//   if (!parser_check(parser, TOK_SEMICOLON)) {
//     // lookahead later
//     scanner_t saved_scanner = *parser->scanner;
//     token_t saved_current = parser->current;
//     token_t saved_next = parser->next;
    
//     bool result = parse_type(parser) && parser_check(parser, TOK_ID);

//     *parser->scanner = saved_scanner;
//     parser->current = saved_current;
//     parser->next = saved_next;

//     if (result) {
//       if (!parse_variable_definition(parser)) {
//         return false;
//       }
//     } else {
//       if (!parse_expression(parser)) {
//         return false;
//       }
//     }
//   }
//   if (!parser_consume(parser, TOK_SEMICOLON)) {
//     return false;
//   }
//   if (!parser_check(parser, TOK_SEMICOLON)) {
//     if (!parse_expression(parser)) {
//       return false;
//     }
//   }
//   if (!parser_consume(parser, TOK_SEMICOLON)) {
//     return false;
//   }
//   if (!parser_check(parser, TOK_LCURL)) {
//     if (!parse_expression(parser)) {
//       return false;
//     }
//   }

//   return parse_block(parser);
// }

// bool parse_while_statement(parser_t* parser) {
//   if (!parser_consume(parser, TOK_WHILE)) {
//     return false;
//   }
//   if (!parse_expression(parser)) {
//     return false;
//   }
//   return parse_block(parser);
// }

// bool parse_return_statement(parser_t* parser) {
//   if (!parser_consume(parser, TOK_RET)) {
//     return false;
//   }
//   if (!parser_check(parser, TOK_SEMICOLON)) {
//     if (!parse_expression(parser)) {
//       return false;
//     }
//   }
//   return parser_consume(parser, TOK_SEMICOLON);
// }

// bool parse_expression_statement(parser_t* parser) {
//   if (!parse_expression(parser)) {
//     return false;
//   }
//   return parser_consume(parser, TOK_SEMICOLON);
// }

// // AI Code, replace later
// bool parse_top_level_declaration(parser_t* parser) {
//   if (parser_check(parser, TOK_RECORD)) {
//     return parse_record_declaration(parser);
//   }

//   scanner_t saved_scanner = *parser->scanner;
//   token_t saved_current = parser->current;
//   token_t saved_next = parser->next;

//   bool is_function = false;

//   if (parse_type(parser) && parser_consume(parser, TOK_ID)) {
//     is_function = parser_check(parser, TOK_LPAREN);
//   } else if (parser->current.type == TOK_ID &&
//              parser->next.type == TOK_LPAREN) {
//     is_function = true;
//   }

//   *parser->scanner = saved_scanner;
//   parser->current = saved_current;
//   parser->next = saved_next;

//   if (is_function) {
//     return parse_function_declaration(parser);
//   }

//   return parse_variable_declaration(parser);
// }