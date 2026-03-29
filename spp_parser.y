%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ================================================================
   S++ Compiler - Bison Parser (AST-based Interpreter)
   ================================================================
   Architecture: Parse -> Build AST -> Semantic Check -> Execute
                 -> Generate TAC -> Optimize -> Generate C Code
   ================================================================ */

void yyerror(const char *s);
int yylex(void);
extern int line;
extern FILE *yyin;

/* ==================== AST Node Types ==================== */
typedef enum {
    N_PROGRAM, N_FUNC_DEF, N_PARAM,
    N_DECL, N_ARR_DECL, N_ASSIGN, N_ARR_ASSIGN,
    N_IF, N_WHILE, N_FOR,
    N_PRINT, N_INPUT, N_RETURN, N_BREAK, N_CONTINUE,
    N_BINOP, N_UNOP, N_SQRT_OP,
    N_INT_LIT, N_FLOAT_LIT, N_CHAR_LIT, N_STRING_LIT, N_BOOL_LIT,
    N_VAR, N_ARR_ACCESS, N_FUNC_CALL, N_INC, N_DEC
} NodeType;

typedef enum { T_INT, T_FLOAT, T_CHAR, T_VOID } VarType;

/* ==================== AST Node Structure ==================== */
typedef struct ASTNode {
    NodeType    ntype;
    int         lineno;
    int         ival;
    double      fval;
    char        cval;
    char        sval[256];
    char        name[64];
    int         var_type;       /* VarType as int */
    int         op;             /* operator token */
    int         arr_size;
    struct ASTNode *left;       /* child 1 / condition / init expr */
    struct ASTNode *right;      /* child 2 / else block / for-init */
    struct ASTNode *body;       /* body for loops/functions */
    struct ASTNode *update;     /* for-loop update */
    struct ASTNode *args;       /* argument list */
    struct ASTNode *params;     /* parameter list */
    struct ASTNode *next;       /* linked list (stmt list, arg list) */
} ASTNode;

/* ==================== AST Node Pool ==================== */
#define MAX_NODES 10000
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

/* ==================== AST Constructors ==================== */
ASTNode* make_int_lit(int v) {
    ASTNode *n = new_node(N_INT_LIT); n->ival = v; return n;
}
ASTNode* make_float_lit(double v) {
    ASTNode *n = new_node(N_FLOAT_LIT); n->fval = v; return n;
}
ASTNode* make_char_lit(char v) {
    ASTNode *n = new_node(N_CHAR_LIT); n->cval = v; return n;
}
ASTNode* make_string_lit(const char *s) {
    ASTNode *n = new_node(N_STRING_LIT); strncpy(n->sval, s, 255); return n;
}
ASTNode* make_bool_lit(int v) {
    ASTNode *n = new_node(N_BOOL_LIT); n->ival = v; return n;
}
ASTNode* make_var(const char *name) {
    ASTNode *n = new_node(N_VAR); strncpy(n->name, name, 63); return n;
}
ASTNode* make_binop(int op, ASTNode *l, ASTNode *r) {
    ASTNode *n = new_node(N_BINOP); n->op = op; n->left = l; n->right = r; return n;
}
ASTNode* make_unop(int op, ASTNode *operand) {
    ASTNode *n = new_node(N_UNOP); n->op = op; n->left = operand; return n;
}
ASTNode* make_sqrt_op(ASTNode *expr) {
    ASTNode *n = new_node(N_SQRT_OP); n->left = expr; return n;
}
ASTNode* make_assign(const char *name, ASTNode *val) {
    ASTNode *n = new_node(N_ASSIGN); strncpy(n->name, name, 63); n->left = val; return n;
}
ASTNode* make_decl(int vt, const char *name, ASTNode *init) {
    ASTNode *n = new_node(N_DECL); n->var_type = vt; strncpy(n->name, name, 63); n->left = init; return n;
}
ASTNode* make_arr_decl(int vt, const char *name, int size) {
    ASTNode *n = new_node(N_ARR_DECL); n->var_type = vt; strncpy(n->name, name, 63); n->arr_size = size; return n;
}
ASTNode* make_arr_assign(const char *name, ASTNode *idx, ASTNode *val) {
    ASTNode *n = new_node(N_ARR_ASSIGN); strncpy(n->name, name, 63); n->left = idx; n->right = val; return n;
}
ASTNode* make_arr_access(const char *name, ASTNode *idx) {
    ASTNode *n = new_node(N_ARR_ACCESS); strncpy(n->name, name, 63); n->left = idx; return n;
}
ASTNode* make_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b) {
    ASTNode *n = new_node(N_IF); n->left = cond; n->body = then_b; n->right = else_b; return n;
}
ASTNode* make_while(ASTNode *cond, ASTNode *body) {
    ASTNode *n = new_node(N_WHILE); n->left = cond; n->body = body; return n;
}
ASTNode* make_for(ASTNode *init, ASTNode *cond, ASTNode *upd, ASTNode *body) {
    ASTNode *n = new_node(N_FOR); n->right = init; n->left = cond; n->update = upd; n->body = body; return n;
}
ASTNode* make_print(ASTNode *expr) {
    ASTNode *n = new_node(N_PRINT); n->left = expr; return n;
}
ASTNode* make_input(const char *name) {
    ASTNode *n = new_node(N_INPUT); strncpy(n->name, name, 63); return n;
}
ASTNode* make_return(ASTNode *expr) {
    ASTNode *n = new_node(N_RETURN); n->left = expr; return n;
}
ASTNode* make_break_node(void) { return new_node(N_BREAK); }
ASTNode* make_continue_node(void) { return new_node(N_CONTINUE); }
ASTNode* make_func_def(int rt, const char *name, ASTNode *par, ASTNode *body) {
    ASTNode *n = new_node(N_FUNC_DEF); n->var_type = rt; strncpy(n->name, name, 63);
    n->params = par; n->body = body; return n;
}
ASTNode* make_func_call(const char *name, ASTNode *args) {
    ASTNode *n = new_node(N_FUNC_CALL); strncpy(n->name, name, 63); n->args = args; return n;
}
ASTNode* make_inc(const char *name) {
    ASTNode *n = new_node(N_INC); strncpy(n->name, name, 63); return n;
}
ASTNode* make_dec(const char *name) {
    ASTNode *n = new_node(N_DEC); strncpy(n->name, name, 63); return n;
}
ASTNode* make_param(int vt, const char *name) {
    ASTNode *n = new_node(N_PARAM); n->var_type = vt; strncpy(n->name, name, 63); return n;
}

/* Append item to end of a linked list */
ASTNode* append_list(ASTNode *list, ASTNode *item) {
    if (!list) return item;
    ASTNode *p = list;
    while (p->next) p = p->next;
    p->next = item;
    return list;
}

/* ==================== Symbol Table (Scoped) ==================== */
#define MAX_SYMBOLS 500
#define MAX_ARR_VALS 1000

typedef struct {
    char name[64];
    VarType type;
    double value;
    double arr_vals[MAX_ARR_VALS];
    int arr_size;
    int is_init;
    int is_array;
    int is_function;
    int param_count;
    ASTNode *func_node;     /* pointer to function def AST */
    int scope;
} Symbol;

Symbol sym_table[MAX_SYMBOLS];
int sym_count = 0;
int cur_scope = 0;

/* Error/warning counters (declared here so sym_add can use them) */
int error_count = 0;
int warning_count = 0;

/* Lookup: find most recent match at or below current scope */
int sym_find(const char *name) {
    int found = -1;
    for (int i = 0; i < sym_count; i++)
        if (strcmp(sym_table[i].name, name) == 0 && sym_table[i].scope <= cur_scope)
            found = i;
    return found;
}

/* Lookup in current scope only (for duplicate detection) */
int sym_find_scope(const char *name) {
    for (int i = 0; i < sym_count; i++)
        if (strcmp(sym_table[i].name, name) == 0 && sym_table[i].scope == cur_scope)
            return i;
    return -1;
}

/* Add symbol to current scope */
int sym_add(const char *name, VarType type, int lineno) {
    if (sym_count >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Symbol table full\n");
        return -1;
    }
    if (sym_find_scope(name) != -1) {
        fprintf(stderr, "Semantic Error at line %d: '%s' already declared in this scope\n", lineno, name);
        error_count++;
        return -1;
    }
    int idx = sym_count++;
    memset(&sym_table[idx], 0, sizeof(Symbol));
    strncpy(sym_table[idx].name, name, 63);
    sym_table[idx].type = type;
    sym_table[idx].scope = cur_scope;
    return idx;
}

