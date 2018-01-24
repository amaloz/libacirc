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

input  { return INPUT; }
const  { return CONST; }
secret { return SECRET; }

add    { yylval.op = ACIRC_OP_ADD; return GATE; }
sub    { yylval.op = ACIRC_OP_SUB; return GATE; }
mul    { yylval.op = ACIRC_OP_MUL; return GATE; }

<test>\n { BEGIN(INITIAL); return ENDL; }
<test>[0-9a-z]+ {yylval.str = strdup(yytext); return STR; }

[ \r\t]+                        /* ignore whitespace */
#.*\n                           /* ignore comments */
\n       { return ENDL; }
:        { return COLON; }
inf      { return INF; }

[0-9]+     { yylval.ref = (ref_t) atoi(yytext); return NUM; }
:binary    { return BINARY; }
:ninputs   { return NINPUTS; }
:nrefs     { return NREFS; }
:consts    { return CONSTS; }
:secrets   { return SECRETS; }
:outputs   { return OUTPUTS; }
:symlens   { return SYMLENS; }
:sigmas    { return SIGMAS; }
:test      { BEGIN(test); return TEST; }
:start     { return START; }

. { fprintf(stderr, "error: unrecognized character: %s\n", yytext); }
