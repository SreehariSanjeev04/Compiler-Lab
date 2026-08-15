#ifndef CODEGEN_UTILS_H
#define CODEGEN_UTILS_H

#include <stdbool.h>
#include "exprtree.h"

bool isAssignmentCompatible(tnode* left, tnode* right);
bool isArithmeticCompatible(tnode* left, tnode* right, int op);
int effectivePointerLevel(tnode* node);

#endif