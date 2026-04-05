/* symtab.c — Symbol table with scope management */

#include "symtab.h"

Symbol sym_table[MAX_SYMBOLS];
int sym_count     = 0;
int cur_scope     = 0;
int error_count   = 0;
int warning_count = 0;

int sym_find(const char *name) {
    int found = -1;
    for (int i = 0; i < sym_count; i++)
        if (strcmp(sym_table[i].name, name) == 0 && sym_table[i].scope <= cur_scope)
            found = i;
    return found;
}

int sym_find_scope(const char *name) {
    for (int i = 0; i < sym_count; i++)
        if (strcmp(sym_table[i].name, name) == 0 && sym_table[i].scope == cur_scope)
            return i;
    return -1;
}

int sym_add(const char *name, VarType type, int lineno) {
    if (sym_count >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Symbol table full\n");
        return -1;
    }
    if (sym_find_scope(name) != -1) {
        fprintf(stderr, "Semantic Error at line %d: '%s' already declared in this scope\n",
                lineno, name);
        error_count++;
        return -1;
    }
    int idx = sym_count++;
    memset(&sym_table[idx], 0, sizeof(Symbol));
    strncpy(sym_table[idx].name, name, 63);
    sym_table[idx].type  = type;
    sym_table[idx].scope = cur_scope;
    return idx;
}

void scope_push(void) { cur_scope++; }

void scope_pop(void) {
    while (sym_count > 0 && sym_table[sym_count - 1].scope == cur_scope)
        sym_count--;
    cur_scope--;
}

const char* type_name(VarType t) {
    switch (t) {
        case T_INT:   return "purno (int)";
        case T_FLOAT: return "dosomik (float)";
        case T_CHAR:  return "chinho (char)";
        case T_VOID:  return "kisu_na (void)";
    }
    return "unknown";
}

void sym_reset(void) {
    sym_count = 0;
    cur_scope = 0;
}

void print_sym_table(FILE *f) {
    fprintf(f, "\n============================================\n");
    fprintf(f, "           SYMBOL TABLE\n");
    fprintf(f, "============================================\n");
    fprintf(f, "%-15s %-15s %-12s %-8s %-8s\n",
            "Name", "Type", "Value", "Init?", "Scope");
    fprintf(f, "------------------------------------------------------------\n");

    for (int i = 0; i < sym_count; i++) {
        if (sym_table[i].is_function) {
            fprintf(f, "%-15s %-15s %-12s %-8s %-8d\n",
                    sym_table[i].name, type_name(sym_table[i].type),
                    "(func)", "-", sym_table[i].scope);
        } else if (sym_table[i].is_array) {
            fprintf(f, "%-15s %-15s %-12s %-8s %-8d\n",
                    sym_table[i].name, type_name(sym_table[i].type),
                    "(array)", "Yes", sym_table[i].scope);
        } else {
            char vbuf[32];
            if (sym_table[i].type == T_INT)
                sprintf(vbuf, "%d", (int)sym_table[i].value);
            else
                sprintf(vbuf, "%.4g", sym_table[i].value);
            fprintf(f, "%-15s %-15s %-12s %-8s %-8d\n",
                    sym_table[i].name, type_name(sym_table[i].type),
                    vbuf, sym_table[i].is_init ? "Yes" : "No",
                    sym_table[i].scope);
        }
    }
    fprintf(f, "============================================\n");
}
