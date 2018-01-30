#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_eval_f(size_t ref, acirc_op op, size_t xref, const void *x, size_t yref, const void *y, void *count_)
{
    (void) ref; (void) xref; (void) yref; (void) x; (void) y;
    long *count = count_;
    if (op == ACIRC_OP_MUL)
        (*count)++;
    return NULL;
}

size_t
acirc_nmuls(const acirc_t *c_)
{
    acirc_t *c = (acirc_t *) c_;
    void **outputs = NULL;

    if (c->_nmuls)
        return c->_nmuls;
    outputs = acirc_traverse(c, NULL, NULL, _eval_f, NULL, NULL, &c->_nmuls, 0);
    if (outputs)
        free(outputs);
    return c->_nmuls;
}
