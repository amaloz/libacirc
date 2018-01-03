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
        mpz_t xs[acirc_ninputs(c)], modulus;
        mpz_init_set_ui(modulus, 2377000);
        printf("inputs: ");
        for (size_t i = 0; i < acirc_ninputs(c); ++i) {
            mpz_init_set_ui(xs[i], 1);
            gmp_printf("%Zd ", xs[i]);
        }
        printf("\n");
        if (acirc_eval_mpz(c, xs, NULL, modulus) == ACIRC_OK) {
            mpz_t *output;
            printf("outputs: ");
            for (size_t i = 0; i < acirc_noutputs(c); ++i) {
                output = acirc_output(c, i);
                gmp_printf("%Zd ", *output);
                mpz_clear(*output);
            }
            printf("\n");
        }
        for (size_t i = 0; i < acirc_ninputs(c); ++i) {
            mpz_clear(xs[i]);
        }
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
    ok |= test("t/circuits/size_test.acirc");
}
