/* codegen.c — Translate S++ AST to equivalent C source code */

#include "codegen.h"
#include "spp_parser.tab.h"

const char* c_type(int vt) {
    switch ((VarType)vt) {
        case T_INT:   return "int";
        case T_FLOAT: return "float";
        case T_CHAR:  return "char";
        case T_VOID:  return "void";
    }
    return "int";
}

/* ---- Expression ---- */

void gen_c_expr(FILE *f, ASTNode *node) {
    if (!node) { fprintf(f, "0"); return; }

    switch (node->ntype) {
    case N_INT_LIT:    fprintf(f, "%d", node->ival);       break;
    case N_FLOAT_LIT:  fprintf(f, "%.4g", node->fval);     break;
    case N_CHAR_LIT:   fprintf(f, "'%c'", node->cval);     break;
    case N_STRING_LIT: fprintf(f, "\"%s\"", node->sval);   break;
    case N_BOOL_LIT:   fprintf(f, "%d", node->ival);       break;
    case N_VAR:        fprintf(f, "%s", node->name);        break;

    case N_ARR_ACCESS:
        fprintf(f, "%s[", node->name);
        gen_c_expr(f, node->left);
        fprintf(f, "]");
        break;

    case N_BINOP: {
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
        fprintf(f, "(");
        gen_c_expr(f, node->left);
        fprintf(f, " %s ", ops);
        gen_c_expr(f, node->right);
        fprintf(f, ")");
        break;
    }

    case N_UNOP:
        fprintf(f, "!("); gen_c_expr(f, node->left); fprintf(f, ")");
        break;

    case N_SQRT_OP:
        fprintf(f, "sqrt("); gen_c_expr(f, node->left); fprintf(f, ")");
        break;

    case N_FUNC_CALL: {
        fprintf(f, "%s(", node->name);
        ASTNode *a = node->args;
        int first = 1;
        while (a) {
            if (!first) fprintf(f, ", ");
            gen_c_expr(f, a);
            a = a->next;  first = 0;
        }
        fprintf(f, ")");
        break;
    }

    default: fprintf(f, "0"); break;
    }
}

/* ---- Statement ---- */

void gen_c_stmt(FILE *f, ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) fprintf(f, "    ");

    switch (node->ntype) {
    case N_DECL:
        fprintf(f, "%s %s", c_type(node->var_type), node->name);
        if (node->left) { fprintf(f, " = "); gen_c_expr(f, node->left); }
        fprintf(f, ";\n");
        break;
    case N_ARR_DECL:
        fprintf(f, "%s %s[%d];\n", c_type(node->var_type), node->name, node->arr_size);
        break;
    case N_ASSIGN:
        fprintf(f, "%s = ", node->name); gen_c_expr(f, node->left); fprintf(f, ";\n");
        break;
    case N_ARR_ASSIGN:
        fprintf(f, "%s[", node->name); gen_c_expr(f, node->left);
        fprintf(f, "] = "); gen_c_expr(f, node->right); fprintf(f, ";\n");
        break;

    case N_IF:
        fprintf(f, "if ("); gen_c_expr(f, node->left); fprintf(f, ") {\n");
        gen_c_list(f, node->body, indent + 1);
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "}");
        if (node->right) {
            fprintf(f, " else {\n");
            gen_c_list(f, node->right, indent + 1);
            for (int i = 0; i < indent; i++) fprintf(f, "    ");
            fprintf(f, "}");
        }
        fprintf(f, "\n");
        break;

    case N_WHILE:
        fprintf(f, "while ("); gen_c_expr(f, node->left); fprintf(f, ") {\n");
        gen_c_list(f, node->body, indent + 1);
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "}\n");
        break;

    case N_FOR:
        fprintf(f, "/* for */ ");
        gen_c_stmt(f, node->right, 0);
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "while ("); gen_c_expr(f, node->left); fprintf(f, ") {\n");
        gen_c_list(f, node->body, indent + 1);
        for (int i = 0; i < indent + 1; i++) fprintf(f, "    ");
        gen_c_stmt(f, node->update, 0);
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "}\n");
        break;

    case N_PRINT:
        if (node->left && node->left->ntype == N_STRING_LIT)
            fprintf(f, "printf(\"%%s\\n\", \"%s\");\n", node->left->sval);
        else {
            fprintf(f, "printf(\"%%g\\n\", (double)(");
            gen_c_expr(f, node->left);
            fprintf(f, "));\n");
        }
        break;
    case N_INPUT:
        fprintf(f, "scanf(\"%%lf\", &%s);\n", node->name);
        break;
    case N_RETURN:
        fprintf(f, "return ");
        if (node->left) gen_c_expr(f, node->left); else fprintf(f, "0");
        fprintf(f, ";\n");
        break;
    case N_BREAK:    fprintf(f, "break;\n");    break;
    case N_CONTINUE: fprintf(f, "continue;\n"); break;
    case N_INC:      fprintf(f, "%s++;\n", node->name); break;
    case N_DEC:      fprintf(f, "%s--;\n", node->name); break;
    case N_FUNC_CALL:
        gen_c_expr(f, node); fprintf(f, ";\n");
        break;
    case N_FUNC_DEF: {
        /* Nested function (GCC extension) */
        fprintf(f, "/* nested */ %s %s(", c_type(node->var_type), node->name);
        ASTNode *p = node->params;
        int first = 1;
        while (p) {
            if (!first) fprintf(f, ", ");
            fprintf(f, "%s %s", c_type(p->var_type), p->name);
            p = p->next;  first = 0;
        }
        fprintf(f, ") {\n");
        gen_c_list(f, node->body, indent + 1);
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "}\n");
        break;
    }
    default: break;
    }
}

void gen_c_list(FILE *f, ASTNode *list, int indent) {
    ASTNode *s = list;
    while (s) { gen_c_stmt(f, s, indent); s = s->next; }
}

void gen_c_program(FILE *f, ASTNode *root) {
    fprintf(f, "\n============================================\n");
    fprintf(f, "  TRANSLATED C CODE\n");
    fprintf(f, "============================================\n");
    fprintf(f, "#include <stdio.h>\n#include <math.h>\n\n");

    ASTNode *func = root;
    while (func) {
        if (func->ntype == N_FUNC_DEF) {
            fprintf(f, "%s %s(", c_type(func->var_type), func->name);
            ASTNode *p = func->params;
            int first = 1;
            while (p) {
                if (!first) fprintf(f, ", ");
                fprintf(f, "%s %s", c_type(p->var_type), p->name);
                p = p->next;  first = 0;
            }
            fprintf(f, ") {\n");
            gen_c_list(f, func->body, 1);
            fprintf(f, "}\n\n");
        }
        func = func->next;
    }
    fprintf(f, "============================================\n");
}
