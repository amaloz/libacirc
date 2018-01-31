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
typedef void * (*acirc_input_f)(size_t, size_t, void *);
typedef void * (*acirc_const_f)(size_t, size_t, long, void *);
typedef void * (*acirc_eval_f)(size_t, acirc_op, size_t, const void *, size_t, const void *, void *);
typedef void * (*acirc_output_f)(size_t, size_t, void *, void *);
typedef void * (*acirc_copy_f)(void *, void *);
typedef void   (*acirc_free_f)(void *, void *);
typedef int    (*acirc_write_f)(size_t, void *, void *);
typedef void * (*acirc_read_f)(size_t, void *);

typedef struct {
    acirc_input_f input_f;
    acirc_const_f const_f;
    acirc_eval_f eval_f;
    void *extra;
} acirc_parse_t;

acirc_t *acirc_new(const char *fname, bool saved, bool mmapped);
void acirc_free(acirc_t *c);

size_t acirc_ngates(const acirc_t *c);
size_t acirc_nmuls(const acirc_t *c);

size_t * acirc_degrees(const acirc_t *c);
size_t   acirc_max_degree(const acirc_t *c);
size_t * acirc_const_degrees(const acirc_t *c);
size_t   acirc_max_const_degree(const acirc_t *c);
size_t * acirc_var_degrees(const acirc_t *c, size_t k);
size_t   acirc_max_var_degree(const acirc_t *c, size_t k);
size_t * acirc_depths(const acirc_t *c);
size_t   acirc_max_depth(const acirc_t *c);
size_t   acirc_delta(const acirc_t *c);

long * acirc_eval(const acirc_t *c, const long *xs, const long *ys);
mpz_t ** acirc_eval_mpz(const acirc_t *c, mpz_t **xs, mpz_t **ys, const mpz_t modulus);

void **
acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
               acirc_eval_f eval_f, acirc_output_f output_f,
               acirc_free_f free_f, acirc_write_f write_f,
               acirc_read_f read_f, void *extra, size_t nthreads);

const char * acirc_fname(const acirc_t *c);
size_t acirc_ninputs(const acirc_t *c);
size_t acirc_nconsts(const acirc_t *c);
size_t acirc_nsecrets(const acirc_t *c);
size_t acirc_noutputs(const acirc_t *c);
size_t acirc_nsymbols(const acirc_t *c);
size_t acirc_nrefs(const acirc_t *c);
size_t acirc_symlen(const acirc_t *c, size_t i);
bool   acirc_is_sigma(const acirc_t *c, size_t i);
bool   acirc_is_binary(const acirc_t *c);
long   acirc_const(const acirc_t *c, size_t i);
long   acirc_secret(const acirc_t *c, size_t i);

size_t acirc_ntests(const acirc_t *c);
long * acirc_test_input(const acirc_t *c, size_t i);
long * acirc_test_output(const acirc_t *c, size_t i);
bool acirc_test(acirc_t *c);

acirc_op acirc_str2op(const char *str);
char *   acirc_op2str(acirc_op op);

#endif
