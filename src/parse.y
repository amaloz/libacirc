/* %define api.pure full */
%code requires {
  #include "_acirc.h"
}
%parse-param { acirc_t *c }
             /* { acirc_input_f input_f } */
             /* { acirc_const_f const_f } */
             /* { acirc_eval_f eval_f } */
             /* { acirc_free_f free_f } */
             /* { acirc_write_f write_f } */
             /* { acirc_read_f read_f } */
             /* { void *extra } */
/* below not available on bison 2.7 */
%define parse.error verbose

/* C declarations */

%{
#include "_acirc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern int yylineno;
extern int yylex(void);

void
yyerror(const acirc_t *c, const char *m)
{
    (void) c;
    fprintf(stderr, "error: [line %d] %s\n", yylineno, m);
}

typedef struct nlist_t {
    ref_t *data;
    size_t n;
} nlist_t;

%}

/* Bison declarations */

%union {
    ref_t ref;
    char *str;
    acirc_op op;
    struct nlist_t *nlist;
};

%token NINPUTS NREFS CONSTS OUTPUTS SECRETS SYMLENS SIGMAS BINARY TEST START
%token INPUT CONST SECRET
%token COLON SAVE SKIP ENDL
%token  <ref>           NUM
%token  <str>           STR
%token  <op>            GATE
%type   <nlist>         numlist

/* Grammar rules */

%%

prog:           prelims start lines
                ;

prelims:        binary ninputs nrefs consts secrets outputs symlens sigmas tests
                ;

binary:         %empty | BINARY ENDL
                {
                    c->binary = true;
                }
                ;

ninputs:        NINPUTS NUM ENDL
                {
                    c->ninputs = $2;
                }
                ;

nrefs:          NREFS NUM ENDL
                {
                    c->nrefs = $2;
                    c->refs = calloc(c->nrefs, sizeof c->refs[0]);
                }
                ;

sigmas:         %empty | SIGMAS numlist ENDL
                {
                    nlist_t *list = $2;
                    acirc_eval_sigmas(c, list->data, list->n);
                    free(list);
                }
                ;

symlens:        SYMLENS numlist ENDL
                {
                    nlist_t *list = $2;
                    acirc_eval_symlens(c, list->data, list->n);
                    free(list);
                }
                ;

tests:          %empty | tests test | test
                ;

test:           TEST STR STR ENDL
                {
                    acirc_eval_test(c, $2, $3);
                    free($2);
                    free($3);
                }
                ;

start:          START ENDL
        ;

lines:          lines line | line
                ;

line:           input | const | secret | gate
                ;

input:          NUM INPUT NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_INPUT, .x = $3, .count = -1 };
                }
        |       NUM INPUT NUM COLON NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_INPUT, .x = $3, .count = $5 };
                }
                ;

const:          NUM CONST NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_CONST, .x = $3, .count = -1 };
                }
        |       NUM CONST NUM COLON NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_CONST, .x = $3, .count = $5 };
                }
                ;

secret:         NUM SECRET NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_SECRET, .x = $3, .count = -1 };
                }
        |       NUM SECRET NUM COLON NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_SECRET, .x = $3, .count = $5 };
                }
                ;

gate:           NUM GATE NUM NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_GATE, .op = $2, .x = $3, .y = $4,
                          .count = -1, .state = REF_STD };
                }
        |       NUM GATE NUM NUM COLON NUM ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_GATE, .op = $2, .x = $3, .y = $4,
                          .count = $6, .state = REF_STD };
                }
        |       NUM GATE NUM NUM COLON NUM SAVE ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_GATE, .op = $2, .x = $3, .y = $4,
                          .count = $6, .state = REF_SAVE };
                }
        |       NUM GATE NUM NUM COLON NUM SKIP ENDL
                {
                    c->refs[$1] = (acirc_ref_t)
                        { .type = REF_GATE, .op = $2, .x = $3, .y = $4,
                          .count = $6, .state = REF_SKIP };
                }
                ;

numlist:        %empty
                {
                    nlist_t *list;
                    list = calloc(1, sizeof list[0]);
                    $$ = list;
                }
        |       numlist NUM
                {
                    nlist_t *list = $1;
                    list->data = realloc(list->data, (list->n + 1) * sizeof list->data[0]);
                    list->data[list->n++] = $2;
                    $$ = list;
                }
                ;

consts:         %empty | CONSTS numlist ENDL
                {
                    nlist_t *list = $2;
                    acirc_eval_consts(c, (long *) list->data, list->n);
                    free(list);
                }
                ;

secrets:        %empty | SECRETS numlist ENDL
                {
                    nlist_t *list = $2;
                    acirc_eval_secrets(c, (long *) list->data, list->n);
                    free(list);
                };

outputs:        OUTPUTS numlist ENDL
                {
                    nlist_t *list = $2;
                    acirc_eval_outputs(c, list->data, list->n);
                    free(list);
                }
                ;

%%
