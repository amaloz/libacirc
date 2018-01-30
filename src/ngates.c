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

size_t
acirc_ngates(const acirc_t *c_)
{
    acirc_t *c = (acirc_t *) c_;
    void **outputs;

    if (c->_ngates)
        return c->_ngates;
    outputs = acirc_traverse(c, NULL, NULL, _eval_f, NULL, NULL, &c->_ngates, 0);
    free(outputs);
    return c->_ngates;
}
