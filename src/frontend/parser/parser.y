%{
    #include "logger.h"
    #include <stdlib.h>
    #include <stdio.h>
    #include <exprtree.h>
    #include <codegen.h>
    #include <gsymboltable.h>
    #include <constants.h>
    #include <gsymboltable_utils.h>
    #include <stdbool.h>
    #include <paramlist.h>
    #include <flabel.h>
    #include <lsymboltable.h>
    #include <typetable.h>
    #include <fieldlist.h>
    #include <declspec.h>

    int yylex(void);
    void yyerror(char const *s);

    extern int yylineno;

    extern FILE* targetFile;
    extern int yydebug;

    struct Gsymbol* currentFunction = NULL;
    struct ClassTable* currentClass = NULL;
    static bool seenMethodDeclaration = false;

    bool insideClass(void) {
        return currentClass != NULL;
    }

    static struct Gsymbol methodDefinitionSymbol;
    static FILE* classMethodsFile = NULL;

    static FILE* functionOutput(void) {
        if (!insideClass()) return targetFile;
        if (classMethodsFile == NULL) {
            classMethodsFile = tmpfile();
            if (classMethodsFile == NULL) {
                LOG_ERROR("Could not create temporary storage for class methods.\n");
                exit(1);
            }
        }
        return classMethodsFile;
    }

    static void finishGlobalDeclarations(void) {
        assignBindingAddresses();
        generateProgramStart(targetFile);
        if (classMethodsFile != NULL) {
            rewind(classMethodsFile);
            int ch;
            while ((ch = fgetc(classMethodsFile)) != EOF) {
                if (fputc(ch, targetFile) == EOF) {
                    LOG_ERROR("Could not write class method code.\n");
                    exit(1);
                }
            }
            if (ferror(classMethodsFile)) {
                LOG_ERROR("Could not read class method code.\n");
                exit(1);
            }
            fclose(classMethodsFile);
            classMethodsFile = NULL;
        }
    }

    static struct Gsymbol* lookupDefinitionSymbol(char* name) {
        if (!insideClass()) return GLookup(name);

        struct MemberFuncList* method = CMemLookup(currentClass, name);
        if (method == NULL) return NULL;

        methodDefinitionSymbol = (struct Gsymbol){
            .name = method->name,
            .type = method->type,
            .paramList = method->paramList,
            .flabel = method->Flabel
        };
        return &methodDefinitionSymbol;
    }

%}

%code requires
{
  #include <declspec.h>
}

%define parse.error verbose
%define parse.trace

%union {
    struct tnode *node;
    int val;
    char* str;
    DeclType* declType;
    Declarator* declarator;
    struct ClassTable* cptr;
}

%token <val> INT STRING NUM
%token PLUS MINUS '*' DIV ASSIGN START END READ WRITE EQ NE LT GT GE LE IF ELSE WHILE DO ENDWHILE THEN ENDIF REPEAT UNTIL DECL ENDDECL MOD MAIN RETURN AND OR NOT BREAKPOINT CONTINUE BREAK CTYPE ENDTYPE ALLOC FREE INITIALIZE NULLLIT CLASS ENDCLASS EXTENDS SELF NEW DELETE
%token <str> ID STRLIT

%type <node> Expr Program SList Stmt InputStmt OutputStmt AssgStmt IfStmt WhileStmt Var AssgLhs Body MainBlock Fdef FdefBlock ArgList
%type <declType> Type
%type <declarator> Gid GidList Lid LidList
%type <cptr> Cname
%type <val> PtrDecl

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left '*' DIV MOD
%right ADDR DEREF NOT

%%

Program
    : TypeDeclBlock ClassDefBlock GDeclBlock FdefBlock MainBlock { $$ = $5; }
    ;

TypeDeclBlock 
    : CTYPE TypeDeclList ENDTYPE
    | %empty
    ;

TypeDeclList
    : TypeDeclList TypeDecl
    | TypeDecl
    ;

TypeDecl
    : ID {
        if (TLookup($1) != NULL) { LOG_ERROR("Type already exists: %s\n", $1); exit(1); }
        TInstall($1, NULL);
        FieldListReset();
    } '{' TypeFieldList '}' {
        TypeTable* typeEntry = TLookup($1);
        typeEntry->fields = FieldListReturnHead();
        typeEntry->size = fieldIndexCounter;
        if (typeEntry->size > 8) { LOG_ERROR("too many member fields\n"); exit(1); }
    }
    ;

