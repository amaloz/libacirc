#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_input_f(size_t inp, void *extra)
{
    (void) inp; (void) extra;
    return (void *) 0;
}

static void *
_const_f(int val, void *extra)
{
    (void) val; (void) extra;
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

int
acirc_depth(acirc_t *c)
{
    return acirc_traverse(c, _input_f, _const_f, _eval_f, NULL);
}

unsigned long
acirc_max_depth(acirc_t *c)
{
    unsigned long max = 0;

    if (acirc_degrees(c) == ACIRC_ERR)
        return 0;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        unsigned long output = (unsigned long) acirc_output(c, i);
        if (output > max)
            max = output;
    }
    return max;
}
