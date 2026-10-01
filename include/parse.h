#include "parser.h"
#include "ast.h"

ast_node_t* parse_program(parser_t*);
ast_node_t* parse_top_level_decl(parser_t*);
ast_node_t* parse_id(parser_t*);

bool is_qual_id(parser_t*);
ast_node_t* parse_qual_id(parser_t*);

ast_node_t* parse_ns_decl(parser_t*);
ast_node_t* parse_use_decl(parser_t*);
ast_node_t* parse_rec_decl(parser_t*);
ast_node_t* parse_rec_field_decl(parser_t*);
ast_node_t* parse_type(parser_t*);

bool is_builtin_type(parser_t*);
ast_node_t* parse_builtin_type(parser_t*);

bool is_primitve_lit(parser_t*);
ast_node_t* parse_primitive_lit(parser_t*);

ast_node_t* parse_bool_lit(parser_t*);
ast_node_t* parse_byte_lit(parser_t*);
ast_node_t* parse_float_lit(parser_t*);
ast_node_t* parse_int_lit(parser_t*);
ast_node_t* parse_str_lit(parser_t*);

ast_node_t* parse_lit(parser_t*);

ast_node_t* parse_rec_lit(parser_t*);
ast_node_t* parse_rec_field_init(parser_t*);
// ast_node_t* parse_qual_id(parser_t*);

bool is_var_decl_start(parser_t*);
ast_node_t* parse_var_decl(parser_t*);

ast_node_t* parse_func_decl(parser_t*);
ast_node_t* parse_func_receiver(parser_t*);
ast_node_t* parse_func_param(parser_t*);

ast_node_t* parse_block(parser_t*);

ast_node_t* parse_expr(parser_t*);


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