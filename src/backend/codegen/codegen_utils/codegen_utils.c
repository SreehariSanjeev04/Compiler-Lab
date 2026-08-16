#include <codegen_utils.h>
#include <exprtree.h>
#include <symboltable.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Returns the effective pointer level of an expression, applying array decay.
 * Array names carry level = number of dimensions and each subscript consumes
 * one level (see makeArrayNode), so an array's level already IS its decayed
 * pointer level and no special casing is required.
 */
int effectivePointerLevel(tnode *node)
{
    if (node == NULL)
    {
        fprintf(stderr, "ERROR: NULL node passed to effectivePointerLevel\n");
        exit(1);
    }
    return node->pointerLevel;
}

bool isAssignmentCompatible(tnode *left, tnode *right)
{
    if (left == NULL || right == NULL)
    {
        fprintf(stderr, "Error: NULL node passed to isAssignmentCompatible\n");
        exit(1);
    }

    // LHS should be a valid lvalue: ID, Array, or Pointer dereference
    if (left->nodetype != NODE_TYPE_ID &&
        left->nodetype != NODE_TYPE_ARRAY &&
        left->nodetype != NODE_TYPE_DEREF)
    {
        return false;
    }

    // Base types should match
    if (left->type != right->type)
    {
        return false;
    }

    // Effective pointer levels must match. A fully subscripted array element
    // (e.g. a[i][j]) is level 0, a row (a[i]) is level 1, and the array name
    // `a` of a 2D array is level 2 -- so `p = a` (int* = 2D array) is rejected
    // like in C, while `p = a[0]`, `p = a` (1D), `p = &a[0][0]` all work.
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
        fprintf(stderr, "Error: NULL node passed to isArithmeticCompatible\n");
        exit(1);
    }
    int lLevel = effectivePointerLevel(left);
    int rLevel = effectivePointerLevel(right);
    bool stringArithmetic = (left->type == TYPE_STRING && right->type == TYPE_STRING) || (left->type == TYPE_INT && right->type == TYPE_STRING) || (left->type == TYPE_STRING && right->type == TYPE_INT);
    bool intArithmetic = (left->type == TYPE_INT && right->type == TYPE_INT);
    if (!intArithmetic && !stringArithmetic)
    {
        return false;
    }
    if (lLevel == 0 && rLevel == 0)
    {
        if (stringArithmetic)
            return false; // string + int or int + string not allowed

        // Plain integer arithmetic or relational operation
        return true;
    }
    switch (op)
    {
    case NODE_TYPE_PLUS:
        // pointer + integer (or integer + pointer) only; the non-pointer side must be an int
        if ((lLevel > 0) == (rLevel > 0))
            return false;
        return (lLevel > 0) ? (right->type == TYPE_INT) : (left->type == TYPE_INT);
    case NODE_TYPE_MINUS:
        // pointer - integer, or pointer - pointer of the same level and base type
        if (lLevel == 0)
            return false;
        if (rLevel == 0)
            return right->type == TYPE_INT;
        return rLevel == lLevel && left->type == right->type;
    default:
        // MUL/DIV/relational operations are not defined for pointers
        return false;
    }
}