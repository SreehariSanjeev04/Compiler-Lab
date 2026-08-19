#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exprtree.h"
#include "constants.h"
#include "gsymboltable.h"

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
    struct Gsymbol *entry = GLookup(c);
    if (entry == NULL) {
        fprintf(stderr, "Error: Variable '%s' not declared\n", c);
        exit(1);
    }
    temp->nodetype = NODE_TYPE_ID;
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
        // pointer + integer (or integer + pointer) keeps the pointer level and base type
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
    else if (strcmp(op, "=") == 0)  { temp->nodetype = NODE_TYPE_ASSIGN; temp->type = TYPE_VOID; }
    else if (strcmp(op, "<") == 0)  { temp->nodetype = NODE_TYPE_LT; temp->type = TYPE_BOOL; }
    else if (strcmp(op, ">") == 0)  { temp->nodetype = NODE_TYPE_GT; temp->type = TYPE_BOOL; }
    else if (strcmp(op, "<=") == 0) { temp->nodetype = NODE_TYPE_LE; temp->type = TYPE_BOOL; }
    else if (strcmp(op, ">=") == 0) { temp->nodetype = NODE_TYPE_GE; temp->type = TYPE_BOOL; }
    else if (strcmp(op, "==") == 0) { temp->nodetype = NODE_TYPE_EQ; temp->type = TYPE_BOOL; }
    else if (strcmp(op, "!=") == 0) { temp->nodetype = NODE_TYPE_NE; temp->type = TYPE_BOOL; }
    else {
        fprintf(stderr, "Error: Unknown operator %s\n", op);
        exit(1);
    }

    return temp;
}

tnode *makeArrayNode(tnode *idNode, tnode *indexExpr) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ARRAY;
    temp->type = idNode->type; // inherit the base type of the variable
    // Each subscript consumes one dimension, so the element is one pointer
    // level shallower than the subscripted expression. With the array name at
    // level = number of dimensions, `a` (2D) is level 2, `a[i]` is level 1
    // (the row, which decays to an int*), and `a[i][j]` is level 0 (an int).
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

tnode *makeDeRefNode(tnode *varNode) {
    // Note: pointer validity (level >= 1) is NOT checked here; it is checked
    // during code generation (codeGen NODE_TYPE_DEREF case).
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