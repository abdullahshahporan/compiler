/* semantic.c — Semantic analysis: type checking, undeclared vars, arity */

#include "semantic.h"
#include "symtab.h"
#include "spp_parser.tab.h"

/* ---- Expression Type Inference ---- */

VarType infer_expr_type(ASTNode *node) {
    if (!node) return T_INT;

    switch (node->ntype) {
    case N_INT_LIT:
    case N_BOOL_LIT:   return T_INT;
    case N_FLOAT_LIT:  return T_FLOAT;
    case N_CHAR_LIT:   return T_CHAR;
    case N_STRING_LIT: return T_VOID;

    case N_VAR: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n",
                    node->lineno, node->name);
            error_count++;
            return T_INT;
        }
        if (sym_table[idx].is_function) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is a function, not a variable\n",
                    node->lineno, node->name);
            error_count++;
        }
        if (sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is an array, use subscript []\n",
                    node->lineno, node->name);
            error_count++;
        }
        return sym_table[idx].type;
    }

    case N_ARR_ACCESS: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared array '%s'\n",
                    node->lineno, node->name);
            error_count++;
            return T_INT;
        }
        if (!sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is not an array\n",
                    node->lineno, node->name);
            error_count++;
        }
        infer_expr_type(node->left);
        return sym_table[idx].type;
    }

    case N_BINOP: {
        VarType lt = infer_expr_type(node->left);
        VarType rt = infer_expr_type(node->right);
        switch (node->op) {
            case EQ: case NEQ: case LT: case GT: case LE: case GE:
            case AND: case OR:
                return T_INT;
        }
        return (lt == T_FLOAT || rt == T_FLOAT) ? T_FLOAT : lt;
    }

    case N_UNOP:
        infer_expr_type(node->left);
        return T_INT;

    case N_SQRT_OP:
        infer_expr_type(node->left);
        return T_FLOAT;

    case N_FUNC_CALL: {
        int idx = sym_find(node->name);
        if (idx == -1 || !sym_table[idx].is_function) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared function '%s'\n",
                    node->lineno, node->name);
            error_count++;
            return T_INT;
        }
        int argc = 0;
        ASTNode *a = node->args;
        while (a) { argc++; a = a->next; }
        if (argc != sym_table[idx].param_count) {
            fprintf(stderr, "Semantic Error at line %d: '%s' expects %d args, got %d\n",
                    node->lineno, node->name, sym_table[idx].param_count, argc);
            error_count++;
        }
        a = node->args;
        ASTNode *param = sym_table[idx].func_node ? sym_table[idx].func_node->params : NULL;
        while (a) {
            VarType at = infer_expr_type(a);
            if (param && (VarType)param->var_type == T_INT && at == T_FLOAT) {
                fprintf(stderr, "Warning at line %d: Narrowing dosomik -> purno for arg '%s' in call to '%s'\n",
                        node->lineno, param->name, node->name);
                warning_count++;
            }
            if (param) param = param->next;
            a = a->next;
        }
        return sym_table[idx].type;
    }

    default: return T_INT;
    }
}

/* ---- Statement Checking ---- */

