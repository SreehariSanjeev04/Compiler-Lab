%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <exprtree.h>
	#include <codegen.h>
	#include <symboltable.h>
	#include <constants.h>
	#include <utils.h>
	#include <stdbool.h>

	int yylex(void);
	void yyerror(char const *s);
	extern FILE* yyin;
	extern int yylineno;

	FILE* targetFile;
	FILE* inputFile;
	extern int yydebug;
	
	tnode* root;
	int currentType = -1;
%}

%define parse.error verbose
%define parse.trace

%union {
    struct tnode *node;
	int vartype;
	char* str;
	bool isAddress;
}

%token <node> NUM TEXT BREAKPOINT CONTINUE BREAK
%token <vartype> INT STRING
%token PLUS MINUS STAR DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL
%token <str> ID

%type <node> expr program Slist Stmt InputStmt OutputStmt AssgStmt Ifstmt Whilestmt var
%type <vartype> type
%type <str> vardecl arraydecl

%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR DIV

%%

program
	: START declarations Slist END ';' 
	{
		root = $3;
	}
	| START END ';'  {printf("Empty program\n"); exit(0);}
	;

declarations
	: DECL decllist ENDDECL { assignBindingAddresses(); } 
	| DECL ENDDECL { assignBindingAddresses(); }
	;

decllist
	: decllist decl {}
	| decl {}
	;

decl
	: type varlist ';' {}
	;

varlist
	: varlist ',' vardecl { }
	| vardecl {}	


// the id is a string, so we need to create a leaf node for it, and then return that node
vardecl
	: ptrList ID {
		if (Lookup($2) != NULL) {
			printf("Error: Variable %s already declared\n", $2);
			exit(1);
		}
		if(currentType == -1) {
			printf("Error: Type not specified for variable %s\n", $2);
			exit(1);
		}
		
		Install($2, returnPointerType(currentType), $1);
		$$ = $2; // Return the ID as a string for further processing
	}
	| arraydecl {
		$$ = $1;
	}
	;

arraydecl
	: ID {
		if (Lookup($1) != NULL) {
			printf("Error: Variable %s already declared\n", $1);
			exit(1);
		}
		if(currentType == -1) {
			printf("Error: Type not specified for variable %s\n", $1	);
			exit(1);
		}
		Install($1, currentType, 0);
		$$ = $1;
	}
	| arraydecl '[' NUM ']' {
		// check if the variable is already declared
		if(Lookup($1) == NULL) {
			printf("Error: Variable %s not declared\n", $1);
			exit(1);
		}
		struct Gsymbol* symbol = Lookup($1);
		addDimensionSizes(symbol, $3->val);
		$$ = $1;
	}

ptrList
	: STAR ptrList {$$ = $2 + 1;} // Increment pointer level for each '*'
	| /* empty */ {$$ = 0;} // No pointer indirection
	;
type
	: INT {
	currentType = TYPE_INT;}
	| STRING {
	currentType = TYPE_STRING;}
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
	: READ '(' var ')' ';' {
		$$ = makeReadNode($3);}
	;

OutputStmt
	: WRITE '(' expr ')' ';' {$$ = makeWriteNode($3);}
	;

expr
	: expr PLUS expr {$$ = makeOperatorNode("+",$1,$3);}
	| expr MINUS expr {$$ = makeOperatorNode("-",$1,$3);}
	| expr STAR expr {$$ = makeOperatorNode("*",$1,$3);}
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
	| addr var {
		if($1) {
			$$ = $2;
		} else {
			$$ = makeAddressNode($2);
		}
	}
	;

// detect if the variable is being used as an address or not, if it is then we will create a new node for it, else we will just return the variable node
addr
	: '&' {$$ = true}
	| /* empty */ {$$ = false}
	;
AssgStmt
	: var ASSIGN expr ';' {
	$$ = makeOperatorNode("=",$1,$3);}
	;

// this would create array node with the last index at the rightmost leaf node, and the left child would be the variable name
var
	: ID {$$ = makeLeafNodeId($1);}
	| var '[' expr ']' {
	$$ = makeArrayNode($1, $3);}
	;
	
%%

void yyerror(char const *s)
{
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}
