#include "_acirc.h"
#include "parse.h"
#include "map.h"

#include <setjmp.h>
#include <stdlib.h>

typedef struct {
    long *xs;
    long *ys;
} eval_t;

static void *
_acirc_input(size_t i, void *args_)
{
    eval_t *args = args_;
    return (void *) args->xs[i];
}

static void *
_acirc_const(size_t i, long val, void *args_)
{
    eval_t *args = args_;
    if (args->ys) {
        return (void *) args->ys[i];
    } else {
        return (void *) val;
    }
}

static void *
_acirc_eval(acirc_op op, void *x_, void *y_, void *_)
{
    (void) _;
    long x, y;
    x = (long) x_; y = (long) y_;
    switch (op) {
    case ACIRC_OP_ADD:
        return (void *) (x + y);
    case ACIRC_OP_SUB:
        return (void *) (x - y);
    case ACIRC_OP_MUL:
        return (void *) (x * y);
    default:
        return NULL;
    }
}

long *
acirc_eval(acirc_t *c, long *xs, long *ys)
{
    eval_t args;
    args.xs = xs;
    args.ys = ys;
    return (long *) acirc_traverse(c, _acirc_input, _acirc_const, _acirc_eval, NULL, NULL, &args);
}

typedef struct {
    mpz_t **xs;
    mpz_t **ys;
    mpz_t *modulus;
} eval_mpz_t;

static void *
_acirc_input_mpz(size_t i, void *args)
{
    eval_mpz_t *s = args;
    return (void *) s->xs[i];
}

static void *
_acirc_const_mpz(size_t id, long val, void *args)
{
    eval_mpz_t *s = args;
    if (s->ys) {
        return (void *) s->ys[id];
    } else {
        mpz_t *x;
        x = calloc(1, sizeof x[0]);
        mpz_init_set_si(*x, val);
        return x;
    }
}

static void *
_acirc_eval_mpz(acirc_op op, void *x_, void *y_, void *args)
{
    eval_mpz_t *s = args;
    mpz_t *x, *y, *modulus, *rop;
    x = x_; y = y_; modulus = s->modulus;

    rop = calloc(1, sizeof rop[0]);
    mpz_init(*rop);
    switch (op) {
    case ACIRC_OP_ADD:
        mpz_add(*rop, *x, *y);
        break;
    case ACIRC_OP_SUB:
        mpz_sub(*rop, *x, *y);
        break;
    case ACIRC_OP_MUL:
        mpz_mul(*rop, *x, *y);
        break;
    }
    if (mpz_cmp_ui(*modulus, 0) != 0)
        mpz_mod(*rop, *rop, *modulus);
    return (void *) rop;
}

static void *
_acirc_copy_mpz(void *x_, void *_)
{
    (void) _;
    mpz_t *out;
    out = calloc(1, sizeof out[0]);
    mpz_init_set(*out, x_);
    return out;
}

static void
_acirc_free_mpz(void *x_, void *_)
{
    (void) _;
    if (x_) {
        mpz_clear(x_);
        free(x_);
    }
}

mpz_t **
acirc_eval_mpz(acirc_t *c, mpz_t **xs, mpz_t **ys, mpz_t modulus)
{
    eval_mpz_t s;
    s.xs = xs;
    s.ys = ys;
    s.modulus = (mpz_t *) &modulus;
    return (mpz_t **) acirc_traverse(c, _acirc_input_mpz, _acirc_const_mpz,
                                     _acirc_eval_mpz, _acirc_copy_mpz, _acirc_free_mpz, &s);
}

void
acirc_eval_mpz_free(acirc_t *c)
{
    map_free(c->map, _acirc_free_mpz, NULL);
    c->map = NULL;
}