void scope_push(void) { cur_scope++; }
void scope_pop(void) {
    /* Remove all symbols at current scope */
    while (sym_count > 0 && sym_table[sym_count-1].scope == cur_scope)
        sym_count--;
    cur_scope--;
}

const char* type_name(VarType t) {
    switch(t) {
        case T_INT:   return "purno (int)";
        case T_FLOAT: return "dosomik (float)";
        case T_CHAR:  return "chinho (char)";
        case T_VOID:  return "kisu_na (void)";
    }
    return "unknown";
}

/* ==================== Debug Trace Macro ==================== */
#define DEBUG_EXEC 0
#if DEBUG_EXEC
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...) ((void)0)
#endif

/* ==================== Execution Result ==================== */
typedef struct {
    double value;
    VarType type;
    int is_break;
    int is_continue;
    int is_return;
    char sval[256];
    int is_string;
    int is_error;
} ExecResult;

ExecResult make_res(double val, VarType t) {
    ExecResult r;
    memset(&r, 0, sizeof(r));
    r.value = val; r.type = t;
    return r;
}

/* Forward declarations */
ExecResult eval_expr(ASTNode *node);
ExecResult exec_node(ASTNode *node);
ExecResult exec_list(ASTNode *list);
void exec_program(ASTNode *root);
void semantic_check_program(ASTNode *root);

/* Reset symbol table between phases */
void sym_reset(void) {
    sym_count = 0;
    cur_scope = 0;
}

/* ==================== TAC Generation ==================== */
#define MAX_TAC 3000
typedef struct {
    char result[64];
    char arg1[64];
    char op[16];
    char arg2[64];
    int kind; /* 0=assign/binop, 1=label, 2=goto, 3=ifFalse_goto, 4=param, 5=call, 6=return, 7=arr_access, 8=arr_store */
} TACLine;

TACLine tac[MAX_TAC];
int tac_count = 0;
int temp_id = 0;
int label_id = 0;

/* Loop label stack for break/continue */
#define MAX_LOOP 32
char loop_start_lbl[MAX_LOOP][16];
char loop_end_lbl[MAX_LOOP][16];
int loop_depth = 0;

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

/* Simple string duplicate */
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
    if (a1) strncpy(t->arg1, a1, 63);
    if (op) strncpy(t->op, op, 15);
    if (a2) strncpy(t->arg2, a2, 63);
}

/* Forward declarations for TAC */
char* gen_expr_tac(ASTNode *node);
void gen_stmt_tac(ASTNode *node);
void gen_list_tac(ASTNode *list);
void gen_program_tac(ASTNode *root);
void print_tac(FILE *f, const char *title);
void optimize_tac(void);
void print_sym_table(FILE *f);

/* ==================== Globals ==================== */
ASTNode *ast_root = NULL;
FILE *output_file = NULL;
int final_sym_count = 0;   /* captured before scope_pop for summary */
int errors_before_exec = 0; /* snapshot of error_count before execution */

%}

/* ==================== BISON UNION ==================== */
%union {
    int ival;
    double fval;
    char cval;
    char sval[256];
    struct ASTNode *node;
    int type_val;
}

/* ==================== TOKENS ==================== */
%token SHURU SHESH JODI NAHOLE GHURAO BARBAR
%token BONDHO CHALIYEJAO FEROT BANAW MAIN
%token TYPE_INT TYPE_FLOAT TYPE_CHAR TYPE_VOID TYPE_ARRAY
%token TRUE_VAL FALSE_VAL
%token PRINT INPUT
%token ADD SUB MUL DIV SQRT
%token EQ NEQ LE GE LT GT
%token AND OR NOT
%token INC DEC
%token ASSIGN
%token SEMICOLON COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET

%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token <cval> CHAR_LIT
%token <sval> STRING_LIT
%token <sval> IDENTIFIER

/* ==================== NON-TERMINAL TYPES ==================== */
%type <node> program function_list function_def
%type <node> stmt_list statement
%type <node> declaration assignment
%type <node> expression term factor primary
%type <node> condition
%type <node> if_stmt while_stmt for_stmt
%type <node> print_stmt input_stmt return_stmt
%type <node> break_stmt continue_stmt
%type <node> func_call inc_dec_stmt for_update
%type <node> param_list_opt param_list
%type <node> arg_list_opt arg_list
%type <type_val> type_spec

/* ==================== PRECEDENCE ==================== */
%left OR
%left AND
%left EQ NEQ
%left LT GT LE GE
%left ADD SUB
%left MUL DIV
%right NOT
%right SQRT

%%
/* =========================================================
   GRAMMAR RULES — Build AST, no execution during parsing
   ========================================================= */

program
    : function_list { ast_root = $1; }
    ;

function_list
    : function_def                  { $$ = $1; }
    | function_list function_def    { $$ = append_list($1, $2); }
    ;

function_def
    : type_spec MAIN LPAREN RPAREN SHURU stmt_list SHESH {
        $$ = make_func_def($1, "main", NULL, $6);
    }
    | type_spec IDENTIFIER LPAREN param_list_opt RPAREN SHURU stmt_list SHESH {
        $$ = make_func_def($1, $2, $4, $7);
    }
    | BANAW IDENTIFIER LPAREN param_list_opt RPAREN SHURU stmt_list SHESH {
        $$ = make_func_def(T_VOID, $2, $4, $7);
    }
    ;

param_list_opt
    : /* empty */       { $$ = NULL; }
    | param_list        { $$ = $1; }
    ;

param_list
    : type_spec IDENTIFIER {
        $$ = make_param($1, $2);
    }
    | param_list COMMA type_spec IDENTIFIER {
        $$ = append_list($1, make_param($3, $4));
    }
    ;

type_spec
    : TYPE_INT    { $$ = T_INT; }
    | TYPE_FLOAT  { $$ = T_FLOAT; }
    | TYPE_CHAR   { $$ = T_CHAR; }
    | TYPE_VOID   { $$ = T_VOID; }
    ;

stmt_list
    : /* empty */           { $$ = NULL; }
    | stmt_list statement   { $$ = append_list($1, $2); }
    ;

statement
    : declaration SEMICOLON     { $$ = $1; }
    | assignment SEMICOLON      { $$ = $1; }
    | print_stmt SEMICOLON      { $$ = $1; }
    | input_stmt SEMICOLON      { $$ = $1; }
    | if_stmt                   { $$ = $1; }
    | while_stmt                { $$ = $1; }
    | for_stmt                  { $$ = $1; }
    | return_stmt SEMICOLON     { $$ = $1; }
    | break_stmt SEMICOLON      { $$ = $1; }
    | continue_stmt SEMICOLON   { $$ = $1; }
    | func_call SEMICOLON       { $$ = $1; }
    | inc_dec_stmt SEMICOLON    { $$ = $1; }
    | error SEMICOLON {
        fprintf(stderr, "Syntax Error: Invalid statement at line %d (recovered)\n", line);
        error_count++;
        yyerrok;
        $$ = NULL;
    }
    ;

/* -------- Declarations -------- */
declaration
    : type_spec IDENTIFIER                          { $$ = make_decl($1, $2, NULL); }
    | type_spec IDENTIFIER ASSIGN expression        { $$ = make_decl($1, $2, $4); }
    | TYPE_ARRAY type_spec IDENTIFIER LBRACKET INT_LIT RBRACKET {
        $$ = make_arr_decl($2, $3, $5);
    }
    ;

/* -------- Assignment -------- */
assignment
    : IDENTIFIER ASSIGN expression {
        $$ = make_assign($1, $3);
    }
    | IDENTIFIER LBRACKET expression RBRACKET ASSIGN expression {
        $$ = make_arr_assign($1, $3, $6);
    }
    ;

/* -------- Expressions (arithmetic) -------- */
expression
    : term                          { $$ = $1; }
    | expression ADD term           { $$ = make_binop(ADD, $1, $3); }
    | expression SUB term           { $$ = make_binop(SUB, $1, $3); }
    ;

term
    : factor                        { $$ = $1; }
    | term MUL factor               { $$ = make_binop(MUL, $1, $3); }
    | term DIV factor               { $$ = make_binop(DIV, $1, $3); }
    ;

