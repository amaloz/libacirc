#include "_acirc.h"
#include "parse.h"
#include "map.h"

#include <setjmp.h>
#include <stdlib.h>


/* static void * */
/* _acirc_eval(acirc_op op, void *x, void *y, void *_) */
/* { */
/*     (void) _; */
/*     long x_, y_; */
/*     x_ = (long) x; */
/*     y_ = (long) y; */
/*     switch (op) { */
/*     case ACIRC_OP_ADD: */
/*         return (void *) (x_ + y_); */
/*     case ACIRC_OP_SUB: */
/*         return (void *) (x_ - y_); */
/*     case ACIRC_OP_MUL: */
/*         return (void *) (x_ * y_); */
/*     default: */
/*         return NULL; */
/*     } */
/* } */

/* long * */
/* acirc_eval(acirc_t *c, long *inputs, size_t n) */
/* { */
/*     if (acirc_traverse(c, (void **) inputs, n, sizeof(long), _acirc_eval, NULL) == ACIRC_ERR) { */
/*         return NULL; */
/*     } */
/*     return (long *) c->outputs; */
/* } */

typedef struct {
    mpz_t *xs;
    mpz_t *ys;
    mpz_t *modulus;
} eval_mpz_t;

static void *
_acirc_input_mpz(size_t id, void *args)
{
    eval_mpz_t *s = args;
    return (void *) s->xs[id];
}

static void *
_acirc_const_mpz(size_t id, int val, void *args)
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
    mpz_mod(*rop, *rop, *modulus);
    return (void *) rop;
}

int
acirc_eval_mpz(acirc_t *c, mpz_t *xs, mpz_t *ys, mpz_t modulus)
{
    eval_mpz_t s;
    s.xs = xs;
    s.ys = ys;
    s.modulus = &modulus;
    return acirc_traverse(c, _acirc_input_mpz, _acirc_const_mpz, _acirc_eval_mpz, &s);
}
