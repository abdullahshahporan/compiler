%{
#include <stdio.h>
#include <string.h>

/* Helper function - prints token name and value */
void tok(char* type) { 
    fprintf(yyout, "%-20s : %s\n", type, yytext); 
}

/* Check if a word is a keyword */
int is_keyword(char* word) {
    char* keywords[] = {
        "shuru", "shesh", "jodi", "nahole", "ghurao", "barbar",
        "bondho", "chaliyejao", "ferot", "banaw", "main",
        "purno", "dosomik", "chinho", "kisu_na", "array",
        "shotto", "mittha", "dekhao", "neo",
        "jog", "biyog", "gun", "vag", "root",
        NULL
    };
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

/* Track line numbers */
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

"shuru"              { tok("SHURU"); }
"shesh"              { tok("SHESH"); }
"jodi"               { tok("JODI"); }
"nahole"             { tok("NAHOLE"); }
"ghurao"             { tok("GHURAO"); }
"barbar"             { tok("BARBAR"); }
"bondho"             { tok("BONDHO"); }
"chaliyejao"         { tok("CHALIYEJAO"); }
"ferot"              { tok("FEROT"); }
"banaw"              { tok("BANAW"); }
"main"               { tok("MAIN"); }

"purno"              { tok("TYPE_INT"); }
"dosomik"            { tok("TYPE_FLOAT"); }
"chinho"             { tok("TYPE_CHAR"); }
"kisu_na"            { tok("TYPE_VOID"); }
"array"              { tok("TYPE_ARRAY"); }

"shotto"             { tok("TRUE"); }
"mittha"             { tok("FALSE"); }

"dekhao"             { tok("PRINT"); }
"neo"                { tok("INPUT"); }

"jog"                { tok("ADD"); }
"biyog"              { tok("SUB"); }
"gun"                { tok("MUL"); }
"vag"                { tok("DIV"); }
"root"               { tok("SQRT"); }

"=="                 { tok("EQ"); }
"!="                 { tok("NEQ"); }
"<="                 { tok("LE"); }
">="                 { tok("GE"); }
"<"                  { tok("LT"); }
">"                  { tok("GT"); }

"&&"                 { tok("AND"); }
"||"                 { tok("OR"); }
"!"                  { tok("NOT"); }

"++"                 { tok("INC"); }
"--"                 { tok("DEC"); }
"="                  { tok("ASSIGN"); }

{FLOAT}              { tok("FLOAT"); }
{INT}                { tok("INT"); }

\'([^'\\]|\\.)?\'    { tok("CHAR"); }
\"([^"\\]|\\.)*\"    { tok("STRING"); }

{ID}                 {
                        if (is_keyword(yytext)) tok("KEYWORD");
                        else tok("ID");
                     }

";"                  { tok("SEMICOLON"); }
","                  { tok("COMMA"); }
"("                  { tok("LPAREN"); }
")"                  { tok("RPAREN"); }
"{"                  { tok("LBRACE"); }
"}"                  { tok("RBRACE"); }
"["                  { tok("LBRACKET"); }
"]"                  { tok("RBRACKET"); }

.                    { 
                        fprintf(stderr, "Error: Unknown '%s' at line %d\n", yytext, line); 
                     }

%%

int main(void) {
    FILE *in = fopen("input.txt", "r");
    if (!in) {
        printf("Error: Can't open input.txt\n");
        return 1;
    }
    
    FILE *out = fopen("output.txt", "w");
    if (!out) {
        printf("Error: Can't create output.txt\n");
        fclose(in);
        return 1;
    }
    
    fprintf(out, "TOKEN                : LEXEME\n");
    fprintf(out, "-----------------------------------\n");
    
    yyin = in;
    yyout = out;
    yylex();
    
    fprintf(out, "-----------------------------------\n");
    fprintf(out, "Total lines: %d\n", line);
    
    fclose(in);
    fclose(out);
    
    printf("Done! Check output.txt\n");
    return 0;
}
