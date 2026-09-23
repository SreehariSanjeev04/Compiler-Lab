#ifndef CODEGEN_UTILS_H
#define CODEGEN_UTILS_H

#include <stdbool.h>
#include "exprtree.h"

bool isAssignmentCompatible(tnode* left, tnode* right); /* same type, pointer level and tuple type */
bool isArithmeticCompatible(tnode* left, tnode* right, int op); /* type-checks an arithmetic/relational expression */
int effectivePointerLevel(tnode* node); /* declared level, minus fully-subscripted dimensions */

#endif