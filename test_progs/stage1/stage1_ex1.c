#include "stage1_ex1.h"

void prefixForm(tnode* root) {
    if (root == NULL) {
        return;
    }
    if (root->op != NULL) {
        printf("%c ", *(root->op));
    } else {
        printf("%d ", root->val);
    }
    prefixForm(root->left);
    prefixForm(root->right);
}

void postfixForm(tnode* root) {
    if (root == NULL) {
        return;
    }
    postfixForm(root->left);
    postfixForm(root->right);
    if (root->op != NULL) {
        printf("%c ", *(root->op));
    } else {
        printf("%d ", root->val);
    }
}