#include "parser.h"
#include "parse.h"
#include "token.h"

bool parse_qualified_identifier(parser_t* parser) {
  if (!parser_consume(parser, TOK_ID)) {
    return false;
  }
  while (parser_consume(parser, TOK_COLCOL)) {
    if (!parser_consume(parser, TOK_ID)) {
      return false;
    }
  }
  return true;
}

bool parse_namespace_declaration(parser_t* parser) {
  if (!parser_consume(parser, TOK_NS)) {
    return false;
  }
  if (!parse_qualified_identifier(parser)) {
    return false;
  }
  return parser_consume(parser, TOK_SEMICOLON);
}

bool parse_use_declaration(parser_t* parser) {
  if (!parser_consume(parser, TOK_USE)) {
    return false;
  }
  if (!parse_qualified_identifier(parser)) {
    return false;
  }
  if (parser_consume(parser, TOK_AS)) {
    if (!parser_consume(parser, TOK_ID)) {
      return false;
    }
  }
  return parser_consume(parser, TOK_SEMICOLON);
}

// type = base_type { "*" | "[" int_literal "]" }
bool parse_type(parser_t* parser) {
  if (!parse_base_type(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_ASTERISK));
  while (parser_consume(parser, TOK_LBRACK)) {
    parser_consume(parser, TOK_INTLIT);
    if (!parser_consume(parser, TOK_RBRACK)) {
      return false;
    }
  }
  return true;
}

bool parse_base_type(parser_t* parser) {
  switch (parser->current.type) {
    case TOK_BOOL:
    case TOK_BYTE:
    case TOK_I8: case TOK_I16: case TOK_I32: case TOK_I64:
    case TOK_U8: case TOK_U16: case TOK_U32: case TOK_U64:
    case TOK_F32: case TOK_F64:
    case TOK_STRING:
      parser_advance(parser);
      return true;
    default: break;
  }
  return parse_qualified_identifier(parser);
}

bool parse_field_declaration(parser_t* parser) {
  if (!parse_type(parser)) {
    return false;
  }
  if (!parser_consume(parser, TOK_ID)) {
    return false;
  }
  return parser_consume(parser, TOK_SEMICOLON); 
}

// record_declaration = "record" identifier "{" { field_declaration | record_declaration } "}" ";"
bool parse_record_declaration(parser_t* parser) {
  if (!parser_consume(parser, TOK_RECORD)) {
    return false;
  }
  if (!parser_consume(parser, TOK_ID)) {
    return false;
  }
  if (!parser_consume(parser, TOK_LCURL)) {
    return false;
  }
  while (!parser_consume(parser, TOK_RCURL)) {
    if (parser_check(parser, TOK_EOF)) {
      return false;
    }
    if (parser_check(parser, TOK_RECORD)) {
      if (!parse_record_declaration(parser)) {
        return false;
      }
    } else {
      if (!parse_field_declaration(parser)) {
        return false;
      }
    }
  }
  return parser_consume(parser, TOK_SEMICOLON);
}

bool parse_variable_declaration(parser_t* parser) {
  if (!parse_type(parser)) {
    return false;
  }
  if (!parser_consume(parser, TOK_ID)) {
    return false;
  }
  if (parser_consume(parser, TOK_EQ)) {
    if (!parse_expression(parser)) {
      return false;
    }
  }
  return parser_consume(parser, TOK_SEMICOLON);
}

bool parse_expression(parser_t* parser) {
  return parse_assignment(parser);
}

//assignment = logical_or [ "=" assignment ]
bool parse_assignment(parser_t* parser) {
  if (!parse_logical_or(parser)) {
    return false;
  }
  if (parser_consume(parser, TOK_EQ)) {
    return parse_assignment(parser);
  }
  return true;
}

bool parse_logical_or(parser_t* parser) {
  if (!parse_logical_xor(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_LOR)) {
    if (!parse_logical_xor(parser)) {
      return false;
    }
  }
  return true;
}

bool parse_logical_xor(parser_t* parser) {
  if (!parse_logical_and(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_LXOR)) {
    if (!parse_logical_and(parser)) {
      return false;
    }
  }
  return true;
}

bool parse_logical_and(parser_t* parser) {
  if (!parse_bitwise_or(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_LAND)) {
    if (!parse_bitwise_or(parser)) {
      return false;
    }
  }
  return true;
}

