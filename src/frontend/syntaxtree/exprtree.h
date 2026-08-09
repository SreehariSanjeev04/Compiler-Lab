#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "symboltable.h"

typedef struct tnode {
    int val;
    int type;
    char* varname;
    int nodetype; 
    struct tnode* left;
    struct tnode* middle;
    struct tnode* right;
    struct Gsymbol* Gentry;
}tnode;

tnode* makeLeafNodeNum(int n);
tnode* makeLeafNodeId(char* c);
tnode* makeLeafNodeString(char* str);
tnode* makeOperatorNode(char *op, tnode *l, tnode *r);
tnode* makeConnectorNode(tnode* l, tnode *r);
tnode* makeReadNode(tnode* l);
tnode* makeWriteNode(tnode* l);
tnode* makeIfElseNode(tnode* l, tnode* m, tnode* r);
tnode* makeIfNode(tnode* l, tnode* r);
tnode* makeWhileNode(tnode* l, tnode* r);
tnode* makeBreakPointNode();
tnode* makeBreakNode();
tnode* makeContinueNode();
tnode* makeRepeatUntilNode(tnode* bodyStmt, tnode* boolExpr);
tnode* makeDoWhileNode(tnode* bodyStmt, tnode* boolExpr);
tnode* makeArrayNode(tnode* idNode, tnode* indexExpr);
tnode* makeAddressNode(tnode* varNode);

#endif // SYNTAXTREE_H