TypeFieldList
    : TypeFieldList TypeField {}
    | TypeField {}
    ;

TypeField
    : Type ID ';' {
        if ($1->cptr != NULL) {
            LOG_ERROR("Class types are not supported in type fields.\n");
            exit(1);
        }
        TypeTable* typeEntry = $1->table;
        if (typeEntry == NULL) {
            LOG_ERROR("Type specified in the field does not exist.");
            exit(1);
        }

        for (FieldList* field = FieldListReturnHead(); field; field = field->next)
            if (strcmp(field->name, $2) == 0) { LOG_ERROR("Duplicate field: %s\n", $2); exit(1); }
        FieldListAppend($2, typeEntry);
        free($1);
    }
    ;

ClassDefBlock
    : CLASS ClassDefList ENDCLASS
    | %empty
    ;

ClassDefList
    : ClassDefList ClassDef {}
    | ClassDef {}
    ;

ClassDef
    : Cname {
        /* for methods */
        currentClass = $1;
        FieldListReset();
        MemberFuncReset();
        seenMethodDeclaration = false;
    } '{' DECL ClassMembers ENDDECL {
        CFieldInstall($1, FieldListReturnHead());
        CMemInstall($1, MemFuncGetHead());
    } MethodDefns '}' {
        currentClass = NULL;
    }
    ;

Cname
    : ID { $$ = CInstall($1, NULL); }
    | ID EXTENDS ID { $$ = CInstall($1, $3); }
    ;

ClassMembers
    : ClassMembers ClassMember {}
    | %empty
    ;

ClassMember
    : Fld {}
    | Mdecl {}
    ;

Fld
    : Type ID ';' {
        if (seenMethodDeclaration) {
            LOG_ERROR("Fields must be declared before methods.\n");
            exit(1);
        }
        if ($1->cptr != NULL)
            FieldListAppendClass($2, $1->cptr);
        else
            FieldListAppend($2, $1->table);
        free($1);
    }
    ;

Mdecl
    : Type ID '(' ParamList ')' ';' {
        seenMethodDeclaration = true;
        if ($1->cptr != NULL) {
            LOG_ERROR("Methods cannot return a class type.\n");
            exit(1);
        }
        MemberFuncList *method = MemFuncAppend($2, $1->table);
        MemberFuncInstallParam(method, ParamListGetHead());
        ParamListReset();
        free($1);
        free($2);
    }
    | Type ID '(' ')' ';' {
        seenMethodDeclaration = true;
        if ($1->cptr != NULL) {
            LOG_ERROR("Methods cannot return a class type.\n");
            exit(1);
        }
        MemberFuncList *method = MemFuncAppend($2, $1->table);
        free($1);
        free($2);
    }
    ;

MethodDefns
    : MethodDefns Fdef {}
    | %empty
    ;

GDeclBlock
    : DECL GDecList ENDDECL {
        finishGlobalDeclarations();
        if(showGlobalSymbolTable) printGlobalSymbolTable(); 
    }
    | DECL ENDDECL         {
        finishGlobalDeclarations();
        if(showGlobalSymbolTable) printGlobalSymbolTable();
    }
    | %empty { finishGlobalDeclarations(); }
    ;

GDecList
    : GDecList GDecl {}
    | GDecl {}
    ;

GDecl
    : Type GidList ';' { installDeclarators($1, $2, true); }
    ;

GidList
    : GidList ',' Gid { $$ = appendDeclarator($1, $3); }
    | Gid { $$ = $1; }
    ;

Gid
    : ID { $$ = makeDeclarator($1, DECL_VARIABLE, 0, 0, NULL); }
    | PtrDecl ID { $$ = makeDeclarator($2, DECL_POINTER, $1, 0, NULL); }
    | ID '[' NUM ']' {
        $$ = makeDeclarator($1, DECL_ARRAY, 0, $3, NULL);
    }
    | ID '(' ParamList ')' {
        $$ = makeDeclarator($1, DECL_FUNCTION, 0, 0, ParamListGetHead());
        ParamListReset();
    }
    | ID '(' ')' {
        $$ = makeDeclarator($1, DECL_FUNCTION, 0, 0, NULL);
        ParamListReset();
    }
    ;

