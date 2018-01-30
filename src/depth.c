#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_input_f(size_t ref, size_t i, void *extra)
{
    (void) ref; (void) i; (void) extra;
    return (void *) 0;
}

static void *
_const_f(size_t ref, size_t i, long val, void *extra)
{
    (void) ref; (void) i; (void) val; (void) extra;
    return (void *) 0;
}

static void *
_eval_f(size_t ref, acirc_op op, size_t xref, const void *x, size_t yref, const void *y, void *_)
{
    (void) ref; (void) xref; (void) yref; (void) op; (void) _;
    unsigned long x_, y_;
    x_ = (unsigned long) x;
    y_ = (unsigned long) y;
    return (void *) ((x_ > y_ ? x_ : y_) + 1);
}

size_t *
acirc_depths(const acirc_t *c)
{
    return (size_t *) acirc_traverse((acirc_t *) c, _input_f, _const_f, _eval_f,
                                     NULL, NULL, NULL, NULL, NULL, 0);
}

size_t
acirc_max_depth(const acirc_t *c_)
{
    acirc_t *c = (acirc_t *) c_;
    size_t *depths;

    if (c->_max_depth)
        return c->_max_depth;
    if ((depths = acirc_depths(c)) == NULL)
        return 0;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (depths[i] > c->_max_depth)
            c->_max_depth = depths[i];
    }
    free(depths);
    return c->_max_depth;
}
