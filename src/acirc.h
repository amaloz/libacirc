#ifndef __ACIRC_H__
#define __ACIRC_H__

#include <gmp.h>
#include <stdio.h>

#define ACIRC_OK    0
#define ACIRC_ERR (-1)

typedef struct acirc_t acirc_t;
typedef enum {
    ACIRC_OP_ADD,
    ACIRC_OP_SUB,
    ACIRC_OP_MUL,
} acirc_op;
typedef void * (*acirc_eval_f)(acirc_op, void *, void *, void *);

acirc_t *acirc_new(const char *fname);
void acirc_free(acirc_t *c);
int acirc_reset(acirc_t *c);
int acirc_traverse(acirc_t *c, void **inputs, size_t ninputs, void **outputs,
                   size_t noutputs, acirc_eval_f f, void *extra);
unsigned long * acirc_degrees(acirc_t *c, size_t n, size_t m);
unsigned long acirc_max_degree(acirc_t *c, size_t n, size_t m);
unsigned long * acirc_depths(acirc_t *c, size_t n, size_t m);
unsigned long acirc_max_depth(acirc_t *c, size_t n, size_t m);
long * acirc_eval(acirc_t *c, long *inputs, size_t n, size_t m);
mpz_t ** acirc_eval_mpz(acirc_t *c, mpz_t **inputs, size_t n, size_t m, mpz_t modulus);

#endif
