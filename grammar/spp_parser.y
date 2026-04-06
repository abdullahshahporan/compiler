%{
/* spp_parser.y — S++ Bison Grammar
   Bangla keywords: shuru/shesh, jodi/nahole, ghurao, barbar,
   bondho, chaliyejao, ferot, banaw, dekhao, neo,
   purno, dosomik, chinho, kisu_na, jog/biyog/gun/vag, root */

#include "common.h"
#include "ast.h"

void yyerror(const char *s);
int yylex(void);

%}

/* ==================== BISON UNION ====================
   Defines what kind of value each token/nonterminal carries.
   ====================================================== */
%union {
    int ival;
    double fval;
    char cval;
    char sval[256];
    struct ASTNode *node;
    int type_val;
}

/* ==================== TOKENS ====================
   Declared here, defined by the lexer (spp_lexer.l).
   Bison generates numeric IDs in spp_parser.tab.h.
   ================================================ */
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
%type <node> nested_func_def
%type <node> param_list_opt param_list
%type <node> arg_list_opt arg_list
%type <type_val> type_spec

/* ==================== OPERATOR PRECEDENCE ====================
   Lower lines = higher precedence.
   ============================================================= */
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

/* --- Program Structure --- */
program
    : function_list { ast_root = $1; }
    ;

function_list
    : function_def                  { $$ = $1; }
    | function_list function_def    { $$ = append_list($1, $2); }
    ;

/* --- Function Definitions ---
   S++ functions use shuru/shesh instead of { }
   "banaw" creates void functions                */
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

/* --- Parameters --- */
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

/* --- Type Specifiers --- */
type_spec
    : TYPE_INT    { $$ = T_INT; }
    | TYPE_FLOAT  { $$ = T_FLOAT; }
    | TYPE_CHAR   { $$ = T_CHAR; }
    | TYPE_VOID   { $$ = T_VOID; }
    ;

/* --- Statement List --- */
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
    | nested_func_def           { $$ = $1; }
    | error SEMICOLON {
        fprintf(stderr, "Syntax Error: Invalid statement at line %d (recovered)\n", line);
        error_count++;
        yyerrok;
        $$ = NULL;
    }
    ;

/* --- Nested Function Definition (inside another function) --- */
nested_func_def
    : type_spec IDENTIFIER LPAREN param_list_opt RPAREN SHURU stmt_list SHESH {
        $$ = make_func_def($1, $2, $4, $7);
    }
    | BANAW IDENTIFIER LPAREN param_list_opt RPAREN SHURU stmt_list SHESH {
        $$ = make_func_def(T_VOID, $2, $4, $7);
    }
    ;

/* --- Declarations --- */
declaration
    : type_spec IDENTIFIER                          { $$ = make_decl($1, $2, NULL); }
    | type_spec IDENTIFIER ASSIGN expression        { $$ = make_decl($1, $2, $4); }
    | TYPE_ARRAY type_spec IDENTIFIER LBRACKET INT_LIT RBRACKET {
        $$ = make_arr_decl($2, $3, $5);
    }
    ;

/* --- Assignment --- */
assignment
    : IDENTIFIER ASSIGN expression {
        $$ = make_assign($1, $3);
    }
    | IDENTIFIER LBRACKET expression RBRACKET ASSIGN expression {
        $$ = make_arr_assign($1, $3, $6);
    }
    ;

/* --- Expressions (arithmetic with Bangla keywords) ---
   jog = +,  biyog = -,  gun = *,  vag = /             */
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

/* --- Arguments --- */
arg_list_opt
    : /* empty */       { $$ = NULL; }
    | arg_list          { $$ = $1; }
    ;

arg_list
    : expression                        { $$ = $1; $$->next = NULL; }
    | arg_list COMMA expression         { $$ = append_list($1, $3); }
    ;

/* --- Conditions (comparison / logical) --- */
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

/* --- If / Else (jodi / nahole) --- */
if_stmt
    : JODI LPAREN condition RPAREN SHURU stmt_list SHESH {
        $$ = make_if($3, $6, NULL);
    }
    | JODI LPAREN condition RPAREN SHURU stmt_list SHESH NAHOLE SHURU stmt_list SHESH {
        $$ = make_if($3, $6, $10);
    }
    ;

/* --- While Loop (ghurao) --- */
while_stmt
    : GHURAO LPAREN condition RPAREN SHURU stmt_list SHESH {
        $$ = make_while($3, $6);
    }
    ;

/* --- For Loop (barbar) --- */
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

/* --- Print (dekhao) --- */
print_stmt
    : PRINT LPAREN expression RPAREN   { $$ = make_print($3); }
    ;

/* --- Input (neo) --- */
input_stmt
    : INPUT LPAREN IDENTIFIER RPAREN   { $$ = make_input($3); }
    ;

/* --- Return (ferot) --- */
return_stmt
    : FEROT expression      { $$ = make_return($2); }
    | FEROT                  { $$ = make_return(NULL); }
    ;

/* --- Break / Continue (bondho / chaliyejao) --- */
break_stmt    : BONDHO        { $$ = make_break_node(); }  ;
continue_stmt : CHALIYEJAO    { $$ = make_continue_node(); } ;

/* --- Function Call (as statement) --- */
func_call
    : IDENTIFIER LPAREN arg_list_opt RPAREN {
        $$ = make_func_call($1, $3);
    }
    ;

/* --- Increment / Decrement --- */
inc_dec_stmt
    : IDENTIFIER INC    { $$ = make_inc($1); }
    | IDENTIFIER DEC    { $$ = make_dec($1); }
    ;

%%

/* =============================================================
   ERROR HANDLER — Called by Bison when a syntax error occurs
   ============================================================= */
void yyerror(const char *s) {
    fprintf(stderr, "\n*** Syntax Error at line %d: %s ***\n", line, s);
    error_count++;
}
