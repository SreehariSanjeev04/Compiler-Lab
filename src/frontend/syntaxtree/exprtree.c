#include "exprtree.h"
#include "constants.h"
#include "symboltable.h"

/*
typedef struct tnode {
    int val;
    int type;
    char* varname;
    int nodetype; 
    struct tnode* left;
    struct tnode* middle;
    struct tnode* right;
}tnode;
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
    struct Gsymbol* entry = Lookup(c);
    if(!entry) {
        fprintf(stderr, "Error: Variable %s not declared\n", c);
        exit(1);
    }
    int type = entry->type;
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = type;
    temp->varname = (char*)malloc(strlen(c) + 1);
    strcpy(temp->varname, c);
    temp->nodetype = NODE_TYPE_ID;
    temp->left = NULL;
    temp->right = NULL;
    temp->middle = NULL;
    temp->Gentry = NULL;
    return temp;
}

tnode* makeLeafNodeString(char* str) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_STRING; 
    temp->varname = (char*)malloc(strlen(str) + 1);
    strcpy(temp->varname, str);
    temp->nodetype = NODE_TYPE_STRING; 
    temp->left = NULL;
    temp->right = NULL;
    temp->middle = NULL;
    temp->Gentry = NULL;
    return temp;
}
tnode* makeOperatorNode(char *op, tnode *l, tnode *r) {
    if(!l || !r) {
        fprintf(stderr, "Error: Invalid node passed to makeOperatorNode\n");
        exit(1);
    }
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->varname = NULL;
    temp->left = l;
    temp->right = r;
    temp->middle = NULL;
    if(strcmp(op, "+") == 0) {
        temp->nodetype = NODE_TYPE_PLUS;
        temp->type = TYPE_INT;
    } else if(strcmp(op, "-") == 0) {
        temp->nodetype = NODE_TYPE_MINUS;
        temp->type = TYPE_INT;
    } else if(strcmp(op, "*") == 0) {
        temp->nodetype = NODE_TYPE_MUL;
        temp->type = TYPE_INT;
    } else if(strcmp(op, "/") == 0) {
        temp->nodetype = NODE_TYPE_DIV;
        temp->type = TYPE_INT;
    } else if(strcmp(op, "=") == 0) {
        temp->nodetype = NODE_TYPE_ASSIGN;
        temp->type = TYPE_VOID; 
    } else if(strcmp(op, "<") == 0) {
        temp->nodetype = NODE_TYPE_LT;
        temp->type = TYPE_BOOL;
    } else if(strcmp(op, ">") == 0) {
        temp->nodetype = NODE_TYPE_GT;
        temp->type = TYPE_BOOL;
    } else if(strcmp(op, "<=") == 0) {
        temp->nodetype = NODE_TYPE_LE;
        temp->type = TYPE_BOOL;
    } else if(strcmp(op, ">=") == 0) {
        temp->nodetype = NODE_TYPE_GE;
        temp->type = TYPE_BOOL;
    } else if(strcmp(op, "==") == 0) {
        temp->nodetype = NODE_TYPE_EQ;
        temp->type = TYPE_BOOL;
    } else if(strcmp(op, "!=") == 0) {
        temp->nodetype = NODE_TYPE_NE;
        temp->type = TYPE_BOOL;
    } else {
        fprintf(stderr, "Error: Invalid operator %s\n", op);
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

tnode* makeBreakPointNode() {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_BREAKPOINT;
    temp->left = NULL;
    temp->right = NULL;
    temp->middle = NULL;
    return temp;
}

tnode* makeBreakNode() {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_BREAK;
    temp->left = NULL;
    temp->right = NULL;
    temp->middle = NULL;
    return temp;
}

tnode* makeContinueNode() {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_CONTINUE;
    temp->left = NULL;
    temp->right = NULL;
    temp->middle = NULL;
    return temp;
}

tnode* makeRepeatUntilNode(tnode* bodyStmt, tnode* boolExpr) {

    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_REPEAT_UNTIL;
    temp->left = bodyStmt;
    temp->right = boolExpr;
    temp->middle = NULL;
    return temp;
}

tnode* makeDoWhileNode(tnode* bodyStmt, tnode* boolExpr) {

    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->val = 0;
    temp->type = TYPE_VOID;
    temp->varname = NULL;
    temp->nodetype = NODE_TYPE_DO_WHILE;
    temp->left = bodyStmt;
    temp->right = boolExpr;
    temp->middle = NULL;
    return temp;
}

tnode* makeArrayNode(tnode* idNode, tnode* indexExpr) {
    tnode* temp = (tnode*)malloc(sizeof(tnode));
    temp->nodetype = NODE_TYPE_ARRAY;
    temp->type = TYPE_INT; // This has to change
    temp->varname = NULL;
    temp->val = 0;
    temp->left = idNode;
    temp->right = indexExpr;
    temp->middle = NULL;
    return temp;
}