#pragma once

#include "_acirc.h"

#pragma GCC visibility push(hidden)

map_t * map_new(void);
void map_free(map_t *m, acirc_free_f f, void *extra);
int map_put(map_t *m, ref_t ref, void *value);
void * map_get(map_t *m, ref_t ref);

#pragma GCC visibility pop
