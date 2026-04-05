/* main.c — S++ Compiler entry point: orchestrates all 6 phases */

#include "common.h"
#include "ast.h"
#include "symtab.h"
#include "semantic.h"
#include "interpreter.h"
#include "tac.h"
#include "optimizer.h"
#include "codegen.h"

int yyparse(void);

/* ---- Globals ---- */
ASTNode *ast_root          = NULL;
FILE    *output_file       = NULL;
int      final_sym_count   = 0;
int      errors_before_exec = 0;

int main(int argc, char *argv[]) {
    printf("============================================\n");
    printf("       S++ Compiler (SPP) v3.0\n");
    printf("  Bangla-Keyword Programming Language\n");
    printf("  Architecture: AST + Semantic + Interpreter + TAC\n");
    printf("============================================\n");

    char *in_file  = "input.txt";
    char *out_file = "output.txt";
    if (argc >= 2) in_file  = argv[1];
    if (argc >= 3) out_file = argv[2];

    FILE *in = fopen(in_file, "r");
    if (!in) { fprintf(stderr, "Error: Cannot open '%s'\n", in_file); return 1; }

    output_file = fopen(out_file, "w");
    if (!output_file) { fprintf(stderr, "Error: Cannot create '%s'\n", out_file); fclose(in); return 1; }

    fprintf(output_file, "============================================\n");
    fprintf(output_file, "       S++ Program Output\n");
    fprintf(output_file, "============================================\n\n");

    yyin = in;

    /* Phase 1: Parsing */
    printf("\n[Phase 1] Parsing '%s'...\n", in_file);
    int parse_result = yyparse();
    if (parse_result != 0 || !ast_root) {
        fprintf(stderr, "Parsing failed.\n");
        fclose(in); fclose(output_file);
        return 1;
    }
    printf("[Phase 1] Parsing complete. AST built successfully.\n");

    /* Phase 2: Semantic Analysis */
    printf("\n[Phase 2] Semantic analysis...\n");
    semantic_check_program(ast_root);
    if (error_count > 0)
        printf("[Phase 2] Found %d semantic error(s).\n", error_count);
    else
        printf("[Phase 2] Semantic analysis passed.\n");
    sym_reset();

    int func_count = 0;
    { ASTNode *fc = ast_root; while (fc) { if (fc->ntype == N_FUNC_DEF) func_count++; fc = fc->next; } }
    int exec_halted = 0;

    /* Phase 3: Execution */
    if (error_count == 0) {
        printf("\n[Phase 3] Executing program...\n");
        errors_before_exec = error_count;
        exec_program(ast_root);
        if (error_count > errors_before_exec) exec_halted = 1;
    }

    if (error_count == 0) {
        /* Phase 4: TAC Generation */
        printf("\n[Phase 4] Generating intermediate code...\n");
        gen_program_tac(ast_root);
        print_tac(stdout, "INTERMEDIATE CODE (Three-Address Code)");
        print_tac(output_file, "INTERMEDIATE CODE (Three-Address Code)");

        /* Phase 5: Optimization */
        printf("\n[Phase 5] Optimizing...\n");
        optimize_tac();
        print_tac(stdout, "OPTIMIZED CODE (Constant Folding + Dead Code Elim)");
        print_tac(output_file, "OPTIMIZED CODE (Constant Folding + Dead Code Elim)");

        /* Phase 6: C Code Generation */
        printf("\n[Phase 6] Generating equivalent C code...\n");
        gen_c_program(stdout, ast_root);
        gen_c_program(output_file, ast_root);
    } else {
        printf("\n[Phase 4-6] Skipped -- %d error(s) detected.\n", error_count);
        fprintf(output_file, "\n[Phase 4-6] Skipped -- %d error(s) detected.\n", error_count);
    }

    fflush(stderr);

    /* Compilation Summary */
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