factor
    : primary                               { $$ = $1; }
    | SQRT LPAREN expression RPAREN         { $$ = make_sqrt_op($3); }
    | NOT factor                            { $$ = make_unop(NOT, $2); }
    ;

primary
    : INT_LIT                       { $$ = make_int_lit($1); }
    | FLOAT_LIT                     { $$ = make_float_lit($1); }
    | CHAR_LIT                      { $$ = make_char_lit($1); }
    | STRING_LIT                    { $$ = make_string_lit($1); }
    | TRUE_VAL                      { $$ = make_bool_lit(1); }
    | FALSE_VAL                     { $$ = make_bool_lit(0); }
    | IDENTIFIER                    { $$ = make_var($1); }
    | LPAREN expression RPAREN      { $$ = $2; }
    | IDENTIFIER LPAREN arg_list_opt RPAREN {
        $$ = make_func_call($1, $3);
    }
    | IDENTIFIER LBRACKET expression RBRACKET {
        $$ = make_arr_access($1, $3);
    }
    ;

/* -------- Arguments -------- */
arg_list_opt
    : /* empty */       { $$ = NULL; }
    | arg_list          { $$ = $1; }
    ;

arg_list
    : expression                        { $$ = $1; $$->next = NULL; }
    | arg_list COMMA expression         { $$ = append_list($1, $3); }
    ;

/* -------- Conditions (comparison / logical) -------- */
condition
    : expression EQ expression          { $$ = make_binop(EQ, $1, $3); }
    | expression NEQ expression         { $$ = make_binop(NEQ, $1, $3); }
    | expression LT expression          { $$ = make_binop(LT, $1, $3); }
    | expression GT expression          { $$ = make_binop(GT, $1, $3); }
    | expression LE expression          { $$ = make_binop(LE, $1, $3); }
    | expression GE expression          { $$ = make_binop(GE, $1, $3); }
    | condition AND condition           { $$ = make_binop(AND, $1, $3); }
    | condition OR condition            { $$ = make_binop(OR, $1, $3); }
    | LPAREN condition RPAREN           { $$ = $2; }
    ;

/* -------- If / Else -------- */
if_stmt
    : JODI LPAREN condition RPAREN SHURU stmt_list SHESH {
        $$ = make_if($3, $6, NULL);
    }
    | JODI LPAREN condition RPAREN SHURU stmt_list SHESH NAHOLE SHURU stmt_list SHESH {
        $$ = make_if($3, $6, $10);
    }
    ;

/* -------- While Loop -------- */
while_stmt
    : GHURAO LPAREN condition RPAREN SHURU stmt_list SHESH {
        $$ = make_while($3, $6);
    }
    ;

/* -------- For Loop -------- */
for_stmt
    : BARBAR LPAREN assignment SEMICOLON condition SEMICOLON for_update RPAREN SHURU stmt_list SHESH {
        $$ = make_for($3, $5, $7, $10);
    }
    ;

for_update
    : assignment                    { $$ = $1; }
    | IDENTIFIER INC                { $$ = make_inc($1); }
    | IDENTIFIER DEC                { $$ = make_dec($1); }
    ;

/* -------- Print -------- */
print_stmt
    : PRINT LPAREN expression RPAREN   { $$ = make_print($3); }
    ;

/* -------- Input -------- */
input_stmt
    : INPUT LPAREN IDENTIFIER RPAREN   { $$ = make_input($3); }
    ;

/* -------- Return -------- */
return_stmt
    : FEROT expression      { $$ = make_return($2); }
    | FEROT                  { $$ = make_return(NULL); }
    ;

/* -------- Break / Continue -------- */
break_stmt    : BONDHO        { $$ = make_break_node(); }  ;
continue_stmt : CHALIYEJAO    { $$ = make_continue_node(); } ;

/* -------- Function Call (as statement) -------- */
func_call
    : IDENTIFIER LPAREN arg_list_opt RPAREN {
        $$ = make_func_call($1, $3);
    }
    ;

/* -------- Increment / Decrement -------- */
inc_dec_stmt
    : IDENTIFIER INC    { $$ = make_inc($1); }
    | IDENTIFIER DEC    { $$ = make_dec($1); }
    ;

%%

/* =============================================================
   ERROR HANDLER
   ============================================================= */
void yyerror(const char *s) {
    fprintf(stderr, "\n*** Syntax Error at line %d: %s ***\n", line, s);
    error_count++;
}

/* =============================================================
   INTERPRETER — Walk the AST with proper control flow
   ============================================================= */

/* Evaluate an expression node, return its result */
ExecResult eval_expr(ASTNode *node) {
    ExecResult r;
    memset(&r, 0, sizeof(r));
    if (!node) return make_res(0, T_INT);

    switch (node->ntype) {
    case N_INT_LIT:
        return make_res(node->ival, T_INT);
    case N_FLOAT_LIT:
        return make_res(node->fval, T_FLOAT);
    case N_CHAR_LIT:
        return make_res(node->cval, T_CHAR);
    case N_BOOL_LIT:
        return make_res(node->ival, T_INT);
    case N_STRING_LIT:
        r = make_res(0, T_VOID);
        r.is_string = 1;
        strncpy(r.sval, node->sval, 255);
        return r;

    case N_VAR: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Runtime Error at line %d: Undefined variable '%s'\n", node->lineno, node->name);
            error_count++;
            r.is_error = 1; return r;
        }
        if (sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is an array, use subscript []\n", node->lineno, node->name);
            error_count++;
            r.is_error = 1; return r;
        }
        if (!sym_table[idx].is_init && !sym_table[idx].is_function) {
            fprintf(stderr, "Warning at line %d: '%s' used before initialization\n", node->lineno, node->name);
            warning_count++;
        }
        return make_res(sym_table[idx].value, sym_table[idx].type);
    }

    case N_ARR_ACCESS: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Runtime Error at line %d: Undefined array '%s'\n", node->lineno, node->name);
            error_count++;
            r.is_error = 1; return r;
        }
        if (!sym_table[idx].is_array) {
            fprintf(stderr, "Runtime Error at line %d: '%s' is not an array\n", node->lineno, node->name);
            error_count++;
            r.is_error = 1; return r;
        }
        ExecResult ir = eval_expr(node->left);
        if (ir.is_error) { r.is_error = 1; return r; }
        int ai = (int)ir.value;
        if (ai < 0 || ai >= sym_table[idx].arr_size) {
            fprintf(stderr, "Runtime Error at line %d: Index %d out of bounds for '%s[%d]'\n",
                    node->lineno, ai, node->name, sym_table[idx].arr_size);
            error_count++;
            r.is_error = 1; return r;
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
                    error_count++;
                    r.is_error = 1; return r;
                }
                /* Integer division when both operands are int */
                if (lv.type == T_INT && rv.type == T_INT)
                    return make_res((double)((int)lv.value / (int)rv.value), T_INT);
                return make_res(lv.value / rv.value, rt);
            case EQ:  return make_res(lv.value == rv.value, T_INT);
            case NEQ: return make_res(lv.value != rv.value, T_INT);
            case LT:  return make_res(lv.value < rv.value, T_INT);
            case GT:  return make_res(lv.value > rv.value, T_INT);
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
            error_count++;
            r.is_error = 1; return r;
        }
        return make_res(sqrt(av.value), T_FLOAT);
    }

    case N_FUNC_CALL: {
        /* Look up function */
        int idx = sym_find(node->name);
        if (idx == -1 || !sym_table[idx].is_function) {
            fprintf(stderr, "Runtime Error at line %d: Undefined function '%s'\n", node->lineno, node->name);
            error_count++;
            r.is_error = 1; return r;
        }
        ASTNode *func = sym_table[idx].func_node;

        /* Count arguments */
        int argc = 0;
        ASTNode *a = node->args;
        while (a) { argc++; a = a->next; }

        /* Check arity */
        if (argc != sym_table[idx].param_count) {
            fprintf(stderr, "Semantic Error at line %d: '%s' expects %d args, got %d\n",
                    node->lineno, node->name, sym_table[idx].param_count, argc);
            error_count++;
            r.is_error = 1; return r;
        }

        /* Evaluate arguments BEFORE pushing scope */
        double arg_vals[64];
        VarType arg_types[64];
        int arg_error = 0;
        a = node->args;
        for (int i = 0; i < argc; i++) {
            ExecResult av = eval_expr(a);
            if (av.is_error) arg_error = 1;
            arg_vals[i] = av.value;
            arg_types[i] = av.type;
            a = a->next;
        }
        if (arg_error) { r.is_error = 1; return r; }

        /* Push new scope, bind parameters */
        scope_push();
        ASTNode *param = func->params;
        for (int i = 0; i < argc && param; i++) {
            int pidx = sym_add(param->name, (VarType)param->var_type, node->lineno);
            if (pidx != -1) {
                /* Type check param vs argument */
                if ((VarType)param->var_type != arg_types[i]) {
                    if ((VarType)param->var_type == T_FLOAT && arg_types[i] == T_INT) {
                        /* implicit int->float: OK */
                    } else if ((VarType)param->var_type == T_INT && arg_types[i] == T_FLOAT) {
                        TRACE("  [Type Warning] Narrowing arg dosomik -> purno in call to %s()\n", node->name);
                        arg_vals[i] = (int)arg_vals[i];
                    }
                }
                sym_table[pidx].value = arg_vals[i];
                sym_table[pidx].is_init = 1;
            }
            param = param->next;
        }

        /* Execute function body */
        ExecResult result = exec_list(func->body);
        scope_pop();

        /* Consume return flag */
        result.is_return = 0;
        result.is_break = 0;
        result.is_continue = 0;
        return result;
    }

    default:
        break;
    }
    return make_res(0, T_INT);
}

