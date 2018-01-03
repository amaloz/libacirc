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
typedef void * (*acirc_input_f)(size_t, void *);
typedef void * (*acirc_const_f)(size_t, int, void *);
typedef void * (*acirc_eval_f)(acirc_op, void *, void *, void *);

typedef struct {
    acirc_input_f input_f;
    acirc_const_f const_f;
    acirc_eval_f eval_f;
    void *extra;
} acirc_parse_t;

acirc_t *acirc_new(const char *fname);
void acirc_free(acirc_t *c);

unsigned long acirc_ngates(acirc_t *c);
unsigned long acirc_nmuls(acirc_t *c);

int acirc_degrees(acirc_t *c);
unsigned long acirc_max_degree(acirc_t *c);
int acirc_const_degrees(acirc_t *c);
unsigned long acirc_max_const_degree(acirc_t *c);
int acirc_var_degrees(acirc_t *c, size_t k);
unsigned long acirc_max_var_degrees(acirc_t *c, size_t k);
int acirc_depths(acirc_t *c);
unsigned long acirc_max_depth(acirc_t *c);
unsigned long acirc_delta(acirc_t *c);

int acirc_eval_mpz(acirc_t *c, mpz_t *xs, mpz_t *ys, mpz_t modulus);

int acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
                   acirc_eval_f eval_f, void *extra);

size_t acirc_ninputs(const acirc_t *c);
size_t acirc_nconsts(const acirc_t *c);
size_t acirc_noutputs(const acirc_t *c);

int acirc_const(acirc_t *c, size_t i);
void *acirc_output(acirc_t *c, size_t i);

#endif
