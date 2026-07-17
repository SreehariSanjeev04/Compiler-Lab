%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "exprtree.h"
	#include "codegen.h"

	int yylex(void);
	void yyerror(char const *s);
	extern FILE* yyin;
	extern int yylineno;

	FILE* targetFile;
	FILE* inputFile;
	
	tnode* root;
%}

%define parse.error verbose

%union {
    struct tnode *node;
}

%token <node> NUM ID
%token PLUS MINUS MUL DIV ASSIGN START END READ WRITE 

%type <node> expr program Slist Stmt InputStmt OutputStmt AssgStmt

%left PLUS MINUS
%left MUL DIV

%%

program
	: START Slist END ';'  
	{
		root = $2;
	}
	| START END ';'  {printf("Empty program\n"); exit(0);}
	;

Slist
	: Slist Stmt {$$ = makeConnectorNode($1,$2);}
	| Stmt {$$ = $1;}
	;

Stmt
	: InputStmt {$$ = $1;}
	| OutputStmt {$$ = $1;}
	| AssgStmt {$$ = $1;}
	;

InputStmt
	: READ '(' ID ')' ';' {$$ = makeReadNode($3);}
	;

OutputStmt
	: WRITE '(' expr ')' ';' {$$ = makeWriteNode($3);}
	;

AssgStmt
	: ID ASSIGN expr ';' {$$ = makeOperatorNode('=',$1,$3);}
	;
	
expr
	: expr PLUS expr {$$ = makeOperatorNode('+',$1,$3);}
	| expr MINUS expr {$$ = makeOperatorNode('-',$1,$3);}
	| expr MUL expr {$$ = makeOperatorNode('*',$1,$3);}
	| expr DIV expr {$$ = makeOperatorNode('/',$1,$3);}
	| '(' expr ')' {$$ = $2;}
	| NUM {$$ = $1;}
	| ID {$$ = $1;}
	;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
		exit(1);
	}
	inputFile = fopen(argv[1], "r");
    targetFile = fopen(argv[2], "w");
	if(!inputFile) {
		fprintf(stderr, "Error: Unable to open input file %s\n", argv[1]);
		exit(1);
	}
    if (!targetFile) {
        fprintf(stderr, "Error: Unable to open target file\n");
        exit(1);
    }
	yyin = inputFile;
    yyparse();
	generateCode(root, targetFile);
	fclose(inputFile);
    fclose(targetFile);
    return 0;
}