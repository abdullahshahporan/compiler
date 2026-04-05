/* optimizer.h — TAC optimization (constant folding, propagation, dead code) */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "common.h"

int  is_numeric(const char *s);
void optimize_tac(void);

#endif /* OPTIMIZER_H */
