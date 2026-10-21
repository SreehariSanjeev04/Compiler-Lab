#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exprtree.h"
#include "constants.h"
#include "gsymboltable.h"
#include <lsymboltable.h>
#include <fieldlist.h>
#include <classtable.h>

static tnode *tnodeInit(void);
static int countCallArgs(tnode *argsNode);
static void flattenCallArgs(tnode *argsNode, tnode **argArray, int *index);
static void checkCallArguments(struct Gsymbol *entry, tnode *args);

static tnode *tnodeInit(void) {
    tnode *temp = (tnode *)malloc(sizeof(tnode));
    if (temp == NULL) {
        LOG_ERROR("Error: Memory allocation failed for AST node\n");
        exit(1);
    }
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->typeEntry = TLookup("void");
    temp->cPtr = NULL;
    temp->pointerLevel = 0;
    temp->varname = NULL;
    temp->nodetype = -1;
    temp->left = NULL;
    temp->middle = NULL;
    temp->right = NULL;
    temp->Gentry = NULL;
    temp->Lentry = NULL;
    return temp;
}


tnode *makeLeafNodeNum(int n) {
    tnode *temp = tnodeInit();
    temp->val = n;
    temp->type = TYPE_INT;
    temp->typeEntry = TLookup("int");
    temp->nodetype = NODE_TYPE_NUM;
    return temp;
}

tnode *makeLeafNodeId(const char *c) {
    tnode *temp = tnodeInit();
    temp->varname = strdup(c);
    if (temp->varname == NULL) {
        LOG_ERROR("Error: Memory allocation failed for variable name\n");
        exit(1);
    }
    temp->nodetype = NODE_TYPE_ID;

    struct Lsymbol *lentry = LLookup(temp->varname);
    if (lentry != NULL) {
        temp->type = lentry->type;
        temp->typeEntry = lentry->typeEntry;
        temp->pointerLevel = lentry->pointerLevel;
        temp->Lentry = lentry;
        temp->cPtr = lentry->cPtr;
        return temp;
    }
    struct Gsymbol *gentry = GLookup(c);

    if (gentry != NULL) {
        if (gentry->flabel >= 0) {
            LOG_ERROR("Error: Function '%s' cannot be used as a variable\n", c);
            exit(1);
        }
        temp->Gentry = gentry;
        temp->type = GTypeKind(gentry);
        temp->typeEntry = gentry->type;
        temp->cPtr = gentry->cPtr;
        temp->pointerLevel = gentry->pointerLevel;
        return temp;
    }
    LOG_ERROR("Error: Variable '%s' not declared\n", c);
    exit(1);
}

tnode *makeLeafNodeString(const char *str) {
    tnode *temp = tnodeInit();
    temp->varname = strdup(str);
    if (temp->varname == NULL) {
        LOG_ERROR("Error: Memory allocation failed for string value\n");
        exit(1);
    }
    temp->type = TYPE_STRING;
    temp->typeEntry = TLookup("str");
    temp->nodetype = NODE_TYPE_STRING;
    return temp;
}



