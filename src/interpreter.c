/* interpreter.c — Tree-walk interpreter for S++ AST */

#include "interpreter.h"
#include "symtab.h"
#include "spp_parser.tab.h"

ExecResult make_res(double val, VarType t) {
    ExecResult r;
    memset(&r, 0, sizeof(r));
    r.value = val;
    r.type  = t;
    return r;
}

/* ---- Expression Evaluation ---- */

ExecResult eval_expr(ASTNode *node) {
    ExecResult r;
    memset(&r, 0, sizeof(r));
    if (!node) return make_res(0, T_INT);

    switch (node->ntype) {
    case N_INT_LIT:    return make_res(node->ival, T_INT);
    case N_FLOAT_LIT:  return make_res(node->fval, T_FLOAT);
    case N_CHAR_LIT:   return make_res(node->cval, T_CHAR);
    case N_BOOL_LIT:   return make_res(node->ival, T_INT);

    case N_STRING_LIT:
        r = make_res(0, T_VOID);
        r.is_string = 1;
        strncpy(r.sval, node->sval, 255);
        return r;

    case N_VAR: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Runtime Error at line %d: Undefined variable '%s'\n",
                    node->lineno, node->name);
            error_count++;  r.is_error = 1;  return r;
        }
        if (sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is an array, use subscript []\n",
                    node->lineno, node->name);
            error_count++;  r.is_error = 1;  return r;
        }
        if (!sym_table[idx].is_init && !sym_table[idx].is_function) {
            fprintf(stderr, "Warning at line %d: '%s' used before initialization\n",
                    node->lineno, node->name);
            warning_count++;
        }
        return make_res(sym_table[idx].value, sym_table[idx].type);
    }

    case N_ARR_ACCESS: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Runtime Error at line %d: Undefined array '%s'\n",
                    node->lineno, node->name);
            error_count++;  r.is_error = 1;  return r;
        }
        if (!sym_table[idx].is_array) {
            fprintf(stderr, "Runtime Error at line %d: '%s' is not an array\n",
                    node->lineno, node->name);
            error_count++;  r.is_error = 1;  return r;
        }
        ExecResult ir = eval_expr(node->left);
        if (ir.is_error) { r.is_error = 1; return r; }
        int ai = (int)ir.value;
        if (ai < 0 || ai >= sym_table[idx].arr_size) {
            fprintf(stderr, "Runtime Error at line %d: Index %d out of bounds for '%s[%d]'\n",
                    node->lineno, ai, node->name, sym_table[idx].arr_size);
            error_count++;  r.is_error = 1;  return r;
        }
        return make_res(sym_table[idx].arr_vals[ai], sym_table[idx].type);
    }

    case N_BINOP: {
        ExecResult lv = eval_expr(node->left);
        ExecResult rv = eval_expr(node->right);
        if (lv.is_error || rv.is_error) { r.is_error = 1; return r; }
        VarType rt = (lv.type == T_FLOAT || rv.type == T_FLOAT) ? T_FLOAT : T_INT;
        switch (node->op) {
            case ADD: return make_res(lv.value + rv.value, rt);
            case SUB: return make_res(lv.value - rv.value, rt);
            case MUL: return make_res(lv.value * rv.value, rt);
            case DIV:
                if (rv.value == 0) {
                    fprintf(stderr, "Runtime Error at line %d: Division by zero\n", node->lineno);
                    error_count++;  r.is_error = 1;  return r;
                }
                if (lv.type == T_INT && rv.type == T_INT)
                    return make_res((double)((int)lv.value / (int)rv.value), T_INT);
                return make_res(lv.value / rv.value, rt);
            case EQ:  return make_res(lv.value == rv.value, T_INT);
            case NEQ: return make_res(lv.value != rv.value, T_INT);
            case LT:  return make_res(lv.value <  rv.value, T_INT);
            case GT:  return make_res(lv.value >  rv.value, T_INT);
            case LE:  return make_res(lv.value <= rv.value, T_INT);
            case GE:  return make_res(lv.value >= rv.value, T_INT);
            case AND: return make_res((int)lv.value && (int)rv.value, T_INT);
            case OR:  return make_res((int)lv.value || (int)rv.value, T_INT);
        }
        break;
    }

    case N_UNOP: {
        ExecResult ov = eval_expr(node->left);
        if (ov.is_error) { r.is_error = 1; return r; }
        if (node->op == NOT) return make_res(!((int)ov.value), T_INT);
        break;
    }

    case N_SQRT_OP: {
        ExecResult av = eval_expr(node->left);
        if (av.is_error) { r.is_error = 1; return r; }
        if (av.value < 0) {
            fprintf(stderr, "Runtime Error at line %d: sqrt of negative number\n", node->lineno);
            error_count++;  r.is_error = 1;  return r;
        }
        return make_res(sqrt(av.value), T_FLOAT);
    }

    case N_FUNC_CALL: {
        int idx = sym_find(node->name);
        if (idx == -1 || !sym_table[idx].is_function) {
            fprintf(stderr, "Runtime Error at line %d: Undefined function '%s'\n",
                    node->lineno, node->name);
            error_count++;  r.is_error = 1;  return r;
        }
        ASTNode *func = sym_table[idx].func_node;

        int argc = 0;
        ASTNode *a = node->args;
        while (a) { argc++; a = a->next; }
        if (argc != sym_table[idx].param_count) {
            fprintf(stderr, "Semantic Error at line %d: '%s' expects %d args, got %d\n",
                    node->lineno, node->name, sym_table[idx].param_count, argc);
            error_count++;  r.is_error = 1;  return r;
        }

        /* Evaluate arguments before pushing scope */
        double arg_vals[64];
        VarType arg_types[64];
        int arg_error = 0;
        a = node->args;
        for (int i = 0; i < argc; i++) {
            ExecResult av = eval_expr(a);
            if (av.is_error) arg_error = 1;
            arg_vals[i]  = av.value;
            arg_types[i] = av.type;
            a = a->next;
        }
        if (arg_error) { r.is_error = 1; return r; }

        /* Push scope and bind parameters */
        scope_push();
        ASTNode *param = func->params;
        for (int i = 0; i < argc && param; i++) {
            int pidx = sym_add(param->name, (VarType)param->var_type, node->lineno);
            if (pidx != -1) {
                if ((VarType)param->var_type == T_INT && arg_types[i] == T_FLOAT)
                    arg_vals[i] = (int)arg_vals[i];
                sym_table[pidx].value   = arg_vals[i];
                sym_table[pidx].is_init = 1;
            }
            param = param->next;
        }

        ExecResult result = exec_list(func->body);
        scope_pop();
        result.is_return = 0;
        result.is_break  = 0;
        result.is_continue = 0;
        return result;
    }

    default: break;
    }
    return make_res(0, T_INT);
}

