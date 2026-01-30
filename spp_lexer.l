%{
#include <stdio.h>
#include <stdlib.h>

int line = 1;
int tokens = 0;
%}

%option noyywrap

/* States for better lexical analysis */
%x COMMENT

DIGIT    [0-9]
LETTER   [a-zA-Z_]
ID       {LETTER}({LETTER}|{DIGIT})*
INT      {DIGIT}+
FLOAT    {DIGIT}+\.{DIGIT}+

%%

    /* Whitespace and newlines */
[ \t]+              { /* skip */ }
\n                  { line++; }

    /* Comments using states */
"//".*              { /* single line comment */ }
"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { line++; }
<COMMENT>.          { /* skip */ }

    /* Keywords */
"shuru"             { printf("BLOCK_START\n"); tokens++; }
"shesh"             { printf("BLOCK_END\n"); tokens++; }
"jodi"              { printf("IF\n"); tokens++; }
"nahole"            { printf("ELSE\n"); tokens++; }
"ghurao"            { printf("WHILE\n"); tokens++; }
"barbar"            { printf("FOR\n"); tokens++; }
"ferot"             { printf("RETURN\n"); tokens++; }
"bondho"            { printf("BREAK\n"); tokens++; }
"chaliyejao"        { printf("CONTINUE\n"); tokens++; }
"dekhao"            { printf("PRINT\n"); tokens++; }
"neo"               { printf("INPUT\n"); tokens++; }
"banaw"             { printf("FUNCTION\n"); tokens++; }
"array"             { printf("ARRAY\n"); tokens++; }
"main"              { printf("MAIN\n"); tokens++; }

    /* Data Types */
"purno"             { printf("INT\n"); tokens++; }
"dosomik"           { printf("FLOAT\n"); tokens++; }
"chinho"            { printf("CHAR\n"); tokens++; }
"kisu_na"           { printf("VOID\n"); tokens++; }
"shotto"            { printf("TRUE\n"); tokens++; }
"mittha"            { printf("FALSE\n"); tokens++; }

    /* Operators */
"jog"               { printf("ADD\n"); tokens++; }
"biyog"             { printf("SUB\n"); tokens++; }
"gun"               { printf("MUL\n"); tokens++; }
"vag"               { printf("DIV\n"); tokens++; }
"root"              { printf("SQRT\n"); tokens++; }
"=="                { printf("EQUAL\n"); tokens++; }
"!="                { printf("NOT_EQUAL\n"); tokens++; }
"<="                { printf("LESS_EQ\n"); tokens++; }
">="                { printf("GREATER_EQ\n"); tokens++; }
"<"                 { printf("LESS\n"); tokens++; }
">"                 { printf("GREATER\n"); tokens++; }
"&&"                { printf("AND\n"); tokens++; }
"||"                { printf("OR\n"); tokens++; }
"!"                 { printf("NOT\n"); tokens++; }
"="                 { printf("ASSIGN\n"); tokens++; }
"++"                { printf("INCREMENT\n"); tokens++; }
"--"                { printf("DECREMENT\n"); tokens++; }

    /* Delimiters */
";"                 { printf("SEMICOLON\n"); tokens++; }
","                 { printf("COMMA\n"); tokens++; }
"("                 { printf("LPAREN\n"); tokens++; }
")"                 { printf("RPAREN\n"); tokens++; }
"{"                 { printf("LBRACE\n"); tokens++; }
"}"                 { printf("RBRACE\n"); tokens++; }
"["                 { printf("LBRACKET\n"); tokens++; }
"]"                 { printf("RBRACKET\n"); tokens++; }

    /* Literals and Identifiers */
{FLOAT}             { printf("FLOAT_LIT(%s)\n", yytext); tokens++; }
{INT}               { printf("INT_LIT(%s)\n", yytext); tokens++; }
{ID}                { printf("ID(%s)\n", yytext); tokens++; }

    /* Errors */
.                   { fprintf(stderr, "Error: '%s' at line %d\n", yytext, line); }

%%

int main(int argc, char *argv[]) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror(argv[1]);
            return 1;
        }
    }
    
    yylex();
    printf("\n--- Statistics ---\n");
    printf("Lines: %d\n", line);
    printf("Tokens: %d\n", tokens);
    
    if (yyin != stdin) fclose(yyin);
    return 0;
}
