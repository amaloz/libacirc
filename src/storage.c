#include "storage.h"

#include <assert.h>
#include <stdlib.h>

typedef struct {
    void *value;
    ssize_t count;
} data_t;

int
storage_init(storage_t *m)
{
    map_init(&m->map);
    return ACIRC_OK;
}

void
storage_clear(storage_t *m, acirc_free_f f, void *extra)
{
    if (m == NULL)
        return;
    map_iter_t iter;
    const char *key;
    iter = map_iter(&m->map);
    while ((key = map_next(&m->map, &iter))) {
        data_t *data = *map_get(&m->map, key);
        if (f)
            f(data->value, extra);
        free(data);
    }
    map_deinit(&m->map);
}

int
storage_put(storage_t *m, ref_t ref, void *value, ssize_t count)
{
    char key[100];
    data_t *data = calloc(1, sizeof data[0]);
    snprintf(key, sizeof key, "%lu", ref);
    data->value = value;
    data->count = count;
    map_set(&m->map, key, data);
    return ACIRC_OK;
}

void *
storage_get(storage_t *m, ref_t ref, bool *done)
{
    (void) done;
    data_t **data;
    char key[100];
    snprintf(key, sizeof key, "%lu", ref);
    data = (data_t **) map_get(&m->map, key);
    if (data == NULL)
        return NULL;
    return (*data)->value;
}

bool
storage_update_item_count(storage_t *m, ref_t ref)
{
    data_t *data;
    char key[100];
    snprintf(key, sizeof key, "%lu", ref);
    data = *map_get(&m->map, key);
    if (data->count > 0 && --data->count == 0)
        return true;
    else
        return false;
}

void
storage_remove_item(storage_t *m, ref_t ref)
{
    data_t *data;
    char key[100];
    snprintf(key, sizeof key, "%lu", ref);
    data = *map_get(&m->map, key);
    free(data);
    map_remove(&m->map, key);
}
