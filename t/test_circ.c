#include <acirc.h>
#include <stdlib.h>
#include <gmp.h>

int
test(const char *fname)
{
    acirc_t *c;
    int ret = 1;

    printf("\n* %s *\n\n", fname);

    if ((c = acirc_new(fname)) == NULL) {
        fprintf(stderr, "error: acirc_new failed\n");
        return ret;
    }

    printf("ninputs: %lu\n", acirc_ninputs(c));
    printf("nconsts: %lu\n", acirc_nconsts(c));
    printf("noutputs: %lu\n", acirc_noutputs(c));

    {
        unsigned long count;
        count = acirc_ngates(c);
        printf("ngates: %lu\n", count);
    }

    {
        unsigned long count;
        count = acirc_nmuls(c);
        printf("nmuls: %lu\n", count);
    }

    {
        unsigned long max;
        max = acirc_max_degree(c);
        printf("max degree: %lu\n", max);
    }

    {
        unsigned long depth;
        depth = acirc_max_depth(c);
        printf("max depth: %lu\n", depth);
    }

    acirc_test(c);

    {
        mpz_t **xs, modulus, **outputs;
        mpz_init_set_ui(modulus, 2377000);
        xs = calloc(acirc_ninputs(c), sizeof xs[0]);
        printf("inputs: ");
        for (size_t i = 0; i < acirc_ninputs(c); ++i) {
            xs[i] = calloc(1, sizeof xs[i][0]);
            mpz_init_set_ui(*xs[i], 0);
            gmp_printf("%Zd ", *xs[i]);
        }
        printf("\n");
        outputs = acirc_eval_mpz(c, xs, NULL, modulus);
        if (outputs) {
            printf("outputs: ");
            for (size_t i = 0; i < acirc_noutputs(c); ++i) {
                gmp_printf("%Zd ", *outputs[i]);
                mpz_clear(*outputs[i]);
                free(outputs[i]);
            }
            printf("\n");
        }
        free(xs);
        free(outputs);
        mpz_clear(modulus);
    }

    acirc_free(c);
    return 0;
}

int
main(int argc, char **argv)
{
    (void) argc; (void) argv;
    int ok = 0;
    ok |= test("t/circuits/test.acirc");
    ok |= test("t/circuits/add.acirc");
    ok |= test("t/circuits/simple.acirc");
    ok |= test("t/circuits/ggm_1_4.dsl.acirc");
    ok |= test("t/circuits/ggm_4_4.dsl.acirc");
    ok |= test("t/circuits/ggm_1_32.dsl.acirc");
    return ok;
}