bool parse_bitwise_or(parser_t* parser) {
  if (!parse_bitwise_xor(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_BOR)) {
    if (!parse_bitwise_xor(parser)) {
      return false;
    }
  }
  return true;
}

bool parse_bitwise_xor(parser_t* parser) {
  if (!parse_bitwise_and(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_BXOR)) {
    if (!parse_bitwise_and(parser)) {
      return false;
    }
  }
  return true;
}

// bitwise_and = equality { "&" equality }
bool parse_bitwise_and(parser_t* parser) {
  if (!parse_equality(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_BAND)) {
    if (!parse_equality(parser)) {
      return false;
    }
  }
  return true;
}

// equality = comparison { ( "==" | "!=" ) comparison }
bool parse_equality(parser_t* parser) {
  if (!parse_comparison(parser)) {
    return false;
  }
  while (true) {
    switch (parser->current.type) {
      case TOK_EQEQ:
      case TOK_NOTEQ:
        parser_advance(parser);
        if (!parse_comparison(parser)) {
          return false;
        }
        break;
      default: return true;
    }
  }
}

// comparison = shift { ( "<" | "<=" | ">" | ">=" ) shift }
bool parse_comparison(parser_t* parser) {
  if (!parse_shift(parser)) {
    return false;
  }
  while (true) {
    switch (parser->current.type) {
      case TOK_LT:
      case TOK_LTEQ:
      case TOK_GT:
      case TOK_GTEQ:
        parser_advance(parser);
        if (!parse_shift(parser)) {
          return false;
        }
        break;
      default: return true;
    }
  }
}

// shift = additive { ( "<<" | ">>" ) additive }
bool parse_shift(parser_t* parser) {
  if (!parse_additive(parser)) {
    return false;
  }
  while (true) {
    switch (parser->current.type) {
      case TOK_LSHIFT:
      case TOK_RSHIFT:
        parser_advance(parser);
        if (!parse_additive(parser)) {
          return false;
        }
        break;
      default: return true;
    }
  }
}

// additive = multiplicative { ( "+" | "-" ) multiplicative }
bool parse_additive(parser_t* parser) {
  if (!parse_multiplicative(parser)) {
    return false;
  }
  while (true) {
    switch (parser->current.type) {
      case TOK_PLUS:
      case TOK_MINUS:
        parser_advance(parser);
        if (!parse_multiplicative(parser)) {
          return false;
        }
        break;
      default: return true;
    }
  }
}

bool parse_multiplicative(parser_t* parser) {
  if (!parse_unary(parser)) {
    return false;
  }
  while (true) {
    switch (parser->current.type) {
      case TOK_ASTERISK:
      case TOK_SLASH:
      case TOK_MOD:
        parser_advance(parser);
        if (!parse_unary(parser)) {
          return false;
        }
        break;
      default: return true;
    }
  }
}

bool parse_unary(parser_t* parser) {
  while (true) {
    switch (parser->current.type) {
      case TOK_ASTERISK:
      case TOK_BAND:
      case TOK_BNOT:
      case TOK_MINUS:
      case TOK_LNOT:
        parser_advance(parser);
        break;
      default: return parse_postfix(parser);
    }
  }
}

bool parse_argument_list(parser_t* parser) {
  if (!parse_expression(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_COMMA)) {
    if (!parse_expression(parser)) {
      return false;
    }
  }
  return true;
}

bool parse_postfix(parser_t* parser) {
  if (!parse_primary(parser)) {
    return false;
  }
  while (true) {
    if (parser_consume(parser, TOK_LPAREN)) {
      if (!parser_check(parser, TOK_RPAREN) && !parse_argument_list(parser)) {
        return false;
      } 
      if (!parser_consume(parser, TOK_RPAREN)) {
        return false;
      }
    } else if (parser_consume(parser, TOK_LBRACK)) {
      if (!parse_expression(parser)) {
        return false;
      }
      if (!parser_consume(parser, TOK_RBRACK)) {
        return false;
      }
    } else if (parser_consume(parser, TOK_DOT)) {
      if (!parser_consume(parser, TOK_ID)) {
        return false;
      }
    } else if (parser_consume(parser, TOK_MINUSGT)) {
      if (!parser_consume(parser, TOK_ID)) {
        return false;
      }
    } else {
      break;
    }
  }
  if (parser_consume(parser, TOK_PLUSPLUS)); else parser_consume(parser, TOK_MINUSMINUS);

  return true;
}