tnode *makeOperatorNode(const char *op, tnode *l, tnode *r) {
    tnode *temp = tnodeInit();
    temp->left = l;
    temp->right = r;

    if (strcmp(op, "+") == 0)
    {
        temp->nodetype = NODE_TYPE_PLUS;
        temp->type = TYPE_INT;
        if (l->pointerLevel > 0)
        {
            temp->pointerLevel = l->pointerLevel;
            temp->type = l->type;
        }
        else if (r->pointerLevel > 0)
        {
            temp->pointerLevel = r->pointerLevel;
            temp->type = r->type;
        }
    }
    else if (strcmp(op, "-") == 0)
    {
        temp->nodetype = NODE_TYPE_MINUS;
        temp->type = TYPE_INT;
        // pointer - integer stays a pointer (keeps its base type); pointer - pointer yields an integer
        if (l->pointerLevel > 0 && r->pointerLevel == 0)
        {
            temp->pointerLevel = l->pointerLevel;
            temp->type = l->type;
        }
    }
    else if (strcmp(op, "*") == 0)  { temp->nodetype = NODE_TYPE_MUL; temp->type = TYPE_INT; }
    else if (strcmp(op, "/") == 0)  { temp->nodetype = NODE_TYPE_DIV; temp->type = TYPE_INT; }
    else if (strcmp(op, "%") == 0)  { temp->nodetype = NODE_TYPE_MOD; temp->type = TYPE_INT; }
    else if (strcmp(op, "=") == 0)  { temp->nodetype = NODE_TYPE_ASSIGN; temp->type = TYPE_VOID; }
    else if (strcmp(op, "<") == 0)  { temp->nodetype = NODE_TYPE_LT; temp->type = TYPE_BOOL; }
    else if (strcmp(op, ">") == 0)  { temp->nodetype = NODE_TYPE_GT; temp->type = TYPE_BOOL; }
    else if (strcmp(op, "<=") == 0) { temp->nodetype = NODE_TYPE_LE; temp->type = TYPE_BOOL; }
    else if (strcmp(op, ">=") == 0) { temp->nodetype = NODE_TYPE_GE; temp->type = TYPE_BOOL; }
    else if (strcmp(op, "==") == 0) { temp->nodetype = NODE_TYPE_EQ; temp->type = TYPE_BOOL; }
    else if (strcmp(op, "!=") == 0) { temp->nodetype = NODE_TYPE_NE; temp->type = TYPE_BOOL; }
    else if (strcmp(op, "AND") == 0) {
        temp->nodetype = NODE_TYPE_AND;
        temp->type = TYPE_BOOL;
        if (!((l->type == TYPE_BOOL || l->type == TYPE_INT) && (r->type == TYPE_BOOL || r->type == TYPE_INT))) {
            LOG_ERROR("Error: AND requires boolean or integer operands\n");
            exit(1);
        }
    }
    else if (strcmp(op, "OR") == 0) {
        temp->nodetype = NODE_TYPE_OR;
        temp->type = TYPE_BOOL;
        if (!((l->type == TYPE_BOOL || l->type == TYPE_INT) && (r->type == TYPE_BOOL || r->type == TYPE_INT))) {
            LOG_ERROR("Error: OR requires boolean or integer operands\n");
            exit(1);
        }
    }
    else {
        LOG_ERROR("Error: Unknown operator %s\n", op);
        exit(1);
    }

    temp->typeEntry = temp->type == TYPE_INT ? TLookup("int") :
                      temp->type == TYPE_BOOL ? TLookup("bool") :
                      temp->type == TYPE_STRING ? TLookup("str") : TLookup("void");
    if (temp->pointerLevel > 0 && l->pointerLevel > 0) temp->typeEntry = l->typeEntry;
    else if (temp->pointerLevel > 0 && r->pointerLevel > 0) temp->typeEntry = r->typeEntry;
    return temp;
}

tnode *makeNotNode(tnode *operand) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_NOT;
    temp->type = TYPE_BOOL;
    temp->typeEntry = TLookup("bool");
    temp->left = operand;
    if (operand == NULL || (operand->type != TYPE_BOOL && operand->type != TYPE_INT)) {
        LOG_ERROR("Error: NOT requires a boolean or integer operand\n");
        exit(1);
    }
    return temp;
}

tnode *makeReturnNode(tnode *expr) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_RETURN;
    temp->left = expr;
    return temp;
}

void freeTree(tnode *root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->middle);
    freeTree(root->right);
    free(root->varname);
    free(root);
}

tnode *makeArrayNode(tnode *idNode, tnode *indexExpr) {
    if (idNode == NULL || idNode->nodetype != NODE_TYPE_ID ||
        idNode->Gentry == NULL || idNode->Gentry->arrayLength == 0) {
        LOG_ERROR("Error: Array access requires one index on a global array\n");
        exit(1);
    }
    if (indexExpr == NULL || indexExpr->type != TYPE_INT || indexExpr->pointerLevel != 0) {
        LOG_ERROR("Error: Array index must be an integer\n");
        exit(1);
    }
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ARRAY;
    temp->type = idNode->type;
    temp->pointerLevel = idNode->type == TYPE_CUSTOM ? 0 : idNode->pointerLevel - 1;
    temp->typeEntry = idNode->typeEntry;
    temp->left = idNode;
    temp->right = indexExpr;
    return temp;
}

tnode *makeAddressNode(tnode *varNode) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ADDRESS;
    temp->left = varNode;
    temp->type = varNode->type;
    temp->typeEntry = varNode->typeEntry;
    temp->pointerLevel = varNode->pointerLevel + 1;
    return temp;
}

tnode *makeDeRefNode(tnode *varNode) {
   
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_DEREF;
    temp->left = varNode;
    temp->type = varNode->type;
    temp->typeEntry = varNode->typeEntry;
    temp->pointerLevel = varNode->pointerLevel - 1;
    return temp;
}



