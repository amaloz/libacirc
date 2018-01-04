#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_eval_f(acirc_op op, const void *x, const void *y, void *count_)
{
    (void) op; (void) x; (void) y;
    long *count = count_;
    (*count)++;
    return NULL;
}

long
acirc_ngates(acirc_t *c)
{
    void **outputs;
    long count = 0;

    outputs = acirc_traverse(c, NULL, NULL, _eval_f, NULL, NULL, &count, 0);
    free(outputs);
    return count;
}
