#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_input_f(size_t i, void *extra)
{
    (void) i; (void) extra;
    return (void *) 0;
}

static void *
_const_f(size_t i, long val, void *extra)
{
    (void) i; (void) val; (void) extra;
    return (void *) 0;
}

static void *
_eval_f(acirc_op op, void *x, void *y, void *_)
{
    (void) op; (void) _;
    unsigned long x_, y_;
    x_ = (unsigned long) x;
    y_ = (unsigned long) y;
    return (void *) ((x_ > y_ ? x_ : y_) + 1);
}

long *
acirc_depths(acirc_t *c)
{
    return (long *) acirc_traverse(c, _input_f, _const_f, _eval_f, NULL, NULL, NULL);
}

long
acirc_max_depth(acirc_t *c)
{
    long *depths;
    long max = 0;

    if ((depths = acirc_depths(c)) == NULL)
        return -1;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (depths[i] > max)
            max = depths[i];
    }
    free(depths);
    return max;
}
