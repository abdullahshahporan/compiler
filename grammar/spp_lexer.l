%{
/* spp_lexer.l — S++ Flex Lexer: tokenizes Bangla-keyword source */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "spp_parser.tab.h"

extern YYSTYPE yylval;
int line = 1;

%}

%option noyywrap

/* States for comments */
%x COMMENT

/* Pattern shortcuts */
DIGIT      [0-9]
LETTER     [a-zA-Z_]
ID         {LETTER}({LETTER}|{DIGIT}|_)*
INT        {DIGIT}+
FLOAT      {DIGIT}+\.{DIGIT}+
SPACE      [ \t\r]+

%%

{SPACE}              ;
\n                   { line++; }

"//".*               ;
"/*"                 { BEGIN(COMMENT); }
<COMMENT>"*/"        { BEGIN(INITIAL); }
<COMMENT>\n          { line++; }
<COMMENT>.           ;

 /* ---------- Keywords ---------- */
"shuru"              { return SHURU; }
"shesh"              { return SHESH; }
"jodi"               { return JODI; }
"nahole"             { return NAHOLE; }
"ghurao"             { return GHURAO; }
"barbar"             { return BARBAR; }
"bondho"             { return BONDHO; }
"chaliyejao"         { return CHALIYEJAO; }
"ferot"              { return FEROT; }
"banaw"              { return BANAW; }
"main"               { return MAIN; }

 /* ---------- Data Types ---------- */
"purno"              { return TYPE_INT; }
"dosomik"            { return TYPE_FLOAT; }
"chinho"             { return TYPE_CHAR; }
"kisu_na"            { return TYPE_VOID; }
"array"              { return TYPE_ARRAY; }

 /* ---------- Boolean Literals ---------- */
"shotto"             { return TRUE_VAL; }
"mittha"             { return FALSE_VAL; }

 /* ---------- I/O ---------- */
"dekhao"             { return PRINT; }
"neo"                { return INPUT; }

 /* ---------- Arithmetic Operators (keyword form) ---------- */
"jog"                { return ADD; }
"biyog"              { return SUB; }
"gun"                { return MUL; }
"vag"                { return DIV; }
"root"               { return SQRT; }

 /* ---------- Comparison Operators ---------- */
"=="                 { return EQ; }
"!="                 { return NEQ; }
"<="                 { return LE; }
">="                 { return GE; }
"<"                  { return LT; }
">"                  { return GT; }

 /* ---------- Logical Operators ---------- */
"&&"                 { return AND; }
"||"                 { return OR; }
"!"                  { return NOT; }

 /* ---------- Increment / Decrement ---------- */
"++"                 { return INC; }
"--"                 { return DEC; }

 /* ---------- Assignment ---------- */
"="                  { return ASSIGN; }

 /* ---------- Literals ---------- */
{FLOAT}              {
                        yylval.fval = atof(yytext);
                        return FLOAT_LIT;
                     }
{INT}                {
                        yylval.ival = atoi(yytext);
                        return INT_LIT;
                     }

\'([^'\\]|\\.)?\'    {
                        yylval.cval = yytext[1];
                        return CHAR_LIT;
                     }

\"([^"\\]|\\.)*\"    {
                        /* Remove quotes and copy string */
                        int len = strlen(yytext);
                        strncpy(yylval.sval, yytext + 1, len - 2);
                        yylval.sval[len - 2] = '\0';
                        return STRING_LIT;
                     }

 /* ---------- Identifiers ---------- */
{ID}                 {
                        strncpy(yylval.sval, yytext, 255);
                        yylval.sval[255] = '\0';
                        return IDENTIFIER;
                     }

 /* ---------- Delimiters ---------- */
";"                  { return SEMICOLON; }
","                  { return COMMA; }
"("                  { return LPAREN; }
")"                  { return RPAREN; }
"{"                  { return LBRACE; }
"}"                  { return RBRACE; }
"["                  { return LBRACKET; }
"]"                  { return RBRACKET; }

 /* ---------- Invalid Token Handling ---------- */
.                    {
                        fprintf(stderr, "Lexical Error: Unrecognized token '%s' at line %d\n", yytext, line);
                     }

%%
