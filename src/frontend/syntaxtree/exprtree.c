#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exprtree.h"
#include "constants.h"
#include "gsymboltable.h"
#include <lsymboltable.h>
#include <tupletable.h>
#include <tuplefieldlist.h>

static tnode *tnodeInit(void);
static int countCallArgs(tnode *argsNode);
static void flattenCallArgs(tnode *argsNode, tnode **argArray, int *index);
static void checkCallArguments(struct Gsymbol *entry, tnode *args);

static tnode *tnodeInit(void) {
    tnode *temp = (tnode *)malloc(sizeof(tnode));
    if (temp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for AST node\n");
        exit(1);
    }
    temp->val = 0;
    temp->type = TYPE_VOID;
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
    temp->nodetype = NODE_TYPE_NUM;
    return temp;
}

tnode *makeLeafNodeId(const char *c) {
    tnode *temp = tnodeInit();
    temp->varname = strdup(c);
    if (temp->varname == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for variable name\n");
        exit(1);
    }
    temp->nodetype = NODE_TYPE_ID;

    struct Lsymbol *lentry = LLookup(temp->varname);
    if (lentry != NULL) {
        temp->type = lentry->type;
        temp->pointerLevel = lentry->pointerLevel;
        return temp;
    }
    struct Gsymbol *entry = GLookup(c);
    if (entry == NULL) {
        fprintf(stderr, "Error: Variable '%s' not declared\n", c);
        exit(1);
    }
    temp->Gentry = entry;
    temp->type = entry->type;
    temp->pointerLevel = entry->pointerLevel;
    return temp;
}

tnode *makeLeafNodeString(const char *str) {
    tnode *temp = tnodeInit();
    temp->varname = strdup(str);
    if (temp->varname == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for string value\n");
        exit(1);
    }
    temp->type = TYPE_STRING;
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
            fprintf(stderr, "Error: AND requires boolean or integer operands\n");
            exit(1);
        }
    }
    else if (strcmp(op, "OR") == 0) {
        temp->nodetype = NODE_TYPE_OR;
        temp->type = TYPE_BOOL;
        if (!((l->type == TYPE_BOOL || l->type == TYPE_INT) && (r->type == TYPE_BOOL || r->type == TYPE_INT))) {
            fprintf(stderr, "Error: OR requires boolean or integer operands\n");
            exit(1);
        }
    }
    else {
        fprintf(stderr, "Error: Unknown operator %s\n", op);
        exit(1);
    }

    return temp;
}

tnode *makeNotNode(tnode *operand) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_NOT;
    temp->type = TYPE_BOOL;
    temp->left = operand;
    if (operand == NULL || (operand->type != TYPE_BOOL && operand->type != TYPE_INT)) {
        fprintf(stderr, "Error: NOT requires a boolean or integer operand\n");
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
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ARRAY;
    temp->type = idNode->type;
    temp->pointerLevel = idNode->pointerLevel - 1;
    temp->left = idNode;
    temp->right = indexExpr;
    return temp;
}

tnode *makeAddressNode(tnode *varNode) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ADDRESS;
    temp->left = varNode;
    temp->type = varNode->type;
    temp->pointerLevel = varNode->pointerLevel + 1;
    return temp;
}

tnode* makeTupleNode(tnode* tupleBase, char* fieldName) {
    if (tupleBase == NULL || fieldName == NULL) {
        fprintf(stderr, "Error: Invalid tuple field access\n");
        exit(1);
    }

    struct TupleTable* entry = NULL;
    if (tupleBase->nodetype == NODE_TYPE_ID) {
        if (tupleBase->Gentry == NULL || tupleBase->Gentry->tupleEntry == NULL) {
            fprintf(stderr, "Error: '%s' is not a tuple variable\n", tupleBase->varname);
            exit(1);
        }
        entry = tupleBase->Gentry->tupleEntry;
    } else if (tupleBase->nodetype == NODE_TYPE_DEREF) {
        
        // Assumes only one dereferencing
        tnode* base = tupleBase->left;
        if (base == NULL || base->nodetype != NODE_TYPE_ID ||
            base->Gentry == NULL || base->Gentry->tupleEntry == NULL) {
            fprintf(stderr, "Error: Cannot access a field of a non-tuple expression\n");
            exit(1);
        }
        entry = base->Gentry->tupleEntry;
    } else {
        fprintf(stderr, "Error: Only tuple variables or dereferenced tuple pointers support field access\n");
        exit(1);
    }

    struct TupleFieldList* field = TupleFieldListLookup(entry->name, fieldName);
    if (field == NULL) {
        fprintf(stderr, "Error: Tuple type '%s' has no field named '%s'\n", entry->name, fieldName);
        exit(1);
    }

    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_TUPLE;
    temp->left = tupleBase;
    temp->varname = strdup(fieldName);
    temp->type = field->type;
    temp->pointerLevel = 0;
    return temp;
}

tnode *makeDeRefNode(tnode *varNode) {
   
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_DEREF;
    temp->left = varNode;
    temp->type = varNode->type;
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
    if (entry == NULL) {
        fprintf(stderr, "Error: Function '%s' not declared\n", name);
        exit(1);
    }
    checkCallArguments(entry, args);
    temp->Gentry = entry;
    temp->type = entry->type;
    temp->pointerLevel = entry->pointerLevel;
    return temp;
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
    // check the argument count first
    if (argc != paramCount) {
        fprintf(stderr, "Error: Function '%s' expects %d argument(s), got %d\n",
                entry->name, paramCount, argc);
        exit(1);
    }
    if (argc == 0) return;
    tnode **argArray = (tnode **)malloc(argc * sizeof(tnode *));
    if (argArray == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for argument array\n");
        exit(1);
    }
    int index = 0;
    flattenCallArgs(args, argArray, &index);
    struct ParamList *param = entry->paramList;
    for (int i = 0; i < argc; i++, param = param->next) {
        
        // check the type and the pointer level of arguments
        if (argArray[i]->type != param->type ||
            argArray[i]->pointerLevel != param->pointerLevel) {
            fprintf(stderr, "Error: Argument %d of function '%s' has an incompatible type\n",
                    i + 1, entry->name);
            exit(1);
        }
    }
    free(argArray);
}