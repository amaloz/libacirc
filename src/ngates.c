#include "_acirc.h"
#include "parse.h"
#include "map.h"

static void *
_eval_f(acirc_op op, void *x, void *y, void *count_)
{
    (void) op; (void) x; (void) y;
    unsigned long *count = count_;
    (*count)++;
    return NULL;
}

unsigned long
acirc_ngates(acirc_t *c)
{
    unsigned long count = 0;
    acirc_traverse(c, NULL, NULL, _eval_f, &count);
    return count;
}
