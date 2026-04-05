/* common.h — Shared types, structures, and global declarations */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---- AST Node Types ---- */
typedef enum {
    N_PROGRAM, N_FUNC_DEF, N_PARAM, N_DECL, N_ARR_DECL,
    N_ASSIGN, N_ARR_ASSIGN, N_IF, N_WHILE, N_FOR,
    N_PRINT, N_INPUT, N_RETURN, N_BREAK, N_CONTINUE,
    N_BINOP, N_UNOP, N_SQRT_OP,
    N_INT_LIT, N_FLOAT_LIT, N_CHAR_LIT, N_STRING_LIT, N_BOOL_LIT,
    N_VAR, N_ARR_ACCESS, N_FUNC_CALL, N_INC, N_DEC
} NodeType;

/* ---- Variable Types ---- */
typedef enum {
    T_INT,      /* purno   */
    T_FLOAT,    /* dosomik */
    T_CHAR,     /* chinho  */
    T_VOID      /* kisu_na */
} VarType;

/* ---- AST Node ---- */
typedef struct ASTNode {
    NodeType ntype;
    int      lineno;
    int      ival;
    double   fval;
    char     cval;
    char     sval[256];
    char     name[64];
    int      var_type;
    int      op;
    int      arr_size;
    struct ASTNode *left, *right, *body, *update;
    struct ASTNode *args, *params, *next;
} ASTNode;

/* ---- Symbol Table Entry ---- */
#define MAX_SYMBOLS  500
#define MAX_ARR_VALS 1000

typedef struct {
    char    name[64];
    VarType type;
    double  value;
    double  arr_vals[MAX_ARR_VALS];
    int     arr_size;
    int     is_init;
    int     is_array;
    int     is_function;
    int     param_count;
    ASTNode *func_node;
    int     scope;
} Symbol;

/* ---- Execution Result ---- */
typedef struct {
    double  value;
    VarType type;
    int     is_break;
    int     is_continue;
    int     is_return;
    char    sval[256];
    int     is_string;
    int     is_error;
} ExecResult;

/* ---- TAC Instruction ----
   kind: 0=assign/binop, 1=label, 2=goto, 3=ifFalse,
         4=param, 5=call, 6=return, 7=arr_access, 8=arr_store */
#define MAX_TAC  3000
#define MAX_LOOP 32

typedef struct {
    char result[64];
    char arg1[64];
    char op[16];
    char arg2[64];
    int  kind;
} TACLine;

/* ---- Constants ---- */
#define MAX_NODES 10000

/* ---- Globals (extern declarations) ---- */

/* AST node pool (defined in ast.c) */
extern ASTNode node_pool[];
extern int node_count;

/* Symbol table (defined in symtab.c) */
extern Symbol sym_table[];
extern int sym_count;
extern int cur_scope;
extern int error_count;
extern int warning_count;

/* TAC storage (defined in tac.c) */
extern TACLine tac[];
extern int tac_count;
extern int temp_id;
extern int label_id;
extern char loop_start_lbl[MAX_LOOP][16];
extern char loop_end_lbl[MAX_LOOP][16];
extern int loop_depth;

/* Main program state (defined in main.c) */
extern ASTNode *ast_root;
extern FILE *output_file;
extern int final_sym_count;
extern int errors_before_exec;

/* Lexer globals (defined in lexer / lex.yy.c) */
extern FILE *yyin;
extern int line;

#endif /* COMMON_H */
