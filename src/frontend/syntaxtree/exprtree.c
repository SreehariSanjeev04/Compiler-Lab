#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exprtree.h"
#include "constants.h"
#include "symboltable.h"

tnode *tnodeInit() {
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
    temp->nodetype = NODE_TYPE_NUM;
    return temp;
}

tnode *makeLeafNodeId(char *c) {
    tnode *temp = tnodeInit();
    temp->varname = strdup(c);
    if (temp->varname == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for variable name\n");
        exit(1);
    }
    temp->nodetype = NODE_TYPE_ID;
    temp->Gentry = Lookup(c); 
    return temp;
}

tnode *makeLeafNodeString(char *str) {
    tnode *temp = tnodeInit();
    temp->varname = strdup(str);
    if (temp->varname == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for string value\n");
        exit(1);
    }
    temp->nodetype = NODE_TYPE_STRING;
    return temp;
}



tnode *makeOperatorNode(char *op, tnode *l, tnode *r) {
    tnode *temp = tnodeInit();
    temp->left = l;
    temp->right = r;

    if (strcmp(op, "+") == 0)       temp->nodetype = NODE_TYPE_PLUS;
    else if (strcmp(op, "-") == 0)  temp->nodetype = NODE_TYPE_MINUS;
    else if (strcmp(op, "*") == 0)  temp->nodetype = NODE_TYPE_MUL;
    else if (strcmp(op, "/") == 0)  temp->nodetype = NODE_TYPE_DIV;
    else if (strcmp(op, "=") == 0)  temp->nodetype = NODE_TYPE_ASSIGN;
    else if (strcmp(op, "<") == 0)  temp->nodetype = NODE_TYPE_LT;
    else if (strcmp(op, ">") == 0)  temp->nodetype = NODE_TYPE_GT;
    else if (strcmp(op, "<=") == 0) temp->nodetype = NODE_TYPE_LE;
    else if (strcmp(op, ">=") == 0) temp->nodetype = NODE_TYPE_GE;
    else if (strcmp(op, "==") == 0) temp->nodetype = NODE_TYPE_EQ;
    else if (strcmp(op, "!=") == 0) temp->nodetype = NODE_TYPE_NE;
    else {
        fprintf(stderr, "Error: Unknown operator %s\n", op);
        exit(1);
    }

    return temp;
}

tnode *makeArrayNode(tnode *idNode, tnode *indexExpr) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ARRAY;
    temp->left = idNode;
    temp->right = indexExpr;
    return temp;
}

tnode *makeAddressNode(tnode *varNode) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_ADDRESS;
    temp->left = varNode;
    return temp;
}

tnode *makeDeRefNode(tnode *varNode) {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_DEREF;
    temp->left = varNode;
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

tnode *makeBreakNode() {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_BREAK;
    return temp;
}

tnode *makeContinueNode() {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_CONTINUE;
    return temp;
}

tnode *makeBreakPointNode() {
    tnode *temp = tnodeInit();
    temp->nodetype = NODE_TYPE_BREAKPOINT;
    return temp;
}