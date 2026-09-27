#include "parser.h"

bool parse_qualified_identifier(parser_t*);
bool parse_namespace_declaration(parser_t*);
bool parse_use_declaration(parser_t*);
bool parse_type(parser_t*);
bool parse_base_type(parser_t*);
bool parse_field_declaration(parser_t*);
bool parse_record_declaration(parser_t*);
bool parse_variable_declaration(parser_t*);
bool parse_expression(parser_t*);
bool parse_assignment(parser_t*);
bool parse_logical_or(parser_t*);
bool parse_logical_xor(parser_t*);
bool parse_logical_and(parser_t*);
bool parse_bitwise_or(parser_t*);
bool parse_bitwise_xor(parser_t*);
bool parse_bitwise_and(parser_t*);
bool parse_equality(parser_t*);
bool parse_comparison(parser_t*);
bool parse_shift(parser_t*);
bool parse_additive(parser_t*);
bool parse_multiplicative(parser_t*);
bool parse_unary(parser_t*);
bool parse_postfix(parser_t*);
bool parse_primary(parser_t*);
bool parse_primitive_literal(parser_t*);
bool parse_record_literal(parser_t*);
bool parse_field_initializer(parser_t*);
bool parse_argument_list(parser_t*);
bool parse_function_declaration(parser_t*);
bool parse_parameter_list(parser_t*);
bool parse_parameter(parser_t*);
bool parse_block(parser_t*);
bool parse_statement(parser_t*);
bool parse_if_statement(parser_t*);
bool parse_for_statement(parser_t*);
bool parse_while_statement(parser_t*);
bool parse_return_statement(parser_t*);
bool parse_expression_statement(parser_t*);
bool parse_variable_definition(parser_t*);
bool parse_top_level_declaration(parser_t*);

// function_declaration = [ type ] identifier "(" [ parameter_list ] ")" block
// parameter_list = parameter { "," parameter }
// parameter = type identifier

// block = "{" { statement } "}"

// statement = variable_declaration
//           | record_declaration
//           | if_statement
//           | for_statement
//           | while_statement
//           | return_statement
//           | expression_statement
//           | block
        
// if_statement = "if" expression block [ "else" ( if_statement | block ) ]

// for_statement = "for" [ variable_definition | expression ] ";" [ expression ] ";" [ expression ] block

// while_statement = "while" expression block

// return_statement = "return" [ expression ] ";"

// expression_statement = expression ";"