/* Execute a single statement node */
ExecResult exec_node(ASTNode *node) {
    if (!node) return make_res(0, T_VOID);

    switch (node->ntype) {

    case N_DECL: {
        VarType vt = (VarType)node->var_type;
        int idx = sym_add(node->name, vt, node->lineno);
        if (idx != -1) {
            if (node->left) {
                ExecResult val = eval_expr(node->left);
                if (val.is_error) {
                    TRACE("  [Declare] %s (init failed)\n", node->name);
                    return make_res(0, T_VOID);
                }
                if (vt != val.type && !val.is_string) {
                    if (vt == T_FLOAT && val.type == T_INT) {
                        TRACE("  [Type Info] Implicit: purno -> dosomik at line %d\n", node->lineno);
                    } else if (vt == T_INT && val.type == T_FLOAT) {
                        TRACE("  [Type Warning] Narrowing: dosomik -> purno at line %d\n", node->lineno);
                        val.value = (int)val.value;
                    }
                }
                sym_table[idx].value = val.value;
                sym_table[idx].is_init = 1;
                TRACE("  [Declare] %s = %.4g\n", node->name, val.value);
            } else {
                TRACE("  [Declare] %s : %s (uninitialized)\n", node->name, type_name(vt));
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
            sym_table[idx].is_init = 1;
            TRACE("  [Declare Array] %s[%d] : %s\n", node->name, node->arr_size, type_name(vt));
        }
        return make_res(0, T_VOID);
    }

    case N_ASSIGN: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n", node->lineno, node->name);
            error_count++;
        } else if (sym_table[idx].is_function) {
            fprintf(stderr, "Semantic Error at line %d: Cannot assign to function '%s'\n", node->lineno, node->name);
            error_count++;
        } else if (sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: Cannot assign to array '%s' without subscript\n", node->lineno, node->name);
            error_count++;
        } else {
            ExecResult val = eval_expr(node->left);
            if (!val.is_error) {
                if (sym_table[idx].type != val.type && !val.is_string) {
                    if (sym_table[idx].type == T_FLOAT && val.type == T_INT) {
                        /* implicit OK */
                    } else if (sym_table[idx].type == T_INT && val.type == T_FLOAT) {
                        TRACE("  [Type Warning] Narrowing: dosomik -> purno at line %d\n", node->lineno);
                        val.value = (int)val.value;
                    }
                }
                sym_table[idx].value = val.value;
                sym_table[idx].is_init = 1;
            }
        }
        return make_res(0, T_VOID);
    }

    case N_ARR_ASSIGN: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared array '%s'\n", node->lineno, node->name);
            error_count++;
        } else if (!sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is not an array\n", node->lineno, node->name);
            error_count++;
        } else {
            ExecResult ir = eval_expr(node->left);
            if (ir.is_error) { return make_res(0, T_VOID); }
            int ai = (int)ir.value;
            if (ai < 0 || ai >= sym_table[idx].arr_size) {
                fprintf(stderr, "Runtime Error at line %d: Index %d out of bounds for '%s[%d]'\n",
                        node->lineno, ai, node->name, sym_table[idx].arr_size);
                error_count++;
            } else {
                ExecResult val = eval_expr(node->right);
                if (!val.is_error) {
                    sym_table[idx].arr_vals[ai] = val.value;
                    TRACE("  [Array Assign] %s[%d] = %.4g\n", node->name, ai, val.value);
                }
            }
        }
        return make_res(0, T_VOID);
    }

    /* -------- IF / ELSE: Execute ONLY the correct branch -------- */
    case N_IF: {
        ExecResult cond = eval_expr(node->left);
        if (cond.is_error) return make_res(0, T_VOID);
        if ((int)cond.value) {
            TRACE("  [If] TRUE at line %d\n", node->lineno);
            return exec_list(node->body);
        } else if (node->right) {
            TRACE("  [Else] at line %d\n", node->lineno);
            return exec_list(node->right);
        } else {
            TRACE("  [If] FALSE, skipped at line %d\n", node->lineno);
        }
        return make_res(0, T_VOID);
    }

    /* -------- WHILE: Real iterative loop -------- */
    case N_WHILE: {
        TRACE("  [While] entering loop at line %d\n", node->lineno);
        int iters = 0;
        while (1) {
            ExecResult cond = eval_expr(node->left);
            if (cond.is_error) break;
            if (!(int)cond.value) break;
            scope_push();  /* new scope for loop body (allows re-declaration) */
            ExecResult body_r = exec_list(node->body);
            scope_pop();
            if (body_r.is_break) break;
            if (body_r.is_return) return body_r;
            iters++;
            if (iters > 100000) {
                fprintf(stderr, "Runtime Error at line %d: Infinite loop (>100000 iters)\n", node->lineno);
                error_count++; break;
            }
        }
        TRACE("  [While] exited after %d iterations\n", iters);
        return make_res(0, T_VOID);
    }

    /* -------- FOR: Real iterative loop -------- */
    case N_FOR: {
        TRACE("  [For] entering loop at line %d\n", node->lineno);
        exec_node(node->right);  /* init */
        int iters = 0;
        while (1) {
            ExecResult cond = eval_expr(node->left);
            if (cond.is_error) break;
            if (!(int)cond.value) break;
            scope_push();  /* new scope for loop body */
            ExecResult body_r = exec_list(node->body);
            scope_pop();
            if (body_r.is_break) break;
            if (body_r.is_return) return body_r;
            exec_node(node->update);  /* update */
            iters++;
            if (iters > 100000) {
                fprintf(stderr, "Runtime Error at line %d: Infinite loop\n", node->lineno);
                error_count++; break;
            }
        }
        TRACE("  [For] exited after %d iterations\n", iters);
        return make_res(0, T_VOID);
    }

    /* -------- PRINT -------- */
    case N_PRINT: {
        ExecResult val = eval_expr(node->left);
        if (val.is_error) {
            /* Expression evaluation failed; skip printing garbage */
            return make_res(0, T_VOID);
        }
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

    /* -------- INPUT: Real runtime input -------- */
    case N_INPUT: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n", node->lineno, node->name);
            error_count++;
        } else {
            double val;
            printf("  [Input] Enter value for '%s': ", node->name);
            fflush(stdout);
            if (scanf("%lf", &val) == 1) {
                sym_table[idx].value = val;
                sym_table[idx].is_init = 1;
                TRACE("  [Input] %s = %.4g\n", node->name, val);
            } else {
                fprintf(stderr, "Runtime Error: Invalid input for '%s'\n", node->name);
                error_count++;
            }
        }
        return make_res(0, T_VOID);
    }

    /* -------- RETURN -------- */
    case N_RETURN: {
        ExecResult r;
        if (node->left) {
            r = eval_expr(node->left);
        } else {
            r = make_res(0, T_VOID);
        }
        r.is_return = 1;
        TRACE("  [Return] %.4g\n", r.value);
        return r;
    }

    /* -------- BREAK / CONTINUE -------- */
    case N_BREAK: {
        ExecResult r = make_res(0, T_VOID);
        r.is_break = 1;
        TRACE("  [Break]\n");
        return r;
    }
    case N_CONTINUE: {
        ExecResult r = make_res(0, T_VOID);
        r.is_continue = 1;
        TRACE("  [Continue]\n");
        return r;
    }

    /* -------- INCREMENT / DECREMENT -------- */
    case N_INC: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared '%s'\n", node->lineno, node->name);
            error_count++;
        } else {
            sym_table[idx].value++;
        }
        return make_res(0, T_VOID);
    }
    case N_DEC: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared '%s'\n", node->lineno, node->name);
            error_count++;
        } else {
            sym_table[idx].value--;
        }
        return make_res(0, T_VOID);
    }

    /* -------- FUNC_CALL as statement -------- */
    case N_FUNC_CALL:
        return eval_expr(node);

    default:
        return eval_expr(node);
    }
}

