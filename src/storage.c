#include "storage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct data_t {
    void *value;
    ssize_t count;
    bool mine;
};

int
storage_init(storage_t *m, size_t nrefs)
{
    m->nrefs = nrefs;
    m->array = calloc(m->nrefs, sizeof m->array[0]);
    pthread_mutex_init(&m->lock, NULL);
    return ACIRC_OK;
}

void
storage_clear(storage_t *m, acirc_free_f f, void *extra)
{
    if (m == NULL)
        return;
    if (f) {
        for (size_t i = 0; i < m->nrefs; ++i) {
            if (m->array[i].value && m->array[i].mine)
                f(m->array[i].value, extra);
        }
    }
    free(m->array);
    pthread_mutex_destroy(&m->lock);
}

int
storage_put(storage_t *m, ref_t ref, void *value, ssize_t count, bool mine)
{
    if (ref >= m->nrefs)
        return ACIRC_ERR;
    m->array[ref].value = value;
    m->array[ref].count = count;
    m->array[ref].mine = mine;
    return ACIRC_OK;
}

void *
storage_get(storage_t *m, ref_t ref)
{
    if (ref >= m->nrefs)
        return NULL;
    return m->array[ref].value;
}

bool
storage_update_item_count(storage_t *m, ref_t ref)
{
    bool result;
    pthread_mutex_lock(&m->lock);
    result = m->array[ref].count > 0 && --m->array[ref].count == 0;
    pthread_mutex_unlock(&m->lock);
    return result;
}

void
storage_remove_item(storage_t *m, ref_t ref)
{
    if (ref >= m->nrefs)
        return;
    m->array[ref].value = NULL;
    m->array[ref].count = 0;
    m->array[ref].mine = false;
}
