#pragma once

#include "acirc.h"
#include "storage.h"

#include <stdbool.h>
#include <threadpool.h>

#pragma GCC visibility push(hidden)

typedef size_t ref_t;

typedef struct {
    long *inps;
    long *outs;
} acirc_test_t;

struct acirc_t {
    FILE *fp;
    size_t ninputs;
    size_t nrefs;
    size_t nconsts;
    size_t noutputs;
    size_t nsymbols;
    size_t ntests;
    size_t base;
    bool binary;                /* true if the circuit is binary */

    long *consts;               /* [nconsts] */
    size_t *symlens;            /* [nsymbols] */
    ref_t *outrefs;             /* [noutputs] */
    acirc_test_t *tests;        /* [ntests] */

    bool circuit;               /* true if we are done parsing the prelim section */

    size_t _nconsts;
    storage_t map;
    threadpool *pool;
    ssize_t _max_const_degree;
};

int acirc_eval_input(acirc_t *c, acirc_input_f f, ref_t ref, size_t inp,
                     ssize_t count, void *extra);
int acirc_eval_const(acirc_t *c, acirc_const_f f, ref_t ref, ssize_t count,
                     void *extra);
int acirc_eval_gate(acirc_t *c, acirc_eval_f eval_f, acirc_free_f free_f,
                    acirc_op op, ref_t ref, ref_t x, ref_t y, ssize_t count,
                    void *extra);
int acirc_eval_output(acirc_t *c, acirc_output_f output_f, void **outputs,
                      ref_t i, ref_t ref, void *extra);
int acirc_eval_consts(acirc_t *c, long *vals, size_t n);
int acirc_eval_outputs(acirc_t *c, ref_t *refs, size_t n);
int acirc_eval_test(acirc_t *c, char *in, char *out);
int acirc_eval_symlens(acirc_t *c, size_t *vals, size_t n);

#pragma GCC visibility pop
