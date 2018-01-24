#include "_acirc.h"
#include "parse.h"

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

typedef struct {
    acirc_output_f output;
    acirc_free_f free;
    void **outputs;
    storage_t *map;
    ref_t i;
    ref_t ref;
    void *extra;
} output_args_t;

extern FILE *yyin;

bool
acirc_is_binary(const acirc_t *c)
{
    return c->binary;
}

size_t
acirc_nrefs(const acirc_t *c)
{
    return c->nrefs;
}

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
acirc_nsecrets(const acirc_t *c)
{
    return c->nsecrets;
}

size_t
acirc_noutputs(const acirc_t *c)
{
    return c->noutputs;
}

size_t
acirc_nsymbols(const acirc_t *c)
{
    return c->nsymbols;
}

size_t
acirc_ntests(const acirc_t *c)
{
    return c->ntests;
}

size_t
acirc_symlen(const acirc_t *c, size_t i)
{
    if (i >= c->nsymbols)
        return 0;
    return c->symlens[i];
}

bool
acirc_is_sigma(const acirc_t *c, size_t i)
{
    if (i >= c->nsymbols)
        return 0;
    return c->sigmas[i] ? true : false;
}

long
acirc_const(acirc_t *c, size_t i)
{
    if (i >= c->nconsts)
        return 0;
    return c->consts[i];
}

long
acirc_secret(acirc_t *c, size_t i)
{
    if (i >= c->nsecrets)
        return 0;
    return c->secrets[i];
}

long *
acirc_test_input(const acirc_t *c, size_t i)
{
    if (i >= c->ntests)
        return NULL;
    return c->tests[i].inps;
}

long *
acirc_test_output(const acirc_t *c, size_t i)
{
    if (i >= c->ntests)
        return NULL;
    return c->tests[i].outs;
}

acirc_t *
acirc_new(const char *fname, bool mmapped)
{
    acirc_t *c;

    if ((c = calloc(1, sizeof c[0])) == NULL)
        return NULL;

    if (mmapped) {
        int fd, len;
        struct stat st;
        void *buf;

        if ((fd = open(fname, O_RDONLY)) == -1) {
            fprintf(stderr, "error: unable to open file '%s'\n", fname);
            goto error;
        }
        stat(fname, &st);
        len = st.st_size;
        buf = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        if (buf == NULL) {
            fprintf(stderr, "error: unable to memory map file\n");
            goto error;
        }
        if ((c->fp = fmemopen(buf, len, "r")) == NULL) {
            fprintf(stderr, "error: unable to open memory mapped region\n");
            goto error;
        }
    } else {
        if ((c->fp = fopen(fname, "r")) == NULL) {
            fprintf(stderr, "error: unable to open file '%s'\n", fname);
            goto error;
        }
    }

    /* set defaults */
    c->_max_const_degree = -1;

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
    if (c->secrets)
        free(c->secrets);
    if (c->outrefs)
        free(c->outrefs);
    if (c->symlens)
        free(c->symlens);
    if (c->sigmas)
        free(c->sigmas);
    if (c->tests) {
        for (size_t i = 0; i < c->ntests; ++i) {
            free(c->tests[i].inps);
            free(c->tests[i].outs);
        }
        free(c->tests);
    }
    free(c);
}

