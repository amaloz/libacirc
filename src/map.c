#include "map.h"
#include <stdlib.h>

struct map_t {
    void **values;
    size_t n;
};

map_t *
map_new(void)
{
    map_t *m;

    m = calloc(1, sizeof m[0]);
    m->n = 10;
    m->values = calloc(m->n, sizeof m->values[0]);
    return m;
}

void
map_free(map_t *m)
{
    if (m == NULL)
        return;
    if (m->values)
        free(m->values);
    free(m);
}

int
map_put(map_t *m, ref_t ref, void *value)
{
    if (ref >= m->n) {
        m->n *= 2;
        m->values = realloc(m->values, m->n * sizeof m->values[0]);
    }
    m->values[ref] = value;
    return ACIRC_OK;
}

void *
map_get(map_t *m, ref_t ref)
{
    return m->values[ref];
}
