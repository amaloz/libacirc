#pragma once

#include "acirc.h"
#include <threadpool.h>

typedef unsigned int ref_t;
typedef struct map_t map_t;

typedef struct {
    acirc_op op;
    unsigned int x, y;
} gate_t;

struct acirc_t {
    FILE *fp;
    size_t elemsize;
    size_t ninputs;
    size_t nconsts;
    size_t noutputs;
    void **inputs;
    ref_t *outrefs;
    size_t nread;
    map_t *map;
};

acirc_op acirc_str2op(const char *str);
int acirc_eval_input(acirc_t *c, acirc_input_f f, ref_t ref, size_t inp, void *extra);
int acirc_eval_const(acirc_t *c, acirc_const_f f, ref_t ref, int val, void *extra);
int acirc_eval_gate(acirc_t *c, acirc_eval_f f, acirc_op op, ref_t ref, ref_t x, ref_t y,
                    threadpool *pool, void *extra);
int acirc_eval_outputs(acirc_t *c, ref_t *refs, size_t n);
