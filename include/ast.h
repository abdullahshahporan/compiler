/* ast.h — AST node pool and constructor functions */

#ifndef AST_H
#define AST_H

#include "common.h"

ASTNode* new_node(NodeType t);

/* Literals */
ASTNode* make_int_lit(int v);
ASTNode* make_float_lit(double v);
ASTNode* make_char_lit(char v);
ASTNode* make_string_lit(const char *s);
ASTNode* make_bool_lit(int v);

/* Variables & Operators */
ASTNode* make_var(const char *name);
ASTNode* make_binop(int op, ASTNode *l, ASTNode *r);
ASTNode* make_unop(int op, ASTNode *operand);
ASTNode* make_sqrt_op(ASTNode *expr);

/* Declarations & Assignments */
ASTNode* make_assign(const char *name, ASTNode *val);
ASTNode* make_decl(int vt, const char *name, ASTNode *init);
ASTNode* make_arr_decl(int vt, const char *name, int size);
ASTNode* make_arr_assign(const char *name, ASTNode *idx, ASTNode *val);
ASTNode* make_arr_access(const char *name, ASTNode *idx);

/* Control Flow */
ASTNode* make_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b);
ASTNode* make_while(ASTNode *cond, ASTNode *body);
ASTNode* make_for(ASTNode *init, ASTNode *cond, ASTNode *upd, ASTNode *body);

/* I/O & Return */
ASTNode* make_print(ASTNode *expr);
ASTNode* make_input(const char *name);
ASTNode* make_return(ASTNode *expr);
ASTNode* make_break_node(void);
ASTNode* make_continue_node(void);

/* Functions */
ASTNode* make_func_def(int rt, const char *name, ASTNode *par, ASTNode *body);
ASTNode* make_func_call(const char *name, ASTNode *args);
ASTNode* make_param(int vt, const char *name);

/* Inc/Dec */
ASTNode* make_inc(const char *name);
ASTNode* make_dec(const char *name);

/* List helper */
ASTNode* append_list(ASTNode *list, ASTNode *item);

#endif /* AST_H */