// bool parse_primary(parser_t* parser) {
//   if (parse_primitive_literal(parser)) {
//     return true;
//   }

//   if (parser->current.type == TOK_ID && (parser->next.type == TOK_LCURL || parser->next.type == TOK_COLCOL)) {
//     return parse_record_literal(parser);
//   }

//   if (parser_check(parser, TOK_ID)) {
//     return parse_qualified_identifier(parser);
//   }
//   if (parser_consume(parser, TOK_LPAREN)) {
//     if (!parse_expression(parser)) {
//       return false;
//     }
//     return parser_consume(parser, TOK_RPAREN);
//   }
//   return false;
// }

bool parse_primary(parser_t* parser) {
  if (parse_primitive_literal(parser)) {
    return true;
  }

  if (parser_check(parser, TOK_ID)) {
    scanner_t saved_scanner = *parser->scanner;
    token_t saved_current = parser->current;
    token_t saved_next = parser->next;

    if (parse_qualified_identifier(parser) &&
        parser_check(parser, TOK_LCURL)) {
      *parser->scanner = saved_scanner;
      parser->current = saved_current;
      parser->next = saved_next;

      return parse_record_literal(parser);
    }

    *parser->scanner = saved_scanner;
    parser->current = saved_current;
    parser->next = saved_next;

    return parse_qualified_identifier(parser);
  }

  if (parser_consume(parser, TOK_LPAREN)) {
    if (!parse_expression(parser)) {
      return false;
    }

    return parser_consume(parser, TOK_RPAREN);
  }

  return false;
}

bool parse_primitive_literal(parser_t* parser) {
  switch (parser->current.type) {
    case TOK_BOOLLIT:
    case TOK_BYTELIT:
    case TOK_FLOATLIT:
    case TOK_INTLIT:
    case TOK_STRLIT:
      parser_advance(parser);
      return true;
    default:
      return false;
  }
}

bool parse_record_literal(parser_t* parser) {
  if (!parse_qualified_identifier(parser)) {
    return false;
  }
  if (!parser_consume(parser, TOK_LCURL)) {
    return false;
  }
  if (!parser_check(parser, TOK_RCURL)) {
    if (!parse_field_initializer(parser)) {
      return false;
    }
    while (parser_consume(parser, TOK_COMMA)) {
      if (!parse_field_initializer(parser)) {
        return false;
      }
    }
  }
  return parser_consume(parser, TOK_RCURL);
}

bool parse_field_initializer(parser_t* parser) {
  if (!parser_consume(parser, TOK_ID)) {
    return false;
  }
  if (!parser_consume(parser, TOK_COL)) {
    return false;
  }
  return parse_expression(parser);
}

bool parse_function_declaration(parser_t* parser) {
  if (!(parser->current.type == TOK_ID &&
        parser->next.type == TOK_LPAREN)) {
    if (!parse_type(parser)) {
      return false;
    }
  }

  if (!parser_consume(parser, TOK_ID)) {
    return false;
  }

  if (!parser_consume(parser, TOK_LPAREN)) {
    return false;
  }

  if (!parser_check(parser, TOK_RPAREN)) {
    if (!parse_parameter_list(parser)) {
      return false;
    }
  }

  if (!parser_consume(parser, TOK_RPAREN)) {
    return false;
  }

  return parse_block(parser);
}

bool parse_parameter_list(parser_t* parser) {
  if (!parse_parameter(parser)) {
    return false;
  }
  while (parser_consume(parser, TOK_COMMA)) {
    if (!parse_parameter(parser)) {
      return false;
    }
  }
  return true;
}

bool parse_parameter(parser_t* parser) {
  if (!parse_type(parser)) {
    return false;
  }
  return parser_consume(parser, TOK_ID);
}

bool parse_block(parser_t* parser) {
  if (!parser_consume(parser, TOK_LCURL)) {
    return false;
  }
  while (!parser_check(parser, TOK_RCURL)) {
    if (!parse_statement(parser)) {
      return false;
    }
  }

  return parser_consume(parser, TOK_RCURL);
}

