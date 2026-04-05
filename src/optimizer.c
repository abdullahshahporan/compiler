/* optimizer.c — TAC optimization: constant folding, propagation, dead code */

#include "optimizer.h"

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

        /* Pass 1: Constant Folding */
        int fold_changes = 1;
        while (fold_changes) {
            fold_changes = 0;
            for (int i = 0; i < tac_count; i++) {
                if (tac[i].kind != 0) continue;

                /* Fold sqrt(const) */
                if (strcmp(tac[i].arg1, "sqrt") == 0 && is_numeric(tac[i].op)) {
                    double v = strtod(tac[i].op, NULL);
                    if (v >= 0) {
                        double res = sqrt(v);
                        if (res == (int)res)
                            sprintf(tac[i].arg1, "%d", (int)res);
                        else
                            sprintf(tac[i].arg1, "%.4g", res);
                        tac[i].op[0] = '\0';
                        fold_changes = 1;  outer_pass = 1;
                    }
                    continue;
                }

                if (strlen(tac[i].op) == 0) continue;
                if (!is_numeric(tac[i].arg1) || !is_numeric(tac[i].arg2)) continue;

                double v1 = strtod(tac[i].arg1, NULL);
                double v2 = strtod(tac[i].arg2, NULL);
                double res = 0;
                int valid = 1;

                if      (strcmp(tac[i].op, "+")  == 0) res = v1 + v2;
                else if (strcmp(tac[i].op, "-")  == 0) res = v1 - v2;
                else if (strcmp(tac[i].op, "*")  == 0) res = v1 * v2;
                else if (strcmp(tac[i].op, "/")  == 0) { if (v2 != 0) res = v1 / v2; else valid = 0; }
                else if (strcmp(tac[i].op, "==") == 0) res = (v1 == v2);
                else if (strcmp(tac[i].op, "!=") == 0) res = (v1 != v2);
                else if (strcmp(tac[i].op, "<")  == 0) res = (v1 <  v2);
                else if (strcmp(tac[i].op, ">")  == 0) res = (v1 >  v2);
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
                    fold_changes = 1;  outer_pass = 1;
                }
            }
        }

        /* Pass 2: Constant Propagation (temps only) */
        for (int i = 0; i < tac_count; i++) {
            if (tac[i].kind != 0) continue;
            if (strlen(tac[i].op) > 0) continue;
            if (!is_numeric(tac[i].arg1)) continue;
            if (tac[i].result[0] != 't') continue;

            const char *var = tac[i].result;
            const char *val = tac[i].arg1;

            for (int j = i + 1; j < tac_count; j++) {
                if ((tac[j].kind == 0 || tac[j].kind == 5 || tac[j].kind == 7)
                    && strcmp(tac[j].result, var) == 0)
                    break;
                int replaced = 0;
                if (strcmp(tac[j].arg1, var) == 0) { strncpy(tac[j].arg1, val, 63); replaced = 1; }
                if (strcmp(tac[j].arg2, var) == 0) { strncpy(tac[j].arg2, val, 63); replaced = 1; }
                if (replaced) outer_pass = 1;
            }
        }
    }

    /* Pass 3a: Remove redundant goto+label pairs */
    for (int i = 0; i < tac_count - 1; i++) {
        if (tac[i].kind == 2 && tac[i + 1].kind == 1
            && strcmp(tac[i].result, tac[i + 1].result) == 0) {
            for (int j = i; j < tac_count - 1; j++) tac[j] = tac[j + 1];
            tac_count--;  i--;
        }
    }

    /* Pass 3b: Remove dead temp assignments */
    for (int i = 0; i < tac_count; i++) {
        if (tac[i].kind != 0 || tac[i].result[0] != 't' || strlen(tac[i].op) > 0)
            continue;
        int used = 0;
        for (int j = i + 1; j < tac_count; j++) {
            if (strcmp(tac[j].arg1, tac[i].result) == 0 ||
                strcmp(tac[j].arg2, tac[i].result) == 0) { used = 1; break; }
            if (tac[j].kind == 0 && strcmp(tac[j].result, tac[i].result) == 0) break;
        }
        if (!used) {
            for (int j = i; j < tac_count - 1; j++) tac[j] = tac[j + 1];
            tac_count--;  i--;
        }
    }
}
