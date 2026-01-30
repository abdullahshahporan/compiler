%{
/*
 * Lexical Analyzer for S++ Language
 * Converts S++ tokens to their C equivalents
 * Author: Compiler Lab Project
 * Date: January 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Token output helper function */
static void tok(const char* token_type) { 
    fprintf(yyout, "%-25s : %s\n", token_type, yytext); 
}

/* Statistics and line tracking */
static int line_num = 1;
static int token_count = 0;
static int keyword_count = 0;
static int operator_count = 0;
static int identifier_count = 0;
static int literal_count = 0;
%}

/* Regular Expression Definitions */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
CHAR_LIT    \'[^\']\'
STRING_LIT  \"[^\"]*\"
WHITESPACE  [ \t]+
NEWLINE     \n

%%

{WHITESPACE}        { /* Ignore whitespace */ }
{NEWLINE}           { line_num++; }

    /* Block Delimiters */
"shuru"             { tok("BLOCK_START"); keyword_count++; token_count++; }
"shesh"             { tok("BLOCK_END"); keyword_count++; token_count++; }

    /* Control Flow Keywords */
"jodi"              { tok("KW_IF"); keyword_count++; token_count++; }
"nahole"            { tok("KW_ELSE"); keyword_count++; token_count++; }
"ghurao"            { tok("KW_WHILE"); keyword_count++; token_count++; }
"barbar"            { tok("KW_FOR"); keyword_count++; token_count++; }
"ferot"             { tok("KW_RETURN"); keyword_count++; token_count++; }
"bondho"            { tok("KW_BREAK"); keyword_count++; token_count++; }
"chaliyejao"        { tok("KW_CONTINUE"); keyword_count++; token_count++; }

    /* I/O Keywords */
"dekhao"            { tok("KW_PRINT"); keyword_count++; token_count++; }
"neo"               { tok("KW_INPUT"); keyword_count++; token_count++; }

    /* Data Types */
"purno"             { tok("TYPE_INT"); keyword_count++; token_count++; }
"dosomik"           { tok("TYPE_FLOAT"); keyword_count++; token_count++; }
"chinho"            { tok("TYPE_CHAR"); keyword_count++; token_count++; }
"kisu_na"           { tok("TYPE_VOID"); keyword_count++; token_count++; }

    /* Boolean Values */
"shotto"            { tok("BOOL_TRUE"); keyword_count++; token_count++; }
"mittha"            { tok("BOOL_FALSE"); keyword_count++; token_count++; }

    /* Function and Array Keywords */
"banaw"             { tok("KW_FUNCTION"); keyword_count++; token_count++; }
"array"             { tok("KW_ARRAY"); keyword_count++; token_count++; }

    /* Main Function */
"main"              { tok("KW_MAIN"); keyword_count++; token_count++; }

    /* Arithmetic Operators */
"jog"               { tok("OP_ADD"); operator_count++; token_count++; }
"biyog"             { tok("OP_SUB"); operator_count++; token_count++; }
"gun"               { tok("OP_MUL"); operator_count++; token_count++; }
"vag"               { tok("OP_DIV"); operator_count++; token_count++; }
"root"              { tok("OP_SQRT"); operator_count++; token_count++; }

    /* Relational Operators */
"=="                { tok("OP_EQUAL"); operator_count++; token_count++; }
"!="                { tok("OP_NOT_EQUAL"); operator_count++; token_count++; }
"<"                 { tok("OP_LESS"); operator_count++; token_count++; }
">"                 { tok("OP_GREATER"); operator_count++; token_count++; }
"<="                { tok("OP_LESS_EQ"); operator_count++; token_count++; }
">="                { tok("OP_GREATER_EQ"); operator_count++; token_count++; }

    /* Logical Operators */
"&&"                { tok("OP_AND"); operator_count++; token_count++; }
"||"                { tok("OP_OR"); operator_count++; token_count++; }
"!"                 { tok("OP_NOT"); operator_count++; token_count++; }

    /* Assignment Operator */
"="                 { tok("OP_ASSIGN"); operator_count++; token_count++; }

    /* Increment/Decrement */
"++"                { tok("OP_INCREMENT"); operator_count++; token_count++; }
"--"                { tok("OP_DECREMENT"); operator_count++; token_count++; }

    /* Delimiters */