/* Execute a linked list of statements, propagating control flow */
ExecResult exec_list(ASTNode *list) {
    ExecResult r = make_res(0, T_VOID);
    ASTNode *s = list;
    while (s) {
        if (error_count > errors_before_exec) {
            printf("  [Halted] Execution stopped due to error(s)\n");
            return r;
        }
        r = exec_node(s);
        if (r.is_break || r.is_continue || r.is_return) return r;
        s = s->next;
    }
    return r;
}

/* Execute the entire program */
void exec_program(ASTNode *root) {
    /* Pass 1: Register all non-main functions */
    ASTNode *f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF && strcmp(f->name, "main") != 0) {
            int idx = sym_add(f->name, (VarType)f->var_type, f->lineno);
            if (idx != -1) {
                sym_table[idx].is_function = 1;
                sym_table[idx].func_node = f;
                int pc = 0;
                ASTNode *p = f->params;
                while (p) { pc++; p = p->next; }
                sym_table[idx].param_count = pc;
            }
        }
        f = f->next;
    }

    /* Pass 2: Execute main */
    f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF && strcmp(f->name, "main") == 0) {
            TRACE("\n--- Executing main() ---\n");
            scope_push();
            ExecResult r = exec_list(f->body);
            TRACE("--- main() finished ---\n");
            if (r.is_return) {
                TRACE("  Program returned: %d\n", (int)r.value);
            }
            /* Print symbol table BEFORE popping scope (to show local vars) */
            print_sym_table(stdout);
            if (output_file) print_sym_table(output_file);
            final_sym_count = sym_count;  /* capture for summary */
            scope_pop();
            return;
        }
        f = f->next;
    }
    fprintf(stderr, "Error: No main() function found!\n");
    error_count++;
}

/* =============================================================
   SEMANTIC VALIDATION PASS — Static checks before execution
   ============================================================= */

/* Forward declarations for semantic pass */
VarType infer_expr_type(ASTNode *node);
void semantic_check_node(ASTNode *node);
void semantic_check_list(ASTNode *list);

/* Infer the type of an expression and report semantic errors */
VarType infer_expr_type(ASTNode *node) {
    if (!node) return T_INT;
    switch (node->ntype) {
    case N_INT_LIT: case N_BOOL_LIT: return T_INT;
    case N_FLOAT_LIT: return T_FLOAT;
    case N_CHAR_LIT: return T_CHAR;
    case N_STRING_LIT: return T_VOID;
    case N_VAR: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n", node->lineno, node->name);
            error_count++;
            return T_INT;
        }
        if (sym_table[idx].is_function) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is a function, not a variable\n", node->lineno, node->name);
            error_count++;
        }
        if (sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is an array, use subscript []\n", node->lineno, node->name);
            error_count++;
        }
        return sym_table[idx].type;
    }
    case N_ARR_ACCESS: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared array '%s'\n", node->lineno, node->name);
            error_count++;
            return T_INT;
        }
        if (!sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is not an array\n", node->lineno, node->name);
            error_count++;
        }
        infer_expr_type(node->left);  /* check index expression */
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
            fprintf(stderr, "Semantic Error at line %d: Undeclared function '%s'\n", node->lineno, node->name);
            error_count++;
            return T_INT;
        }
        /* Count and check arguments */
        int argc = 0;
        ASTNode *a = node->args;
        while (a) { argc++; a = a->next; }
        if (argc != sym_table[idx].param_count) {
            fprintf(stderr, "Semantic Error at line %d: '%s' expects %d args, got %d\n",
                    node->lineno, node->name, sym_table[idx].param_count, argc);
            error_count++;
        }
        /* Type-check each argument against parameter types */
        a = node->args;
        ASTNode *param = sym_table[idx].func_node ? sym_table[idx].func_node->params : NULL;
        while (a) {
            VarType at = infer_expr_type(a);
            if (param) {
                if ((VarType)param->var_type == T_INT && at == T_FLOAT) {
                    fprintf(stderr, "Warning at line %d: Narrowing dosomik -> purno for arg '%s' in call to '%s'\n",
                            node->lineno, param->name, node->name);
                    warning_count++;
                }
                param = param->next;
            }
            a = a->next;
        }
        return sym_table[idx].type;
    }
    default: return T_INT;
    }
}

/* Check a single statement node for semantic errors */
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
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n", node->lineno, node->name);
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
            fprintf(stderr, "Semantic Error at line %d: Undeclared array '%s'\n", node->lineno, node->name);
            error_count++;
        } else if (!sym_table[idx].is_array) {
            fprintf(stderr, "Semantic Error at line %d: '%s' is not an array\n", node->lineno, node->name);
            error_count++;
        }
        infer_expr_type(node->left);   /* index */
        infer_expr_type(node->right);  /* value */
        break;
    }
    case N_IF:
        infer_expr_type(node->left);    /* condition */
        semantic_check_list(node->body);
        if (node->right) semantic_check_list(node->right);
        break;
    case N_WHILE:
        infer_expr_type(node->left);    /* condition */
        scope_push();
        semantic_check_list(node->body);
        scope_pop();
        break;
    case N_FOR:
        semantic_check_node(node->right);   /* init */
        infer_expr_type(node->left);         /* condition */
        scope_push();
        semantic_check_list(node->body);
        semantic_check_node(node->update);   /* update */
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
        infer_expr_type(node);  /* reuse function call type-checking */
        break;
    default: break;
    }
}

/* Check a list of statements */
void semantic_check_list(ASTNode *list) {
    ASTNode *s = list;
    while (s) {
        semantic_check_node(s);
        s = s->next;
    }
}

/* Semantic validation entry point */
void semantic_check_program(ASTNode *root) {
    /* Pass 1: Register all non-main functions */
    ASTNode *f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF && strcmp(f->name, "main") != 0) {
            int idx = sym_add(f->name, (VarType)f->var_type, f->lineno);
            if (idx != -1) {
                sym_table[idx].is_function = 1;
                sym_table[idx].func_node = f;
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
            /* Add parameters to scope */
            ASTNode *p = f->params;
            while (p) {
                int pidx = sym_add(p->name, (VarType)p->var_type, p->lineno);
                if (pidx != -1) sym_table[pidx].is_init = 1;
                p = p->next;
            }
            semantic_check_list(f->body);
            scope_pop();
        }
        f = f->next;
    }
}

/* =============================================================
   TAC GENERATION — Separate pass over the AST (correct labels)
   ============================================================= */

