%{
	#include "parser.tab.h"
	#include "exprtree.h"
	void yyerror(char const *s);
%}

%option yylineno
digit  [0-9]
number  {digit}+

 // In stage2 we are only considering single letter identifiers
identifier  [a-z]
%%

"begin"	{return START;}
"end"	{return END;}
"read"	{return READ;}
"write"	{return WRITE;}
"if"	{return IF;}
"while"	{return WHILE;}
"then"	{return THEN;}
"else"	{return ELSE;}
"do"	{return DO;}
"endwhile"	{return ENDWHILE;}
"endif"	{return ENDIF;}
"breakpoint"	{return BREAKPOINT;}
"break"	{return BREAK;}
"continue"	{return CONTINUE;}
"repeat" {return REPEAT;}
"until" {return UNTIL;}

{number}	{
yylval.node = makeLeafNodeNum(atoi(yytext)); return NUM;}

{identifier} {
yylval.node = makeLeafNodeId(yytext); return ID;}

"==" {return EQ;}
"!=" {return NE;}
"<" {return LT;}
">" {return GT;}
"<=" {return LE;}
">=" {return GE;}
"+"	{return PLUS;}
"-"	{return MINUS;}
"*"	{return MUL;}
"/"	{return DIV;}
"=" {return ASSIGN;}
"(" { return *yytext; }
")" { return *yytext; }
";" { return *yytext; }
[ \t]	{}
[\n]	{}
.	{yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	return 1;
}