bool parse_statement(parser_t* parser) {
  switch (parser->current.type) {
    case TOK_RECORD: return parse_record_declaration(parser);
    case TOK_IF: return parse_if_statement(parser);
    case TOK_FOR: return parse_for_statement(parser);
    case TOK_WHILE: return parse_while_statement(parser);
    case TOK_RET: return parse_return_statement(parser);
    case TOK_LCURL: return parse_block(parser);
    default: break;
  }

  scanner_t saved_scanner = *parser->scanner;
  token_t saved_current = parser->current;
  token_t saved_next = parser->next;

  bool result = parse_type(parser) && parser_check(parser, TOK_ID);

  *parser->scanner = saved_scanner;
  parser->current = saved_current;
  parser->next = saved_next;

  if (result) return parse_variable_declaration(parser);

  return parse_expression_statement(parser);
}

bool parse_if_statement(parser_t* parser) {
  if (!parser_consume(parser, TOK_IF)) {
    return false;
  }
  if (!parse_expression(parser)) {
    return false;
  }
  if (!parse_block(parser)) {
    return false;
  }
 
  if (!parser_consume(parser, TOK_ELSE)) {
    return true;
  }
  if (parser_check(parser, TOK_IF)) {
    return parse_if_statement(parser);
  }
  if (parser_check(parser, TOK_LCURL)) {
    return parse_block(parser);
  }
  return false;
}

bool parse_variable_definition(parser_t* parser) {
  if (!parse_type(parser)) {
    return false;
  }
  if (!parser_consume(parser, TOK_ID)) {
    return false;
  }
  if (parser_consume(parser, TOK_EQ)) {
    if (!parse_expression(parser)) {
      return false;
    }
  }
  return true;
}

bool parse_for_statement(parser_t* parser) {
  if (!parser_consume(parser, TOK_FOR)) {
    return false;
  }
  if (!parser_check(parser, TOK_SEMICOLON)) {
    // lookahead later
    scanner_t saved_scanner = *parser->scanner;
    token_t saved_current = parser->current;
    token_t saved_next = parser->next;
    
    bool result = parse_type(parser) && parser_check(parser, TOK_ID);

    *parser->scanner = saved_scanner;
    parser->current = saved_current;
    parser->next = saved_next;

    if (result) {
      if (!parse_variable_definition(parser)) {
        return false;
      }
    } else {
      if (!parse_expression(parser)) {
        return false;
      }
    }
  }
  if (!parser_consume(parser, TOK_SEMICOLON)) {
    return false;
  }
  if (!parser_check(parser, TOK_SEMICOLON)) {
    if (!parse_expression(parser)) {
      return false;
    }
  }
  if (!parser_consume(parser, TOK_SEMICOLON)) {
    return false;
  }
  if (!parser_check(parser, TOK_LCURL)) {
    if (!parse_expression(parser)) {
      return false;
    }
  }

  return parse_block(parser);
}

bool parse_while_statement(parser_t* parser) {
  if (!parser_consume(parser, TOK_WHILE)) {
    return false;
  }
  if (!parse_expression(parser)) {
    return false;
  }
  return parse_block(parser);
}

bool parse_return_statement(parser_t* parser) {
  if (!parser_consume(parser, TOK_RET)) {
    return false;
  }
  if (!parser_check(parser, TOK_SEMICOLON)) {
    if (!parse_expression(parser)) {
      return false;
    }
  }
  return parser_consume(parser, TOK_SEMICOLON);
}

bool parse_expression_statement(parser_t* parser) {
  if (!parse_expression(parser)) {
    return false;
  }
  return parser_consume(parser, TOK_SEMICOLON);
}

// AI Code, replace later
bool parse_top_level_declaration(parser_t* parser) {
  if (parser_check(parser, TOK_RECORD)) {
    return parse_record_declaration(parser);
  }

  scanner_t saved_scanner = *parser->scanner;
  token_t saved_current = parser->current;
  token_t saved_next = parser->next;

  bool is_function = false;

  if (parse_type(parser) && parser_consume(parser, TOK_ID)) {
    is_function = parser_check(parser, TOK_LPAREN);
  } else if (parser->current.type == TOK_ID &&
             parser->next.type == TOK_LPAREN) {
    is_function = true;
  }

  *parser->scanner = saved_scanner;
  parser->current = saved_current;
  parser->next = saved_next;

  if (is_function) {
    return parse_function_declaration(parser);
  }

  return parse_variable_declaration(parser);
}