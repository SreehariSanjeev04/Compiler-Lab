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
}

%token <node> NUM TEXT BREAKPOINT CONTINUE BREAK
%token <vartype> INT STRING
%token PLUS MINUS STAR DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL MOD
%token <str> ID

%type <node> expr program slist stmt inputstmt outputstmt assgstmt ifstmt whilestmt var assg_lhs
%type <vartype> type ptr_decl
%type <str> vardecl arraydecl


%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR DIV
%right ADDR DEREF 
%%

program
    : START declarations slist END ';' 
    {
        root = $3;
    }
    | START END ';'  { printf("Empty program\n"); exit(0); }
    ;

declarations
    : DECL decllist ENDDECL { assignBindingAddresses(); } 
    | DECL ENDDECL         { assignBindingAddresses(); }
    ;

decllist
    : decllist decl {}
    | decl {}
    ;

decl
    : type varlist ';' {}
    ;

varlist
    : varlist ',' vardecl {}
    | vardecl {}    
    ;

vardecl
    : ptr_decl ID {
        if (Lookup($2) != NULL) {
            printf("Error: Variable %s already declared\n", $2);
            exit(1);
        }
        if (currentType == -1) {
            printf("Error: Type not specified for variable %s\n", $2);
            exit(1);
        }
        
        Install($2, currentType, $1);
        $$ = $2;
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
        if (currentType == -1) {
            printf("Error: Type not specified for variable %s\n", $1);
            exit(1);
        }
        Install($1, currentType, 0);
        $$ = $1;
    }
    | arraydecl '[' NUM ']' {
        if (Lookup($1) == NULL) {
            printf("Error: Variable %s not declared\n", $1);
            exit(1);
        }
        struct Gsymbol* symbol = Lookup($1);
        addDimensionSizes(symbol, $3->val);
        $$ = $1;
    }
    ;

ptr_decl
    : STAR ptr_decl { $$ = $2 + 1; }  
    | /* empty */   { $$ = 0; }     
    ;

type
    : INT    { currentType = TYPE_INT; }
    | STRING { currentType = TYPE_STRING; }
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
    | READ '(' STAR expr ')' ';' { $$ = makeReadNode(makeDeRefNode($4)); }
    ;

outputstmt
    : WRITE '(' expr ')' ';' { $$ = makeWriteNode($3); }
    ;


expr
    : expr PLUS expr          { $$ = makeOperatorNode("+", $1, $3); }
    | expr MINUS expr         { $$ = makeOperatorNode("-", $1, $3); }
    | expr STAR expr          { $$ = makeOperatorNode("*", $1, $3); }
    | expr DIV expr           { $$ = makeOperatorNode("/", $1, $3); }
    | expr LE expr            { $$ = makeOperatorNode("<=", $1, $3); }
    | expr GE expr            { $$ = makeOperatorNode(">=", $1, $3); }
    | expr LT expr            { $$ = makeOperatorNode("<", $1, $3); }
    | expr GT expr            { $$ = makeOperatorNode(">", $1, $3); }
    | expr EQ expr            { $$ = makeOperatorNode("==", $1, $3); }
    | expr NE expr            { $$ = makeOperatorNode("!=", $1, $3); }
    | expr MOD expr           { $$ = makeOperatorNode("/", $1, $3); }
    | '(' expr ')'            { $$ = $2; }
    | NUM                     { $$ = $1; }
    | TEXT                    { $$ = $1; }
    | var                     { $$ = $1; }
    | '&' var %prec ADDR      { $$ = makeAddressNode($2); }
    | STAR expr %prec DEREF   { $$ = makeDeRefNode($2); }
    ;

assgstmt
    : assg_lhs ASSIGN expr ';' { $$ = makeOperatorNode("=", $1, $3); }
    ;


assg_lhs
    : var                       { $$ = $1; }
    | STAR expr                 { $$ = makeDeRefNode($2); }
    ;

var
    : ID               { $$ = makeLeafNodeId($1); }
    | var '[' expr ']' { $$ = makeArrayNode($1, $3); }
    ;
    
%%

void yyerror(char const *s)
{
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}