void **
acirc_traverse(acirc_t *c, acirc_input_f input_f, acirc_const_f const_f,
               acirc_eval_f eval_f, acirc_output_f output_f,
               acirc_free_f free_f, void *extra, size_t nthreads)
{
    void **outputs;

    storage_init(&c->map, c->nrefs);
    c->pool = nthreads ? threadpool_create(nthreads) : NULL;
    if (yyparse(c, input_f, const_f, eval_f, free_f, extra) != 0) {
        fprintf(stderr, "error: parsing circuit failed\n");
        return NULL;
    }
    if ((outputs = calloc(acirc_noutputs(c), sizeof outputs[0])) == NULL) {
        fprintf(stderr, "error: memory allocation failed\n");
        return NULL;
    }
    for (size_t i = 0; i < acirc_noutputs(c); ++i)
        acirc_eval_output(c, output_f, outputs, i, c->outrefs[i], extra);
    if (c->pool)
        threadpool_destroy(c->pool);
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

        if ((outputs = acirc_eval(c, acirc_test_input(c, t), NULL)) == NULL)
            return false;
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
        array_printstring(outputs, acirc_noutputs(c));
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
acirc_eval_input(acirc_t *c, acirc_input_f f, ref_t ref, size_t idx,
                 ssize_t count, void *extra)
{
    void *value;
    if (idx >= c->ninputs)
        return ACIRC_ERR;
    value = f ? f(ref, idx, extra) : NULL;
    return storage_put(&c->map, ref, value, count, f ? true : false);
}

int
acirc_eval_const(acirc_t *c, acirc_const_f f, ref_t ref, size_t idx,
                 ssize_t count, void *extra)
{
    void *value;
    if (idx >= c->nconsts)
        return ACIRC_ERR;
    value = f ? f(ref, idx, c->consts[idx], extra) : NULL;
    return storage_put(&c->map, ref, value, count, f ? true : false);
}

int
acirc_eval_secret(acirc_t *c, acirc_const_f f, ref_t ref, size_t idx,
                  ssize_t count, void *extra)
{
    void *value;
    if (idx >= c->nsecrets)
        return ACIRC_ERR;
    /* secrets are stored within the constants array */
    value = f ? f(ref, c->nconsts + idx, c->secrets[idx], extra) : NULL;
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
acirc_eval_secrets(acirc_t *c, long *vals, size_t n)
{
    c->secrets = vals;
    c->nsecrets = n;
    return ACIRC_OK;
}

int
acirc_eval_symlens(acirc_t *c, size_t *vals, size_t n)
{
    c->symlens = vals;
    c->nsymbols = n;
    return ACIRC_OK;
}

int
acirc_eval_sigmas(acirc_t *c, size_t *vals, size_t n)
{
    (void) n;
    c->sigmas = vals;
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
    if ((c->tests = realloc(c->tests, sizeof c->tests[0] * (c->ntests + 1))) == NULL)
        return ACIRC_ERR;
    c->tests[c->ntests].inps = calloc(acirc_ninputs(c), sizeof(long));
    c->tests[c->ntests].outs = calloc(acirc_noutputs(c), sizeof(long));
    for (size_t i = 0; i < acirc_ninputs(c); ++i)
        c->tests[c->ntests].inps[i] = char2long(in[i]);
    for (size_t i = 0; i < acirc_noutputs(c); ++i)
        c->tests[c->ntests].outs[i] = char2long(out[i]);
    c->ntests++;
    return ACIRC_OK;
}


static void
eval_worker(void *vargs)
{
    void *rop, *x, *y;
    bool x_done, y_done;
    eval_args_t *args = vargs;

    x = storage_get(args->map, args->xref);
    y = storage_get(args->map, args->yref);

    rop = args->eval(args->ref, args->op, args->xref, x, args->yref, y, args->extra);

    x_done = storage_update_item_count(args->map, args->xref);
    y_done = storage_update_item_count(args->map, args->yref);

    storage_put(args->map, args->ref, rop, args->count, true);

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
acirc_eval_gate(acirc_t *c, acirc_eval_f eval_f, acirc_free_f free_f,
                acirc_op op, ref_t ref, ref_t xref, ref_t yref,
                ssize_t count, void *extra)
{
    eval_args_t *args;
    if ((args = calloc(1, sizeof args[0])) == NULL)
        return ACIRC_ERR;
    args->eval = eval_f;
    args->free = free_f;
    args->map = &c->map;
    args->op = op;
    args->ref = ref;
    args->count = count;
    args->xref = xref;
    args->yref = yref;
    args->extra = extra;
    if (c->pool)
        threadpool_add_job(c->pool, eval_worker, args);
    else
        eval_worker(args);
    return ACIRC_OK;
}

static void
output_worker(void *vargs)
{
    output_args_t *args = vargs;
    void *x;
    x = storage_get(args->map, args->ref);
    args->outputs[args->i] = args->output ? args->output(args->ref, args->i, x, args->extra) : x;
    free(args);
}

int
acirc_eval_output(acirc_t *c, acirc_output_f output_f, void **outputs, ref_t i,
                  ref_t ref, void *extra)
{
    output_args_t *args;
    if ((args = calloc(1, sizeof args[0])) == NULL)
        return ACIRC_ERR;
    args->output = output_f;
    args->outputs = outputs;
    args->map = &c->map;
    args->i = i;
    args->ref = ref;
    args->extra = extra;
    if (c->pool)
        threadpool_add_job(c->pool, output_worker, args);
    else
        output_worker(args);
    return ACIRC_OK;
}

/* Extra functions */

char *
acirc_op2str(acirc_op op)
{
    switch (op) {
    case ACIRC_OP_ADD:
        return "add";
    case ACIRC_OP_SUB:
        return "sub";
    case ACIRC_OP_MUL:
        return "mul";
    }
    return "";
}

acirc_op
acirc_str2op(const char *s)
{
    if (strcmp(s, "add") == 0) {
        return ACIRC_OP_ADD;
    } else if (strcmp(s, "sub") == 0) {
        return ACIRC_OP_SUB;
    } else if (strcmp(s, "mul") == 0) {
        return ACIRC_OP_MUL;
    } else {
        fprintf(stderr, "error: unknown acirc operation '%s'\n", s);
        abort();
    }
}
