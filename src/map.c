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
map_free(map_t *m, acirc_free_f f, void *extra)
{
    if (m == NULL)
        return;
    if (m->values) {
        if (f)
            for (size_t i = 0; i < m->n; ++i) {
                f(m->values[i], extra);
            }
        free(m->values);
    }
    free(m);
}

int
map_put(map_t *m, ref_t ref, void *value)
{
    if (ref >= m->n) {
        m->n = ref + 1;
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
