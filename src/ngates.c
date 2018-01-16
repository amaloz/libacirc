#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_eval_f(size_t ref, acirc_op op, size_t xref, const void *x, size_t yref, const void *y, void *count_)
{
    (void) ref; (void) xref; (void) yref; (void) op; (void) x; (void) y;
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
