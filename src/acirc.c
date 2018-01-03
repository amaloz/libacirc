#include "_acirc.h"
#include "parse.h"
#include "map.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    acirc_eval_f eval;
    threadpool *pool;
    map_t *map;
    acirc_op op;
    ref_t ref;
    void *x;
    void *y;
    void *extra;
} eval_args_t;

extern FILE *yyin;

size_t
acirc_ninputs(const acirc_t *c)
{
    return c->ninputs;
}

size_t
acirc_nconsts(const acirc_t *c)
{
    return c->nconsts;
}

size_t
acirc_noutputs(const acirc_t *c)
{
    return c->noutputs;
}

acirc_t *
acirc_new(const char *fname)
{
    acirc_t *c;

    if ((c = calloc(1, sizeof c[0])) == NULL)
        return NULL;
    if ((c->fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "error: unable to open file '%s'\n", fname);
        goto error;
    }

    {
        jmp_buf env;
        setjmp(env);
        yyin = c->fp;
        if (yyparse(c, NULL, NULL, NULL, NULL, NULL, env) != 0) {
            fprintf(stderr, "error: parsing circuit failed\n");
            goto error;
        }
    }
    c->map = map_new();
    return c;
error:
    acirc_free(c);
    return NULL;
}

void
acirc_free(acirc_t *c)
{
    if (c == NULL)
        return;
    if (c->fp)
        fclose(c->fp);
    if (c->outrefs)
        free(c->outrefs);
    if (c->map)
        free(c->map);
    free(c);
}

int
acirc_const(acirc_t *c, size_t i)
{
    return c->consts[i];
}

void *
acirc_output(acirc_t *c, size_t i)
{
    return map_get(c->map, c->outrefs[i]);
}

int
acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
               acirc_eval_f eval_f, void *extra)
{
    jmp_buf env;
    setjmp(env);
    if (yyparse(c, input_f, const_f, eval_f, extra, NULL, env) != 0) {
        fprintf(stderr, "error: parsing circuit failed\n");
        return ACIRC_ERR;
    }
    fseek(c->fp, 0, SEEK_SET);
    return ACIRC_OK;
}

/*
 * circuit parsing functions
 */

int
acirc_eval_input(acirc_t *c, acirc_input_f f, ref_t ref, size_t inp, void *extra)
{
    if (f == NULL)
        return ACIRC_OK;
    map_put(c->map, ref, f(inp, extra));
    return ACIRC_OK;
}

int
acirc_eval_const(acirc_t *c, acirc_const_f f, ref_t ref, int val, void *extra)
{
    if (f == NULL)
        return ACIRC_OK;
    map_put(c->map, ref, f(ref, val, extra));
    return ACIRC_OK;
}

int
acirc_eval_consts(acirc_t *c, int *vals, size_t n)
{
    c->consts = vals;
    c->nconsts = n;
    return ACIRC_OK;
}

int
acirc_eval_outputs(acirc_t *c, ref_t *refs, size_t n)
{
    c->outrefs = refs;
    c->noutputs = n;
    return ACIRC_OK;
}


static void
eval_worker(void *vargs)
{
    void *out;
    eval_args_t *args = vargs;

    out = args->eval(args->op, args->x, args->y, args->extra);
    {
        map_put(args->map, args->ref, out);
    }
    free(args);
}

int
acirc_eval_gate(acirc_t *c, acirc_eval_f f, acirc_op op, ref_t ref, ref_t x, ref_t y,
                threadpool *pool, void *extra)
{
    void *x_, *y_, *_out;
    x_ = map_get(c->map, x);
    y_ = map_get(c->map, y);
    if (pool) {
        eval_args_t *args;
        args = calloc(1, sizeof args[0]);
        args->eval = f;
        args->pool = pool;
        args->map = c->map;
        args->op = op;
        args->ref = ref;
        args->x = x_;
        args->y = y_;
        args->extra = extra;
        threadpool_add_job(pool, eval_worker, args);
    } else {
        _out = f(op, x_, y_, extra);
        map_put(c->map, ref, _out);
    }
    return ACIRC_OK;
}

acirc_op
acirc_str2op(const char *s)
{
    if (strcmp(s, "ADD") == 0) {
        return ACIRC_OP_ADD;
    } else if (strcmp(s, "SUB") == 0) {
        return ACIRC_OP_SUB;
    } else if (strcmp(s, "MUL") == 0) {
        return ACIRC_OP_MUL;
    } else {
        fprintf(stderr, "error: unknown acirc operation '%s'\n", s);
        abort();
    }
}

