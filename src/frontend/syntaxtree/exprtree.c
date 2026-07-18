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
    temp->middle = NULL;
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
    temp->middle = NULL;
    return temp;
}
tnode* makeOperatorNode(char op, tnode *l, tnode *r) {
    if(!l || !r) {
        fprintf(stderr, "Error: Invalid node passed to makeOperatorNode\n");
        exit(1);
    }
    // as of now the operation takes between two number type nodes
    if(!(l->type == TYPE_INT) || !(r->type == TYPE_INT)) {
        fprintf(stderr, "Error: Type Mistmatch");
        exit(1);
    }
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->varname = NULL;
    temp->left = l;
    temp->right = r;
    temp->middle = NULL;
    switch (op)
    {
    case '+':
        temp->nodetype = NODE_TYPE_PLUS;
        temp->type = TYPE_INT;
        break;
    case '-':
        temp->nodetype = NODE_TYPE_MINUS;
        temp->type = TYPE_INT;
        break;
    case '*':
        temp->nodetype = NODE_TYPE_MUL;
        temp->type = TYPE_INT;
        break;
    case '/':
        temp->nodetype = NODE_TYPE_DIV;
        temp->type = TYPE_INT;
        break;
    case '=':
        temp->nodetype = NODE_TYPE_ASSIGN;
        temp->type = TYPE_INT;
        break;      
    case '<':
        temp->nodetype = NODE_TYPE_LT;
        temp->type = TYPE_BOOL;
        break;
    case '>':
        temp->nodetype = NODE_TYPE_GT;
        temp->type = TYPE_BOOL;
        break;
    case '<=':
        temp->nodetype = NODE_TYPE_LE;
        temp->type = TYPE_BOOL;
        break;
    case '>=':
        temp->nodetype = NODE_TYPE_GE;
        temp->type = TYPE_BOOL;
        break;
    case '==':
        temp->nodetype = NODE_TYPE_EQ;
        temp->type = TYPE_BOOL;
        break;
    case '!=':
        temp->nodetype = NODE_TYPE_NE;
        temp->type = TYPE_BOOL;
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
    temp->middle = NULL;
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
    temp->middle = NULL;
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
    temp->middle = NULL;
    return temp;
}

tnode* makeIfElseNode(tnode* boolExpr, tnode* thenStmt, tnode* elseStmt) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_IF_ELSE;
    temp->left = boolExpr;
    temp->right = thenStmt;
    temp->middle = elseStmt;
    return temp;
}

tnode* makeIfNode(tnode* boolExpr, tnode* thenStmt) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_IF;
    temp->left = boolExpr;
    temp->right = thenStmt;
    temp->middle = NULL;
    return temp;
}

tnode* makeWhileNode(tnode* boolExpr, tnode* bodyStmt) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_WHILE;
    temp->left = boolExpr;
    temp->right = bodyStmt;
    temp->middle = NULL;
    return temp;
}