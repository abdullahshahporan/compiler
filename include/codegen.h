/* codegen.h — C code generation from AST */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "common.h"

const char* c_type(int vt);
void gen_c_expr(FILE *f, ASTNode *node);
void gen_c_stmt(FILE *f, ASTNode *node, int indent);
void gen_c_list(FILE *f, ASTNode *list, int indent);
void gen_c_program(FILE *f, ASTNode *root);

#endif /* CODEGEN_H */
