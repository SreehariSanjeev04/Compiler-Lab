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
    #include <tupletable.h>
    #include <tuplefieldlist.h>

    int yylex(void);
    void yyerror(char const *s);

    extern int yylineno;

    extern FILE* targetFile;
    extern int yydebug;
    
    int currentType = -1; 
    struct Gsymbol* currentFunction = NULL;
    struct TupleTable* currentTupleEntry = NULL;
%}

%define parse.error verbose
%define parse.trace

%union {
    struct tnode *node;
    int val;
    char* str;
}

%token <val> INT STRING NUM
%token PLUS MINUS '*' DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL MOD MAIN RETURN AND OR NOT TUPLE BREAKPOINT CONTINUE BREAK
%token <str> ID STRLIT

%type <node> Expr Program SList Stmt InputStmt OutputStmt AssgStmt IfStmt WhileStmt Var AssgLhs Body MainBlock Fdef FdefBlock ArgList TupleDecl TupleFieldList FieldPtrAccess
%type <val> Type PtrDecl

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left '*' DIV MOD
%right ADDR DEREF NOT

%%

Program
    : GDeclBlock FdefBlock MainBlock    { }
    | GDeclBlock MainBlock           { }
    | MainBlock                      { }
    ;

GDeclBlock
    : DECL GDecList ENDDECL {
        assignBindingAddresses();
        generateProgramStart(targetFile);
        if(showGlobalSymbolTable) printGlobalSymbolTable(); // optional btw
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
    : Type GidList ';' { currentType = -1; }
    ;

GidList
    : GidList ',' Gid {}
    | Gid {}
    ;

Gid
    : ID {
        struct Gsymbol* symbol = GInstall($1, currentType, 0);
        if (currentType == TYPE_TUPLE) {
            if (currentTupleEntry == NULL) {
                fprintf(stderr, "Error: No tuple type is associated with the declaration of '%s'\n", $1);
                exit(1);
            }
            symbol->tupleEntry = currentTupleEntry;
            symbol->size = currentTupleEntry->size;
        }
    }
    | PtrDecl ID {
        struct Gsymbol* symbol = GInstall($2, currentType, $1);
        if (currentType == TYPE_TUPLE) {
            if (currentTupleEntry == NULL) {
                fprintf(stderr, "Error: No tuple type is associated with the declaration of '%s'\n", $2);
                exit(1);
            }
            symbol->tupleEntry = currentTupleEntry;
        }
    }
    | ID DeclDimList {
        if (currentType == TYPE_TUPLE) {
            fprintf(stderr, "Error: Arrays of tuple type are not supported\n");
            exit(1);
        }
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
        DimNodeAppendNode($3);
    }
    | '[' NUM ']' {
        DimNodeAppendNode($2);
    }
    ;

PtrDecl
    : '*' PtrDecl { $$ = $2 + 1; }  
    | '*'          { $$ = 1; }     
    ;

Type
    : INT    { currentType = TYPE_INT; $$ = TYPE_INT; }
    | STRING { currentType = TYPE_STRING; $$ = TYPE_STRING; }
    | TupleDecl { currentType = TYPE_TUPLE; $$ = TYPE_TUPLE; }
    ;

TupleDecl
    : TUPLE ID '(' TupleFieldList ')' {
        struct TupleFieldList* head = TupleFieldListGetHead();
        TupleTableAppend($2, head);
        TupleFieldListReset();
        currentTupleEntry = TupleTableLookup($2);
        if (currentTupleEntry == NULL) {
            fprintf(stderr, "Error: Tuple type '%s' could not be registered\n", $2);
            exit(1);
        }
        $$ = NULL;
    }
    ;

TupleFieldList
    : TupleFieldList ',' Type ID {
        if ($3 == TYPE_TUPLE) {
            fprintf(stderr, "Error: Only scalar fields are supported in tuple declarations\n");
            exit(1);
        }
        TupleFieldListAppend($4, $3);
        $$ = NULL;
    }
    | Type ID {
        if ($1 == TYPE_TUPLE) {
            fprintf(stderr, "Error: Only scalar fields are supported in tuple declarations\n");
            exit(1);
        }
        TupleFieldListAppend($2, $1);
        $$ = NULL;
    }
    ;

FdefBlock
    : FdefBlock Fdef { $$ = $1; }
    | Fdef           { $$ = $1; }
    ;

Fdef
    : FdefSig '{' LdeclBlock Body '}' {
        generateFunctionCode(currentFunction, $4, targetFile);
        $$ = NULL;
    }
    | FdefSig '{' Body '}' {
        generateFunctionCode(currentFunction, $3, targetFile);
        $$ = NULL;
    }
    ;

FdefSig
    : Type ID '(' ParamList ')' {
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

        int paramBinding = -3; // the first parameter would be at BP-3
        while(current != NULL) {
            LInstall(current->name, current->type, current->pointerLevel, paramBinding--);
            current = current->next;
        }
        ParamListDestroy();
        currentFunction = funcSymbol;
    }
    | Type ID '(' ')' {
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
    : Type ID { 
        if ($1 == TYPE_TUPLE) {
            fprintf(stderr, "Error: Tuple parameters are not supported\n");
            exit(1);
        }
        ParamListAppendNode($2, $1, 0);
    }
    | Type PtrDecl ID {
        if ($1 == TYPE_TUPLE) {
            fprintf(stderr, "Error: Tuple parameters are not supported\n");
            exit(1);
        }
        ParamListAppendNode($3, $1, $2);
    }
    ;

MainBlock
    : Type MAIN '(' ')' '{' {
        LSymbolReset();
    } LdeclBlock Body '}' {
        if($1 != TYPE_INT) {
            fprintf(stderr, "Error: Main function must have return Type int.\n");
            exit(1);
        }

        struct Gsymbol* funcSymbol = GInstall("MAIN", TYPE_INT, 0);
        funcSymbol->flabel = generateFunctionLabel();
        currentFunction = funcSymbol;
        generateProgramStart(targetFile);
        generateFunctionCode(funcSymbol, $8, targetFile);
        $$ = NULL;
    }
    | Type MAIN '(' ')' '{' {
        LSymbolReset();
    } Body '}'  {
        if(currentType != TYPE_INT) {
            fprintf(stderr, "Error: Main function must have return Type int.\n");
            exit(1);
        }

        struct Gsymbol* funcSymbol = GInstall("MAIN", TYPE_INT, 0);
        funcSymbol->flabel = generateFunctionLabel();
        currentFunction = funcSymbol;
        generateProgramStart(targetFile); 
        generateFunctionCode(funcSymbol, $7, targetFile);
        $$ = NULL;
    }
    ;

LdeclBlock
    : DECL LdeclList ENDDECL { LSymbolAssignBindingAddresses(); }
    | DECL ENDDECL           { LSymbolAssignBindingAddresses(); }
    ;

LdeclList
    : LdeclList Ldecl 
    | Ldecl
    ;

Ldecl
    : Type LidList ';'
    ;

LidList
    : LidList ',' Lid
    | Lid
    ;

Lid
    : ID {
        if (currentType == TYPE_TUPLE) {
            fprintf(stderr, "Error: Local tuple variables are not supported yet\n");
            exit(1);
        }
        LInstall($1, currentType, 0, 0);
    }
    ;

Body
    : START SList END ';' { $$ = $2; }
    | START SList END     { $$ = $2; }
    | START END ';'       { printf("Empty Program\n"); $$ = NULL; }
    | START END           { printf("Empty Program\n"); $$ = NULL; }
    ;

SList
    : SList Stmt { $$ = makeConnectorNode($1, $2); }
    | Stmt       { $$ = $1; }
    ;

Stmt
    : InputStmt      { $$ = $1; }
    | OutputStmt     { $$ = $1; }
    | AssgStmt       { $$ = $1; }
    | IfStmt         { $$ = $1; }
    | WhileStmt      { $$ = $1; }
    | BREAKPOINT ';' { $$ = makeBreakPointNode(); }
    | BREAK ';'      { $$ = makeBreakNode(); }
    | CONTINUE ';'   { $$ = makeContinueNode(); }
    | RETURN Expr ';' {
        // check the return type of the function as well as the pointer level
        if(currentFunction != NULL &&
           ($2->type != currentFunction->type || $2->pointerLevel != 0)) {
            fprintf(stderr, "Error: Return Type of function '%s' does not match its declaration\n",
                    currentFunction->name);
            exit(1);
        }
        $$ = makeReturnNode($2);
    }
    ;

IfStmt
    : IF '(' Expr ')' THEN SList ELSE SList ENDIF ';' { $$ = makeIfElseNode($3, $6, $8); }
    | IF '(' Expr ')' THEN SList ENDIF ';'            { $$ = makeIfNode($3, $6); }
    ;

WhileStmt
    : WHILE '(' Expr ')' DO SList ENDWHILE ';' { $$ = makeWhileNode($3, $6); }
    | DO SList WHILE '(' Expr ')' ENDWHILE ';' { $$ = makeDoWhileNode($2, $5); }
    | REPEAT SList UNTIL '(' Expr ')' ';'      { $$ = makeRepeatUntilNode($2, $5); }
    ;

InputStmt
    : READ '(' Var ')' ';'       { $$ = makeReadNode($3); }
    | READ '(' '*' Expr ')' ';' { $$ = makeReadNode(makeDeRefNode($4)); }
    | READ '(' FieldPtrAccess ')' ';' { $$ = makeReadNode($3); }
    ;

FieldPtrAccess
    : '(' '*' Expr ')' '.' ID { $$ = makeTupleNode(makeDeRefNode($3), $6); }
    ;

OutputStmt
    : WRITE '(' Expr ')' ';' { $$ = makeWriteNode($3); }
    ;

AssgStmt
    : AssgLhs ASSIGN Expr ';' { $$ = makeOperatorNode("=", $1, $3); }
    ;

AssgLhs
    : Var       { $$ = $1; }
    | '*' Expr { $$ = makeDeRefNode($2); }
    | FieldPtrAccess { $$ = $1; }
    ;

Var
    : ID               { $$ = makeLeafNodeId($1); }
    | Var '[' Expr ']' { $$ = makeArrayNode($1, $3); }
    | Var '.' ID { $$ = makeTupleNode($1, $3); }
    ;

Expr
    : Expr PLUS Expr          { $$ = makeOperatorNode("+", $1, $3); }
    | Expr MINUS Expr         { $$ = makeOperatorNode("-", $1, $3); }
    | Expr '*' Expr          { $$ = makeOperatorNode("*", $1, $3); }
    | Expr DIV Expr           { $$ = makeOperatorNode("/", $1, $3); }
    | Expr LE Expr            { $$ = makeOperatorNode("<=", $1, $3); }
    | Expr GE Expr            { $$ = makeOperatorNode(">=", $1, $3); }
    | Expr LT Expr            { $$ = makeOperatorNode("<", $1, $3); }
    | Expr GT Expr            { $$ = makeOperatorNode(">", $1, $3); }
    | Expr EQ Expr            { $$ = makeOperatorNode("==", $1, $3); }
    | Expr NE Expr            { $$ = makeOperatorNode("!=", $1, $3); }
    | Expr AND Expr           { $$ = makeOperatorNode("AND", $1, $3); }
    | Expr OR Expr            { $$ = makeOperatorNode("OR", $1, $3); }
    | Expr MOD Expr           { $$ = makeOperatorNode("%", $1, $3); }
    | '(' Expr ')'            { $$ = $2; }
    | NUM                     { $$ = makeLeafNodeNum($1); }
    | STRLIT                  { $$ = makeLeafNodeString($1); }
    | Var                     { $$ = $1; }
    | ID '(' ArgList ')'      { $$ = makeFuncCallNode($1, $3); }
    | ID '(' ')'              { $$ = makeFuncCallNode($1, NULL); }
    | '&' Var %prec ADDR      { $$ = makeAddressNode($2); }
    | '*' Expr %prec DEREF   { $$ = makeDeRefNode($2); }
    | NOT Expr %prec NOT     { $$ = makeNotNode($2); }
    | '(' '*' Expr ')' '.' ID { $$ = makeTupleNode(makeDeRefNode($3), $6); } 
    ;

ArgList
    : ArgList ',' Expr        { $$ = makeArgNode($1, $3); }
    | Expr                    { $$ = $1; }
    ;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}