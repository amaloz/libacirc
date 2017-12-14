#pragma once

#include "acirc.h"

typedef unsigned int ref_t;

acirc_op acirc_str2op(const char *str);
int acirc_eval_input(acirc_t *c, ref_t ref, ref_t inp);
int acirc_eval_const(acirc_t *c, ref_t ref, int val);
int acirc_eval_gate(acirc_t *c, acirc_op op, ref_t ref, ref_t x, ref_t y, acirc_eval_f f, void *extra);
int acirc_eval_outputs(acirc_t *c, ref_t *refs, size_t n);
