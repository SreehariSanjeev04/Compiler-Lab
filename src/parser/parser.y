%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "exprtree.h"
	#include "stage1_ex1.h"
	int yylex(void);
%}

%union {
    struct tnode *node;
}

%token <node> NUM
%token PLUS MINUS MUL DIV END

%type <node> expr program

%left PLUS MINUS
%left MUL DIV

%%

program : expr END {
                // Evaluation function
				$$ = $1;
				printf("Prefix form of the expression: ");
				prefixForm($1);
				printf("\n");
				exit(1);
			}
		;

expr : expr PLUS expr		{$$ = makeOperatorNode('+',$1,$3);}
	 | expr MINUS expr  	{$$ = makeOperatorNode('-',$1,$3);}
	 | expr MUL expr	{$$ = makeOperatorNode('*',$1,$3);}
	 | expr DIV expr	{$$ = makeOperatorNode('/',$1,$3);}
	 | '(' expr ')'		{$$ = $2;}
	 | NUM			{$$ = $1;}
	 ;

%%

yyerror(char const *s)
{
    printf("yyerror %s",s);
}


int main(void) {
	yyparse();
	return 0;
}