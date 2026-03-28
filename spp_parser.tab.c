
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "spp_parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ================================================================
   S++ Compiler - Bison Parser (AST-based Interpreter)
   ================================================================
   Architecture: Parse -> Build AST -> Execute AST -> Generate TAC
   This ensures correct control flow for if/else, loops, functions.
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

/* ==================== Execution Result ==================== */
typedef struct {
    double value;
    VarType type;
    int is_break;
    int is_continue;
    int is_return;
    char sval[256];
    int is_string;
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



/* Line 189 of yacc.c  */
#line 403 "spp_parser.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SHURU = 258,
     SHESH = 259,
     JODI = 260,
     NAHOLE = 261,
     GHURAO = 262,
     BARBAR = 263,
     BONDHO = 264,
     CHALIYEJAO = 265,
     FEROT = 266,
     BANAW = 267,
     MAIN = 268,
     TYPE_INT = 269,
     TYPE_FLOAT = 270,
     TYPE_CHAR = 271,
     TYPE_VOID = 272,
     TYPE_ARRAY = 273,
     TRUE_VAL = 274,
     FALSE_VAL = 275,
     PRINT = 276,
     INPUT = 277,
     ADD = 278,
     SUB = 279,
     MUL = 280,
     DIV = 281,
     SQRT = 282,
     EQ = 283,
     NEQ = 284,
     LE = 285,
     GE = 286,
     LT = 287,
     GT = 288,
     AND = 289,
     OR = 290,
     NOT = 291,
     INC = 292,
     DEC = 293,
     ASSIGN = 294,
     SEMICOLON = 295,
     COMMA = 296,
     LPAREN = 297,
     RPAREN = 298,
     LBRACE = 299,
     RBRACE = 300,
     LBRACKET = 301,
     RBRACKET = 302,
     INT_LIT = 303,
     FLOAT_LIT = 304,
     CHAR_LIT = 305,
     STRING_LIT = 306,
     IDENTIFIER = 307
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 331 "spp_parser.y"

    int ival;
    double fval;
    char cval;
    char sval[256];
    struct ASTNode *node;
    int type_val;



/* Line 214 of yacc.c  */
#line 502 "spp_parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 514 "spp_parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   302

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  83
/* YYNRULES -- Number of states.  */
#define YYNSTATES  189

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   307

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    18,    27,    36,    37,
      39,    42,    47,    49,    51,    53,    55,    56,    59,    62,
      65,    68,    71,    73,    75,    77,    80,    83,    86,    89,
      92,    95,    98,   103,   110,   114,   121,   123,   127,   131,
     133,   137,   141,   143,   148,   151,   153,   155,   157,   159,
     161,   163,   165,   169,   174,   179,   180,   182,   184,   188,
     192,   196,   200,   204,   208,   212,   216,   220,   224,   232,
     244,   252,   264,   266,   269,   272,   277,   282,   285,   287,
     289,   291,   296,   299
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      54,     0,    -1,    55,    -1,    56,    -1,    55,    56,    -1,
      59,    13,    42,    43,     3,    60,     4,    -1,    59,    52,
      42,    57,    43,     3,    60,     4,    -1,    12,    52,    42,
      57,    43,     3,    60,     4,    -1,    -1,    58,    -1,    59,
      52,    -1,    58,    41,    59,    52,    -1,    14,    -1,    15,
      -1,    16,    -1,    17,    -1,    -1,    60,    61,    -1,    62,
      40,    -1,    63,    40,    -1,    75,    40,    -1,    76,    40,
      -1,    71,    -1,    72,    -1,    73,    -1,    77,    40,    -1,
      78,    40,    -1,    79,    40,    -1,    80,    40,    -1,    81,
      40,    -1,     1,    40,    -1,    59,    52,    -1,    59,    52,
      39,    64,    -1,    18,    59,    52,    46,    48,    47,    -1,
      52,    39,    64,    -1,    52,    46,    64,    47,    39,    64,
      -1,    65,    -1,    64,    23,    65,    -1,    64,    24,    65,
      -1,    66,    -1,    65,    25,    66,    -1,    65,    26,    66,
      -1,    67,    -1,    27,    42,    64,    43,    -1,    36,    66,
      -1,    48,    -1,    49,    -1,    50,    -1,    51,    -1,    19,
      -1,    20,    -1,    52,    -1,    42,    64,    43,    -1,    52,
      42,    68,    43,    -1,    52,    46,    64,    47,    -1,    -1,
      69,    -1,    64,    -1,    69,    41,    64,    -1,    64,    28,
      64,    -1,    64,    29,    64,    -1,    64,    32,    64,    -1,
      64,    33,    64,    -1,    64,    30,    64,    -1,    64,    31,
      64,    -1,    70,    34,    70,    -1,    70,    35,    70,    -1,
      42,    70,    43,    -1,     5,    42,    70,    43,     3,    60,
       4,    -1,     5,    42,    70,    43,     3,    60,     4,     6,
       3,    60,     4,    -1,     7,    42,    70,    43,     3,    60,
       4,    -1,     8,    42,    63,    40,    70,    40,    74,    43,
       3,    60,     4,    -1,    63,    -1,    52,    37,    -1,    52,
      38,    -1,    21,    42,    64,    43,    -1,    22,    42,    52,
      43,    -1,    11,    64,    -1,    11,    -1,     9,    -1,    10,
      -1,    52,    42,    68,    43,    -1,    52,    37,    -1,    52,
      38,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   389,   389,   393,   394,   398,   401,   404,   410,   411,
     415,   418,   424,   425,   426,   427,   431,   432,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   458,   459,   460,   467,   470,   477,   478,   479,   483,
     484,   485,   489,   490,   491,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   506,   513,   514,   518,   519,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   537,   540,
     547,   554,   560,   561,   562,   567,   572,   577,   578,   582,
     583,   587,   594,   595
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SHURU", "SHESH", "JODI", "NAHOLE",
  "GHURAO", "BARBAR", "BONDHO", "CHALIYEJAO", "FEROT", "BANAW", "MAIN",
  "TYPE_INT", "TYPE_FLOAT", "TYPE_CHAR", "TYPE_VOID", "TYPE_ARRAY",
  "TRUE_VAL", "FALSE_VAL", "PRINT", "INPUT", "ADD", "SUB", "MUL", "DIV",
  "SQRT", "EQ", "NEQ", "LE", "GE", "LT", "GT", "AND", "OR", "NOT", "INC",
  "DEC", "ASSIGN", "SEMICOLON", "COMMA", "LPAREN", "RPAREN", "LBRACE",
  "RBRACE", "LBRACKET", "RBRACKET", "INT_LIT", "FLOAT_LIT", "CHAR_LIT",
  "STRING_LIT", "IDENTIFIER", "$accept", "program", "function_list",
  "function_def", "param_list_opt", "param_list", "type_spec", "stmt_list",
  "statement", "declaration", "assignment", "expression", "term", "factor",
  "primary", "arg_list_opt", "arg_list", "condition", "if_stmt",
  "while_stmt", "for_stmt", "for_update", "print_stmt", "input_stmt",
  "return_stmt", "break_stmt", "continue_stmt", "func_call",
  "inc_dec_stmt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    53,    54,    55,    55,    56,    56,    56,    57,    57,
      58,    58,    59,    59,    59,    59,    60,    60,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      61,    62,    62,    62,    63,    63,    64,    64,    64,    65,
      65,    65,    66,    66,    66,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    68,    68,    69,    69,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    71,    71,
      72,    73,    74,    74,    74,    75,    76,    77,    77,    78,
      79,    80,    81,    81
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     7,     8,     8,     0,     1,
       2,     4,     1,     1,     1,     1,     0,     2,     2,     2,
       2,     2,     1,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     4,     6,     3,     6,     1,     3,     3,     1,
       3,     3,     1,     4,     2,     1,     1,     1,     1,     1,
       1,     1,     3,     4,     4,     0,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     7,    11,
       7,    11,     1,     2,     2,     4,     4,     2,     1,     1,
       1,     4,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    12,    13,    14,    15,     0,     2,     3,     0,
       0,     1,     4,     0,     0,     8,     0,     8,     0,     9,
       0,     0,     0,     0,     0,    10,    16,     0,    16,     0,
       0,    16,     0,    11,     0,     5,     0,     0,     0,    79,
      80,    78,     0,     0,     0,     0,     0,    17,     0,     0,
      22,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     7,    30,     0,     0,     0,    49,    50,     0,     0,
       0,    45,    46,    47,    48,    51,    77,    36,    39,    42,
       0,     0,     0,    82,    83,     0,    55,     0,    31,    18,
      19,    20,    21,    25,    26,    27,    28,    29,     6,     0,
       0,     0,     0,     0,     0,     0,    44,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,    34,    57,     0,
      56,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
      37,    38,    40,    41,     0,    75,    76,    81,     0,     0,
      32,    67,    59,    60,    63,    64,    61,    62,    65,    66,
      16,    16,     0,    43,    53,    54,     0,    58,     0,     0,
       0,     0,    33,    35,    68,    70,     0,    72,     0,     0,
      73,    74,     0,    16,    16,     0,     0,    69,    71
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,     7,     8,    18,    19,    46,    30,    47,    48,
      49,   100,    77,    78,    79,   119,   120,   101,    50,    51,
      52,   178,    53,    54,    55,    56,    57,    58,    59
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -63
static const yytype_int16 yypact[] =
{
     285,   -50,   -63,   -63,   -63,   -63,    11,   285,   -63,     4,
     -34,   -63,   -63,    -8,    -3,   185,     9,   185,    15,    23,
      28,    78,    50,    92,   185,   -63,   -63,    99,   -63,    52,
       5,   -63,   108,   -63,    67,   -63,    79,    85,    94,   -63,
     -63,   215,   185,   100,   101,   208,    97,   -63,    80,   110,
     -63,   -63,   -63,   117,   123,   129,   136,   148,   149,   155,
     130,   -63,   -63,   233,   233,   118,   -63,   -63,   154,   215,
     215,   -63,   -63,   -63,   -63,   -18,    25,    41,   -63,   -63,
     151,   215,   153,   -63,   -63,   215,   215,   215,   167,   -63,
     -63,   -63,   -63,   -63,   -63,   -63,   -63,   -63,   -63,   233,
     263,    19,    36,   -10,   175,   215,   -63,    20,   215,   215,
     215,   215,   215,   215,   170,    51,   179,    25,    25,   198,
     207,     8,   215,    68,    71,   215,   215,   215,   215,   215,
     215,   233,   233,   246,   253,   233,    60,   -63,   216,    14,
      41,    41,   -63,   -63,   210,   -63,   -63,   -63,   215,   223,
      25,   -63,    25,    25,    25,    25,    25,    25,   -63,   227,
     -63,   -63,   119,   -63,   -63,   -63,   221,    25,   215,   157,
     176,   218,   -63,    25,   270,   -63,   234,   -63,   235,   274,
     -63,   -63,   276,   -63,   -63,   203,   222,   -63,   -63
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -63,   -63,   -63,   281,   272,   -63,    18,   -28,   -63,   -63,
     -61,   -40,   -33,   -62,   -63,   182,   -63,   -59,   -63,   -63,
     -63,   -63,   -63,   -63,   -63,   -63,   -63,   -63,   -63
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      32,    76,    10,    60,   104,   102,    34,   106,    15,    35,
      36,    11,    37,    38,    39,    40,    41,    13,     9,     2,
       3,     4,     5,    42,   108,     9,    43,    44,   109,    85,
     107,   110,   111,    20,    16,    20,    87,   110,   111,    17,
     124,   115,    29,   110,   111,   117,   118,   121,   110,   111,
     142,   143,    21,   131,   132,   149,    14,    45,    23,   123,
      80,   165,   133,   137,    24,   136,   112,   113,   118,   139,
     131,   132,   158,   159,   110,   111,   162,   140,   141,   134,
      25,    26,   150,   110,   111,   152,   153,   154,   155,   156,
     157,   110,   111,    27,   145,    28,   125,   126,   127,   128,
     129,   130,    31,   163,    33,   131,   132,    62,   167,    34,
     177,   137,    61,    36,   151,    37,    38,    39,    40,    41,
      89,    63,     2,     3,     4,     5,    42,    64,   173,    43,
      44,    34,   169,   170,    98,    36,    65,    37,    38,    39,
      40,    41,    81,    82,     2,     3,     4,     5,    42,    88,
      90,    43,    44,   131,   132,   185,   186,    91,    34,   171,
      45,   174,    36,    92,    37,    38,    39,    40,    41,    93,
     103,     2,     3,     4,     5,    42,    94,    34,    43,    44,
     175,    36,    45,    37,    38,    39,    40,    41,    95,    96,
       2,     3,     4,     5,    42,    97,   105,    43,    44,     2,
       3,     4,     5,   114,    34,   116,   122,   187,    36,    45,
      37,    38,    39,    40,    41,   135,   144,     2,     3,     4,
       5,    42,   146,    34,    43,    44,   188,    36,    45,    37,
      38,    39,    40,    41,    66,    67,     2,     3,     4,     5,
      42,   147,    68,    43,    44,    83,    84,    85,   148,   160,
      86,    69,    66,    67,    87,    45,   161,    70,   166,   164,
      68,   131,   168,    71,    72,    73,    74,    75,   172,    69,
     176,   180,   181,    85,    45,    99,   179,   183,   182,   184,
      87,    71,    72,    73,    74,    75,   110,   111,    12,    22,
     138,   125,   126,   127,   128,   129,   130,     1,     0,     2,
       3,     4,     5
};

static const yytype_int16 yycheck[] =
{
      28,    41,    52,    31,    65,    64,     1,    69,    42,     4,
       5,     0,     7,     8,     9,    10,    11,    13,     0,    14,
      15,    16,    17,    18,    42,     7,    21,    22,    46,    39,
      70,    23,    24,    15,    42,    17,    46,    23,    24,    42,
      99,    81,    24,    23,    24,    85,    86,    87,    23,    24,
     112,   113,    43,    34,    35,    47,    52,    52,    43,    99,
      42,    47,    43,    43,    41,   105,    25,    26,   108,   109,
      34,    35,   131,   132,    23,    24,   135,   110,   111,    43,
      52,     3,   122,    23,    24,   125,   126,   127,   128,   129,
     130,    23,    24,    43,    43,     3,    28,    29,    30,    31,
      32,    33,     3,    43,    52,    34,    35,    40,   148,     1,
     171,    43,     4,     5,    43,     7,     8,     9,    10,    11,
      40,    42,    14,    15,    16,    17,    18,    42,   168,    21,
      22,     1,   160,   161,     4,     5,    42,     7,     8,     9,
      10,    11,    42,    42,    14,    15,    16,    17,    18,    52,
      40,    21,    22,    34,    35,   183,   184,    40,     1,    40,
      52,     4,     5,    40,     7,     8,     9,    10,    11,    40,
      52,    14,    15,    16,    17,    18,    40,     1,    21,    22,
       4,     5,    52,     7,     8,     9,    10,    11,    40,    40,
      14,    15,    16,    17,    18,    40,    42,    21,    22,    14,
      15,    16,    17,    52,     1,    52,    39,     4,     5,    52,
       7,     8,     9,    10,    11,    40,    46,    14,    15,    16,
      17,    18,    43,     1,    21,    22,     4,     5,    52,     7,
       8,     9,    10,    11,    19,    20,    14,    15,    16,    17,
      18,    43,    27,    21,    22,    37,    38,    39,    41,     3,
      42,    36,    19,    20,    46,    52,     3,    42,    48,    43,
      27,    34,    39,    48,    49,    50,    51,    52,    47,    36,
      52,    37,    38,    39,    52,    42,     6,     3,    43,     3,
      46,    48,    49,    50,    51,    52,    23,    24,     7,    17,
     108,    28,    29,    30,    31,    32,    33,    12,    -1,    14,
      15,    16,    17
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    12,    14,    15,    16,    17,    54,    55,    56,    59,
      52,     0,    56,    13,    52,    42,    42,    42,    57,    58,
      59,    43,    57,    43,    41,    52,     3,    43,     3,    59,
      60,     3,    60,    52,     1,     4,     5,     7,     8,     9,
      10,    11,    18,    21,    22,    52,    59,    61,    62,    63,
      71,    72,    73,    75,    76,    77,    78,    79,    80,    81,
      60,     4,    40,    42,    42,    42,    19,    20,    27,    36,
      42,    48,    49,    50,    51,    52,    64,    65,    66,    67,
      59,    42,    42,    37,    38,    39,    42,    46,    52,    40,
      40,    40,    40,    40,    40,    40,    40,    40,     4,    42,
      64,    70,    70,    52,    63,    42,    66,    64,    42,    46,
      23,    24,    25,    26,    52,    64,    52,    64,    64,    68,
      69,    64,    39,    64,    70,    28,    29,    30,    31,    32,
      33,    34,    35,    43,    43,    40,    64,    43,    68,    64,
      65,    65,    66,    66,    46,    43,    43,    43,    41,    47,
      64,    43,    64,    64,    64,    64,    64,    64,    70,    70,
       3,     3,    70,    43,    43,    47,    48,    64,    39,    60,
      60,    40,    47,    64,     4,     4,    52,    63,    74,     6,
      37,    38,    43,     3,     3,    60,    60,     4,     4
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 389 "spp_parser.y"
    { ast_root = (yyvsp[(1) - (1)].node); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 393 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 394 "spp_parser.y"
    { (yyval.node) = append_list((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 398 "spp_parser.y"
    {
        (yyval.node) = make_func_def((yyvsp[(1) - (7)].type_val), "main", NULL, (yyvsp[(6) - (7)].node));
    ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 401 "spp_parser.y"
    {
        (yyval.node) = make_func_def((yyvsp[(1) - (8)].type_val), (yyvsp[(2) - (8)].sval), (yyvsp[(4) - (8)].node), (yyvsp[(7) - (8)].node));
    ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 404 "spp_parser.y"
    {
        (yyval.node) = make_func_def(T_VOID, (yyvsp[(2) - (8)].sval), (yyvsp[(4) - (8)].node), (yyvsp[(7) - (8)].node));
    ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 410 "spp_parser.y"
    { (yyval.node) = NULL; ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 411 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 415 "spp_parser.y"
    {
        (yyval.node) = make_param((yyvsp[(1) - (2)].type_val), (yyvsp[(2) - (2)].sval));
    ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 418 "spp_parser.y"
    {
        (yyval.node) = append_list((yyvsp[(1) - (4)].node), make_param((yyvsp[(3) - (4)].type_val), (yyvsp[(4) - (4)].sval)));
    ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 424 "spp_parser.y"
    { (yyval.type_val) = T_INT; ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 425 "spp_parser.y"
    { (yyval.type_val) = T_FLOAT; ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 426 "spp_parser.y"
    { (yyval.type_val) = T_CHAR; ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 427 "spp_parser.y"
    { (yyval.type_val) = T_VOID; ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 431 "spp_parser.y"
    { (yyval.node) = NULL; ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 432 "spp_parser.y"
    { (yyval.node) = append_list((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 436 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 437 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 438 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 439 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 440 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 441 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 442 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 443 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 444 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 445 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 446 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 447 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 448 "spp_parser.y"
    {
        fprintf(stderr, "Syntax Error: Invalid statement at line %d (recovered)\n", line);
        error_count++;
        yyerrok;
        (yyval.node) = NULL;
    ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 458 "spp_parser.y"
    { (yyval.node) = make_decl((yyvsp[(1) - (2)].type_val), (yyvsp[(2) - (2)].sval), NULL); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 459 "spp_parser.y"
    { (yyval.node) = make_decl((yyvsp[(1) - (4)].type_val), (yyvsp[(2) - (4)].sval), (yyvsp[(4) - (4)].node)); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 460 "spp_parser.y"
    {
        (yyval.node) = make_arr_decl((yyvsp[(2) - (6)].type_val), (yyvsp[(3) - (6)].sval), (yyvsp[(5) - (6)].ival));
    ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 467 "spp_parser.y"
    {
        (yyval.node) = make_assign((yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 470 "spp_parser.y"
    {
        (yyval.node) = make_arr_assign((yyvsp[(1) - (6)].sval), (yyvsp[(3) - (6)].node), (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 477 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 478 "spp_parser.y"
    { (yyval.node) = make_binop(ADD, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 479 "spp_parser.y"
    { (yyval.node) = make_binop(SUB, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 483 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 484 "spp_parser.y"
    { (yyval.node) = make_binop(MUL, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 485 "spp_parser.y"
    { (yyval.node) = make_binop(DIV, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 489 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 490 "spp_parser.y"
    { (yyval.node) = make_sqrt_op((yyvsp[(3) - (4)].node)); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 491 "spp_parser.y"
    { (yyval.node) = make_unop(NOT, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 495 "spp_parser.y"
    { (yyval.node) = make_int_lit((yyvsp[(1) - (1)].ival)); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 496 "spp_parser.y"
    { (yyval.node) = make_float_lit((yyvsp[(1) - (1)].fval)); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 497 "spp_parser.y"
    { (yyval.node) = make_char_lit((yyvsp[(1) - (1)].cval)); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 498 "spp_parser.y"
    { (yyval.node) = make_string_lit((yyvsp[(1) - (1)].sval)); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 499 "spp_parser.y"
    { (yyval.node) = make_bool_lit(1); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 500 "spp_parser.y"
    { (yyval.node) = make_bool_lit(0); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 501 "spp_parser.y"
    { (yyval.node) = make_var((yyvsp[(1) - (1)].sval)); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 502 "spp_parser.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 503 "spp_parser.y"
    {
        (yyval.node) = make_func_call((yyvsp[(1) - (4)].sval), (yyvsp[(3) - (4)].node));
    ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 506 "spp_parser.y"
    {
        (yyval.node) = make_arr_access((yyvsp[(1) - (4)].sval), (yyvsp[(3) - (4)].node));
    ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 513 "spp_parser.y"
    { (yyval.node) = NULL; ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 514 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 518 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); (yyval.node)->next = NULL; ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 519 "spp_parser.y"
    { (yyval.node) = append_list((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 524 "spp_parser.y"
    { (yyval.node) = make_binop(EQ, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 525 "spp_parser.y"
    { (yyval.node) = make_binop(NEQ, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 526 "spp_parser.y"
    { (yyval.node) = make_binop(LT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 527 "spp_parser.y"
    { (yyval.node) = make_binop(GT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 528 "spp_parser.y"
    { (yyval.node) = make_binop(LE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 529 "spp_parser.y"
    { (yyval.node) = make_binop(GE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 530 "spp_parser.y"
    { (yyval.node) = make_binop(AND, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 531 "spp_parser.y"
    { (yyval.node) = make_binop(OR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 532 "spp_parser.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 537 "spp_parser.y"
    {
        (yyval.node) = make_if((yyvsp[(3) - (7)].node), (yyvsp[(6) - (7)].node), NULL);
    ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 540 "spp_parser.y"
    {
        (yyval.node) = make_if((yyvsp[(3) - (11)].node), (yyvsp[(6) - (11)].node), (yyvsp[(10) - (11)].node));
    ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 547 "spp_parser.y"
    {
        (yyval.node) = make_while((yyvsp[(3) - (7)].node), (yyvsp[(6) - (7)].node));
    ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 554 "spp_parser.y"
    {
        (yyval.node) = make_for((yyvsp[(3) - (11)].node), (yyvsp[(5) - (11)].node), (yyvsp[(7) - (11)].node), (yyvsp[(10) - (11)].node));
    ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 560 "spp_parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 561 "spp_parser.y"
    { (yyval.node) = make_inc((yyvsp[(1) - (2)].sval)); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 562 "spp_parser.y"
    { (yyval.node) = make_dec((yyvsp[(1) - (2)].sval)); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 567 "spp_parser.y"
    { (yyval.node) = make_print((yyvsp[(3) - (4)].node)); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 572 "spp_parser.y"
    { (yyval.node) = make_input((yyvsp[(3) - (4)].sval)); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 577 "spp_parser.y"
    { (yyval.node) = make_return((yyvsp[(2) - (2)].node)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 578 "spp_parser.y"
    { (yyval.node) = make_return(NULL); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 582 "spp_parser.y"
    { (yyval.node) = make_break_node(); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 583 "spp_parser.y"
    { (yyval.node) = make_continue_node(); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 587 "spp_parser.y"
    {
        (yyval.node) = make_func_call((yyvsp[(1) - (4)].sval), (yyvsp[(3) - (4)].node));
    ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 594 "spp_parser.y"
    { (yyval.node) = make_inc((yyvsp[(1) - (2)].sval)); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 595 "spp_parser.y"
    { (yyval.node) = make_dec((yyvsp[(1) - (2)].sval)); ;}
    break;



/* Line 1455 of yacc.c  */
#line 2502 "spp_parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 598 "spp_parser.y"


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
            return make_res(0, T_INT);
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
            return make_res(0, T_INT);
        }
        if (!sym_table[idx].is_array) {
            fprintf(stderr, "Runtime Error at line %d: '%s' is not an array\n", node->lineno, node->name);
            error_count++;
            return make_res(0, T_INT);
        }
        ExecResult ir = eval_expr(node->left);
        int ai = (int)ir.value;
        if (ai < 0 || ai >= sym_table[idx].arr_size) {
            fprintf(stderr, "Runtime Error at line %d: Index %d out of bounds for '%s[%d]'\n",
                    node->lineno, ai, node->name, sym_table[idx].arr_size);
            error_count++;
            return make_res(0, sym_table[idx].type);
        }
        return make_res(sym_table[idx].arr_vals[ai], sym_table[idx].type);
    }

    case N_BINOP: {
        ExecResult lv = eval_expr(node->left);
        ExecResult rv = eval_expr(node->right);
        VarType rt = (lv.type == T_FLOAT || rv.type == T_FLOAT) ? T_FLOAT : T_INT;
        switch (node->op) {
            case ADD: return make_res(lv.value + rv.value, rt);
            case SUB: return make_res(lv.value - rv.value, rt);
            case MUL: return make_res(lv.value * rv.value, rt);
            case DIV:
                if (rv.value == 0) {
                    fprintf(stderr, "Runtime Error at line %d: Division by zero\n", node->lineno);
                    error_count++;
                    return make_res(0, rt);
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
        if (node->op == NOT) return make_res(!((int)ov.value), T_INT);
        break;
    }

    case N_SQRT_OP: {
        ExecResult av = eval_expr(node->left);
        if (av.value < 0) {
            fprintf(stderr, "Runtime Error at line %d: sqrt of negative number\n", node->lineno);
            error_count++;
            return make_res(0, T_FLOAT);
        }
        return make_res(sqrt(av.value), T_FLOAT);
    }

    case N_FUNC_CALL: {
        /* Look up function */
        int idx = sym_find(node->name);
        if (idx == -1 || !sym_table[idx].is_function) {
            fprintf(stderr, "Runtime Error at line %d: Undefined function '%s'\n", node->lineno, node->name);
            error_count++;
            return make_res(0, T_INT);
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
            return make_res(0, T_INT);
        }

        /* Evaluate arguments BEFORE pushing scope */
        double arg_vals[64];
        VarType arg_types[64];
        a = node->args;
        for (int i = 0; i < argc; i++) {
            ExecResult av = eval_expr(a);
            arg_vals[i] = av.value;
            arg_types[i] = av.type;
            a = a->next;
        }

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
                        printf("  [Type Warning] Narrowing arg dosomik -> purno in call to %s()\n", node->name);
                        warning_count++;
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
                if (vt != val.type && !val.is_string) {
                    if (vt == T_FLOAT && val.type == T_INT) {
                        printf("  [Type Info] Implicit: purno -> dosomik at line %d\n", node->lineno);
                    } else if (vt == T_INT && val.type == T_FLOAT) {
                        printf("  [Type Warning] Narrowing: dosomik -> purno at line %d\n", node->lineno);
                        warning_count++;
                        val.value = (int)val.value;
                    }
                }
                sym_table[idx].value = val.value;
                sym_table[idx].is_init = 1;
                printf("  [Declare] %s = %.4g\n", node->name, val.value);
            } else {
                printf("  [Declare] %s : %s (uninitialized)\n", node->name, type_name(vt));
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
            printf("  [Declare Array] %s[%d] : %s\n", node->name, node->arr_size, type_name(vt));
        }
        return make_res(0, T_VOID);
    }

    case N_ASSIGN: {
        int idx = sym_find(node->name);
        if (idx == -1) {
            fprintf(stderr, "Semantic Error at line %d: Undeclared variable '%s'\n", node->lineno, node->name);
            error_count++;
        } else {
            ExecResult val = eval_expr(node->left);
            if (sym_table[idx].type != val.type && !val.is_string) {
                if (sym_table[idx].type == T_FLOAT && val.type == T_INT) {
                    /* implicit OK */
                } else if (sym_table[idx].type == T_INT && val.type == T_FLOAT) {
                    printf("  [Type Warning] Narrowing: dosomik -> purno at line %d\n", node->lineno);
                    warning_count++;
                    val.value = (int)val.value;
                }
            }
            sym_table[idx].value = val.value;
            sym_table[idx].is_init = 1;
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
            int ai = (int)ir.value;
            if (ai < 0 || ai >= sym_table[idx].arr_size) {
                fprintf(stderr, "Runtime Error at line %d: Index %d out of bounds for '%s[%d]'\n",
                        node->lineno, ai, node->name, sym_table[idx].arr_size);
                error_count++;
            } else {
                ExecResult val = eval_expr(node->right);
                sym_table[idx].arr_vals[ai] = val.value;
                printf("  [Array Assign] %s[%d] = %.4g\n", node->name, ai, val.value);
            }
        }
        return make_res(0, T_VOID);
    }

    /* -------- IF / ELSE: Execute ONLY the correct branch -------- */
    case N_IF: {
        ExecResult cond = eval_expr(node->left);
        if ((int)cond.value) {
            printf("  [If] TRUE at line %d\n", node->lineno);
            return exec_list(node->body);
        } else if (node->right) {
            printf("  [Else] at line %d\n", node->lineno);
            return exec_list(node->right);
        } else {
            printf("  [If] FALSE, skipped at line %d\n", node->lineno);
        }
        return make_res(0, T_VOID);
    }

    /* -------- WHILE: Real iterative loop -------- */
    case N_WHILE: {
        printf("  [While] entering loop at line %d\n", node->lineno);
        int iters = 0;
        while (1) {
            ExecResult cond = eval_expr(node->left);
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
        printf("  [While] exited after %d iterations\n", iters);
        return make_res(0, T_VOID);
    }

    /* -------- FOR: Real iterative loop -------- */
    case N_FOR: {
        printf("  [For] entering loop at line %d\n", node->lineno);
        exec_node(node->right);  /* init */
        int iters = 0;
        while (1) {
            ExecResult cond = eval_expr(node->left);
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
        printf("  [For] exited after %d iterations\n", iters);
        return make_res(0, T_VOID);
    }

    /* -------- PRINT -------- */
    case N_PRINT: {
        int errs_before = error_count;
        ExecResult val = eval_expr(node->left);
        if (error_count > errs_before) {
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
                printf("  [Input] %s = %.4g\n", node->name, val);
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
        printf("  [Return] %.4g\n", r.value);
        return r;
    }

    /* -------- BREAK / CONTINUE -------- */
    case N_BREAK: {
        ExecResult r = make_res(0, T_VOID);
        r.is_break = 1;
        printf("  [Break]\n");
        return r;
    }
    case N_CONTINUE: {
        ExecResult r = make_res(0, T_VOID);
        r.is_continue = 1;
        printf("  [Continue]\n");
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
            printf("\n--- Executing main() ---\n");
            scope_push();
            ExecResult r = exec_list(f->body);
            printf("--- main() finished ---\n");
            if (r.is_return) {
                printf("  Program returned: %d\n", (int)r.value);
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
    printf("       S++ Compiler (SPP) v2.0\n");
    printf("  Bangla-Keyword Programming Language\n");
    printf("  Architecture: AST + Interpreter + TAC\n");
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

    /* Phase 2: Execute the program via AST interpretation */
    printf("\n[Phase 2] Executing program...\n");
    errors_before_exec = error_count;  /* snapshot so exec_list detects NEW errors */
    exec_program(ast_root);

    if (error_count == 0) {
        /* Phase 3: Generate TAC from AST */
        printf("\n[Phase 3] Generating intermediate code...\n");
        gen_program_tac(ast_root);
        print_tac(stdout, "INTERMEDIATE CODE (Three-Address Code)");
        print_tac(output_file, "INTERMEDIATE CODE (Three-Address Code)");

        /* Phase 4: Optimize */
        printf("\n[Phase 4] Optimizing...\n");
        optimize_tac();
        print_tac(stdout, "OPTIMIZED CODE (Constant Folding + Dead Code Elim)");
        print_tac(output_file, "OPTIMIZED CODE (Constant Folding + Dead Code Elim)");

        /* Phase 5: C Code Generation */
        printf("\n[Phase 5] Generating equivalent C code...\n");
        gen_c_program(stdout, ast_root);
        gen_c_program(output_file, ast_root);
    } else {
        printf("\n[Phase 3-5] Skipped -- %d error(s) detected, no code generation.\n", error_count);
        fprintf(output_file, "\n[Phase 3-5] Skipped -- %d error(s) detected, no code generation.\n", error_count);
    }

    /* Flush stderr so errors appear before summary */
    fflush(stderr);

    /* Summary */
    printf("\n============================================\n");
    printf("           COMPILATION SUMMARY\n");
    printf("============================================\n");
    printf("  Errors   : %d\n", error_count);
    printf("  Warnings : %d\n", warning_count);
    printf("  Symbols  : %d\n", final_sym_count);
    printf("  TAC Lines: %d\n", tac_count);
    printf("  AST Nodes: %d\n", node_count);
    printf("  Status   : %s\n", (error_count == 0) ? "SUCCESS" : "FAILED");
    printf("============================================\n");

    fprintf(output_file, "\n============================================\n");
    fprintf(output_file, "           COMPILATION SUMMARY\n");
    fprintf(output_file, "============================================\n");
    fprintf(output_file, "  Errors   : %d\n", error_count);
    fprintf(output_file, "  Warnings : %d\n", warning_count);
    fprintf(output_file, "  Symbols  : %d\n", final_sym_count);
    fprintf(output_file, "  TAC Lines: %d\n", tac_count);
    fprintf(output_file, "  AST Nodes: %d\n", node_count);
    fprintf(output_file, "  Status   : %s\n", (error_count == 0) ? "SUCCESS" : "FAILED");
    fprintf(output_file, "============================================\n");

    fclose(in);
    fclose(output_file);
    return (error_count == 0) ? 0 : 1;
}

