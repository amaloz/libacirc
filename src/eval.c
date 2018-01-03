#include "_acirc.h"
#include "parse.h"
#include "map.h"

#include <setjmp.h>
#include <stdlib.h>

int
acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
               acirc_eval_f eval_f, void *extra)
{
    jmp_buf env;
    setjmp(env);
    if (yyparse(c, input_f, const_f, eval_f, extra, NULL, env) != 0) {
        fprintf(stderr, "error: parsing circuit failed\n");
        return ACIRC_ERR;
    }
    return ACIRC_OK;
}

/* static void * */
/* _acirc_input_ulong(size_t inp, void *inputs) */
/* { */
/*     unsigned long *inputs_ = inputs; */
/*     return (void *) inputs[inp]; */
/* } */

/* static void * */
/* _acirc_const_ulong(int val, void *_) */
/* { */
/*     (void) _; */
/*     return (void *) val; */
/* } */

/* static void * */
/* _acirc_eval_degrees(acirc_op op, void *x, void *y, void *_) */
/* { */
/*     (void) _; */
/*     unsigned long x_, y_; */
/*     x_ = (unsigned long) x; */
/*     y_ = (unsigned long) y; */
/*     switch (op) { */
/*     case ACIRC_OP_ADD: case ACIRC_OP_SUB: */
/*         return (void *) (x_ > y_ ? x_ : y_); */
/*     case ACIRC_OP_MUL: */
/*         return (void *) (x_ + y_); */
/*     default: */
/*         return NULL; */
/*     } */
/* } */

/* unsigned long * */
/* acirc_degrees(acirc_t *c, size_t n) */
/* { */
/*     unsigned long *inputs; */

/*     inputs = calloc(n, sizeof inputs[0]); */
/*     for (size_t i = 0; i < n; ++i) inputs[i] = 1; */
/*     if (acirc_traverse(c, (void **) inputs, n, sizeof(unsigned long), _acirc_degrees, NULL) == ACIRC_ERR) { */
/*         free(inputs); */
/*         return NULL; */
/*     } */
/*     free(inputs); */
/*     return (unsigned long *) c->outputs; */
/* } */

/* unsigned long */
/* acirc_max_degree(acirc_t *c, size_t n) */
/* { */
/*     unsigned long max = 0, *outputs; */

/*     if ((outputs = acirc_degrees(c, n)) == NULL) */
/*         return 0; */
/*     for (size_t i = 0; i < c->noutputs; ++i) { */
/*         if (outputs[i] > max) */
/*             max = outputs[i]; */
/*     } */
/*     return max; */
/* } */

/* static void * */
/* _acirc_depths(acirc_op op, void *x, void *y, void *_) */
/* { */
/*     (void) op; (void) _; */
/*     unsigned long x_, y_; */
/*     x_ = (unsigned long) x; */
/*     y_ = (unsigned long) y; */
/*     return (void *) ((x_ > y_ ? x_ : y_) + 1); */
/* } */

/* unsigned long * */
/* acirc_depths(acirc_t *c, size_t n) */
/* { */
/*     unsigned long *inputs; */
/*     inputs = calloc(n, sizeof inputs[0]); */
/*     if (acirc_traverse(c, (void **) inputs, n, sizeof(unsigned long), _acirc_depths, NULL) == ACIRC_ERR) { */
/*         free(inputs); */
/*         return NULL; */
/*     } */
/*     free(inputs); */
/*     return (unsigned long *) c->outputs; */
/* } */

/* unsigned long */
/* acirc_max_depth(acirc_t *c, size_t n) */
/* { */
/*     unsigned long max = 0, *outputs; */

/*     if ((outputs = acirc_depths(c, n)) == NULL) */
/*         return 0; */
/*     for (size_t i = 0; i < c->noutputs; ++i) { */
/*         if (outputs[i] > max) */
/*             max = outputs[i]; */
/*     } */
/*     return max; */
/* } */


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
    mpz_t **inputs;
    mpz_t *modulus;
} eval_mpz_t;

static void *
_acirc_input_mpz(size_t id, void *extra)
{
    eval_mpz_t *s = extra;
    return (void *) s->inputs[id];
}

static void *
_acirc_const_mpz(int val, void *_)
{
    (void) _;
    mpz_t *x;
    x = calloc(1, sizeof x[0]);
    mpz_init_set_si(*x, val);
    return x;
}

static void *
_acirc_eval_mpz(acirc_op op, void *x, void *y, void *modulus)
{
    mpz_t *x_, *y_, *modulus_, *rop;
    x_ = (mpz_t *) x; y_ = (mpz_t *) y; modulus_ = (mpz_t *) modulus;

    rop = calloc(1, sizeof rop[0]);
    mpz_init(*rop);
    switch (op) {
    case ACIRC_OP_ADD:
        mpz_add(*rop, *x_, *y_);
        break;
    case ACIRC_OP_SUB:
        mpz_sub(*rop, *x_, *y_);
        break;
    case ACIRC_OP_MUL:
        mpz_mul(*rop, *x_, *y_);
        break;
    }
    mpz_mod(*rop, *rop, *modulus_);
    return (void *) rop;
}

int
acirc_eval_mpz(acirc_t *c, mpz_t **inputs, size_t n, mpz_t modulus)
{
    eval_mpz_t s;
    s.inputs = inputs;
    s.modulus = &modulus;
    return acirc_traverse(c, _acirc_input_mpz, _acirc_const_mpz, _acirc_eval_mpz, &s);
}
