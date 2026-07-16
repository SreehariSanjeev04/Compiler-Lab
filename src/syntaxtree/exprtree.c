#include "exprtree.h"
#include <stdlib.h>

tnode* makeLeafNode(int n) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = n;
    temp->op = NULL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}
tnode* makeOperatorNode(char op, tnode *l, tnode *r) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->op = (char*)malloc(sizeof(char));
    *(temp->op) = op;
    temp->left = l;
    temp->right = r;
    return temp;
}