char* gen_expr_tac(ASTNode *node) {
    char buf[256];
    if (!node) return sdup("0");

    switch (node->ntype) {
    case N_INT_LIT:
        sprintf(buf, "%d", node->ival);
        return sdup(buf);
    case N_FLOAT_LIT:
        sprintf(buf, "%.4g", node->fval);
        return sdup(buf);
    case N_CHAR_LIT:
        sprintf(buf, "'%c'", node->cval);
        return sdup(buf);
    case N_STRING_LIT:
        sprintf(buf, "\"%s\"", node->sval);
        return sdup(buf);
    case N_BOOL_LIT:
        sprintf(buf, "%d", node->ival);
        return sdup(buf);
    case N_VAR:
        return sdup(node->name);
    case N_ARR_ACCESS: {
        char *idx = gen_expr_tac(node->left);
        char *t = sdup(new_temp());
        tac_add(7, t, node->name, "[]", idx);  /* kind 7 = arr access */
        free(idx);
        return t;
    }
    case N_BINOP: {
        char *l = gen_expr_tac(node->left);
        char *r = gen_expr_tac(node->right);
        const char *ops;
        switch (node->op) {
            case ADD: ops = "+"; break;
            case SUB: ops = "-"; break;
            case MUL: ops = "*"; break;
            case DIV: ops = "/"; break;
            case EQ:  ops = "=="; break;
            case NEQ: ops = "!="; break;
            case LT:  ops = "<"; break;
            case GT:  ops = ">"; break;
            case LE:  ops = "<="; break;
            case GE:  ops = ">="; break;
            case AND: ops = "&&"; break;
            case OR:  ops = "||"; break;
            default:  ops = "?"; break;
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
            tac_add(4, "", av, "", ""); /* param */
            free(av);
            a = a->next;
            ac++;
        }
        char *t = sdup(new_temp());
        char nargs[8]; sprintf(nargs, "%d", ac);
        tac_add(5, t, node->name, "", nargs); /* call */
        return t;
    }
    default:
        return sdup("0");
    }
}

void gen_stmt_tac(ASTNode *node) {
    char buf[64];
    if (!node) return;

    switch (node->ntype) {
    case N_DECL: {
        if (node->left) {
            char *v = gen_expr_tac(node->left);
            tac_add(0, node->name, v, "", "");
            free(v);
        } else {
            tac_add(0, node->name, "0", "", "");
        }
        break;
    }
    case N_ARR_DECL: {
        sprintf(buf, "%d", node->arr_size);
        tac_add(0, node->name, "array", "", buf);
        break;
    }
    case N_ASSIGN: {
        char *v = gen_expr_tac(node->left);
        tac_add(0, node->name, v, "", "");
        free(v);
        break;
    }
    case N_ARR_ASSIGN: {
        char *idx = gen_expr_tac(node->left);
        char *v = gen_expr_tac(node->right);
        tac_add(8, node->name, idx, "[]=", v); /* kind 8 = arr store */
        free(idx); free(v);
        break;
    }
    case N_IF: {
        char *cond = gen_expr_tac(node->left);
        char *l_false = sdup(new_label());
        char *l_end = sdup(new_label());

        tac_add(3, l_false, cond, "", ""); /* ifFalse goto l_false */
        free(cond);

        gen_list_tac(node->body);

        if (node->right) {
            tac_add(2, l_end, "", "", ""); /* goto l_end */
            tac_add(1, l_false, "", "", ""); /* label l_false */
            gen_list_tac(node->right);
            tac_add(1, l_end, "", "", ""); /* label l_end */
        } else {
            tac_add(1, l_false, "", "", ""); /* label l_false */
        }
        free(l_false); free(l_end);
        break;
    }
    case N_WHILE: {
        char *l_start = sdup(new_label());
        char *l_end = sdup(new_label());

        /* Push loop labels for break/continue */
        if (loop_depth < MAX_LOOP) {
            strcpy(loop_start_lbl[loop_depth], l_start);
            strcpy(loop_end_lbl[loop_depth], l_end);
            loop_depth++;
        }

        tac_add(1, l_start, "", "", ""); /* label l_start */
        char *cond = gen_expr_tac(node->left);
        tac_add(3, l_end, cond, "", ""); /* ifFalse goto l_end */
        free(cond);  

        gen_list_tac(node->body);

        tac_add(2, l_start, "", "", ""); /* goto l_start */
        tac_add(1, l_end, "", "", ""); /* label l_end */

        loop_depth--;
        free(l_start); free(l_end);
        break;
    }
    case N_FOR: {
        char *l_start = sdup(new_label());
        char *l_end = sdup(new_label());

        if (loop_depth < MAX_LOOP) {
            strcpy(loop_start_lbl[loop_depth], l_start);
            strcpy(loop_end_lbl[loop_depth], l_end);
            loop_depth++;
        }

        gen_stmt_tac(node->right); /* init */
        tac_add(1, l_start, "", "", ""); /* label l_start */
        char *cond = gen_expr_tac(node->left);
        tac_add(3, l_end, cond, "", ""); /* ifFalse goto l_end */
        free(cond);

        gen_list_tac(node->body);
        gen_stmt_tac(node->update);

        tac_add(2, l_start, "", "", ""); /* goto l_start */
        tac_add(1, l_end, "", "", ""); /* label l_end */

        loop_depth--;
        free(l_start); free(l_end);
        break;
    }
    case N_PRINT: {
        char *v = gen_expr_tac(node->left);
        tac_add(4, "", v, "", ""); /* param */
        tac_add(5, "", "print", "", "1"); /* call */
        free(v);
        break;
    }
    case N_INPUT: {
        tac_add(5, node->name, "input", "", "0"); /* call */
        break;
    }
    case N_RETURN: {
        if (node->left) {
            char *v = gen_expr_tac(node->left);
            tac_add(6, "", v, "", ""); /* return */
            free(v);
        } else {
            tac_add(6, "", "", "", "");
        }
        break;
    }
    case N_BREAK: {
        if (loop_depth > 0)
            tac_add(2, loop_end_lbl[loop_depth-1], "", "", "");
        else
            tac_add(2, "???", "", "", "");
        break;
    }
    case N_CONTINUE: {
        if (loop_depth > 0)
            tac_add(2, loop_start_lbl[loop_depth-1], "", "", "");
        else
            tac_add(2, "???", "", "", "");
        break;
    }
    case N_INC: {
        tac_add(0, node->name, node->name, "+", "1");
        break;
    }
    case N_DEC: {
        tac_add(0, node->name, node->name, "-", "1");
        break;
    }
    case N_FUNC_CALL: {
        ASTNode *a = node->args;
        int ac = 0;
        while (a) {
            char *av = gen_expr_tac(a);
            tac_add(4, "", av, "", "");
            free(av);
            a = a->next; ac++;
        }
        char nargs[8]; sprintf(nargs, "%d", ac);
        tac_add(5, "", node->name, "", nargs);
        break;
    }
    default:
        break;
    }
}

void gen_list_tac(ASTNode *list) {
    ASTNode *s = list;
    while (s) { gen_stmt_tac(s); s = s->next; }
}

void gen_program_tac(ASTNode *root) {
    ASTNode *f = root;
    while (f) {
        if (f->ntype == N_FUNC_DEF) {
            tac_add(1, f->name, "", "", ""); /* label: func name */
            gen_list_tac(f->body);
            char end_lbl[80];
            sprintf(end_lbl, "end_%s", f->name);
            tac_add(1, end_lbl, "", "", ""); /* label: end_func */
        }
        f = f->next;
    }
}

/* Print TAC */
void print_tac(FILE *f, const char *title) {
    fprintf(f, "\n============================================\n");
    fprintf(f, "  %s\n", title);
    fprintf(f, "============================================\n");
    for (int i = 0; i < tac_count; i++) {
        TACLine *t = &tac[i];
        switch (t->kind) {
        case 1: /* label */
            fprintf(f, "%s:\n", t->result);
            break;
        case 2: /* goto */
            fprintf(f, "  goto %s\n", t->result);
            break;
        case 3: /* ifFalse goto */
            fprintf(f, "  ifFalse %s goto %s\n", t->arg1, t->result);
            break;
        case 4: /* param */
            fprintf(f, "  param %s\n", t->arg1);
            break;
        case 5: /* call */
            if (strlen(t->result) > 0)
                fprintf(f, "  %s = call %s, %s\n", t->result, t->arg1, t->arg2);
            else
                fprintf(f, "  call %s, %s\n", t->arg1, t->arg2);
            break;
        case 6: /* return */
            if (strlen(t->arg1) > 0)
                fprintf(f, "  return %s\n", t->arg1);
            else
                fprintf(f, "  return\n");
            break;
        case 7: /* arr access: result = arg1[arg2] */
            fprintf(f, "  %s = %s[%s]\n", t->result, t->arg1, t->arg2);
            break;
        case 8: /* arr store: result[arg1] = arg2 */
            fprintf(f, "  %s[%s] = %s\n", t->result, t->arg1, t->arg2);
            break;
        default: /* 0: assignment or binop */
            if (strlen(t->op) > 0 && strlen(t->arg2) > 0)
                fprintf(f, "  %s = %s %s %s\n", t->result, t->arg1, t->op, t->arg2);
            else if (strlen(t->op) > 0)
                fprintf(f, "  %s = %s(%s)\n", t->result, t->arg1, t->op);
            else
                fprintf(f, "  %s = %s\n", t->result, t->arg1);
            break;
        }
    }
    fprintf(f, "============================================\n");
}

