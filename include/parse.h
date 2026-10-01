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

bool is_rec_lit(parser_t*);
ast_node_t* parse_rec_lit(parser_t*);
ast_node_t* parse_rec_field_init(parser_t*);
// ast_node_t* parse_qual_id(parser_t*);

bool is_var_decl_start(parser_t*);
ast_node_t* parse_var_decl(parser_t*);

ast_node_t* parse_func_decl(parser_t*);
ast_node_t* parse_func_receiver(parser_t*);
ast_node_t* parse_func_param(parser_t*);

ast_node_t* parse_block(parser_t*);

ast_node_t* parse_stmt(parser_t*);

ast_node_t* parse_if_stmt(parser_t*);
ast_node_t* parse_for_stmt(parser_t*);
ast_node_t* parse_while_stmt(parser_t*);
ast_node_t* parse_return_stmt(parser_t*);
ast_node_t* parse_expr_stmt(parser_t*);

ast_node_t* parse_assignment(parser_t*);

ast_node_t* parse_expr_stmt(parser_t*);

ast_node_t* parse_expr(parser_t*);

ast_node_t* parse_assignment(parser_t*);
ast_node_t* parse_logical_or(parser_t*);
ast_node_t* parse_logical_xor(parser_t*);
ast_node_t* parse_logical_and(parser_t*);
ast_node_t* parse_bitwise_or(parser_t*);
ast_node_t* parse_bitwise_xor(parser_t*);
ast_node_t* parse_bitwise_and(parser_t*);
ast_node_t* parse_equality(parser_t*);
ast_node_t* parse_comparison(parser_t*);
ast_node_t* parse_shift(parser_t*);
ast_node_t* parse_additive(parser_t*);
ast_node_t* parse_multiplicative(parser_t*);
ast_node_t* parse_unary(parser_t*);
ast_node_t* parse_postfix(parser_t*);
ast_node_t* parse_primary(parser_t*);

ast_node_t* parse_call_suffix(parser_t*, ast_node_t*);
ast_node_t* parse_index_suffix(parser_t*, ast_node_t*);
ast_node_t* parse_member_suffix(parser_t*, ast_node_t*);
