/* ast.c — AST node pool and constructor implementations */

#include "ast.h"

ASTNode node_pool[MAX_NODES];
int node_count = 0;

ASTNode* new_node(NodeType t) {
    if (node_count >= MAX_NODES) {
        fprintf(stderr, "Fatal: AST node pool exhausted\n");
        exit(1);
    }
    ASTNode *n = &node_pool[node_count++];
    memset(n, 0, sizeof(ASTNode));
    n->ntype = t;
    n->lineno = line;
    return n;
}

/* ---- Literals ---- */

ASTNode* make_int_lit(int v) {
    ASTNode *n = new_node(N_INT_LIT);  n->ival = v;  return n;
}
ASTNode* make_float_lit(double v) {
    ASTNode *n = new_node(N_FLOAT_LIT);  n->fval = v;  return n;
}
ASTNode* make_char_lit(char v) {
    ASTNode *n = new_node(N_CHAR_LIT);  n->cval = v;  return n;
}
ASTNode* make_string_lit(const char *s) {
    ASTNode *n = new_node(N_STRING_LIT);  strncpy(n->sval, s, 255);  return n;
}
ASTNode* make_bool_lit(int v) {
    ASTNode *n = new_node(N_BOOL_LIT);  n->ival = v;  return n;
}

/* ---- Variables & Operators ---- */

ASTNode* make_var(const char *name) {
    ASTNode *n = new_node(N_VAR);  strncpy(n->name, name, 63);  return n;
}
ASTNode* make_binop(int op, ASTNode *l, ASTNode *r) {
    ASTNode *n = new_node(N_BINOP);  n->op = op;  n->left = l;  n->right = r;  return n;
}
ASTNode* make_unop(int op, ASTNode *operand) {
    ASTNode *n = new_node(N_UNOP);  n->op = op;  n->left = operand;  return n;
}
ASTNode* make_sqrt_op(ASTNode *expr) {
    ASTNode *n = new_node(N_SQRT_OP);  n->left = expr;  return n;
}

/* ---- Declarations & Assignments ---- */

ASTNode* make_assign(const char *name, ASTNode *val) {
    ASTNode *n = new_node(N_ASSIGN);  strncpy(n->name, name, 63);  n->left = val;  return n;
}
ASTNode* make_decl(int vt, const char *name, ASTNode *init) {
    ASTNode *n = new_node(N_DECL);
    n->var_type = vt;  strncpy(n->name, name, 63);  n->left = init;
    return n;
}
ASTNode* make_arr_decl(int vt, const char *name, int size) {
    ASTNode *n = new_node(N_ARR_DECL);
    n->var_type = vt;  strncpy(n->name, name, 63);  n->arr_size = size;
    return n;
}
ASTNode* make_arr_assign(const char *name, ASTNode *idx, ASTNode *val) {
    ASTNode *n = new_node(N_ARR_ASSIGN);
    strncpy(n->name, name, 63);  n->left = idx;  n->right = val;
    return n;
}
ASTNode* make_arr_access(const char *name, ASTNode *idx) {
    ASTNode *n = new_node(N_ARR_ACCESS);  strncpy(n->name, name, 63);  n->left = idx;  return n;
}

/* ---- Control Flow ---- */

ASTNode* make_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b) {
    ASTNode *n = new_node(N_IF);
    n->left = cond;  n->body = then_b;  n->right = else_b;
    return n;
}
ASTNode* make_while(ASTNode *cond, ASTNode *body) {
    ASTNode *n = new_node(N_WHILE);  n->left = cond;  n->body = body;  return n;
}
ASTNode* make_for(ASTNode *init, ASTNode *cond, ASTNode *upd, ASTNode *body) {
    ASTNode *n = new_node(N_FOR);
    n->right = init;  n->left = cond;  n->update = upd;  n->body = body;
    return n;
}

/* ---- I/O & Return ---- */

ASTNode* make_print(ASTNode *expr) {
    ASTNode *n = new_node(N_PRINT);  n->left = expr;  return n;
}
ASTNode* make_input(const char *name) {
    ASTNode *n = new_node(N_INPUT);  strncpy(n->name, name, 63);  return n;
}
ASTNode* make_return(ASTNode *expr) {
    ASTNode *n = new_node(N_RETURN);  n->left = expr;  return n;
}
ASTNode* make_break_node(void)    { return new_node(N_BREAK); }
ASTNode* make_continue_node(void) { return new_node(N_CONTINUE); }

/* ---- Functions ---- */

ASTNode* make_func_def(int rt, const char *name, ASTNode *par, ASTNode *body) {
    ASTNode *n = new_node(N_FUNC_DEF);
    n->var_type = rt;  strncpy(n->name, name, 63);  n->params = par;  n->body = body;
    return n;
}
ASTNode* make_func_call(const char *name, ASTNode *args) {
    ASTNode *n = new_node(N_FUNC_CALL);  strncpy(n->name, name, 63);  n->args = args;  return n;
}
ASTNode* make_param(int vt, const char *name) {
    ASTNode *n = new_node(N_PARAM);  n->var_type = vt;  strncpy(n->name, name, 63);  return n;
}

/* ---- Inc/Dec ---- */

ASTNode* make_inc(const char *name) {
    ASTNode *n = new_node(N_INC);  strncpy(n->name, name, 63);  return n;
}
ASTNode* make_dec(const char *name) {
    ASTNode *n = new_node(N_DEC);  strncpy(n->name, name, 63);  return n;
}

/* ---- List Helper ---- */

ASTNode* append_list(ASTNode *list, ASTNode *item) {
    if (!list) return item;
    ASTNode *p = list;
    while (p->next) p = p->next;
    p->next = item;
    return list;
}
