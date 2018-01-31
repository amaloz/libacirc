#pragma once

#include "acirc.h"

#include <pthread.h>
#include <stdbool.h>

typedef struct data_t data_t;

typedef struct {
    data_t *array;
    size_t nrefs;
} storage_t;

#pragma GCC visibility push(hidden)

int storage_init(storage_t *, size_t nrefs);
void storage_clear(storage_t *m, acirc_free_f f, void *extra);

int storage_put(storage_t *m, size_t ref, void *value, ssize_t count, bool mine,
                bool save);
void * storage_get(storage_t *m, size_t ref, acirc_read_f read_f, void *extra);

void storage_remove_item(storage_t *m, size_t ref, acirc_write_f write_f,
                         void *extra);
bool storage_update_item_count(storage_t *m, size_t ref);


#pragma GCC visibility pop
