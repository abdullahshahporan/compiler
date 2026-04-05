/* symtab.h — Symbol table management */

#ifndef SYMTAB_H
#define SYMTAB_H

#include "common.h"

int  sym_find(const char *name);
int  sym_find_scope(const char *name);
int  sym_add(const char *name, VarType type, int lineno);
void scope_push(void);
void scope_pop(void);
const char* type_name(VarType t);
void sym_reset(void);
void print_sym_table(FILE *f);

#endif /* SYMTAB_H */
