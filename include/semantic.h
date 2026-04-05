/* semantic.h — Semantic analysis pass */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "common.h"

VarType infer_expr_type(ASTNode *node);
void semantic_check_node(ASTNode *node);
void semantic_check_list(ASTNode *list);
void semantic_check_program(ASTNode *root);

#endif /* SEMANTIC_H */
