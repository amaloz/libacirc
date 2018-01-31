#include "_acirc.h"
#include "parse.h"

#include <stdlib.h>

static void *
_input_zero_f(size_t ref, size_t i, void *extra)
{
    (void) ref; (void) i; (void) extra;
    return (void *) 0;
}


static void *
_input_one_f(size_t ref, size_t i, void *extra)
{
    (void) ref; (void) i; (void) extra;
    return (void *) 1;
}

static void *
_const_zero_f(size_t ref, size_t i, long val, void *extra)
{
    (void) ref; (void) i; (void) val; (void) extra;
    return (void *) 0;
}

static void *
_const_one_f(size_t ref, size_t i, long val, void *extra)
{
    (void) ref; (void) i; (void) val; (void) extra;
    return (void *) 1;
}

static void *
_eval_f(size_t ref, acirc_op op, size_t xref, const void *x, size_t yref, const void *y, void *_)
{
    (void) ref; (void) xref; (void) yref; (void) _;
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

size_t *
acirc_degrees(const acirc_t *c)
{
    return (size_t *) acirc_traverse((acirc_t *) c, _input_one_f, _const_one_f,
                                     _eval_f, NULL, NULL, NULL, NULL, NULL, 0);
}

size_t
acirc_max_degree(const acirc_t *c_)
{
    acirc_t *c = (acirc_t *) c_;
    size_t *degrees;

    if (c->_max_degree)
        return c->_max_degree;

    if ((degrees = acirc_degrees(c)) == NULL)
        return 0;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (degrees[i] > c->_max_degree)
            c->_max_degree = degrees[i];
    }
    free(degrees);
    return c->_max_degree;
}

size_t *
acirc_const_degrees(const acirc_t *c)
{
    return (size_t *) acirc_traverse((acirc_t *) c, _input_zero_f, _const_one_f,
                                     _eval_f, NULL, NULL, NULL, NULL, NULL, 0);
}

size_t
acirc_max_const_degree(const acirc_t *c_)
{
    acirc_t *c = (acirc_t *) c_;
    size_t *degrees;

    if (c->_max_const_degree)
        return c->_max_const_degree;

    if ((degrees = acirc_const_degrees(c)) == NULL)
        return 0;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (degrees[i] > c->_max_const_degree)
            c->_max_const_degree = degrees[i];
    }
    free(degrees);
    return c->_max_const_degree;
}

typedef struct {
    size_t k;
    acirc_t *circ;
} var_args_t;

static void *
_input_var_f(size_t ref, size_t i, void *args_)
{
    (void) ref;
    var_args_t *args = args_;
    size_t lower = 0, upper = 0;
    for (size_t i = 0; i < args->k; ++i)
        lower += acirc_symlen(args->circ, i);
    upper = lower + acirc_symlen(args->circ, args->k);
    return (void *) (long) (i >= lower && i <= upper);
}

size_t *
acirc_var_degrees(const acirc_t *c, size_t k)
{
    var_args_t args = {
        .k = k,
        .circ = (acirc_t *) c,
    };
    return (size_t *) acirc_traverse((acirc_t *) c, _input_var_f, _const_zero_f,
                                     _eval_f, NULL, NULL, NULL, NULL, &args, 0);
}

size_t
acirc_max_var_degree(const acirc_t *c, size_t k)
{
    size_t *degrees;
    size_t max = 0;
    if ((degrees = acirc_var_degrees(c, k)) == NULL)
        return 0;
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        if (degrees[i] > max)
            max = degrees[i];
    }
    free(degrees);
    return max;
}

size_t
acirc_delta(const acirc_t *c_)
{
    acirc_t *c = (acirc_t *) c_;
    if (c->_delta)
        return c->_delta;
    if (c->nconsts)
        if ((c->_delta = acirc_max_const_degree(c)) == 0)
            return 0;
    for (size_t k = 0 ; k < acirc_nsymbols(c); ++k) {
        c->_delta += acirc_max_var_degree(c, k);
    }
    return c->_delta;
}
