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
    #include <lsymboltable.h>

    int yylex(void);
    void yyerror(char const *s);

    extern FILE* yyin;
    extern int yylineno;

    FILE* targetFile;
    FILE* inputFile;
    extern int yydebug;
    
    tnode* root; // Root of the AST
    int currentType = -1; // Keeping track of the current type
    struct Gsymbol* currentFunction = NULL; // Function whose body is being parsed
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
%token PLUS MINUS '*' DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL MOD MAIN RETURN AND OR NOT
%token <str> ID

%type <node> expr program SList Stmt inputstmt outputstmt assgstmt ifstmt whilestmt var assg_lhs Body MainBlock Fdef FdefBlock ArgList
%type <vartype> type PtrDecl

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left '*' DIV MOD
%right ADDR DEREF NOT

%%

program
    : GDeclBlock FdefBlock MainBlock    { root = $3; }
    | GDeclBlock MainBlock           { root = $2; }
    | MainBlock                      { root = $1; }
    ;

GDeclBlock
    : DECL GDecList ENDDECL {
        assignBindingAddresses();
        generateProgramStart(targetFile); // stack base is final now
        if(showGlobalSymbolTable) printGlobalSymbolTable();
    }
    | DECL ENDDECL         {
        assignBindingAddresses();
        generateProgramStart(targetFile);
        if(showGlobalSymbolTable) printGlobalSymbolTable();
    }
    ;

GDecList
    : GDecList GDecl {}
    | GDecl {}
    ;

GDecl
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
        struct DimNode* headDimNode = DimNodeGetHead();
        handleDimensionSizes(GLookup($1), headDimNode);
        DimNodeReset();
    }
    | ID '(' ParamList ')' {
        struct Gsymbol* funcSymbol = GInstall($1, currentType, 0);
        struct ParamList* head = ParamListGetHead();
        funcSymbol->paramList = head;
        funcSymbol->flabel = generateFunctionLabel();
        ParamListReset();
    }
    | ID '(' ')' {
        struct Gsymbol* funcSymbol = GInstall($1, currentType, 0);
        funcSymbol->flabel = generateFunctionLabel();
        ParamListReset();
    }
    ;

DeclDimList
// Todo - change the lexer to only return the token, not the node
    : DeclDimList '[' NUM ']' {
        DimNodeAppendNode($3->val);
    }
    | '[' NUM ']' {
        DimNodeAppendNode($2->val);
    }
    ;

PtrDecl
    : '*' PtrDecl { $$ = $2 + 1; }  
    | '*'          { $$ = 1; }     
    ;

type
    : INT    { currentType = TYPE_INT; $$ = TYPE_INT; }
    | STRING { currentType = TYPE_STRING; $$ = TYPE_STRING; }
    ;

FdefBlock
    : FdefBlock Fdef { $$ = $1; }
    | Fdef           { $$ = $1; }
    ;

Fdef
    : FdefSig '{' LdeclBlock Body '}' {
        // On-the-fly: emit this function's code now that its body is parsed,
        // then deallocate its AST and local symbol table.
        generateFunctionCode(currentFunction, $4, targetFile);
        $$ = NULL;
    }
    ;

