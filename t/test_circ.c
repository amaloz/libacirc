#include <acirc.h>
#include <stdlib.h>
#include <gmp.h>

int
test(const char *fname)
{
    acirc_t *c;
    const size_t n = 128;
    int ret = 1;

    if ((c = acirc_new(fname)) == NULL) {
        fprintf(stderr, "error: acirc_new failed\n");
        return ret;
    }

    /* { */
    /*     unsigned long *outputs; */

    /*     outputs = acirc_degrees(c, n, &m); */
    /*     printf("degrees: "); */
    /*     for (size_t i = 0; i < m; ++i) { */
    /*         printf("%lu ", outputs[i]); */
    /*     } */
    /*     printf("\n"); */
    /*     free(outputs); */
    /*     acirc_reset(c); */

    /*     outputs = acirc_depths(c, n, &m); */
    /*     printf("depth: "); */
    /*     for (size_t i = 0; i < m; ++i) { */
    /*         printf("%lu ", outputs[i]); */
    /*     } */
    /*     printf("\n"); */
    /*     free(outputs); */
    /*     acirc_reset(c); */
    /* } */

    /* { */
    /*     long inputs[n], *outputs; */
    /*     for (size_t i = 0; i < n; ++i) inputs[i] = 1; */
    /*     outputs = acirc_eval(c, inputs, n, &m); */
    /*     printf("outputs: "); */
    /*     for (size_t i = 0; i < m; ++i) { */
    /*         printf("%ld ", outputs[i]); */
    /*     } */
    /*     printf("\n"); */
    /*     free(outputs); */
    /*     acirc_reset(c); */
    /* } */

    {
        mpz_t *inputs[n], modulus;
        mpz_init_set_ui(modulus, 2377000);
        for (size_t i = 0; i < n; ++i) {
            inputs[i] = calloc(1, sizeof inputs[i][0]);
            mpz_init_set_ui(*inputs[i], 1);
            gmp_printf("%Zd ", inputs[i]);
        }
        printf("\n");
        if (acirc_eval_mpz(c, inputs, n, modulus) == ACIRC_OK) {
            mpz_t *output;
            printf("outputs: ");
            while ((output = acirc_next_output(c)) != NULL) {
                gmp_printf("%Zd ", *output);
                mpz_clear(*output);
            }
            printf("\n");
        }
    }
    acirc_free(c);
    return 0;
}

int
main(int argc, char **argv)
{
    (void) argc; (void) argv;
    return test("t/circuits/size_test.acirc");
}
