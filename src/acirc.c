#include "_acirc.h"
#include "parse.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    acirc_eval_f eval;
    acirc_free_f free;
    storage_t *map;
    acirc_op op;
    ref_t ref;
    ssize_t count;
    ref_t xref;
    ref_t yref;
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
    free(c);
}

long
acirc_const(acirc_t *c, size_t i)
{
    return c->consts[i];
}

void **
acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
               acirc_eval_f eval_f, acirc_copy_f copy_f, acirc_free_f free_f,
               void *extra, size_t nthreads)
{
    void **outputs;

    storage_init(&c->map, c->nrefs);
    if (nthreads)
        c->pool = threadpool_create(nthreads);
    if (yyparse(c, input_f, const_f, eval_f, free_f, extra) != 0) {
        fprintf(stderr, "error: parsing circuit failed\n");
        return NULL;
    }
    if (nthreads) {
        threadpool_destroy(c->pool);
        c->pool = NULL;
    }

    outputs = calloc(acirc_noutputs(c), sizeof outputs[0]);
    for (size_t i = 0; i < acirc_noutputs(c); ++i) {
        bool done = false;
        void *out = storage_get(&c->map, c->outrefs[i]);
        outputs[i] = copy_f ? copy_f(out, extra) : out;
        if (done) {
            storage_remove_item(&c->map, c->outrefs[i]);
            if (free_f)
                free_f(out, extra);
        }
    }
    storage_clear(&c->map, free_f, extra);
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
        printf("\n  want: ");
        array_printstring(acirc_test_output(c, t), acirc_noutputs(c));
        printf("\n  got:  ");
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
acirc_eval_input(acirc_t *c, acirc_input_f f, ref_t ref, size_t inp, ssize_t count, void *extra)
{
    void *value;
    value = f ? f(inp, extra) : NULL;
    return storage_put(&c->map, ref, value, count, f ? true : false);
}

int
acirc_eval_const(acirc_t *c, acirc_const_f f, ref_t ref, ssize_t count, void *extra)
{
    void *value;
    if (f) {
        value = f(c->_nconsts, c->consts[c->_nconsts], extra);
        if (++c->_nconsts == c->nconsts)
            c->_nconsts = 0;
    } else {
        value = NULL;
    }
    return storage_put(&c->map, ref, value, count, f ? true : false);
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
    void *out, *x = NULL, *y = NULL;
    bool x_done = false, y_done = false;
    eval_args_t *args = vargs;

    while (x == NULL) {
        x = storage_get(args->map, args->xref);
    }
    while (y == NULL) {
        y = storage_get(args->map, args->yref);
    }

    out = args->eval(args->op, x, y, args->extra);

    {
        x_done = storage_update_item_count(args->map, args->xref);
        y_done = storage_update_item_count(args->map, args->yref);
    }

    {
        storage_put(args->map, args->ref, out, args->count, true);
    }

    if (x_done) {
        storage_remove_item(args->map, args->xref);
        if (args->free)
            args->free(x, args->extra);
    }
    if (y_done) {
        storage_remove_item(args->map, args->yref);
        if (args->free)
            args->free(y, args->extra);
    }
    free(args);
}

int
acirc_eval_gate(acirc_t *c, acirc_eval_f eval_f, acirc_free_f free_f, acirc_op op,
                ref_t ref, ref_t xref, ref_t yref, ssize_t count, void *extra)
{
    bool x_done = false, y_done = false;
    if (c->pool) {
        eval_args_t *args;
        args = calloc(1, sizeof args[0]);
        args->eval = eval_f;
        args->free = free_f;
        args->map = &c->map;
        args->op = op;
        args->ref = ref;
        args->count = count;
        args->xref = xref;
        args->yref = yref;
        args->extra = extra;
        threadpool_add_job(c->pool, eval_worker, args);
    } else {
        void *x, *y, *out;
        x = storage_get(&c->map, xref);
        y = storage_get(&c->map, yref);
        out = eval_f(op, x, y, extra);
        storage_put(&c->map, ref, out, count, true);
        if (x_done) {
            storage_remove_item(&c->map, xref);
            if (free_f)
                free_f(x, extra);
        }
        if (y_done) {
            storage_remove_item(&c->map, yref);
            if (free_f)
                free_f(y, extra);
        }
    }
    return ACIRC_OK;
}

char *
acirc_op2str(acirc_op op)
{
    switch (op) {
    case ACIRC_OP_ADD:
        return "ADD";
    case ACIRC_OP_SUB:
        return "SUB";
    case ACIRC_OP_MUL:
        return "MUL";
    }
    return "";
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