FdefSig
    : type ID '(' ParamList ')' {
        if(strcmp($2, "main") == 0) {
            fprintf(stderr, "Error: Function name 'main' is reserved\n");
            exit(1);
        }
        LSymbolReset();
        struct Gsymbol* funcSymbol = GLookup($2);
        if (funcSymbol == NULL) {
            fprintf(stderr, "Error: Function '%s' not declared.\n", $2);
            exit(1);
        }
        struct ParamList* head = ParamListGetHead();
        struct ParamList* funcParams = funcSymbol->paramList;
        if(ParamListCheckIfParamsMatch(head, funcParams) == false) {
            fprintf(stderr, "Error: Function '%s' parameters do not match declaration.\n", $2);
            exit(1);
        }
        struct ParamList* current = head;

        // Install parameters into the local symbol table
        int paramBinding = -3; // Todo - check this value
        while(current != NULL) {
            LInstall(current->name, current->type, paramBinding--);
            current = current->next;
        }
        ParamListDestroy();
        currentFunction = funcSymbol;
    }
    | type ID '(' ')' {
        if(strcmp($2, "main") == 0) {
            fprintf(stderr, "Error: Function name 'main' is reserved\n");
            exit(1);
        }
        LSymbolReset();
        struct Gsymbol* funcSymbol = GLookup($2);
        if (funcSymbol == NULL) {
            fprintf(stderr, "Error: Function '%s' not declared.\n", $2);
            exit(1);
        }
        if(funcSymbol->paramList != NULL) {
            fprintf(stderr, "Error: Function '%s' parameters do not match declaration.\n", $2);
            exit(1);
        }
        currentFunction = funcSymbol;
    }
    ;

ParamList
    : ParamList ',' Param {}
    | Param {}
    ;

Param
    : type ID { 
        ParamListAppendNode($2, $1);
    }
    ;

MainBlock
    : type MAIN '(' ')' '{' {
        LSymbolReset();
        generateProgramStart(targetFile); // covers programs without a decl block
    } LdeclBlock Body '}' {
        if($1 != TYPE_INT) {
            fprintf(stderr, "Error: Main function must have return type int.\n");
            exit(1);
        }

        struct Gsymbol* funcSymbol = GInstall("MAIN", TYPE_INT, 0);
        funcSymbol->flabel = generateFunctionLabel();
        currentFunction = funcSymbol;
        generateFunctionCode(funcSymbol, $8, targetFile);
        $$ = NULL;
    }
    | type MAIN '(' ')' '{' {
        LSymbolReset();
        generateProgramStart(targetFile); // covers programs without a decl block
    } Body '}'  {
        if(currentType != TYPE_INT) {
            fprintf(stderr, "Error: Main function must have return type int.\n");
            exit(1);
        }

        struct Gsymbol* funcSymbol = GInstall("MAIN", TYPE_INT, 0);
        funcSymbol->flabel = generateFunctionLabel();
        currentFunction = funcSymbol;
        generateFunctionCode(funcSymbol, $7, targetFile);
        $$ = NULL;
    }
    ;

LdeclBlock
    : DECL LdeclList ENDDECL { assignLocalBindingAddresses(); }
    | DECL ENDDECL           { assignLocalBindingAddresses(); }
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
    : ID             { LInstall($1, currentType, 0); }
    ;

Body
    : START SList END ';' { $$ = $2; }
    | START SList END     { $$ = $2; }
    | START END ';'       { printf("Empty program\n"); $$ = NULL; }
    | START END           { printf("Empty program\n"); $$ = NULL; }
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
    | RETURN expr ';' {
        if(currentFunction != NULL &&
           ($2->type != currentFunction->type || $2->pointerLevel != 0)) {
            fprintf(stderr, "Error: Return type of function '%s' does not match its declaration\n",
                    currentFunction->name);
            exit(1);
        }
        $$ = makeReturnNode($2);
    }
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
    | expr AND expr           { $$ = makeOperatorNode("AND", $1, $3); }
    | expr OR expr            { $$ = makeOperatorNode("OR", $1, $3); }
    | expr MOD expr           { $$ = makeOperatorNode("%", $1, $3); }
    | '(' expr ')'            { $$ = $2; }
    | NUM                     { $$ = $1; }
    | STRLIT                  { $$ = $1; }
    | var                     { $$ = $1; }
    | ID '(' ArgList ')'      { $$ = makeFuncCallNode($1, $3); }
    | ID '(' ')'              { $$ = makeFuncCallNode($1, NULL); }
    | '&' var %prec ADDR      { $$ = makeAddressNode($2); }
    | '*' expr %prec DEREF   { $$ = makeDeRefNode($2); }
    | NOT expr %prec NOT     { $$ = makeNotNode($2); }
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