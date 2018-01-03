#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_input_zero_f(size_t i, void *extra)
{
    (void) i; (void) extra;
    return (void *) 0;
}


static void *
_input_one_f(size_t i, void *extra)
{
    (void) i; (void) extra;
    return (void *) 1;
}

static void *
_const_zero_f(size_t i, int val, void *extra)
{
    (void) i; (void) val; (void) extra;
    return (void *) 0;
}

static void *
_const_one_f(size_t i, int val, void *extra)
{
    (void) i; (void) val; (void) extra;
    return (void *) 1;
}

static void *
_eval_f(acirc_op op, void *x, void *y, void *_)
{
    (void) _;
    unsigned long x_, y_;
    x_ = (unsigned long) x;
    y_ = (unsigned long) y;
    switch (op) {
    case ACIRC_OP_ADD: case ACIRC_OP_SUB:
        return (void *) (x_ > y_ ? x_ : y_);
    case ACIRC_OP_MUL:
        return (void *) (x_ + y_);
    default:
        return NULL;
    }
}

int
acirc_degrees(acirc_t *c)
{
    return acirc_traverse(c, _input_one_f, _const_one_f, _eval_f, NULL);
}

unsigned long
acirc_max_degree(acirc_t *c)
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

int
acirc_const_degrees(acirc_t *c)
{
    return acirc_traverse(c, _input_zero_f, _const_one_f, _eval_f, NULL);
}

unsigned long
acirc_max_const_degree(acirc_t *c)
{
    unsigned long max = 0;
    if (acirc_const_degrees(c) == ACIRC_ERR)
        return 0;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        unsigned long output = (unsigned long) acirc_output(c, i);
        if (output > max)
            max = output;
    }
    return max;
}

static void *
_input_var_f(size_t i, void *extra)
{
    const size_t k = *((size_t *) extra);
    return (void *) (i == k ? 1 : 0);
}


int
acirc_var_degrees(acirc_t *c, size_t k)
{
    return acirc_traverse(c, _input_var_f, _const_zero_f, _eval_f, &k);
}

unsigned long
acirc_max_var_degree(acirc_t *c, size_t k)
{
    unsigned long max = 0;
    if (acirc_var_degrees(c, k) == ACIRC_ERR)
        return 0;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        unsigned long output = (unsigned long) acirc_output(c, i);
        if (output > max)
            max = output;
    }
    return max;
}

unsigned long
acirc_delta(acirc_t *c)
{
    unsigned long delta;

    delta = acirc_max_const_degree(c);
    for (size_t i = 0 ; i < acirc_ninputs(c); ++i) {
        delta += acirc_max_var_degree(c, i);
    }
    return delta;
}
