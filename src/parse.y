/* %define api.pure full */
%code requires {
  #include "_acirc.h"
}
%parse-param { acirc_t *c }
             { acirc_input_f input_f }
             { acirc_const_f const_f }
             { acirc_eval_f eval_f }
             { acirc_free_f free_f }
             { acirc_write_f write_f }
             { acirc_read_f read_f }
             { void *extra }
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
yyerror(const acirc_t *c, const acirc_input_f input_f, const acirc_const_f const_f,
        const acirc_eval_f eval_f, const acirc_free_f free_f, const acirc_write_f write_f,
        const acirc_read_f read_f, void *extra, const char *m)
{
    (void) c; (void) input_f; (void) const_f; (void) eval_f; (void) free_f;
    (void) write_f; (void) read_f; (void) extra;
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

prog:           lines | prelims start lines
                ;

prelims:        binary ninputs nrefs consts secrets outputs symlens sigmas tests
                ;

binary:         %empty | BINARY ENDL
                {
                    if (!c->circuit)
                        c->binary = true;
                }
                ;

ninputs:        NINPUTS NUM ENDL
                {
                    if (!c->circuit)
                        c->ninputs = $2;
                }
                ;

nrefs:          NREFS NUM ENDL
                {
                    if (!c->circuit)
                        c->nrefs = $2;
                }
                ;

sigmas:         %empty | SIGMAS numlist ENDL
                {
                    nlist_t *list = $2;
                    if (!c->circuit) {
                        acirc_eval_sigmas(c, list->data, list->n);
                        free(list);
                    }
                }
                ;

symlens:        SYMLENS numlist ENDL
                {
                    nlist_t *list = $2;
                    if (!c->circuit) {
                        acirc_eval_symlens(c, list->data, list->n);
                        free(list);
                    }
                }
                ;

tests:          %empty | tests test | test
                ;

test:           TEST STR STR ENDL
                {
                    if (!c->circuit)
                        acirc_eval_test(c, $2, $3);
                    free($2);
                    free($3);
                }
                ;

start:          START ENDL
                {
                    if (!c->circuit) {
                        c->circuit = true;
                        YYACCEPT;
                    }
                }
                ;

lines:          lines line | line
                ;

line:           input | const | secret | gate
                ;

input:          NUM INPUT NUM ENDL
                {
                    if (acirc_eval_input(c, input_f, $1, $3, -1, extra) == ACIRC_ERR)
                        YYABORT;
                }
        |       NUM INPUT NUM COLON NUM ENDL
                {
                    if (acirc_eval_input(c, input_f, $1, $3, $5, extra) == ACIRC_ERR)
                        YYABORT;
                }
                ;

const:          NUM CONST NUM ENDL
                {
                    if (acirc_eval_const(c, const_f, $1, $3, -1, extra) == ACIRC_ERR)
                        YYABORT;
                }
        |       NUM CONST NUM COLON NUM ENDL
                {
                    if (acirc_eval_const(c, const_f, $1, $3, $5, extra) == ACIRC_ERR)
                        YYABORT;
                }
                ;

secret:         NUM SECRET NUM ENDL
                {
                    if (acirc_eval_secret(c, const_f, $1, $3, -1, extra) == ACIRC_ERR)
                        YYABORT;
                }
        |       NUM SECRET NUM COLON NUM ENDL
                {
                    if (acirc_eval_secret(c, const_f, $1, $3, $5, extra) == ACIRC_ERR)
                        YYABORT;
                }
                ;

gate:           NUM GATE NUM NUM ENDL
                {
                    acirc_eval_gate(c, eval_f, free_f, write_f, read_f, $2, $1, $3, $4, -1, REF_STD, extra);
                }
        |       NUM GATE NUM NUM COLON NUM ENDL
                {
                    acirc_eval_gate(c, eval_f, free_f, write_f, read_f, $2, $1, $3, $4, $6, REF_STD, extra);
                }
        |       NUM GATE NUM NUM COLON NUM SAVE ENDL
                {
                    acirc_eval_gate(c, eval_f, free_f, write_f, read_f, $2, $1, $3, $4, $6, REF_SAVE, extra);
                }
        |       NUM GATE NUM NUM COLON NUM SKIP ENDL
                {
                    acirc_eval_gate(c, eval_f, free_f, write_f, read_f, $2, $1, $3, $4, $6, REF_SKIP, extra);
                }
                ;

numlist:        %empty
                {
                    nlist_t *list;
                    if (c->circuit)
                        list = NULL;
                    else
                        list = calloc(1, sizeof list[0]);
                    $$ = list;
                }
        |       numlist NUM
                {
                    if (!c->circuit) {
                        nlist_t *list = $1;
                        list->data = realloc(list->data, (list->n + 1) * sizeof list->data[0]);
                        list->data[list->n++] = $2;
                        $$ = list;
                    } else {
                        $$ = NULL;
                    }
                }
                ;

consts:         %empty | CONSTS numlist ENDL
                {
                    nlist_t *list = $2;
                    if (!c->circuit) {
                        acirc_eval_consts(c, (long *) list->data, list->n);
                        free(list);
                    }
                }
                ;

secrets:        %empty | SECRETS numlist ENDL
                {
                    nlist_t *list = $2;
                    if (!c->circuit) {
                        acirc_eval_secrets(c, (long *) list->data, list->n);
                        free(list);
                    }
                };

outputs:        OUTPUTS numlist ENDL
                {
                    nlist_t *list = $2;
                    if (!c->circuit) {
                        acirc_eval_outputs(c, list->data, list->n);
                        free(list);
                    }
                }
                ;

%%
