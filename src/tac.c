/* tac.c — Three-Address Code generation */

#include "tac.h"
#include "spp_parser.tab.h"

TACLine tac[MAX_TAC];
int tac_count  = 0;
int temp_id    = 0;
int label_id   = 0;
char loop_start_lbl[MAX_LOOP][16];
char loop_end_lbl[MAX_LOOP][16];
int  loop_depth = 0;

/* ---- Helpers ---- */

char* new_temp(void) {
    static char buf[16];
    sprintf(buf, "t%d", temp_id++);
    return buf;
}

char* new_label(void) {
    static char buf[16];
    sprintf(buf, "L%d", label_id++);
    return buf;
}

char* sdup(const char *s) {
    char *d = (char*)malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

void tac_add(int kind, const char *res, const char *a1, const char *op, const char *a2) {
    if (tac_count >= MAX_TAC) return;
    TACLine *t = &tac[tac_count++];
    memset(t, 0, sizeof(TACLine));
    t->kind = kind;
    if (res) strncpy(t->result, res, 63);
    if (a1)  strncpy(t->arg1,   a1, 63);
    if (op)  strncpy(t->op,     op, 15);
    if (a2)  strncpy(t->arg2,   a2, 63);
}

/* ---- Expression TAC ---- */

char* gen_expr_tac(ASTNode *node) {
    char buf[256];
    if (!node) return sdup("0");

    switch (node->ntype) {
    case N_INT_LIT:    sprintf(buf, "%d", node->ival);       return sdup(buf);
    case N_FLOAT_LIT:  sprintf(buf, "%.4g", node->fval);     return sdup(buf);
    case N_CHAR_LIT:   sprintf(buf, "'%c'", node->cval);     return sdup(buf);
    case N_STRING_LIT: sprintf(buf, "\"%s\"", node->sval);   return sdup(buf);
    case N_BOOL_LIT:   sprintf(buf, "%d", node->ival);       return sdup(buf);
    case N_VAR:        return sdup(node->name);

    case N_ARR_ACCESS: {
        char *idx = gen_expr_tac(node->left);
        char *t = sdup(new_temp());
        tac_add(7, t, node->name, "[]", idx);
        free(idx);
        return t;
    }

    case N_BINOP: {
        char *l = gen_expr_tac(node->left);
        char *r = gen_expr_tac(node->right);
        const char *ops;
        switch (node->op) {
            case ADD: ops = "+";  break;  case SUB: ops = "-";  break;
            case MUL: ops = "*";  break;  case DIV: ops = "/";  break;
            case EQ:  ops = "=="; break;  case NEQ: ops = "!="; break;
            case LT:  ops = "<";  break;  case GT:  ops = ">";  break;
            case LE:  ops = "<="; break;  case GE:  ops = ">="; break;
            case AND: ops = "&&"; break;  case OR:  ops = "||"; break;
            default:  ops = "?";  break;
        }
        char *t = sdup(new_temp());
        tac_add(0, t, l, ops, r);
        free(l); free(r);
        return t;
    }

    case N_UNOP: {
        char *o = gen_expr_tac(node->left);
        char *t = sdup(new_temp());
        tac_add(0, t, "!", o, "");
        free(o);
        return t;
    }

    case N_SQRT_OP: {
        char *a = gen_expr_tac(node->left);
        char *t = sdup(new_temp());
        tac_add(0, t, "sqrt", a, "");
        free(a);
        return t;
    }

    case N_FUNC_CALL: {
        ASTNode *a = node->args;
        int ac = 0;
        while (a) {
            char *av = gen_expr_tac(a);
            tac_add(4, "", av, "", "");
            free(av);
            a = a->next;  ac++;
        }
        char *t = sdup(new_temp());
        char nargs[8];
        sprintf(nargs, "%d", ac);
        tac_add(5, t, node->name, "", nargs);
        return t;
    }

    default: return sdup("0");
    }
}

/* ---- Statement TAC ---- */

void gen_stmt_tac(ASTNode *node) {
    char buf[64];
    if (!node) return;

    switch (node->ntype) {
    case N_DECL:
        if (node->left) {
            char *v = gen_expr_tac(node->left);
            tac_add(0, node->name, v, "", "");
            free(v);
        } else {
            tac_add(0, node->name, "0", "", "");
        }
        break;
    case N_ARR_DECL:
        sprintf(buf, "%d", node->arr_size);
        tac_add(0, node->name, "array", "", buf);
        break;
    case N_ASSIGN: {
        char *v = gen_expr_tac(node->left);
        tac_add(0, node->name, v, "", "");
        free(v);
        break;
    }
    case N_ARR_ASSIGN: {
        char *idx = gen_expr_tac(node->left);
        char *v   = gen_expr_tac(node->right);
        tac_add(8, node->name, idx, "[]=", v);
        free(idx); free(v);
        break;
    }

    case N_IF: {
        char *cond    = gen_expr_tac(node->left);
        char *l_false = sdup(new_label());
        char *l_end   = sdup(new_label());
        tac_add(3, l_false, cond, "", "");
        free(cond);
        gen_list_tac(node->body);
        if (node->right) {
            tac_add(2, l_end, "", "", "");
            tac_add(1, l_false, "", "", "");
            gen_list_tac(node->right);
            tac_add(1, l_end, "", "", "");
        } else {
            tac_add(1, l_false, "", "", "");
        }
        free(l_false); free(l_end);
        break;
    }

    case N_WHILE: {
        char *l_start = sdup(new_label());
        char *l_end   = sdup(new_label());
        if (loop_depth < MAX_LOOP) {
            strcpy(loop_start_lbl[loop_depth], l_start);
            strcpy(loop_end_lbl[loop_depth], l_end);
            loop_depth++;
        }
        tac_add(1, l_start, "", "", "");
        char *cond = gen_expr_tac(node->left);
        tac_add(3, l_end, cond, "", "");
        free(cond);
        gen_list_tac(node->body);
        tac_add(2, l_start, "", "", "");
        tac_add(1, l_end, "", "", "");
        loop_depth--;
        free(l_start); free(l_end);
        break;
    }

    case N_FOR: {
        char *l_start = sdup(new_label());
        char *l_end   = sdup(new_label());
        if (loop_depth < MAX_LOOP) {
            strcpy(loop_start_lbl[loop_depth], l_start);
            strcpy(loop_end_lbl[loop_depth], l_end);
            loop_depth++;
        }
        gen_stmt_tac(node->right);
        tac_add(1, l_start, "", "", "");
        char *cond = gen_expr_tac(node->left);
        tac_add(3, l_end, cond, "", "");
        free(cond);
        gen_list_tac(node->body);
        gen_stmt_tac(node->update);
        tac_add(2, l_start, "", "", "");
        tac_add(1, l_end, "", "", "");
        loop_depth--;
        free(l_start); free(l_end);
        break;
    }

    case N_PRINT: {
        char *v = gen_expr_tac(node->left);
        tac_add(4, "", v, "", "");
        tac_add(5, "", "print", "", "1");
        free(v);
        break;
    }
    case N_INPUT:
        tac_add(5, node->name, "input", "", "0");
        break;
    case N_RETURN:
        if (node->left) {
            char *v = gen_expr_tac(node->left);
            tac_add(6, "", v, "", "");
            free(v);
        } else {
            tac_add(6, "", "", "", "");
        }
        break;
    case N_BREAK:
        tac_add(2, loop_depth > 0 ? loop_end_lbl[loop_depth-1] : "???", "", "", "");
        break;
    case N_CONTINUE:
        tac_add(2, loop_depth > 0 ? loop_start_lbl[loop_depth-1] : "???", "", "", "");
        break;
    case N_INC:
        tac_add(0, node->name, node->name, "+", "1");
        break;
    case N_DEC:
        tac_add(0, node->name, node->name, "-", "1");
        break;
    case N_FUNC_CALL: {
        ASTNode *a = node->args;
        int ac = 0;
        while (a) {
            char *av = gen_expr_tac(a);
            tac_add(4, "", av, "", "");
            free(av);
            a = a->next;  ac++;
        }
        char nargs[8];
        sprintf(nargs, "%d", ac);
        tac_add(5, "", node->name, "", nargs);
        break;
    }
    default: break;
    }
}

/* ---- List & Program ---- */

void gen_list_tac(ASTNode *list) {
    ASTNode *s = list;
    while (s) { gen_stmt_tac(s); s = s->next; }
}

void gen_program_tac(ASTNode *root) {
    ASTNode *f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF) {
            tac_add(1, f->name, "", "", "");
            gen_list_tac(f->body);
            char end_lbl[80];
            sprintf(end_lbl, "end_%s", f->name);
            tac_add(1, end_lbl, "", "", "");
        }
        f = f->next;
    }
}

