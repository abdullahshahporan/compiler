
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
#line 331 "spp_parser.y"

    int ival;
    double fval;
    char cval;
    char sval[256];
    struct ASTNode *node;
    int type_val;



/* Line 1676 of yacc.c  */
#line 115 "spp_parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


