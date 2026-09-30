#include "logger.h"
#include <codegen_utils.h>
#include <exprtree.h>
#include <gsymboltable.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static struct TupleTable *tupleTypeOf(tnode *node)
{
    if (node == NULL) return NULL;
    if (node->nodetype == NODE_TYPE_ID || node->nodetype == NODE_TYPE_FUNC_CALL)
        return node->Gentry ? node->Gentry->tupleEntry :
               node->Lentry ? node->Lentry->tupleEntry : NULL;
    if (node->nodetype == NODE_TYPE_DEREF || node->nodetype == NODE_TYPE_ADDRESS)
        return tupleTypeOf(node->left);
    return NULL;
}

int effectivePointerLevel(tnode *node)
{
    if (node == NULL)
    {
        LOG_ERROR("NULL node passed to effectivePointerLevel\n");
        exit(1);
    }
    return node->pointerLevel;
}

bool isAssignmentCompatible(tnode *left, tnode *right)
{
    if (left == NULL || right == NULL)
    {
        LOG_ERROR("NULL node passed to isAssignmentCompatible\n");
        exit(1);
    }

    // LHS should be a valid lvalue: ID, Array, or Pointer dereference
    if (left->nodetype != NODE_TYPE_ID &&
        left->nodetype != NODE_TYPE_ARRAY &&
        left->nodetype != NODE_TYPE_DEREF &&
        left->nodetype != NODE_TYPE_TUPLE)
    {
        return false;
    }

    int lType = left->type;
    int rType = right->type;

    if (lType != rType)
    {
        return false;
    }

    // Tuples: both operands must resolve to the same tuple type.
    if (lType == TYPE_TUPLE)
    {
        struct TupleTable *lTuple = tupleTypeOf(left);
        struct TupleTable *rTuple = tupleTypeOf(right);

        if (lTuple == NULL || rTuple == NULL || lTuple != rTuple)
        {
            return false;
        }
    }

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
