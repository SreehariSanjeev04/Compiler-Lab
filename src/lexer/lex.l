%{
	#include "parser.tab.h"
	#include "exprtree.h"
%}

%%

[0-9]+	{
yylval.node = makeLeafNode(atoi(yytext)); return NUM;}
"+"	{return PLUS;}
"-"	{return MINUS;}
"*"	{return MUL;}
"/"	{return DIV;}
[ \t]	{}
[()]	{return *yytext;}
[\n]	{return END;}
.	{yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	return 1;
}
