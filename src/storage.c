#include "storage.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

struct data_t {
    void *value;
    pthread_mutex_t lock;
    _Atomic unsigned long count;
    bool mine, save;
};

int
storage_init(storage_t *m, size_t nrefs)
{
    m->nrefs = nrefs;
    m->array = calloc(m->nrefs, sizeof m->array[0]);
    for (size_t i = 0; i < m->nrefs; ++i) {
        pthread_mutex_init(&m->array[i].lock, NULL);
        pthread_mutex_lock(&m->array[i].lock);
    }
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
            pthread_mutex_destroy(&m->array[i].lock);
        }
    }
    free(m->array);
}

int
storage_put(storage_t *m, size_t ref, void *value, ssize_t count,
            bool mine, bool save)
{
    if (ref >= m->nrefs)
        return ACIRC_ERR;
    m->array[ref].value = value;
    m->array[ref].count = ATOMIC_VAR_INIT(count);
    m->array[ref].mine = mine;
    m->array[ref].save = save;
    pthread_mutex_unlock(&m->array[ref].lock);
    return ACIRC_OK;
}

void *
storage_get(storage_t *m, size_t ref, acirc_read_f read_f, void *extra)
{
    void *data;

    if (ref >= m->nrefs)
        return NULL;
    pthread_mutex_lock(&m->array[ref].lock);
    data = m->array[ref].value;
    if (data == NULL && read_f)
        data = m->array[ref].value = read_f(ref, extra);
    pthread_mutex_unlock(&m->array[ref].lock);
    return data;
}

bool
storage_update_item_count(storage_t *m, size_t ref)
{
    return atomic_fetch_sub(&m->array[ref].count, 1) == 1;
}

void
storage_remove_item(storage_t *m, size_t ref, acirc_write_f write_f,
                    void *extra)
{
    if (ref >= m->nrefs)
        return;
    /* Don't need to lock, as at this point no other thread will be accessing
     * this item */
    if (m->array[ref].save && write_f)
        (void) write_f(ref, m->array[ref].value, extra);
    m->array[ref].value = NULL;
    m->array[ref].mine = false;
}