void semantic_check_node(ASTNode *node) {
    if (!node) return;

    switch (node->ntype) {
    case N_DECL: {
        int idx = sym_add(node->name, (VarType)node->var_type, node->lineno);
        if (idx != -1 && node->left) {
            VarType et = infer_expr_type(node->left);
            if ((VarType)node->var_type == T_INT && et == T_FLOAT) {
                fprintf(stderr, "Warning at line %d: Narrowing dosomik -> purno in declaration of '%s'\n",
                        node->lineno, node->name);
                warning_count++;
            }
        }
        break;
    }
    case N_ARR_DECL: {
        int idx = sym_add(node->name, (VarType)node->var_type, node->lineno);
        if (idx != -1) {
            sym_table[idx].is_array = 1;
            sym_table[idx].arr_size = node->arr_size;
        }
        break;
    }
    case N_ASSIGN: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n",
                    node->lineno, node->name);
            error_count++;
        } else {
            if (sym_table[idx].is_function) {
                fprintf(stderr, "Semantic Error at line %d: Cannot assign to function '%s'\n",
                        node->lineno, node->name);
                error_count++;
            }
            if (sym_table[idx].is_array) {
                fprintf(stderr, "Semantic Error at line %d: Cannot assign to array '%s' without subscript\n",
                        node->lineno, node->name);
                error_count++;
            }
            VarType et = infer_expr_type(node->left);
            if (idx >= 0 && !sym_table[idx].is_function && !sym_table[idx].is_array
                && sym_table[idx].type == T_INT && et == T_FLOAT) {
                fprintf(stderr, "Warning at line %d: Narrowing dosomik -> purno in assignment to '%s'\n",
                        node->lineno, node->name);
                warning_count++;
            }
        }
        break;
    }
    case N_ARR_ASSIGN: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared array '%s'\n",
                    node->lineno, node->name);
            error_count++;
        } else if (!sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is not an array\n",
                    node->lineno, node->name);
            error_count++;
        }
        infer_expr_type(node->left);
        infer_expr_type(node->right);
        break;
    }
    case N_IF:
        infer_expr_type(node->left);
        semantic_check_list(node->body);
        if (node->right) semantic_check_list(node->right);
        break;
    case N_WHILE:
        infer_expr_type(node->left);
        scope_push();
        semantic_check_list(node->body);
        scope_pop();
        break;
    case N_FOR:
        semantic_check_node(node->right);
        infer_expr_type(node->left);
        scope_push();
        semantic_check_list(node->body);
        semantic_check_node(node->update);
        scope_pop();
        break;
    case N_PRINT:
        infer_expr_type(node->left);
        break;
    case N_INPUT: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s' for input\n",
                    node->lineno, node->name);
            error_count++;
        }
        break;
    }
    case N_RETURN:
        if (node->left) infer_expr_type(node->left);
        break;
    case N_INC: case N_DEC: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n",
                    node->lineno, node->name);
            error_count++;
        }
        break;
    }
    case N_FUNC_CALL:
        infer_expr_type(node);
        break;
    case N_FUNC_DEF: {
        /* Nested function: register in current scope and check body */
        int idx = sym_add(node->name, (VarType)node->var_type, node->lineno);
        if (idx != -1) {
            sym_table[idx].is_function = 1;
            sym_table[idx].func_node   = node;
            int pc = 0;
            ASTNode *p = node->params;
            while (p) { pc++; p = p->next; }
            sym_table[idx].param_count = pc;
        }
        scope_push();
        ASTNode *p = node->params;
        while (p) {
            sym_add(p->name, (VarType)p->var_type, p->lineno);
            p = p->next;
        }
        semantic_check_list(node->body);
        scope_pop();
        break;
    }
    default:
        break;
    }
}

void semantic_check_list(ASTNode *list) {
    ASTNode *s = list;
    while (s) { semantic_check_node(s); s = s->next; }
}

void semantic_check_program(ASTNode *root) {
    /* Pass 1: Register all non-main function definitions */
    ASTNode *f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF && strcmp(f->name, "main") != 0) {
            int idx = sym_add(f->name, (VarType)f->var_type, f->lineno);
            if (idx != -1) {
                sym_table[idx].is_function = 1;
                sym_table[idx].func_node   = f;
                int pc = 0;
                ASTNode *p = f->params;
                while (p) { pc++; p = p->next; }
                sym_table[idx].param_count = pc;
            }
        }
        f = f->next;
    }
    /* Pass 2: Check each function body */
    f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF) {
            scope_push();
            ASTNode *p = f->params;
            while (p) {
                sym_add(p->name, (VarType)p->var_type, p->lineno);
                p = p->next;
            }
            semantic_check_list(f->body);
            scope_pop();
        }
        f = f->next;
    }
}
