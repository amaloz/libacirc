#include "acirc.h"
#include "_acirc.h"
#include "map.h"
#include "parse.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;

typedef struct {
    acirc_op op;
    unsigned int x, y;
} gate_t;

struct acirc_t {
    FILE *fp;
    void **inputs;
    size_t ninputs;
    ref_t *outrefs;
    size_t noutrefs;
    map_t *map;
};

acirc_t *
acirc_new(const char *fname)
{
    acirc_t *c;

    if ((c = calloc(1, sizeof c[0])) == NULL)
        return NULL;
    if ((c->fp = fopen(fname, "r")) == NULL)
        goto error;
    c->map = map_new();
    return c;
error:
    acirc_free(c);
    return NULL;
}

void
acirc_free(acirc_t *c)
{
    if (c == NULL)
        return;
    if (c->fp)
        fclose(c->fp);
    if (c->outrefs)
        free(c->outrefs);
    if (c->map)
        free(c->map);
    free(c);
}

int
acirc_reset(acirc_t *c)
{
    if (c == NULL || c->fp == NULL)
        return ACIRC_ERR;
    rewind(c->fp);
    map_free(c->map);
    c->map = map_new();
    return ACIRC_OK;
}

int
acirc_traverse(acirc_t *c, void **inputs, size_t ninputs, void **outputs,
               size_t noutputs, acirc_eval_f eval, void *extra)
{
    jmp_buf env;
    int ret = ACIRC_ERR;
    c->inputs = inputs;
    c->ninputs = ninputs;
    yyin = c->fp;
    setjmp(env);
    if (yyparse(c, eval, extra, env) != 0) {
        fprintf(stderr, "error: parsing circuit failed\n");
        goto cleanup;
    }
    for (size_t i = 0; i < noutputs; ++i) {
        outputs[i] = map_get(c->map, c->outrefs[i]);
    }
    ret = ACIRC_OK;
cleanup:
    c->inputs = NULL;
    c->ninputs = 0;
    return ret;
}

static void *
_acirc_degrees(acirc_op op, void *x, void *y, void *_)
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

unsigned long *
acirc_degrees(acirc_t *c, size_t n, size_t m)
{
    unsigned long *inputs, *outputs;

    inputs = calloc(n, sizeof inputs[0]);
    outputs = calloc(m, sizeof outputs[0]);
    for (size_t i = 0; i < n; ++i) inputs[i] = 1;
    if (acirc_traverse(c, (void **) inputs, n, (void **) outputs, m, _acirc_degrees, NULL) == ACIRC_ERR) {
        free(inputs);
        free(outputs);
        return NULL;
    }
    free(inputs);
    return outputs;
}

unsigned long
acirc_max_degree(acirc_t *c, size_t n, size_t m)
{
    unsigned long max = 0, *outputs;

    if ((outputs = acirc_degrees(c, n, m)) == NULL)
        return 0;
    for (size_t i = 0; i < m; ++i) {
        if (outputs[i] > max)
            max = outputs[i];
    }
    return max;
}

static void *
_acirc_depths(acirc_op op, void *x, void *y, void *_)
{
    (void) op; (void) _;
    unsigned long x_, y_;
    x_ = (unsigned long) x;
    y_ = (unsigned long) y;
    return (void *) ((x_ > y_ ? x_ : y_) + 1);
}

unsigned long *
acirc_depths(acirc_t *c, size_t n, size_t m)
{
    unsigned long *inputs, *outputs;
    inputs = calloc(n, sizeof inputs[0]);
    outputs = calloc(m, sizeof outputs[0]);
    if (acirc_traverse(c, (void **) inputs, n, (void **) outputs, m, _acirc_depths, NULL) == ACIRC_ERR) {
        free(inputs);
        free(outputs);
        return NULL;
    }
    free(inputs);
    return outputs;
}

unsigned long
acirc_max_depth(acirc_t *c, size_t n, size_t m)
{
    unsigned long max = 0, *outputs;

    if ((outputs = acirc_depths(c, n, m)) == NULL)
        return 0;
    for (size_t i = 0; i < m; ++i) {
        if (outputs[i] > max)
            max = outputs[i];
    }
    return max;
}


static void *
_acirc_eval(acirc_op op, void *x, void *y, void *_)
{
    (void) _;
    long x_, y_;
    x_ = (long) x;
    y_ = (long) y;
    switch (op) {
    case ACIRC_OP_ADD:
        return (void *) (x_ + y_);
    case ACIRC_OP_SUB:
        return (void *) (x_ - y_);
    case ACIRC_OP_MUL:
        return (void *) (x_ * y_);
    default:
        return NULL;
    }
}

long *
acirc_eval(acirc_t *c, long *inputs, size_t n, size_t m)
{
    long *outputs;

    outputs = calloc(m, sizeof outputs[0]);
    if (acirc_traverse(c, (void **) inputs, n, (void **) outputs, m, _acirc_eval, NULL) == ACIRC_ERR) {
        free(outputs);
        return NULL;
    }
    return outputs;
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

mpz_t **
acirc_eval_mpz(acirc_t *c, mpz_t **inputs, size_t n, size_t m, mpz_t modulus)
{
    mpz_t **outputs;

    outputs = calloc(m, sizeof outputs[0]);
    if (acirc_traverse(c, (void **) inputs, n, (void **) outputs, m, _acirc_eval_mpz, modulus) == ACIRC_ERR) {
        free(outputs);
        return NULL;
    }
    return outputs;
}

/*
 * private circuit evaluation functions
 */

int
acirc_eval_input(acirc_t *c, ref_t ref, ref_t inp)
{
    if (inp >= c->ninputs) {
        fprintf(stderr, "Invalid input index '%u'\n", inp);
        return ACIRC_ERR;
    }
    map_put(c->map, ref, c->inputs[inp]);
    return ACIRC_OK;
}

int
acirc_eval_const(acirc_t *c, ref_t ref, int val)
{
    (void) c; (void) ref; (void) val;
    return ACIRC_OK;
}

int
acirc_eval_gate(acirc_t *c, acirc_op op, ref_t ref, ref_t x, ref_t y, acirc_eval_f eval, void *extra)
{
    void *x_, *y_, *_out;
    x_ = map_get(c->map, x);
    y_ = map_get(c->map, y);
    _out = eval(op, x_, y_, extra);
    map_put(c->map, ref, _out);
    return ACIRC_OK;
}

int
acirc_eval_outputs(acirc_t *c, ref_t *refs, size_t n)
{
    c->outrefs = refs;
    c->noutrefs = n;
    return ACIRC_OK;
}

acirc_op
acirc_str2op(const char *s)
{
    if (strcmp(s, "ADD") == 0) {
        return ACIRC_OP_ADD;
    } else if (strcmp(s, "SUB") == 0) {
        return ACIRC_OP_SUB;
    } else if (strcmp(s, "MUL") == 0) {
        return ACIRC_OP_MUL;
    } else {
        fprintf(stderr, "error: unknown acirc operation '%s'\n", s);
        abort();
    }
}