/* ---- Statement Execution ---- */

ExecResult exec_node(ASTNode *node) {
    if (!node) return make_res(0, T_VOID);

    switch (node->ntype) {
    case N_DECL: {
        VarType vt = (VarType)node->var_type;
        int idx = sym_add(node->name, vt, node->lineno);
        if (idx != -1 && node->left) {
            ExecResult val = eval_expr(node->left);
            if (!val.is_error) {
                if (vt == T_INT && val.type == T_FLOAT) val.value = (int)val.value;
                sym_table[idx].value   = val.value;
                sym_table[idx].is_init = 1;
            }
        }
        return make_res(0, T_VOID);
    }

    case N_ARR_DECL: {
        VarType vt = (VarType)node->var_type;
        int idx = sym_add(node->name, vt, node->lineno);
        if (idx != -1) {
            sym_table[idx].is_array = 1;
            sym_table[idx].arr_size = node->arr_size;
            sym_table[idx].is_init  = 1;
        }
        return make_res(0, T_VOID);
    }

    case N_ASSIGN: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n",
                    node->lineno, node->name);
            error_count++;
        } else if (sym_table[idx].is_function) {
            fprintf(stderr, "Semantic Error at line %d: Cannot assign to function '%s'\n",
                    node->lineno, node->name);
            error_count++;
        } else if (sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: Cannot assign to array '%s' without subscript\n",
                    node->lineno, node->name);
            error_count++;
        } else {
            ExecResult val = eval_expr(node->left);
            if (!val.is_error) {
                if (sym_table[idx].type == T_INT && val.type == T_FLOAT)
                    val.value = (int)val.value;
                sym_table[idx].value   = val.value;
                sym_table[idx].is_init = 1;
            }
        }
        return make_res(0, T_VOID);
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
        } else {
            ExecResult ir = eval_expr(node->left);
            if (ir.is_error) return make_res(0, T_VOID);
            int ai = (int)ir.value;
            if (ai < 0 || ai >= sym_table[idx].arr_size) {
                fprintf(stderr, "Runtime Error at line %d: Index %d out of bounds for '%s[%d]'\n",
                        node->lineno, ai, node->name, sym_table[idx].arr_size);
                error_count++;
            } else {
                ExecResult val = eval_expr(node->right);
                if (!val.is_error)
                    sym_table[idx].arr_vals[ai] = val.value;
            }
        }
        return make_res(0, T_VOID);
    }

    case N_IF: {
        ExecResult cond = eval_expr(node->left);
        if (cond.is_error) return make_res(0, T_VOID);
        if ((int)cond.value)
            return exec_list(node->body);
        else if (node->right)
            return exec_list(node->right);
        return make_res(0, T_VOID);
    }

    case N_WHILE: {
        int iters = 0;
        while (1) {
            ExecResult cond = eval_expr(node->left);
            if (cond.is_error) break;
            if (!(int)cond.value) break;
            scope_push();
            ExecResult body_r = exec_list(node->body);
            scope_pop();
            if (body_r.is_break)  break;
            if (body_r.is_return) return body_r;
            if (++iters > 100000) {
                fprintf(stderr, "Runtime Error at line %d: Infinite loop (>100000 iters)\n",
                        node->lineno);
                error_count++;  break;
            }
        }
        return make_res(0, T_VOID);
    }

    case N_FOR: {
        exec_node(node->right);  /* init */
        int iters = 0;
        while (1) {
            ExecResult cond = eval_expr(node->left);
            if (cond.is_error) break;
            if (!(int)cond.value) break;
            scope_push();
            ExecResult body_r = exec_list(node->body);
            scope_pop();
            if (body_r.is_break)  break;
            if (body_r.is_return) return body_r;
            exec_node(node->update);
            if (++iters > 100000) {
                fprintf(stderr, "Runtime Error at line %d: Infinite loop\n", node->lineno);
                error_count++;  break;
            }
        }
        return make_res(0, T_VOID);
    }

    case N_PRINT: {
        ExecResult val = eval_expr(node->left);
        if (val.is_error) return make_res(0, T_VOID);
        if (val.is_string) {
            printf("  [Output] %s\n", val.sval);
            if (output_file) fprintf(output_file, "%s\n", val.sval);
        } else if (val.type == T_INT) {
            printf("  [Output] %d\n", (int)val.value);
            if (output_file) fprintf(output_file, "%d\n", (int)val.value);
        } else {
            printf("  [Output] %.4g\n", val.value);
            if (output_file) fprintf(output_file, "%.4g\n", val.value);
        }
        return make_res(0, T_VOID);
    }

    case N_INPUT: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n",
                    node->lineno, node->name);
            error_count++;
        } else {
            double val;
            printf("  [Input] Enter value for '%s': ", node->name);
            fflush(stdout);
            if (scanf("%lf", &val) == 1) {
                sym_table[idx].value   = val;
                sym_table[idx].is_init = 1;
            } else {
                fprintf(stderr, "Runtime Error: Invalid input for '%s'\n", node->name);
                error_count++;
            }
        }
        return make_res(0, T_VOID);
    }

    case N_RETURN: {
        ExecResult r = node->left ? eval_expr(node->left) : make_res(0, T_VOID);
        r.is_return = 1;
        return r;
    }

    case N_BREAK:    { ExecResult r = make_res(0, T_VOID); r.is_break = 1;    return r; }
    case N_CONTINUE: { ExecResult r = make_res(0, T_VOID); r.is_continue = 1; return r; }

    case N_INC: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared '%s'\n",
                    node->lineno, node->name);
            error_count++;
        } else {
            sym_table[idx].value++;
        }
        return make_res(0, T_VOID);
    }
    case N_DEC: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared '%s'\n",
                    node->lineno, node->name);
            error_count++;
        } else {
            sym_table[idx].value--;
        }
        return make_res(0, T_VOID);
    }

    case N_FUNC_CALL:
        return eval_expr(node);

    default:
        return eval_expr(node);
    }
}

/* ---- Statement List ---- */

ExecResult exec_list(ASTNode *list) {
    ExecResult r = make_res(0, T_VOID);
    ASTNode *s = list;
    while (s) {
        if (error_count > errors_before_exec) {
            printf("  [Halted] Execution stopped due to error(s)\n");
            return r;
        }
        r = exec_node(s);
        if (r.is_break || r.is_continue || r.is_return)
            return r;
        s = s->next;
    }
    return r;
}

/* ---- Program Execution ---- */

void exec_program(ASTNode *root) {
    /* Register all non-main functions */
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

    /* Find and execute main() */
    f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF && strcmp(f->name, "main") == 0) {
            scope_push();
            ExecResult r = exec_list(f->body);
            (void)r;
            print_sym_table(stdout);
            if (output_file) print_sym_table(output_file);
            final_sym_count = sym_count;
            scope_pop();
            return;
        }
        f = f->next;
    }
    fprintf(stderr, "Error: No main() function found!\n");
    error_count++;
}
