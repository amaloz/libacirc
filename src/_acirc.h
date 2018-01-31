#pragma once

#include "acirc.h"
#include "storage.h"

#include <stdbool.h>
#include <threadpool.h>

typedef enum {
    REF_STD,
    REF_SAVE,
    REF_SKIP,
} ref_state_e;

#pragma GCC visibility push(hidden)

typedef size_t ref_t;

typedef struct {
    long *inps;
    long *outs;
} acirc_test_t;

struct acirc_t {
    FILE *fp;
    char *fname;
    size_t ninputs;
    size_t nrefs;
    size_t nconsts;
    size_t nsecrets;
    size_t noutputs;
    size_t nsymbols;
    size_t ntests;
    bool binary;                /* true if the circuit is binary */

    long *consts;               /* [nconsts] */
    long *secrets;              /* [nsecrets] */
    size_t *symlens;            /* [nsymbols] */
    size_t *sigmas;             /* [nsymbols] */
    ref_t *outrefs;             /* [noutputs] */
    acirc_test_t *tests;        /* [ntests] */

    bool circuit;               /* true if we are done parsing the prelim section */

    storage_t map;
    threadpool *pool;
    size_t _ngates;
    size_t _nmuls;
    size_t _max_depth;
    size_t _max_degree;
    size_t _max_const_degree;
    size_t _delta;
};

int acirc_eval_input(acirc_t *c, acirc_input_f f, ref_t ref, size_t idx,
                     ssize_t count, void *extra);
int acirc_eval_const(acirc_t *c, acirc_const_f f, ref_t ref, size_t idx,
                     ssize_t count, void *extra);
int acirc_eval_secret(acirc_t *c, acirc_const_f f, ref_t ref, size_t idx,
                      ssize_t count, void *extra);
int acirc_eval_gate(acirc_t *c, acirc_eval_f eval_f, acirc_free_f free_f,
                    acirc_fwrite_f fwrite_f, acirc_fread_f fread_f,
                    acirc_op op, ref_t ref, ref_t x, ref_t y, ssize_t count,
                    ref_state_e state, void *extra);
int acirc_eval_output(acirc_t *c, acirc_output_f output_f, void **outputs,
                      ref_t i, ref_t ref, void *extra);
int acirc_eval_consts(acirc_t *c, long *vals, size_t n);
int acirc_eval_secrets(acirc_t *c, long *vals, size_t n);
int acirc_eval_symlens(acirc_t *c, size_t *vals, size_t n);
int acirc_eval_sigmas(acirc_t *c, size_t *vals, size_t n);
int acirc_eval_outputs(acirc_t *c, ref_t *refs, size_t n);
int acirc_eval_test(acirc_t *c, char *in, char *out);

#pragma GCC visibility pop
