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

%token <node> NUM ID BREAKPOINT CONTINUE BREAK
%token PLUS MINUS MUL DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL INT BOOL

%type <node> expr program Slist Stmt InputStmt OutputStmt AssgStmt Ifstmt Whilestmt

%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV

%%

program
	: START decl Slist END ';'  
	{
		root = $3;
	}
	| START END ';'  {printf("Empty program\n"); exit(0);}
	;

decl
	: DECL vardecl ENDDECL {}
	| DECL ENDDECL {}
	;

vardecl
	: vardecl type idlist ';' {}
	| type idlist ';' {}
	;

idlist
	: idlist ',' ID {}
	| ID {printf("Declared variable: %s\n", $1->varname);}
	;
type
	: INT {}
	| BOOL {}
	;

Slist
	: Slist Stmt {$$ = makeConnectorNode($1,$2);}
	| Stmt {$$ = $1;}
	;

Stmt
	: InputStmt {$$ = $1;}
	| OutputStmt {$$ = $1;}
	| AssgStmt {$$ = $1;}
	| Ifstmt {$$ = $1;}
	| Whilestmt {$$ = $1;}
	| BREAKPOINT ';' {$$ = makeBreakPointNode();}
	| BREAK ';' {$$ = makeBreakNode();}
	| CONTINUE ';' {$$ = makeContinueNode();}
	;

Ifstmt
	: IF '(' expr ')' THEN Slist ELSE Slist ENDIF ';' {$$ = makeIfElseNode($3, $6, $8);}
	| IF '(' expr ')' THEN Slist ENDIF ';' {$$ = makeIfNode($3, $6);}
	;

Whilestmt
	: WHILE '(' expr ')' DO Slist ENDWHILE ';' {$$ = makeWhileNode($3, $6);}
	| DO Slist WHILE '(' expr ')' ENDWHILE ';' {$$ = makeDoWhileNode($2, $5);}
	| REPEAT Slist UNTIL '(' expr ')' ';' {$$ = makeRepeatUntilNode($2, $5);}
	;
InputStmt
	: READ '(' ID ')' ';' {$$ = makeReadNode($3);}
	;

OutputStmt
	: WRITE '(' expr ')' ';' {$$ = makeWriteNode($3);}
	;

expr
	: expr PLUS expr {$$ = makeOperatorNode("+",$1,$3);}
	| expr MINUS expr {$$ = makeOperatorNode("-",$1,$3);}
	| expr MUL expr {$$ = makeOperatorNode("*",$1,$3);}
	| expr DIV expr {$$ = makeOperatorNode("/",$1,$3);}
	| expr LE expr {$$ = makeOperatorNode("<=",$1,$3);}
	| expr GE expr {$$ = makeOperatorNode(">=",$1,$3);}
	| expr LT expr {$$ = makeOperatorNode("<",$1,$3);}
	| expr GT expr {$$ = makeOperatorNode(">",$1,$3);}
	| expr EQ expr {$$ = makeOperatorNode("==",$1,$3);}
	| expr NE expr {$$ = makeOperatorNode("!=",$1,$3);}
	| '(' expr ')' {$$ = $2;}
	| NUM {$$ = $1;}
	| ID {$$ = $1;}
	;


AssgStmt
	: ID ASSIGN expr ';' {$$ = makeOperatorNode("=",$1,$3);}
	;
	
%%

void yyerror(char const *s)
{
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}