PtrDecl
    : '*' PtrDecl { $$ = $2 + 1; }
    | '*'          { $$ = 1; }
    ;

Type
    : INT    { $$ = makeDeclTypeForType(TYPE_INT, TLookup("int")); }
    | STRING { $$ = makeDeclTypeForType(TYPE_STRING, TLookup("str")); }
    | ID {
        if (TLookup($1) != NULL) {
            $$ = makeDeclTypeForType(TYPE_CUSTOM, TLookup($1));
        } else if (CLookup($1) != NULL) {
            $$ = makeDeclTypeForClass(TYPE_CLASS, CLookup($1));
        } else {
            LOG_ERROR("Type '%s' does not exist.\n", $1);
            exit(1);
        }
    }
    ;

FdefBlock
    : FdefBlock Fdef { $$ = NULL; }
    | %empty         { $$ = NULL; }
    ;

/* shared between normal declarations and class methods */
Fdef
    : FdefSig '{' LdeclBlock Body '}' {
        generateFunctionCode(currentFunction, $4, functionOutput());
        $$ = NULL;
    }
    | FdefSig '{' Body '}' {
        generateFunctionCode(currentFunction, $3, functionOutput());
        $$ = NULL;
    }
    ;

FdefSig
    : Type ID '(' ParamList ')' {
        if ($1->cptr != NULL) {
            LOG_ERROR("Functions and methods cannot return a class type.\n");
            exit(1);
        }
        if(strcmp($2, "main") == 0) {
            LOG_ERROR("Error: Function name 'main' is reserved\n");
            exit(1);
        }
        LSymbolReset();

        struct Gsymbol* funcSymbol = lookupDefinitionSymbol($2);

        if (funcSymbol == NULL) {
            LOG_ERROR("Error: Function '%s' not declared.\n", $2);
            exit(1);
        }

        if ($1->table != funcSymbol->type) {
            LOG_ERROR("The function type does not match.");
            exit(1);
        }
        struct ParamList* head = ParamListGetHead();
        struct ParamList* funcParams = funcSymbol->paramList;
        if(ParamListCheckIfParamsMatch(head, funcParams) == false) {
            LOG_ERROR("Error: Function '%s' parameters do not match declaration.\n", $2);
            exit(1);
        }
        struct ParamList* current = head;

        int nextParamWord = -3;
        while(current != NULL) {
            struct Lsymbol* local = LInstall(current->name, current->type,
                                             current->pointerLevel, nextParamWord);
            nextParamWord--;
            current = current->next;
        }
        if (insideClass()) LInstall("self", TLookup("int"), 0, nextParamWord); // installing self into local symbol table
        ParamListDestroy();
        currentFunction = funcSymbol;
        free($1);
    }
    | Type ID '(' ')' {
        if ($1->cptr != NULL) {
            LOG_ERROR("Functions and methods cannot return a class type.\n");
            exit(1);
        }
        if(strcmp($2, "main") == 0) {
            LOG_ERROR("Error: Function name 'main' is reserved\n");
            exit(1);
        }
        LSymbolReset();
        struct Gsymbol* funcSymbol = lookupDefinitionSymbol($2);
        if (funcSymbol == NULL) {
            LOG_ERROR("Error: Function '%s' not declared.\n", $2);
            exit(1);
        }

        if ($1->table != funcSymbol->type) {
            LOG_ERROR("The function type does not match.");
            exit(1);
        }

        if(funcSymbol->paramList != NULL) {
            LOG_ERROR("Error: Function '%s' parameters do not match declaration.\n", $2);
            exit(1);
        }
        if (insideClass()) LInstall("self", TLookup("int"), 0, -3); // installing self into local symbol table
        currentFunction = funcSymbol;
        free($1);
    }
    ;

ParamList
    : ParamList ',' Param {}
    | Param {}
    ;

Param
    : Type ID {
        if ($1->cptr != NULL) {
            LOG_ERROR("Class types cannot be function parameters.\n");
            exit(1);
        }
        ParamListAppendNode($2, $1->table, 0);
        free($1);
    }
    | Type PtrDecl ID {
        if ($1->cptr != NULL) {
            LOG_ERROR("Class types cannot be function parameters.\n");
            exit(1);
        }
        ParamListAppendNode($3, $1->table, $2);
        free($1);
    }

    ;

