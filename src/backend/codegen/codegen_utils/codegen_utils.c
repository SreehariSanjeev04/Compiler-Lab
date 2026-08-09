#include <codegen_utils.h>
#include <exprtree.h>
#include <symboltable.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool isAssignmentCompatible(tnode* left, tnode* right) {
    if (left == NULL || right == NULL) {
        fprintf(stderr, "Error: NULL node passed to isAssignmentCompatible\n");
        exit(1);
    }

    // LHS should be a valid variable (ID, Array, or Pointer dereference)
    if (left->nodetype != NODE_TYPE_ID && 
        left->nodetype != NODE_TYPE_ARRAY && 
        left->nodetype != NODE_TYPE_DEREF) {
        return false;
    }

    // Base types should match
    if (left->type != right->type) {
        return false;
    }

    // 4. Effective Pointer Level Check (including array decay)
   if(left->pointerLevel != right->pointerLevel) {
        return false;
    }

    return true;
}