/* =============================================================
   CODE OPTIMIZATION — Constant Folding + Propagation + Dead Code
   ============================================================= */

/* Helper: check if string is a numeric constant */
int is_numeric(const char *s) {
    if (!s || !*s) return 0;
    char *end;
    strtod(s, &end);
    return (*end == '\0');
}

void optimize_tac(void) {
    int outer_pass = 1;
    while (outer_pass) {
        outer_pass = 0;

        /* --- Pass 1: Constant Folding (arithmetic on two constants) --- */
        int fold_changes = 1;
        while (fold_changes) {
            fold_changes = 0;
            for (int i = 0; i < tac_count; i++) {
                if (tac[i].kind != 0) continue;

                /* Fold sqrt(const) → result */
                if (strcmp(tac[i].arg1, "sqrt") == 0 && is_numeric(tac[i].op)) {
                    double v = strtod(tac[i].op, NULL);
                    if (v >= 0) {
                        double res = sqrt(v);
                        if (res == (int)res)
                            sprintf(tac[i].arg1, "%d", (int)res);
                        else
                            sprintf(tac[i].arg1, "%.4g", res);
                        tac[i].op[0] = '\0';
                        fold_changes = 1; outer_pass = 1;
                    }
                    continue;
                }

                if (strlen(tac[i].op) == 0) continue;
                if (!is_numeric(tac[i].arg1) || !is_numeric(tac[i].arg2)) continue;

                double v1 = strtod(tac[i].arg1, NULL);
                double v2 = strtod(tac[i].arg2, NULL);

                double res = 0;
                int valid = 1;
                if (strcmp(tac[i].op, "+") == 0)       res = v1 + v2;
                else if (strcmp(tac[i].op, "-") == 0)  res = v1 - v2;
                else if (strcmp(tac[i].op, "*") == 0)  res = v1 * v2;
                else if (strcmp(tac[i].op, "/") == 0) {
                    if (v2 != 0) res = v1 / v2; else valid = 0;
                }
                else if (strcmp(tac[i].op, "==") == 0) res = (v1 == v2);
                else if (strcmp(tac[i].op, "!=") == 0) res = (v1 != v2);
                else if (strcmp(tac[i].op, "<") == 0)  res = (v1 < v2);
                else if (strcmp(tac[i].op, ">") == 0)  res = (v1 > v2);
                else if (strcmp(tac[i].op, "<=") == 0) res = (v1 <= v2);
                else if (strcmp(tac[i].op, ">=") == 0) res = (v1 >= v2);
                else valid = 0;

                if (valid) {
                    if (res == (int)res)
                        sprintf(tac[i].arg1, "%d", (int)res);
                    else
                        sprintf(tac[i].arg1, "%.4g", res);
                    tac[i].op[0] = '\0';
                    tac[i].arg2[0] = '\0';
                    fold_changes = 1; outer_pass = 1;
                }
            }
        }

        /* --- Pass 2: Constant Propagation (for temp variables) --- */
        for (int i = 0; i < tac_count; i++) {
            if (tac[i].kind != 0) continue;
            if (strlen(tac[i].op) > 0) continue;      /* must be simple assign: x = const */
            if (!is_numeric(tac[i].arg1)) continue;    /* RHS must be numeric constant */
            if (tac[i].result[0] != 't') continue;     /* only propagate temp vars (tN) */

            const char *var = tac[i].result;
            const char *val = tac[i].arg1;

            for (int j = i + 1; j < tac_count; j++) {
                /* Stop if var is reassigned */
                if ((tac[j].kind == 0 || tac[j].kind == 5 || tac[j].kind == 7)
                    && strcmp(tac[j].result, var) == 0) break;

                int replaced = 0;
                if (strcmp(tac[j].arg1, var) == 0) {
                    strncpy(tac[j].arg1, val, 63);
                    replaced = 1;
                }
                if (strcmp(tac[j].arg2, var) == 0) {
                    strncpy(tac[j].arg2, val, 63);
                    replaced = 1;
                }
                if (replaced) outer_pass = 1;
            }
        }
    }

    /* --- Pass 3: Dead code elimination --- */
    /* Remove goto immediately followed by its own label */
    for (int i = 0; i < tac_count - 1; i++) {
        if (tac[i].kind == 2 && tac[i+1].kind == 1) {
            if (strcmp(tac[i].result, tac[i+1].result) == 0) {
                for (int j = i; j < tac_count - 1; j++)
                    tac[j] = tac[j+1];
                tac_count--;
                i--;
            }
        }
    }

    /* Remove dead temp assignments: tN = val where tN is never used later */
    for (int i = 0; i < tac_count; i++) {
        if (tac[i].kind != 0) continue;
        if (tac[i].result[0] != 't') continue;
        if (strlen(tac[i].op) > 0) continue;  /* only simple assigns */

        int used = 0;
        for (int j = i + 1; j < tac_count; j++) {
            if (strcmp(tac[j].arg1, tac[i].result) == 0 ||
                strcmp(tac[j].arg2, tac[i].result) == 0) {
                used = 1; break;
            }
            /* If reassigned before used, it's dead */
            if (tac[j].kind == 0 && strcmp(tac[j].result, tac[i].result) == 0) break;
        }
        if (!used) {
            for (int j = i; j < tac_count - 1; j++)
                tac[j] = tac[j+1];
            tac_count--;
            i--;
        }
    }
}

/* =============================================================
   C CODE GENERATION — Translate S++ AST to equivalent C code
   ============================================================= */
void gen_c_expr(FILE *f, ASTNode *node) {
    if (!node) { fprintf(f, "0"); return; }
    switch (node->ntype) {
    case N_INT_LIT: fprintf(f, "%d", node->ival); break;
    case N_FLOAT_LIT: fprintf(f, "%.4g", node->fval); break;
    case N_CHAR_LIT: fprintf(f, "'%c'", node->cval); break;
    case N_STRING_LIT: fprintf(f, "\"%s\"", node->sval); break;
    case N_BOOL_LIT: fprintf(f, "%d", node->ival); break;
    case N_VAR: fprintf(f, "%s", node->name); break;
    case N_ARR_ACCESS:
        fprintf(f, "%s[", node->name);
        gen_c_expr(f, node->left);
        fprintf(f, "]");
        break;
    case N_BINOP: {
        const char *ops;
        switch (node->op) {
            case ADD: ops = "+"; break; case SUB: ops = "-"; break;
            case MUL: ops = "*"; break; case DIV: ops = "/"; break;
            case EQ: ops = "=="; break; case NEQ: ops = "!="; break;
            case LT: ops = "<"; break; case GT: ops = ">"; break;
            case LE: ops = "<="; break; case GE: ops = ">="; break;
            case AND: ops = "&&"; break; case OR: ops = "||"; break;
            default: ops = "?"; break;
        }
        fprintf(f, "(");
        gen_c_expr(f, node->left);
        fprintf(f, " %s ", ops);
        gen_c_expr(f, node->right);
        fprintf(f, ")");
        break;
    }
    case N_UNOP: fprintf(f, "!("); gen_c_expr(f, node->left); fprintf(f, ")"); break;
    case N_SQRT_OP: fprintf(f, "sqrt("); gen_c_expr(f, node->left); fprintf(f, ")"); break;
    case N_FUNC_CALL: {
        fprintf(f, "%s(", node->name);
        ASTNode *a = node->args;
        int first = 1;
        while (a) {
            if (!first) fprintf(f, ", ");
            gen_c_expr(f, a);
            a = a->next; first = 0;
        }
        fprintf(f, ")");
        break;
    }
    default: fprintf(f, "0"); break;
    }
}

const char* c_type(int vt) {
    switch ((VarType)vt) {
        case T_INT: return "int"; case T_FLOAT: return "float";
        case T_CHAR: return "char"; case T_VOID: return "void";
    }
    return "int";
}

