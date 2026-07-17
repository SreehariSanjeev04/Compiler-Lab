#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "constants.h"

typedef struct tnode {
    int val;
    int type;
    char* varname;
    int nodetype; 
    struct tnode* left;
    struct tnode* right;
}tnode;

tnode* makeLeafNodeNum(int n);
tnode* makeLeafNodeId(char* c);
tnode* makeOperatorNode(char op, tnode *l, tnode *r);
tnode* makeConnectorNode(tnode* l, tnode *r);
tnode* makeReadNode(tnode* l);
tnode* makeWriteNode(tnode* l);

#endif // SYNTAXTREE_H