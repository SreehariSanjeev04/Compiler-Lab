%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "exprtree.h"
	#include "codegen.h"
	#include "symboltable.h"

	int yylex(void);
	void yyerror(char const *s);
	extern FILE* yyin;
	extern int yylineno;

	FILE* targetFile;
	FILE* inputFile;
	extern int yydebug;
	
	tnode* root;
%}

%define parse.error verbose
%define parse.trace

%union {
    struct tnode *node;
	int vartype;
	char* str;
}

%token <node> NUM TEXT BREAKPOINT CONTINUE BREAK
%token <vartype> INT STRING
%token PLUS MINUS MUL DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL
%token <str> ID

%type <node> expr program Slist Stmt InputStmt OutputStmt AssgStmt Ifstmt Whilestmt varlist
%type <vartype> type

%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV

%%

program
	: START declarations Slist END ';'  
	{
		root = $3;
	}
	| START END ';'  {printf("Empty program\n"); exit(0);}
	;

declarations
	: DECL decllist ENDDECL
	| DECL ENDDECL
	;

decllist
	: decllist decl
	| decl
	;

decl
	: type varlist ';'
	;

varlist
	: varlist ',' ID {
	printf("Installing variable: %s of type %d\n", $3, $<vartype>0);
	Install($3, $<vartype>0);}
	| ID {
	printf("Installing variable: %s of type %d\n", $1, $<vartype>0);
	Install($1, $<vartype>0);}
	;
type
	: INT {
	printf("Type is INT\n");
	$$ = TYPE_INT;}
	| STRING {
	printf("Type is STRING\n");
	$$ = TYPE_STRING;}
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
	: READ '(' ID ')' ';' {
	tnode* idNode = makeLeafNodeId($3);
	$$ = makeReadNode(idNode);}
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
	| TEXT {$$ = $1;}
	| ID {$$ = makeLeafNodeId($1);}
	;


AssgStmt
	: ID ASSIGN expr ';' {
	tnode* idNode = makeLeafNodeId($1);
	$$ = makeOperatorNode("=",idNode,$3);}
	;
	
%%

void yyerror(char const *s)
{
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}