%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <exprtree.h>
    #include <codegen.h>
    #include <gsymboltable.h>
    #include <constants.h>
    #include <gsymboltable_utils.h>
    #include <stdbool.h>
    #include <dimnode.h>
    #include <paramlist.h>
    #include <flabel.h>

    int yylex(void);
    void yyerror(char const *s);

    extern FILE* yyin;
    extern int yylineno;

    FILE* targetFile;
    FILE* inputFile;
    extern int yydebug;
    
    tnode* root; // Root of the AST
    int currentType = -1; // Keeping track of the current type
%}

%define parse.error verbose
%define parse.trace

%union {
    struct tnode *node;
    int vartype;
    char* str;
}

%token <node> NUM STRLIT BREAKPOINT CONTINUE BREAK
%token <vartype> INT STRING
%token PLUS MINUS '*' DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL MOD MAIN
%token <str> ID

%type <node> expr program SList Stmt inputstmt outputstmt assgstmt ifstmt whilestmt var assg_lhs Body MainBlock Fdef FdefBlock
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
    : GdeclList Gdecl {}
    | Gdecl {}
    ;

Gdecl
    : type GidList ';' { currentType = -1; }
    ;

GidList
    : GidList ',' Gid {}
    | Gid {}
    ;

Gid
    : ID {
        GInstall($1, currentType, 0);
    }
    | PtrDecl ID {
        GInstall($2, currentType, $1);
    }
    | ID DeclDimList {
        GInstall($1, currentType, 0);
        struct DimNode* headDimNode = getDimNodeHead();
        handleDimensionSizes(GLookup($1), headDimNode);
        setDimNodeHead(NULL);
    }
    | ID '(' ParamList ')' {
        struct Gsymbol* funcSymbol = GInstall($1, currentType, 0);
        struct ParamList* head = getParamListHead();
        funcSymbol->paramList = head;
        funcSymbol->flabel = generateFunctionLabel();
        setParamListHead(NULL);
    }
    | ID '(' ')' {
        GInstall($1, currentType, 0);
        funcSymbol->flabel = generateFunctionLabel();
        setParamListHead(NULL);
    }
    ;

DeclDimList
    : DeclDimList '[' NUM ']' {
        headDimNode = appendDimNode(headDimNode, $3);
    }
    | '[' NUM ']' {
        headDimNode = appendDimNode(headDimNode, $2);
    }
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
    : type ID '(' ParamList ')' '{' LdeclBlock Body '}' { $$ = $8; }
    | type ID '(' ')' '{' LdeclBlock Body '}'           { $$ = $7; }
    ;

ParamList
    : ParamList ',' Param {}
    | Param {}
    ;

Param
    : type ID { 
        struct ParamList* head = getParamListHead();
        appendParamListNode(head, $2, $1);
    }
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
    : START SList END ';' { $$ = $2; }
    | START END ';'       { printf("Empty program\n"); $$ = NULL; }
    ;

SList
    : SList Stmt { $$ = makeConnectorNode($1, $2); }
    | Stmt       { $$ = $1; }
    ;

Stmt
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
    : IF '(' expr ')' THEN SList ELSE SList ENDIF ';' { $$ = makeIfElseNode($3, $6, $8); }
    | IF '(' expr ')' THEN SList ENDIF ';'            { $$ = makeIfNode($3, $6); }
    ;

whilestmt
    : WHILE '(' expr ')' DO SList ENDWHILE ';' { $$ = makeWhileNode($3, $6); }
    | DO SList WHILE '(' expr ')' ENDWHILE ';' { $$ = makeDoWhileNode($2, $5); }
    | REPEAT SList UNTIL '(' expr ')' ';'      { $$ = makeRepeatUntilNode($2, $5); }
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
    | STRLIT                  { $$ = $1; }
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