tnode *makeConnectorNode(tnode *l, tnode *r) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_CONNECTOR;
    temp->left = l;
    temp->right = r;
    return temp;
}

tnode *makeReadNode(tnode *l) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_READ;
    temp->left = l;
    return temp;
}

tnode *makeWriteNode(tnode *l) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_WRITE;
    temp->left = l;
    return temp;
}

tnode *makeIfNode(tnode *boolExpr, tnode *thenStmt) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_IF;
    temp->left = boolExpr;
    temp->right = thenStmt;
    return temp;
}

tnode *makeIfElseNode(tnode *boolExpr, tnode *thenStmt, tnode *elseStmt) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_IF_ELSE;
    temp->left = boolExpr;
    temp->right = thenStmt;
    temp->middle = elseStmt;
    return temp;
}

tnode *makeWhileNode(tnode *boolExpr, tnode *bodyStmt) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_WHILE;
    temp->left = boolExpr;
    temp->right = bodyStmt;
    return temp;
}

tnode *makeDoWhileNode(tnode *bodyStmt, tnode *boolExpr) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_DO_WHILE;
    temp->left = bodyStmt;
    temp->right = boolExpr;
    return temp;
}

tnode *makeRepeatUntilNode(tnode *bodyStmt, tnode *boolExpr) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_REPEAT_UNTIL;
    temp->left = bodyStmt;
    temp->right = boolExpr;
    return temp;
}

tnode *makeBreakNode(void) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_BREAK;
    return temp;
}

tnode *makeContinueNode(void) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_CONTINUE;
    return temp;
}

tnode *makeBreakPointNode(void) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_BREAKPOINT;
    return temp;
}

tnode *makeArgNode(tnode *argList, tnode *arg) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ARG;
    temp->left = argList;
    temp->right = arg;
    return temp;
}

tnode *makeFuncCallNode(char *name, tnode *args) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_FUNC_CALL;
    temp->varname = strdup(name);
    temp->left = args;
    struct Gsymbol *entry = GLookup(name);
    if (entry == NULL || entry->flabel < 0) {
        LOG_ERROR("Error: Function '%s' not declared\n", name);
        exit(1);
    }
    checkCallArguments(entry, args);
    temp->Gentry = entry;
    temp->type = GTypeKind(entry);
    temp->typeEntry = entry->type;
    temp->pointerLevel = entry->pointerLevel;
    return temp;
}

tnode *makeUserTypeNode(tnode* base, char* fieldName) {
    if (base->type != TYPE_CUSTOM || base->typeEntry == NULL || base->pointerLevel != 0) {
        LOG_ERROR("Field access requires a user-defined type\n"); exit(1);
    }
    FieldList* field = FLookup(base->typeEntry, fieldName);
    if (field == NULL) { LOG_ERROR("Unknown field '%s' in type '%s'\n", fieldName, base->typeEntry->name); exit(1); }
    tnode* temp = tnodeInit();
    temp->nodetype = NODE_TYPE_USER_DEREF;
    temp->left = base;
    temp->varname = strdup(fieldName);
    temp->typeEntry = field->type;
    temp->type = field->type == TLookup("int") ? TYPE_INT :
                 field->type == TLookup("str") ? TYPE_STRING :
                 field->type == TLookup("bool") ? TYPE_BOOL : TYPE_CUSTOM;
    temp->val = field->fieldIndex;
    return temp;
}

tnode *makeNullNode(void) {
    tnode *node = tnodeInit();
    node->nodetype = NODE_TYPE_NULL;
    return node;
}

tnode *makeAllocNode(void) {
    tnode *node = tnodeInit();
    node->nodetype = NODE_TYPE_ALLOC;
    return node;
}

tnode *makeFreeNode(tnode *value) {
    if (value->type != TYPE_CUSTOM || value->pointerLevel != 0) {
        LOG_ERROR("free requires a user-defined type value\n"); exit(1);
    }
    tnode *node = tnodeInit();
    node->nodetype = NODE_TYPE_FREE;
    node->left = value;
    node->type = TYPE_INT;
    node->typeEntry = TLookup("int");
    return node;
}

tnode *makeInitializeNode(void) {
    tnode *node = tnodeInit();
    node->nodetype = NODE_TYPE_INITIALIZE;
    node->type = TYPE_INT;
    node->typeEntry = TLookup("int");
    return node;
}

