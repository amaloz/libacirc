#ifndef __ACIRC_H__
#define __ACIRC_H__

#include <gmp.h>
#include <stdbool.h>
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
typedef void * (*acirc_const_f)(size_t, long, void *);
typedef void * (*acirc_eval_f)(acirc_op, void *, void *, void *);
typedef void * (*acirc_copy_f)(void *);
typedef void (*acirc_free_f)(void *);

typedef struct {
    acirc_input_f input_f;
    acirc_const_f const_f;
    acirc_eval_f eval_f;
    void *extra;
} acirc_parse_t;

acirc_t *acirc_new(const char *fname);
void acirc_free(acirc_t *c, acirc_free_f f);

long acirc_ngates(acirc_t *c);
long acirc_nmuls(acirc_t *c);

long * acirc_degrees(acirc_t *c);
long acirc_max_degree(acirc_t *c);
long * acirc_const_degrees(acirc_t *c);
long acirc_max_const_degree(acirc_t *c);
long * acirc_var_degrees(acirc_t *c, size_t k);
long acirc_max_var_degrees(acirc_t *c, size_t k);
long * acirc_depths(acirc_t *c);
long acirc_max_depth(acirc_t *c);
long acirc_delta(acirc_t *c);

long * acirc_eval(acirc_t *c, long *xs, long *ys);
mpz_t ** acirc_eval_mpz(acirc_t *c, mpz_t **xs, mpz_t **ys, mpz_t modulus);
void acirc_eval_mpz_free(acirc_t *c);

void ** acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
                       acirc_eval_f eval_f, acirc_copy_f copy_f, acirc_free_f free_f,
                       void *extra);

size_t acirc_ninputs(const acirc_t *c);
size_t acirc_nconsts(const acirc_t *c);
size_t acirc_noutputs(const acirc_t *c);

size_t acirc_ntests(const acirc_t *c);
long * acirc_test_input(const acirc_t *c, size_t i);
long * acirc_test_output(const acirc_t *c, size_t i);
bool acirc_test(acirc_t *c);


long acirc_const(acirc_t *c, size_t i);

#endif
