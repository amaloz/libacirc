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
_const_zero_f(size_t i, long val, void *extra)
{
    (void) i; (void) val; (void) extra;
    return (void *) 0;
}

static void *
_const_one_f(size_t i, long val, void *extra)
{
    (void) i; (void) val; (void) extra;
    return (void *) 1;
}

static void *
_eval_f(acirc_op op, const void *x, const void *y, void *_)
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

long *
acirc_degrees(acirc_t *c)
{
    return (long *) acirc_traverse(c, _input_one_f, _const_one_f, _eval_f, NULL, NULL, NULL, 0);
}

long
acirc_max_degree(acirc_t *c)
{
    long max = 0;
    long *degrees;

    if ((degrees = acirc_degrees(c)) == NULL)
        return -1;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (degrees[i] > max)
            max = degrees[i];
    }
    free(degrees);
    return max;
}

long *
acirc_const_degrees(acirc_t *c)
{
    return (long *) acirc_traverse(c, _input_zero_f, _const_one_f, _eval_f, NULL, NULL, NULL, 0);
}

long
acirc_max_const_degree(acirc_t *c)
{
    long *degrees;
    long max = 0;

    if (c->_max_const_degree != -1)
        return c->_max_const_degree;

    if ((degrees = acirc_const_degrees(c)) == NULL)
        return -1;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (degrees[i] > max)
            max = degrees[i];
    }
    c->_max_const_degree = max;
    free(degrees);
    return max;
}

typedef struct {
    size_t k;
    acirc_t *circ;
} var_args_t;

static void *
_input_var_f(size_t i, void *args_)
{
    var_args_t *args = args_;
    size_t lower = 0, upper = 0;
    for (size_t i = 0; i < args->k; ++i)
        lower += acirc_symlen(args->circ, i);
    upper = lower + acirc_symlen(args->circ, args->k);
    return (void *) (long) (i >= lower && i <= upper);
}

long *
acirc_var_degrees(acirc_t *c, size_t k)
{
    var_args_t args = {
        .k = k,
        .circ = c,
    };
    return (long *) acirc_traverse(c, _input_var_f, _const_zero_f, _eval_f, NULL, NULL, &args, 0);
}

long
acirc_max_var_degree(acirc_t *c, size_t k)
{
    long *degrees;
    long max = 0;
    if ((degrees = acirc_var_degrees(c, k)) == NULL)
        return -1;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (degrees[i] > max)
            max = degrees[i];
    }
    free(degrees);
    return max;
}

long
acirc_delta(acirc_t *c)
{
    long delta;

    delta = acirc_max_const_degree(c);
    for (size_t k = 0 ; k < acirc_ninputs(c); k += acirc_symlen(c, k)) {
        delta += acirc_max_var_degree(c, k);
    }
    return delta;
}
