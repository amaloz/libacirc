#include "storage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct data_t {
    void *value;
    pthread_mutex_t lock;
    ssize_t count;
    bool mine, save;
};

static char *
filename(const storage_t *m, size_t ref)
{
    char *fname = NULL;
    int length;

    if (m->dirname == NULL)
        return NULL;

    length = snprintf(NULL, 0, "%s/%lu", m->dirname, ref);
    fname = calloc(length, sizeof fname[0]);
    (void) snprintf(fname, length, "%s/%lu", m->dirname, ref);
    return fname;
}

int
storage_init(storage_t *m, size_t nrefs, const char *dirname)
{
    m->nrefs = nrefs;
    m->array = calloc(m->nrefs, sizeof m->array[0]);
    m->dirname = dirname;
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
    m->array[ref].count = count;
    m->array[ref].mine = mine;
    m->array[ref].save = save;
    pthread_mutex_unlock(&m->array[ref].lock);
    return ACIRC_OK;
}

void *
storage_get(storage_t *m, size_t ref, acirc_fread_f fread_f)
{
    void *data;

    if (ref >= m->nrefs)
        return NULL;
    pthread_mutex_lock(&m->array[ref].lock);
    data = m->array[ref].value;
    if (data == NULL) {
        char *fname = NULL;
        FILE *fp = NULL;

        if ((fname = filename(m, ref)) == NULL)
            goto cleanup;
        if ((fp = fopen(fname, "r")) == NULL)
            goto cleanup;
        data = m->array[ref].value = fread_f(fp);
    cleanup:
        if (fp)
            fclose(fp);
        if (fname)
            free(fname);
    }
    pthread_mutex_unlock(&m->array[ref].lock);
    return data;
}

bool
storage_update_item_count(storage_t *m, size_t ref)
{
    bool result;
    pthread_mutex_lock(&m->array[ref].lock);
    result = m->array[ref].count > 0 && --m->array[ref].count == 0;
    pthread_mutex_unlock(&m->array[ref].lock);
    return result;
}

void
storage_remove_item(storage_t *m, size_t ref, acirc_fwrite_f fwrite_f)
{
    if (ref >= m->nrefs)
        return;
    assert(m->array[ref].count == 0);
    pthread_mutex_lock(&m->array[ref].lock);
    if (m->array[ref].save) {
        char *fname = NULL;
        FILE *fp = NULL;

        if ((fname = filename(m, ref)) == NULL)
            goto cleanup;
        if ((fp = fopen(fname, "w")) == NULL)
            goto cleanup;
        fwrite_f(m->array[ref].value, fp);
    cleanup:
        if (fp)
            fclose(fp);
        if (fname)
            free(fname);
    }
    m->array[ref].value = NULL;
    m->array[ref].count = 0;
    m->array[ref].mine = false;
    pthread_mutex_unlock(&m->array[ref].lock);
}
