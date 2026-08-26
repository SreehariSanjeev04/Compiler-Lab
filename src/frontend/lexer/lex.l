%{
	#include "parser.tab.h"
	#include "exprtree.h"
	void yyerror(char const *s);
%}

%option yylineno
digit  [0-9]
number  {digit}+

text  \"[^\"\\]*\"

identifier  [a-zA-Z][a-zA-Z0-9_]*
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
"decl" {return DECL;}
"enddecl" {return ENDDECL;}
"int" {return INT;}
"str" {return STRING;}
"return" {return RETURN;}
"AND"|"and" {return AND;}
"OR"|"or" {return OR;}
"NOT"|"not" {return NOT;}
"main" {return MAIN;}

{text} {
	yylval.node = makeLeafNodeString(yytext); 
	return STRLIT;
}
{number}	{
yylval.node = makeLeafNodeNum(atoi(yytext)); return NUM;}

{identifier} {
yylval.str = strdup(yytext);
return ID;
}

"==" {return EQ;}
"!=" {return NE;}
"<" {return LT;}
">" {return GT;}
"<=" {return LE;}
">=" {return GE;}
"+"	{return PLUS;}
"-"	{return MINUS;}
"*"	{return *yytext;}
"/"	{return DIV;}
"=" {return ASSIGN;}
"%" {return MOD;}
"(" { return *yytext; }
")" { return *yytext; }
"[" { return *yytext; }
"]" { return *yytext; }
"{" { return *yytext; }
"}" { return *yytext; }
";" { return *yytext; }
"," { return *yytext; }
"&" { return *yytext; }
[ \t]	{}
[\n]	{}
.	{yyerror("unknown character\n");
	fprintf(stderr, "Error: Unknown character '%c' at line %d\n", *yytext, yylineno);
	exit(1);}

%%

int yywrap(void) {
	return 1;
}