";"                 { tok("DELIMITER_SEMICOLON"); token_count++; }
","                 { tok("DELIMITER_COMMA"); token_count++; }
"("                 { tok("DELIMITER_LPAREN"); token_count++; }
")"                 { tok("DELIMITER_RPAREN"); token_count++; }
"{"                 { tok("DELIMITER_LBRACE"); token_count++; }
"}"                 { tok("DELIMITER_RBRACE"); token_count++; }
"["                 { tok("DELIMITER_LBRACKET"); token_count++; }
"]"                 { tok("DELIMITER_RBRACKET"); token_count++; }

    /* Literals */
{FLOAT}             { tok("LITERAL_FLOAT"); literal_count++; token_count++; }
{INTEGER}           { tok("LITERAL_INTEGER"); literal_count++; token_count++; }
{CHAR_LIT}          { tok("LITERAL_CHAR"); literal_count++; token_count++; }
{STRING_LIT}        { tok("LITERAL_STRING"); literal_count++; token_count++; }

    /* Identifiers */
{ID}                { tok("IDENTIFIER"); identifier_count++; token_count++; }

    /* Comments - Single Line */
"//".*              { /* Ignore single line comments */ }

    /* Comments - Multi Line */
"/*"([^*]|\*+[^*/])*\*+"/"  { 
    /* Count newlines in multi-line comments */
    char *p = yytext;
    while (*p) {
        if (*p == '\n') line_num++;
        p++;
    }
}

    /* Unrecognized Characters */
.                   { 
    fprintf(yyout, "LEXICAL_ERROR         : Unrecognized character '%s' at line %d\n", 
            yytext, line_num); 
    token_count++;
}

%%

/* Required function for flex */
int yywrap(void) { 
    return 1;
}

/* Main function */
int main(int argc, char *argv[]) {
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    
    /* Determine input source */
    if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (!input_file) {
            fprintf(stderr, "Error: Cannot open input file '%s'\n", argv[1]);
            return 1;
        }
        yyin = input_file;
    } else {
        input_file = fopen("input.spp", "r");
        if (!input_file) {
            fprintf(stderr, "Error: No input file specified and 'input.spp' not found\n");
            fprintf(stderr, "Usage: %s [input_file.spp] [output_file.txt]\n", argv[0]);
            return 1;
        }
        yyin = input_file;
    }
    
    /* Determine output destination */
    if (argc > 2) {
        output_file = fopen(argv[2], "w");
        if (!output_file) {
            fprintf(stderr, "Error: Cannot create output file '%s'\n", argv[2]);
            if (input_file) fclose(input_file);
            return 1;
        }
        yyout = output_file;
    } else {
        output_file = fopen("output.txt", "w");
        if (!output_file) {
            fprintf(stderr, "Error: Cannot create 'output.txt'\n");
            if (input_file) fclose(input_file);
            return 1;
        }
        yyout = output_file;
    }
    
    /* Print header */
    fprintf(yyout, "===============================================\n");
    fprintf(yyout, "    S++ LEXICAL ANALYZER - TOKEN OUTPUT\n");
    fprintf(yyout, "===============================================\n\n");
    fprintf(yyout, "%-25s : %-30s\n\n", "TOKEN TYPE", "LEXEME");
    
    /* Run lexical analysis */
    yylex();
    
    /* Print statistics */
    fprintf(yyout, "\n===============================================\n");
    fprintf(yyout, "              ANALYSIS STATISTICS\n");
    fprintf(yyout, "===============================================\n");
    fprintf(yyout, "Total Lines:               %d\n", line_num);
    fprintf(yyout, "Total Tokens:              %d\n", token_count);
    fprintf(yyout, "Keywords:                  %d\n", keyword_count);
    fprintf(yyout, "Operators:                 %d\n", operator_count);
    fprintf(yyout, "Identifiers:               %d\n", identifier_count);
    fprintf(yyout, "Literals:                  %d\n", literal_count);
    fprintf(yyout, "===============================================\n");
    
    /* Print to console as well */
    printf("\n=== Lexical Analysis Complete ===\n");
    printf("Input File:  %s\n", argc > 1 ? argv[1] : "input.spp");
    printf("Output File: %s\n", argc > 2 ? argv[2] : "output.txt");
    printf("Total Tokens: %d\n", token_count);
    printf("Total Lines: %d\n", line_num);
    
    /* Cleanup */
    if (input_file) fclose(input_file);
    if (output_file) fclose(output_file);
    
    return 0;
}
