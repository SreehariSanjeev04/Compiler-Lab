%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <exprtree.h>
    #include <codegen.h>
    #include <symboltable.h>
    #include <constants.h>
    #include <symboltable_utils.h>
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
}

%token <node> NUM TEXT BREAKPOINT CONTINUE BREAK
%token <vartype> INT STRING
%token PLUS MINUS '*' DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL MOD MAIN
%token <str> ID

%type <node> expr program slist stmt inputstmt outputstmt assgstmt ifstmt whilestmt var assg_lhs Body MainBlock Fdef FdefBlock
%type <vartype> type PtrDecl

%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left '*' DIV MOD
%right ADDR DEREF 

%%

program
    : GDeclBlock FdefBlock MainBlock    { root = $3; }
    | GDeclBlock MainBlock           { root = $2; }
    | MainBlock                      { root = $1; }
    ;

GDeclBlock
    : DECL GdeclList ENDDECL { assignBindingAddresses(); }
    | DECL ENDDECL         { assignBindingAddresses(); }
    ;

GdeclList
    : GdeclList Gdecl
    | Gdecl
    ;

Gdecl
    : type GidList ';'
    ;

GidList
    : GidList ',' Gid
    | Gid
    ;

Gid
    : ID {
        if (Lookup($1) != NULL) {
            printf("Error: Variable %s already declared\n", $1); exit(1);
        }
        Install($1, currentType, 0);
    }
    | PtrDecl ID {
        if (Lookup($2) != NULL) {
            printf("Error: Variable %s already declared\n", $2); exit(1);
        }
        Install($2, currentType, $1);
    }
    | ID DeclDimList {
    }
    | ID '(' paramlist ')' {
    }
    | ID '(' ')' {
    }
    ;

DeclDimList
    : DeclDimList '[' NUM ']'
    | '[' NUM ']'
    ;

PtrDecl
    : '*' PtrDecl { $$ = $2 + 1; }  
    | '*'          { $$ = 1; }     
    ;

type
    : INT    { currentType = TYPE_INT; }
    | STRING { currentType = TYPE_STRING; }
    ;

FdefBlock
    : FdefBlock Fdef { $$ = $1; }
    | Fdef           { $$ = $1; }
    ;

Fdef
    : type ID '(' paramlist ')' '{' LdeclBlock Body '}' { $$ = $8; }
    | type ID '(' ')' '{' LdeclBlock Body '}'           { $$ = $7; }
    ;

paramlist
    : paramlist ',' param
    | param
    ;

param
    : type ID
    | type PtrDecl ID
    ;

MainBlock
    : INT MAIN '(' ')' '{' LdeclBlock Body '}' { $$ = $7; }
    | INT MAIN '(' ')' '{' Body '}'            { $$ = $6; }
    ;

LdeclBlock
    : DECL LdeclList ENDDECL
    | DECL ENDDECL
    ;

LdeclList
    : LdeclList Ldecl
    | Ldecl
    ;

Ldecl
    : type LidList ';'
    ;

LidList
    : LidList ',' Lid
    | Lid
    ;

Lid
    : ID
    | PtrDecl ID
    ;

Body
    : START slist END ';' { $$ = $2; }
    | START END ';'       { printf("Empty program\n"); $$ = NULL; }
    ;

slist
    : slist stmt { $$ = makeConnectorNode($1, $2); }
    | stmt       { $$ = $1; }
    ;

stmt
    : inputstmt      { $$ = $1; }
    | outputstmt     { $$ = $1; }
    | assgstmt       { $$ = $1; }
    | ifstmt         { $$ = $1; }
    | whilestmt      { $$ = $1; }
    | BREAKPOINT ';' { $$ = makeBreakPointNode(); }
    | BREAK ';'      { $$ = makeBreakNode(); }
    | CONTINUE ';'   { $$ = makeContinueNode(); }
    ;

ifstmt
    : IF '(' expr ')' THEN slist ELSE slist ENDIF ';' { $$ = makeIfElseNode($3, $6, $8); }
    | IF '(' expr ')' THEN slist ENDIF ';'            { $$ = makeIfNode($3, $6); }
    ;

whilestmt
    : WHILE '(' expr ')' DO slist ENDWHILE ';' { $$ = makeWhileNode($3, $6); }
    | DO slist WHILE '(' expr ')' ENDWHILE ';' { $$ = makeDoWhileNode($2, $5); }
    | REPEAT slist UNTIL '(' expr ')' ';'      { $$ = makeRepeatUntilNode($2, $5); }
    ;

inputstmt
    : READ '(' var ')' ';'       { $$ = makeReadNode($3); }
    | READ '(' '*' expr ')' ';' { $$ = makeReadNode(makeDeRefNode($4)); }
    ;

outputstmt
    : WRITE '(' expr ')' ';' { $$ = makeWriteNode($3); }
    ;

assgstmt
    : assg_lhs ASSIGN expr ';' { $$ = makeOperatorNode("=", $1, $3); }
    ;

assg_lhs
    : var       { $$ = $1; }
    | '*' expr { $$ = makeDeRefNode($2); }
    ;

var
    : ID               { $$ = makeLeafNodeId($1); }
    | var '[' expr ']' { $$ = makeArrayNode($1, $3); }
    ;

expr
    : expr PLUS expr          { $$ = makeOperatorNode("+", $1, $3); }
    | expr MINUS expr         { $$ = makeOperatorNode("-", $1, $3); }
    | expr '*' expr          { $$ = makeOperatorNode("*", $1, $3); }
    | expr DIV expr           { $$ = makeOperatorNode("/", $1, $3); }
    | expr LE expr            { $$ = makeOperatorNode("<=", $1, $3); }
    | expr GE expr            { $$ = makeOperatorNode(">=", $1, $3); }
    | expr LT expr            { $$ = makeOperatorNode("<", $1, $3); }
    | expr GT expr            { $$ = makeOperatorNode(">", $1, $3); }
    | expr EQ expr            { $$ = makeOperatorNode("==", $1, $3); }
    | expr NE expr            { $$ = makeOperatorNode("!=", $1, $3); }
    | expr MOD expr           { $$ = makeOperatorNode("%", $1, $3); }
    | '(' expr ')'            { $$ = $2; }
    | NUM                     { $$ = $1; }
    | TEXT                    { $$ = $1; }
    | var                     { $$ = $1; }
    | ID '(' ArgList ')'      { $$ = makeFuncCallNode($1, $3); }
    | ID '(' ')'              { $$ = makeFuncCallNode($1, NULL); }
    | '&' var %prec ADDR      { $$ = makeAddressNode($2); }
    | '*' expr %prec DEREF   { $$ = makeDeRefNode($2); }
    ;

ArgList
    : ArgList ',' expr        { $$ = makeArgNode($1, $3); }
    | expr                    { $$ = $1; }
    ;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}