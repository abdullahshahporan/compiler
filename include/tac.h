/* tac.h — Three-Address Code generation */

#ifndef TAC_H
#define TAC_H

#include "common.h"

char* new_temp(void);
char* new_label(void);
char* sdup(const char *s);
void  tac_add(int kind, const char *res, const char *a1, const char *op, const char *a2);

char* gen_expr_tac(ASTNode *node);
void  gen_stmt_tac(ASTNode *node);
void  gen_list_tac(ASTNode *list);
void  gen_program_tac(ASTNode *root);
void  print_tac(FILE *f, const char *title);

#endif /* TAC_H */
