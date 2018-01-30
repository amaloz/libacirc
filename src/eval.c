#include "_acirc.h"
#include "parse.h"

#include <setjmp.h>
#include <stdlib.h>

typedef struct {
    const long *xs;
    const long *ys;
} eval_t;

static void *
_acirc_input(size_t ref, size_t i, void *args_)
{
    (void) ref;
    eval_t *args = args_;
    return (void *) args->xs[i];
}

static void *
_acirc_const(size_t ref, size_t i, long val, void *args_)
{
    (void) ref;
    eval_t *args = args_;
    if (args->ys) {
        return (void *) args->ys[i];
    } else {
        return (void *) val;
    }
}

static void *
_acirc_eval(size_t ref, acirc_op op, size_t xref, const void *x_, size_t yref, const void *y_, void *_)
{
    (void) ref; (void) xref; (void) yref; (void) _;
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
acirc_eval(const acirc_t *c, const long *xs, const long *ys)
{
    eval_t args = { .xs = xs, .ys = ys };
    return (long *) acirc_traverse((acirc_t *) c, _acirc_input, _acirc_const,
                                   _acirc_eval, NULL, NULL, NULL, NULL,
                                   NULL, &args, 0);
}

typedef struct {
    mpz_t **xs;
    mpz_t **ys;
    mpz_t *modulus;
} eval_mpz_t;

static void *
_acirc_copy_mpz(void *x_, void *_)
{
    (void) _;
    mpz_t *x = x_;
    mpz_t *out;
    out = calloc(1, sizeof out[0]);
    mpz_init_set(*out, *x);
    return out;
}

static void *
_acirc_input_mpz(size_t ref, size_t i, void *args_)
{
    (void) ref;
    eval_mpz_t *args = args_;
    return _acirc_copy_mpz((mpz_t *) args->xs[i], args);
}

static void *
_acirc_const_mpz(size_t ref, size_t i, long val, void *args_)
{
    (void) ref;
    eval_mpz_t *args = args_;
    if (args->ys) {
        return _acirc_copy_mpz((mpz_t *) args->ys[i], args);
    } else {
        mpz_t *x;
        x = calloc(1, sizeof x[0]);
        mpz_init_set_si(*x, val);
        return x;
    }
}

static void *
_acirc_eval_mpz(size_t ref, acirc_op op, size_t xref, const void *x_, size_t yref, const void *y_, void *args_)
{
    (void) ref; (void) xref; (void) yref;
    eval_mpz_t *args = args_;
    const mpz_t *x = (const mpz_t *) x_;
    const mpz_t *y = (const mpz_t *) y_;
    mpz_t *modulus, *rop;
    modulus = args->modulus;

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
_acirc_output_mpz(size_t ref, size_t i, void *x, void *args)
{
    (void) ref; (void) i;
    return _acirc_copy_mpz(x, args);
}

static void
_acirc_free_mpz(void *x_, void *_)
{
    (void) _;
    mpz_t *x = x_;
    if (x) {
        mpz_clear(*x);
        free(x);
    }
}

mpz_t **
acirc_eval_mpz(const acirc_t *c, mpz_t **xs, mpz_t **ys, const mpz_t modulus)
{
    eval_mpz_t s = { .xs = xs, .ys = ys, .modulus = (mpz_t *) modulus };
    return (mpz_t **) acirc_traverse((acirc_t *) c, _acirc_input_mpz,
                                     _acirc_const_mpz, _acirc_eval_mpz,
                                     _acirc_output_mpz, _acirc_free_mpz,
                                     NULL, NULL, NULL, &s, 0);
}