/* ---- Print TAC ---- */

void print_tac(FILE *f, const char *title) {
    fprintf(f, "\n============================================\n");
    fprintf(f, "  %s\n", title);
    fprintf(f, "============================================\n");

    for (int i = 0; i < tac_count; i++) {
        TACLine *t = &tac[i];
        switch (t->kind) {
        case 1: fprintf(f, "%s:\n", t->result); break;
        case 2: fprintf(f, "  goto %s\n", t->result); break;
        case 3: fprintf(f, "  ifFalse %s goto %s\n", t->arg1, t->result); break;
        case 4: fprintf(f, "  param %s\n", t->arg1); break;
        case 5:
            if (strlen(t->result) > 0)
                fprintf(f, "  %s = call %s, %s\n", t->result, t->arg1, t->arg2);
            else
                fprintf(f, "  call %s, %s\n", t->arg1, t->arg2);
            break;
        case 6:
            if (strlen(t->arg1) > 0)
                fprintf(f, "  return %s\n", t->arg1);
            else
                fprintf(f, "  return\n");
            break;
        case 7: fprintf(f, "  %s = %s[%s]\n", t->result, t->arg1, t->arg2); break;
        case 8: fprintf(f, "  %s[%s] = %s\n", t->result, t->arg1, t->arg2); break;
        default:
            if (strlen(t->op) > 0)
                fprintf(f, "  %s = %s %s %s\n", t->result, t->arg1, t->op, t->arg2);
            else
                fprintf(f, "  %s = %s\n", t->result, t->arg1);
            break;
        }
    }
    fprintf(f, "============================================\n");
}
