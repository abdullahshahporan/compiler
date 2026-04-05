/* interpreter.h — AST tree-walk interpreter */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "common.h"

ExecResult make_res(double val, VarType t);
ExecResult eval_expr(ASTNode *node);
ExecResult exec_node(ASTNode *node);
ExecResult exec_list(ASTNode *list);
void exec_program(ASTNode *root);

#endif /* INTERPRETER_H */