MainBlock
    : MainBlockSig '{' LdeclBlock Body '}' {

        struct Gsymbol* funcSymbol = GLookup("MAIN");
        generateFunctionCode(funcSymbol, $4, targetFile);
        $$ = NULL;
    }
    | MainBlockSig '{' Body '}'  {
        struct Gsymbol* funcSymbol = GLookup("MAIN");
        generateFunctionCode(funcSymbol, $3, targetFile);
        $$ = NULL;
    }
    ;

MainBlockSig :
    Type MAIN '(' ')' {
        if($1->kind != TYPE_INT) {
            LOG_ERROR("Error: Main function must have return Type int.\n");
            exit(1);
        }

        struct Gsymbol* funcSymbol = GInstall("MAIN", TLookup("int"), NULL, 0);
        funcSymbol->flabel = generateFunctionLabel();
        currentFunction = funcSymbol;
        LSymbolReset();
        free($1);
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
    : Type LidList ';' { installDeclarators($1, $2, false); }
    ;

LidList
    : LidList ',' Lid { $$ = appendDeclarator($1, $3); }
    | Lid { $$ = $1; }
    ;

Lid
    : ID { $$ = makeDeclarator($1, DECL_VARIABLE, 0, 0, NULL); }
    ;
Body
    : START SList END ';' { $$ = $2; }
    | START SList END     { $$ = $2; }
    | START END ';'       { LOG_INFO("Empty Program\n"); $$ = NULL; }
    | START END           { LOG_INFO("Empty Program\n"); $$ = NULL; }
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
    | INITIALIZE '(' ')' ';' { $$ = makeInitializeNode(); $$->val = 1; }
    | FREE '(' Var ')' ';' { $$ = makeFreeNode($3); $$->val = 1; }
    | RETURN Expr ';' {
        if(currentFunction != NULL &&
           !(($2->typeEntry == currentFunction->type && $2->pointerLevel == 0) ||
             ($2->nodetype == NODE_TYPE_NULL && GTypeKind(currentFunction) == TYPE_CUSTOM))) {
            LOG_INFO("Type: %d -> %d", $2->type, GTypeKind(currentFunction));
            LOG_ERROR("Error: Return Type of function '%s' does not match its declaration\n",
                    currentFunction->name);
            exit(1);
        }
        $$ = makeReturnNode($2);
    }
    | DELETE '(' ID ')' ';' { $$ = makeDeleteClassNode($3); }
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
    ;

Var
    : ID               { $$ = makeLeafNodeId($1); }
    | SELF { $$ = makeSelfNode(currentClass); }
    | Var '[' Expr ']' { $$ = makeArrayNode($1, $3); }
    | Var '.' ID {
        $$ = $1->type == TYPE_CLASS ? makeClassFieldNode($1, $3)
                                    : makeUserTypeNode($1, $3);
    }
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
    | Var '.' ID '(' ArgList ')' { $$ = makeClassMethodNode($1, $3, $5); }
    | Var '.' ID '(' ')'      { $$ = makeClassMethodNode($1, $3, NULL); }
    | ID '(' ArgList ')'      { $$ = makeFuncCallNode($1, $3); }
    | ID '(' ')'              { $$ = makeFuncCallNode($1, NULL); }
    | ALLOC '(' ')'           { $$ = makeAllocNode(); }
    | INITIALIZE '(' ')'      { $$ = makeInitializeNode(); }
    | FREE '(' Var ')'        { $$ = makeFreeNode($3); }
    | NULLLIT                { $$ = makeNullNode(); }
    | '&' Var %prec ADDR      { $$ = makeAddressNode($2); }
    | '*' Expr %prec DEREF   { $$ = makeDeRefNode($2); }
    | NOT Expr %prec NOT     { $$ = makeNotNode($2); }
    | NEW '(' ID ')' { $$ = makeNewClassNode($3); }
    ;

ArgList
    : ArgList ',' Expr        { $$ = makeArgNode($1, $3); }
    | Expr                    { $$ = $1; }
    ;

%%

void yyerror(char const *s)
{
    LOG_ERROR("Syntax Error: %s at line %d\n", s, yylineno);
}
