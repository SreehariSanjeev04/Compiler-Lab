#include "logger.h"
#include <codegen_utils.h>
#include <exprtree.h>
#include <gsymboltable.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int effectivePointerLevel(tnode *node)
{
    if (node == NULL)
    {
        LOG_ERROR("NULL node passed to effectivePointerLevel\n");
        exit(1);
    }

    return node->pointerLevel;
}

bool isUserType(tnode* root) {
    return root->nodetype == NODE_TYPE_USER_DEREF;
}

bool isAssignmentCompatible(tnode *left, tnode *right)
{
    if (left == NULL || right == NULL)
    {
        LOG_ERROR("NULL node passed to isAssignmentCompatible\n");
        exit(1);
    }

    if (left->nodetype != NODE_TYPE_ID &&
        left->nodetype != NODE_TYPE_ARRAY &&
        left->nodetype != NODE_TYPE_DEREF &&
        left->nodetype != NODE_TYPE_USER_DEREF)
    {
        return false;
    }

    if (left->nodetype == NODE_TYPE_ID && left->Gentry != NULL &&
        left->Gentry->arrayLength > 0) return false;

    if (!typesCompatible(left, right)) return false;

    // pointers for user defined type not supported as of now
    if (effectivePointerLevel(left) != effectivePointerLevel(right))
    {
        return false;
    }

    return true;
}

bool isArithmeticCompatible(tnode *left, tnode *right, int op)
{
    if (left == NULL || right == NULL)
    {
        LOG_ERROR("NULL node passed to isArithmeticCompatible\n");
        exit(1);
    }
    int lLevel = effectivePointerLevel(left);
    int rLevel = effectivePointerLevel(right);

    int lType = left->type;
    int rType = right->type;

    if ((op == NODE_TYPE_EQ || op == NODE_TYPE_NE) &&
        lLevel == 0 && rLevel == 0 &&
        ((left->typeEntry == right->typeEntry && lType == TYPE_CUSTOM) ||
         (lType == TYPE_CUSTOM && right->nodetype == NODE_TYPE_NULL) ||
         (rType == TYPE_CUSTOM && left->nodetype == NODE_TYPE_NULL))) return true;

    if (op == NODE_TYPE_AND || op == NODE_TYPE_OR)
    {
        bool leftOk = (lType == TYPE_BOOL || lType == TYPE_INT) && lLevel == 0;
        bool rightOk = (rType == TYPE_BOOL || rType == TYPE_INT) && rLevel == 0;
        return leftOk && rightOk;
    }
    bool stringArithmetic = (lType == TYPE_STRING && rType == TYPE_STRING) || (lType == TYPE_INT && rType == TYPE_STRING) || (lType == TYPE_STRING && rType == TYPE_INT);
    bool intArithmetic = (lType == TYPE_INT && rType == TYPE_INT);
    if (!intArithmetic && !stringArithmetic)
    {
        return false;
    }
    if (lLevel == 0 && rLevel == 0)
    {
        if (stringArithmetic)
            return false;

        return true;
    }
    switch (op)
    {
    case NODE_TYPE_PLUS:
        // pointer + integer (or integer + pointer) only; the non-pointer side must be an int
        if ((lLevel > 0) == (rLevel > 0))
            return false;
        return (lLevel > 0) ? (rType == TYPE_INT) : (lType == TYPE_INT);
    case NODE_TYPE_MINUS:
        // pointer - integer, or pointer - pointer of the same level and base type
        if (lLevel == 0)
            return false;
        if (rLevel == 0)
            return rType == TYPE_INT;
        return rLevel == lLevel && lType == rType;
    default:
        // MUL/DIV/relational operations are not defined for pointers
        return false;
    }
}