/* This function is strictly for class field access */
tnode *makeClassFieldNode(tnode* base, char* fieldName) {
    if (base == NULL || base->type != TYPE_CLASS || base->cPtr == NULL || base->pointerLevel != 0) {
        LOG_ERROR("Invalid AST for class field access.\n"); exit(1);
    }
    FieldList* field = CFieldLookup(base->cPtr, fieldName); // looking the field in the class
    if (field == NULL) { LOG_ERROR("Unknown field '%s' in class '%s'\n", fieldName, base->cPtr->name); exit(1); }
    tnode* temp = tnodeInit();
    temp->nodetype = NODE_TYPE_CLASS_FIELD_ACCESS;
    temp->left = base;
    temp->varname = strdup(fieldName);
    temp->cPtr = field->ctype; // if a normal field, no ctype
    temp->typeEntry = field->type; // rather than cptr, then typeEntry
    if (field->ctype != NULL) {
        temp->type = TYPE_CLASS;
    } else if (field->type == TLookup("int")) {
        temp->type = TYPE_INT;
    } else if (field->type == TLookup("str")) {
        temp->type = TYPE_STRING;
    } else if (field->type == TLookup("bool")) {
        temp->type = TYPE_BOOL;
    } else if (field->type != NULL) {
        temp->type = TYPE_CUSTOM;
    } else {
        LOG_ERROR("Field '%s' has no type\n", fieldName);
        exit(1);
    }
    temp->val = field->fieldIndex;
    return temp;
}

/**
tnode *makeFuncCallNode(char *name, tnode *args) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_FUNC_CALL;
    temp->varname = strdup(name);
    temp->left = args;
    struct Gsymbol *entry = GLookup(name);
    if (entry == NULL) {
        LOG_ERROR("Error: Function '%s' not declared\n", name);
        exit(1);
    }
    checkCallArguments(entry, args);
    temp->Gentry = entry;
    temp->type = GTypeKind(entry);
    temp->typeEntry = entry->type;
    temp->pointerLevel = entry->pointerLevel;
    return temp;
}
 */

tnode* makeClassMethodNode(tnode* base, char* funcName, tnode* args) {
    if (base == NULL || base->type != TYPE_CLASS || base->cPtr == NULL ||
        funcName == NULL || funcName[0] == '\0') {
        LOG_ERROR("The base node and function name cannot be NULL or empty.\n");
        exit(1);
    }
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_CLASS_METHOD_ACCESS;
    temp->varname = strdup(funcName);
    temp->left = base;
    temp->right = args;
    struct MemberFuncList* funcPtr = CMemLookup(base->cPtr, funcName);
    if (funcPtr == NULL) {
        LOG_ERROR("%s is not a method is class %s.\n", funcName, base->cPtr->name);
        exit(1);
    }
    struct ParamList *funcParams = funcPtr->paramList;
    struct Gsymbol expectedFormat = (struct Gsymbol) {
        .paramList = funcParams,
        .name = funcPtr->name
    };
    checkCallArguments(&expectedFormat, args);
    if (funcPtr->type == TLookup("int")) temp->type = TYPE_INT;
    else if (funcPtr->type == TLookup("str")) temp->type = TYPE_STRING;
    else if (funcPtr->type == TLookup("bool")) temp->type = TYPE_BOOL;
    else if (funcPtr->type != NULL) temp->type = TYPE_CUSTOM;
    else {
        LOG_ERROR("Method '%s' has no type\n", funcName);
        exit(1);
    }
    
    temp->typeEntry = TLookup(funcPtr->type->name);
    temp->pointerLevel = 0; // no pointers for my sake
    return temp;
}

tnode *makeSelfNode(struct ClassTable* currentClass) {
    if (currentClass == NULL) {
        LOG_ERROR(" 'self' is only valid inside a method.\n");
        exit(1);
    }

    tnode* node = tnodeInit();
    node->cPtr = currentClass;
    node->typeEntry = NULL;
    node->type = TYPE_CLASS;
    node->nodetype = NODE_TYPE_SELF;
    node->pointerLevel = 0; // no pointers allowed as of now

    return node;
}

tnode* makeNewClassNode(char* className) {
    struct ClassTable *cptr = CLookup(className);
    if (cptr == NULL) {
        LOG_ERROR(" 'self' is only valid inside a method.\n");
        exit(1);
    }

    tnode* node = tnodeInit();
    node->cPtr = cptr;
    node->typeEntry = NULL;
    node->nodetype = NODE_TYPE_NEW;
    node->type = TYPE_CLASS;
    node->pointerLevel = 0;

    return node;
}

