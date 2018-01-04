/* make yytext a pointer */
%pointer
/* not supplying yywrap() function */
%option noyywrap
 /* track line numbers */
%option yylineno
%option never-interactive
%option warn nodefault

%s test

%{
#include "_acirc.h"
#include "parse.h"
#include <stdlib.h>
%}

%%

input       { return INPUT; }
const       { return CONST; }

ADD|SUB|MUL {
    yylval.op = acirc_str2op(yytext);
    return GATE;
}

<test>\n { BEGIN(INITIAL); return ENDL; }

\n       { return ENDL; }
[ \r\t]+                        /* ignore whitespace */
#.*\n                           /* ignore comments */
:        { return COLON; }

<test>[0-9a-z]+ {yylval.str = strdup(yytext); return STR; }
[0-9]+     { yylval.ref = (ref_t) atoi(yytext); return NUM; }
:ninputs   { return NINPUTS; }
:consts    { return CONSTS; }
:outputs   { return OUTPUTS; }
:secrets   { return SECRETS; }
:symlen    { return SYMLEN; }
:base      { return BASE; }
:test      { BEGIN(test); return TEST; }
:start     { return START; }

. { fprintf(stderr, "error: unrecognized character: %s\n", yytext); }
