/* %define api.pure full */
%code requires {
  #include "_acirc.h"
  #include <setjmp.h>
}
%parse-param { acirc_t *c }
             { acirc_input_f input_f }
             { acirc_const_f const_f }
             { acirc_eval_f eval_f }
             { void *extra }
             { threadpool *pool }
             { jmp_buf env }
/* below not available on bison 2.7 */
%define parse.error verbose

/* C declarations */

%{
#include "_acirc.h"
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern int yylineno;
extern int yylex(void);

void
yyerror(const acirc_t *c, const acirc_input_f input_f, const acirc_const_f const_f,
        const acirc_eval_f eval_f, void *extra, threadpool *pool, jmp_buf env, const char *m)
{
    (void) c; (void) input_f; (void) const_f; (void) eval_f; (void) extra; (void) pool; (void) env;
    fprintf(stderr, "error: [line %d] %s\n", yylineno, m);
}

struct ll_node {
    struct ll_node *next;
    ref_t data;
};

struct ll {
    struct ll_node *start;
    struct ll_node *end;
    size_t length;
};

%}

/* Bison declarations */

%union {
    ref_t ref;
    acirc_op op;
    struct ll *ll;
};

%token NINPUTS CONSTS OUTPUTS TEST START INPUT CONST ENDL
%token  <ref>           NUM
%token  <op>            GATE
%type   <ll>            numlist

/* Grammar rules */

%%

prog:           lines | prelim lines
                ;

prelim:         ninputs consts outputs start
                ;

lines:          lines line | line
                ;

line:           input | const | gate
                ;

ninputs:        NINPUTS NUM ENDLS
                {
                    if (!c->prelim)
                        c->ninputs = $2;
                }
                ;

start:          START ENDL
                {
                    if (!c->prelim) {
                        c->prelim = true;
                        YYACCEPT;
                    }
                }
                ;

input:          NUM INPUT NUM ENDLS
                {
                    if (acirc_eval_input(c, input_f, $1, $3, extra) == ACIRC_ERR)
                        longjmp(env, ACIRC_ERR);
                }
                ;

const:          NUM CONST NUM ENDLS
                {
                    if (acirc_eval_const(c, const_f, $1, $3, extra) == ACIRC_ERR)
                        longjmp(env, ACIRC_ERR);
                }
                ;

gate:           NUM GATE NUM NUM ENDLS
                {
                    acirc_eval_gate(c, eval_f, $2, $1, $3, $4, pool, extra);
                }
                ;

numlist:       /* empty */
                {
                    struct ll *list = calloc(1, sizeof list[0]);
                    list->start = list->end = NULL;
                    $$ = list;
                }
        |       numlist NUM
                {
                    struct ll *list = $1;
                    struct ll_node *node = calloc(1, sizeof node[0]);
                    node->data = $2;
                    if (list->start == NULL) {
                        list->start = node;
                        list->end = node;
                    } else {
                        list->end->next = node;
                        list->end = node;
                    }
                    list->length++;
                    $$ = list;
                }
                ;

consts:         CONSTS numlist ENDLS
                {
                    int *vals;
                    struct ll *list = $2;
                    struct ll_node *node = list->start;
                    vals = calloc(list->length, sizeof vals[0]);
                    for (size_t i = 0; i < list->length; ++i) {
                        struct ll_node *tmp;
                        vals[i] = node->data;
                        tmp = node->next;
                        free(node);
                        node = tmp;
                    }
                acirc_eval_consts(c, vals, list->length);
                free(list);
                }
                ;

outputs:        OUTPUTS numlist ENDLS
                {
                    ref_t *refs;
                    struct ll *list = $2;
                    struct ll_node *node = list->start;
                    refs = calloc(list->length, sizeof refs[0]);
                    for (size_t i = 0; i < list->length; ++i) {
                        struct ll_node *tmp;
                        refs[i] = node->data;
                        tmp = node->next;
                        free(node);
                        node = tmp;
                    }
                    acirc_eval_outputs(c, refs, list->length);
                    free(list);
                }
                ;

ENDLS:          ENDLS ENDL | ENDL
                ;

%%
