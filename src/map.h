#pragma once

#include "_acirc.h"

typedef struct map_t map_t;

map_t * map_new(void);
void map_free(map_t *m);
int map_put(map_t *m, ref_t ref, void *value);
void * map_get(map_t *m, ref_t ref);