void gen_c_list(FILE *f, ASTNode *list, int indent);

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
        fprintf(f, "%s = ", node->name);
        gen_c_expr(f, node->left);
        fprintf(f, ";\n");
        break;
    case N_ARR_ASSIGN:
        fprintf(f, "%s[", node->name);
        gen_c_expr(f, node->left);
        fprintf(f, "] = ");
        gen_c_expr(f, node->right);
        fprintf(f, ";\n");
        break;
    case N_IF:
        fprintf(f, "if (");
        gen_c_expr(f, node->left);
        fprintf(f, ") {\n");
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
        fprintf(f, "while (");
        gen_c_expr(f, node->left);
        fprintf(f, ") {\n");
        gen_c_list(f, node->body, indent + 1);
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "}\n");
        break;
    case N_FOR:
        fprintf(f, "/* for */ ");
        gen_c_stmt(f, node->right, 0); /* init (writes its own semicolon) */
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "while (");
        gen_c_expr(f, node->left);
        fprintf(f, ") {\n");
        gen_c_list(f, node->body, indent + 1);
        for (int i = 0; i < indent + 1; i++) fprintf(f, "    ");
        gen_c_stmt(f, node->update, 0);
        for (int i = 0; i < indent; i++) fprintf(f, "    ");
        fprintf(f, "}\n");
        break;
    case N_PRINT:
        if (node->left && node->left->ntype == N_STRING_LIT) {
            fprintf(f, "printf(\"%%s\\n\", \"%s\");\n", node->left->sval);
        } else {
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
        if (node->left) gen_c_expr(f, node->left);
        else fprintf(f, "0");
        fprintf(f, ";\n");
        break;
    case N_BREAK: fprintf(f, "break;\n"); break;
    case N_CONTINUE: fprintf(f, "continue;\n"); break;
    case N_INC: fprintf(f, "%s++;\n", node->name); break;
    case N_DEC: fprintf(f, "%s--;\n", node->name); break;
    case N_FUNC_CALL:
        gen_c_expr(f, node);
        fprintf(f, ";\n");
        break;
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
                p = p->next; first = 0;
            }
            fprintf(f, ") {\n");
            gen_c_list(f, func->body, 1);
            fprintf(f, "}\n\n");
        }
        func = func->next;
    }
    fprintf(f, "============================================\n");
}

/* =============================================================
   PRINT SYMBOL TABLE
   ============================================================= */
void print_sym_table(FILE *f) {
    fprintf(f, "\n============================================\n");
    fprintf(f, "           SYMBOL TABLE\n");
    fprintf(f, "============================================\n");
    fprintf(f, "%-15s %-15s %-12s %-8s %-8s\n", "Name", "Type", "Value", "Init?", "Scope");
    fprintf(f, "------------------------------------------------------------\n");
    for (int i = 0; i < sym_count; i++) {
        if (!sym_table[i].is_function) {
            if (sym_table[i].is_array) {
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
                    vbuf,
                    sym_table[i].is_init ? "Yes" : "No",
                    sym_table[i].scope);
            }
        } else {
            fprintf(f, "%-15s %-15s %-12s %-8s %-8d\n",
                sym_table[i].name, type_name(sym_table[i].type),
                "(func)", "-", sym_table[i].scope);
        }
    }
    fprintf(f, "============================================\n");
}

/* =============================================================
   MAIN
   ============================================================= */
int main(int argc, char *argv[]) {
    printf("============================================\n");
    printf("       S++ Compiler (SPP) v3.0\n");
    printf("  Bangla-Keyword Programming Language\n");
    printf("  Architecture: AST + Semantic + Interpreter + TAC\n");
    printf("============================================\n");

    char *in_file = "input.txt";
    char *out_file = "output.txt";
    if (argc >= 2) in_file = argv[1];
    if (argc >= 3) out_file = argv[2];

    FILE *in = fopen(in_file, "r");
    if (!in) { fprintf(stderr, "Error: Cannot open '%s'\n", in_file); return 1; }

    output_file = fopen(out_file, "w");
    if (!output_file) { fprintf(stderr, "Error: Cannot create '%s'\n", out_file); fclose(in); return 1; }

    fprintf(output_file, "============================================\n");
    fprintf(output_file, "       S++ Program Output\n");
    fprintf(output_file, "============================================\n\n");

    yyin = in;

    /* Phase 1: Parse and build AST */
    printf("\n[Phase 1] Parsing '%s'...\n", in_file);
    int parse_result = yyparse();

    if (parse_result != 0 || !ast_root) {
        fprintf(stderr, "Parsing failed.\n");
        fclose(in); fclose(output_file);
        return 1;
    }
    printf("[Phase 1] Parsing complete. AST built successfully.\n");

    /* Phase 2: Semantic analysis */
    printf("\n[Phase 2] Semantic analysis...\n");
    semantic_check_program(ast_root);
    if (error_count > 0) {
        printf("[Phase 2] Found %d semantic error(s).\n", error_count);
    } else {
        printf("[Phase 2] Semantic analysis passed.\n");
    }
    sym_reset();  /* clean up symbol table for execution phase */

    /* Count functions in AST */
    int func_count = 0;
    { ASTNode *fc = ast_root; while (fc) { if (fc->ntype == N_FUNC_DEF) func_count++; fc = fc->next; } }
    int exec_halted = 0;

    /* Phase 3: Execute the program via AST interpretation */
    if (error_count == 0) {
        printf("\n[Phase 3] Executing program...\n");
        errors_before_exec = error_count;  /* snapshot so exec_list detects NEW errors */
        exec_program(ast_root);
        if (error_count > errors_before_exec) exec_halted = 1;
    }

    if (error_count == 0) {
        /* Phase 4: Generate TAC from AST */
        printf("\n[Phase 4] Generating intermediate code...\n");
        gen_program_tac(ast_root);
        print_tac(stdout, "INTERMEDIATE CODE (Three-Address Code)");
        print_tac(output_file, "INTERMEDIATE CODE (Three-Address Code)");

        /* Phase 5: Optimize */
        printf("\n[Phase 5] Optimizing...\n");
        optimize_tac();
        print_tac(stdout, "OPTIMIZED CODE (Constant Folding + Dead Code Elim)");
        print_tac(output_file, "OPTIMIZED CODE (Constant Folding + Dead Code Elim)");

        /* Phase 6: C Code Generation */
        printf("\n[Phase 6] Generating equivalent C code...\n");
        gen_c_program(stdout, ast_root);
        gen_c_program(output_file, ast_root);
    } else {
        printf("\n[Phase 4-6] Skipped -- %d error(s) detected, no code generation.\n", error_count);
        fprintf(output_file, "\n[Phase 4-6] Skipped -- %d error(s) detected, no code generation.\n", error_count);
    }

    /* Flush stderr so errors appear before summary */
    fflush(stderr);

    /* Summary */
    printf("\n============================================\n");
    printf("           COMPILATION SUMMARY\n");
    printf("============================================\n");
    printf("  Errors     : %d\n", error_count);
    printf("  Warnings   : %d\n", warning_count);
    printf("  Functions  : %d\n", func_count);
    printf("  Symbols    : %d\n", final_sym_count);
    printf("  TAC Lines  : %d\n", tac_count);
    printf("  AST Nodes  : %d\n", node_count);
    printf("  Exec Halted: %s\n", exec_halted ? "Yes" : "No");
    printf("  Status     : %s\n", (error_count == 0) ? "SUCCESS" : "FAILED");
    printf("============================================\n");

    fprintf(output_file, "\n============================================\n");
    fprintf(output_file, "           COMPILATION SUMMARY\n");
    fprintf(output_file, "============================================\n");
    fprintf(output_file, "  Errors     : %d\n", error_count);
    fprintf(output_file, "  Warnings   : %d\n", warning_count);
    fprintf(output_file, "  Functions  : %d\n", func_count);
    fprintf(output_file, "  Symbols    : %d\n", final_sym_count);
    fprintf(output_file, "  TAC Lines  : %d\n", tac_count);
    fprintf(output_file, "  AST Nodes  : %d\n", node_count);
    fprintf(output_file, "  Exec Halted: %s\n", exec_halted ? "Yes" : "No");
    fprintf(output_file, "  Status     : %s\n", (error_count == 0) ? "SUCCESS" : "FAILED");
    fprintf(output_file, "============================================\n");

    fclose(in);
    fclose(output_file);
    return (error_count == 0) ? 0 : 1;
}