tnode* makeDeleteClassNode(char* objName) {
    struct Gsymbol *gentry = GLookup(objName);
    struct Lsymbol *lentry = LLookup(objName);
    if (gentry == NULL && lentry == NULL) {
        LOG_ERROR("%s is not declared\n", objName);
        exit(1);
    }

    if ((lentry != NULL ? lentry->cPtr : gentry->cPtr) == NULL) {
        LOG_ERROR("%s is not a class\n", objName);
        exit(1);
    }

    tnode* node = tnodeInit();
    if (lentry != NULL) {
        node->Lentry = lentry;
        node->Gentry = NULL;
    } else {
        node->Gentry = gentry;
        node->Lentry = NULL;
    }
    node->typeEntry = NULL;
    node->nodetype = NODE_TYPE_DELETE;
    node->type = TYPE_CLASS;
    node->pointerLevel = 0;
    node->cPtr = lentry != NULL ? lentry->cPtr : gentry->cPtr;

    return node;
}

static bool isDescendentClass(struct ClassTable *ancestor, struct ClassTable* child) {
    if (ancestor == NULL || child == NULL) {
        LOG_ERROR("The ancestor / child should not be NULL\n");
        exit(1);
    }
    while(child != ancestor) {
        if (child->parentPtr == NULL) return false;
        child = child->parentPtr;
    }
    return true;
}

int typesCompatible(tnode *expected, tnode *actual) {
    if (actual->nodetype == NODE_TYPE_NULL)
        return (expected->type == TYPE_CUSTOM || expected->type == TYPE_CLASS) &&
               expected->pointerLevel == 0;
    if (actual->nodetype == NODE_TYPE_ALLOC)
        return expected->type == TYPE_CUSTOM && expected->pointerLevel == 0;
    if (expected->type == TYPE_CLASS || actual->type == TYPE_CLASS)
        // should work for A x; x = new(A)
        // the left class should be the parent while right should be child or same class
        return expected->type == TYPE_CLASS && actual->type == TYPE_CLASS &&
               isDescendentClass(expected->cPtr, actual->cPtr);
    return expected->typeEntry == actual->typeEntry &&
           expected->pointerLevel == actual->pointerLevel;
}
static int countCallArgs(tnode *argsNode) {
    if (argsNode == NULL) return 0;
    if (argsNode->nodetype != NODE_TYPE_ARG) return 1;
    return countCallArgs(argsNode->left) + 1;
}

static void flattenCallArgs(tnode *argsNode, tnode **argArray, int *index) {
    if (argsNode == NULL) return;
    if (argsNode->nodetype != NODE_TYPE_ARG) {
        argArray[(*index)++] = argsNode;
        return;
    }
    flattenCallArgs(argsNode->left, argArray, index);
    argArray[(*index)++] = argsNode->right;
}

static void checkCallArguments(struct Gsymbol *entry, tnode *args) {
    int argc = countCallArgs(args);
    int paramCount = 0;
    for (struct ParamList *p = entry->paramList; p != NULL; p = p->next) {
        paramCount++;
    }
    if (argc != paramCount) {
        LOG_ERROR("Error: Function '%s' expects %d argument(s), got %d\n",
                entry->name, paramCount, argc);
        exit(1);
    }
    if (argc == 0) return;
    tnode **argArray = (tnode **)malloc(argc * sizeof(tnode *));
    if (argArray == NULL) {
        LOG_ERROR("Error: Memory allocation failed for argument array\n");
        exit(1);
    }
    int index = 0;
    flattenCallArgs(args, argArray, &index);
    struct ParamList *param = entry->paramList;
    for (int i = 0; i < argc; i++, param = param->next) {
        
        if (((argArray[i]->typeEntry != param->type || argArray[i]->cPtr != param->cPtr) &&
             !(argArray[i]->nodetype == NODE_TYPE_NULL && param->type != TLookup("int") &&
               param->type != TLookup("bool") && param->type != TLookup("str") &&
               param->type != TLookup("void"))) ||
            argArray[i]->pointerLevel != param->pointerLevel) {
            LOG_ERROR("Error: Argument %d of function '%s' has an incompatible type\n",
                    i + 1, entry->name);
            exit(1);
        }
    }
    free(argArray);
}
