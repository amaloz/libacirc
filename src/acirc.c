#include "_acirc.h"
#include "parse.h"
#include "map.h"

#include <ctype.h>
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

size_t
acirc_ntests(const acirc_t *c)
{
    return c->ntests;
}

long *
acirc_test_input(const acirc_t *c, size_t i)
{
    return c->tests[i].inps;
}

long *
acirc_test_output(const acirc_t *c, size_t i)
{
    return c->tests[i].outs;
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

    yyin = c->fp;
    if (yyparse(c, NULL, NULL, NULL, NULL, NULL) != 0) {
        fprintf(stderr, "error: parsing circuit failed\n");
        goto error;
    }
    return c;
error:
    acirc_free(c, NULL);
    return NULL;
}

void
acirc_free(acirc_t *c, acirc_free_f f)
{
    if (c == NULL)
        return;
    if (c->fp)
        fclose(c->fp);
    if (c->consts)
        free(c->consts);
    if (c->outrefs)
        free(c->outrefs);
    if (c->tests) {
        for (size_t i = 0; i < c->ntests; ++i) {
            free(c->tests[i].inps);
            free(c->tests[i].outs);
        }
        free(c->tests);
    }
    if (c->map)
        map_free(c->map, f);
    free(c);
}

long
acirc_const(acirc_t *c, size_t i)
{
    return c->consts[i];
}

static void *
acirc_output(acirc_t *c, size_t i, acirc_copy_f f)
{
    void *res;

    res = map_get(c->map, c->outrefs[i]);
    return f ? f(res) : res;
}

void **
acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
               acirc_eval_f eval_f, acirc_copy_f copy_f, acirc_free_f free_f,
               void *extra)
{
    void **outputs;
    c->map = map_new();
    if (yyparse(c, input_f, const_f, eval_f, extra, NULL) != 0) {
        fprintf(stderr, "error: parsing circuit failed\n");
        return NULL;
    }
    outputs = calloc(acirc_noutputs(c), sizeof outputs[0]);
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        outputs[i] = acirc_output(c, i, copy_f);
    }
    map_free(c->map, free_f);
    c->map = NULL;
    fseek(c->fp, 0, SEEK_SET);
    return outputs;
}

static void
array_printstring(long *xs, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        printf("%ld", xs[i]);
}

bool
acirc_test(acirc_t *c)
{
    bool ok = true;
    for (size_t t = 0; t < acirc_ntests(c); ++t) {
        long *outputs;
        bool test_ok = true;

        outputs = acirc_eval(c, acirc_test_input(c, t), NULL);
        for (size_t o = 0; o < acirc_noutputs(c); ++o) {
            test_ok = test_ok && (outputs[o] == acirc_test_output(c, t)[o]);
        }

        if (!test_ok)
            printf("\033[1;41m");
        printf("test #%lu: ", t + 1);
        array_printstring(acirc_test_input(c, t), acirc_ninputs(c));
        printf("\n\twant: ");
        array_printstring(acirc_test_output(c, t), acirc_noutputs(c));
        printf("\n\tgot:  ");
        for (size_t o = 0; o < acirc_noutputs(c); ++o) {
            printf("%ld", outputs[o]);
        }
        if (!test_ok)
            printf("\033[0m");
        printf("\n");

        ok = ok && test_ok;
        free(outputs);
    }
    return ok;
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
acirc_eval_const(acirc_t *c, acirc_const_f f, ref_t ref, long val, void *extra)
{
    if (f == NULL)
        return ACIRC_OK;
    map_put(c->map, ref, f(ref - acirc_ninputs(c), val, extra));
    return ACIRC_OK;
}

int
acirc_eval_consts(acirc_t *c, long *vals, size_t n)
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

static long
char2long(char c)
{
    if (toupper(c) >= 'A' && toupper(c) <= 'Z')
        return toupper(c) - 'A' + 10;
    else if (c >= '0' && c <= '9')
        return c - '0';
    else {
        fprintf(stderr, "error: invalid test input '%c'\n", c);
        return -1;
    }
}

int
acirc_eval_test(acirc_t *c, char *in, char *out)
{
    if (strlen(in) != acirc_ninputs(c)) {
        fprintf(stderr, "error: test input length ≠ %lu\n", acirc_ninputs(c));
        return ACIRC_ERR;
    }
    if (strlen(out) != acirc_noutputs(c)) {
        fprintf(stderr, "error: test output length ≠ %lu\n", acirc_noutputs(c));
        return ACIRC_ERR;
    }
    c->tests = realloc(c->tests, sizeof c->tests[0] * (c->ntests + 1));
    c->tests[c->ntests].inps = calloc(acirc_ninputs(c), sizeof(long));
    c->tests[c->ntests].outs = calloc(acirc_noutputs(c), sizeof(long));
    for (size_t i = 0; i < acirc_ninputs(c); ++i) {
        c->tests[c->ntests].inps[i] = char2long(in[i]);
    }
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        c->tests[c->ntests].outs[i] = char2long(out[i]);
    }
    c->ntests++;
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

