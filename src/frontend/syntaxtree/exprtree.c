#include "exprtree.h"
#include "constants.h"

/*
    int val;
    int type;
    char* varname;
    int nodetype; 
    struct tnode* left;
    struct tnode* right;
*/

tnode* makeLeafNodeNum(int n) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = n;
    temp->type = TYPE_INT;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_NUM;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}
tnode* makeLeafNodeId(char* c) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_INT; // as of now the variables are of type int, but this can be changed later
    temp->varname = (char*)malloc(strlen(c) + 1);
    strcpy(temp->varname, c);
    temp->nodetype = NODE_TYPE_ID;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}
tnode* makeOperatorNode(char op, tnode *l, tnode *r) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->left = l;
    temp->right = r;

    switch (op)
    {
    case '+':
        temp->nodetype = NODE_TYPE_PLUS;
        break;
    case '-':
        temp->nodetype = NODE_TYPE_MINUS;
        break;
    case '*':
        temp->nodetype = NODE_TYPE_MUL;
        break;
    case '/':
        temp->nodetype = NODE_TYPE_DIV;
        break;
    case '=':
        temp->nodetype = NODE_TYPE_ASSIGN;
        break;      
    default:
        printf("Error: Invalid operator %c\n", op);
        exit(1);
    }
    return temp;
}

tnode* makeConnectorNode(tnode* l, tnode *r) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_CONNECTOR;
    temp->left = l;
    temp->right = r;
    return temp;
}

tnode* makeReadNode(tnode* l) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_READ;
    temp->left = l;
    temp->right = NULL;
    return temp;
}

tnode* makeWriteNode(tnode* l) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_WRITE;
    temp->left = l;
    temp->right = NULL;
    return temp;
}
