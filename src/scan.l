/* make yytext a pointer */
%pointer
/* not supplying yywrap() function */
%option noyywrap
 /* track line numbers */
%option yylineno
%option never-interactive
%option warn nodefault

%{
#include "_acirc.h"
#include "parse.h"
#include <stdlib.h>
%}

%%

input       { return INPUT; }
const       { return CONST; }

ADD|SUB|MUL|SET {
    yylval.op = acirc_str2op(yytext);
    return GATE;
}

\n       { return ENDL; }
[ \r\t]+                        /* ignore whitespace */
#.*\n                           /* ignore comments */

[0-9]+     { yylval.ref = (ref_t) atoi(yytext); return NUM; }
:ninputs   { return NINPUTS; }
:consts    { return CONSTS; }
:outputs   { return OUTPUTS; }
:test      { return TEST; }
:start     { return START; }

. { fprintf(stderr, "error: unrecognized character: %s\n", yytext); }
