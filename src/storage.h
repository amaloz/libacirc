#pragma once

#include "acirc.h"

#include <pthread.h>
#include <stdbool.h>

typedef size_t ref_t;
typedef struct data_t data_t;

typedef struct {
    data_t *array;
    size_t nrefs;
} storage_t;

#pragma GCC visibility push(hidden)

int storage_init(storage_t *, size_t nrefs);
void storage_clear(storage_t *m, acirc_free_f f, void *extra);
int storage_put(storage_t *m, ref_t ref, void *value, ssize_t count, bool mine);
void * storage_get(storage_t *m, ref_t ref);
void storage_remove_item(storage_t *m, ref_t ref);
bool storage_update_item_count(storage_t *m, ref_t ref);


#pragma GCC visibility pop
