#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

typedef struct tnode {
    int val;
    char *op;
    struct tnode* left;
    struct tnode* right;
}tnode;

tnode* makeLeafNode(int n);
tnode* makeOperatorNode(char op, tnode *l, tnode *r);

#endif // SYNTAXTREE_H