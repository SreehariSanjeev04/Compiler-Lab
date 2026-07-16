%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "exprtree.h"
	#include "codegen.h"
	int yylex(void);
	FILE* targetFile;
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

program
	: expr END {
	// Evaluation function
		$$ = $1;
		int outputReg = codeGen($1, targetFile);
		if(outputReg != -1) {
		printValue(outputReg, targetFile);
		exitProgram(targetFile);
		printf("Code generation completed successfully.\n");
		exit(0);
	} else {
		fprintf(stderr, "Error: Code generation failed.\n");
		exit(1);
		}
	};

expr
	: expr PLUS expr {$$ = makeOperatorNode('+',$1,$3);}
	| expr MINUS expr {$$ = makeOperatorNode('-',$1,$3);}
	| expr MUL expr {$$ = makeOperatorNode('*',$1,$3);}
	| expr DIV expr {$$ = makeOperatorNode('/',$1,$3);}
	| '(' expr ')' {$$ = $2;}
	| NUM {$$ = $1;}
	;

%%

yyerror(char const *s)
{
    printf("yyerror %s",s);
}


int main(int argc, char* argv[]) {
    targetFile = fopen(argv[1], "w");
    if (!targetFile) {
        fprintf(stderr, "Error: Unable to open target file\n");
        exit(1);
    }
    addHeader(targetFile);
    yyparse();
    fclose(targetFile);
    return 0;
}