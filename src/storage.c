#include "storage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct data_t {
    void *value;
    ssize_t count;
};

int
storage_init(storage_t *m)
{
    m->n = 1;
    m->array = calloc(m->n, sizeof m->array[0]);
    return ACIRC_OK;
}

void
storage_clear(storage_t *m, acirc_free_f f, void *extra)
{
    if (m == NULL)
        return;
    if (f) {
        for (size_t i = 0; i < m->n; ++i) {
            if (m->array[i].value)
                f(m->array[i].value, extra);
        }
    }
    free(m->array);
}

int
storage_put(storage_t *m, ref_t ref, void *value, ssize_t count)
{
    while (ref >= m->n) {
        data_t *tmp;
        tmp = calloc(2 * m->n, sizeof m->array[0]);
        memcpy(tmp, m->array, m->n * sizeof m->array[0]);
        free(m->array);
        m->array = tmp;
        m->n *= 2;
    }
    m->array[ref].value = value;
    m->array[ref].count = count;
    return ACIRC_OK;
}

void *
storage_get(storage_t *m, ref_t ref)
{
    return m->array[ref].value;
}

bool
storage_update_item_count(storage_t *m, ref_t ref)
{
    return m->array[ref].count > 0 && --m->array[ref].count == 0;
}

void
storage_remove_item(storage_t *m, ref_t ref)
{
    m->array[ref].value = NULL;
    m->array[ref].count = 0;
}
