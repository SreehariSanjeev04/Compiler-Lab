#ifndef EXPR_CODEGEN_H
#define EXPR_CODEGEN_H

#include <stdio.h>
#include "exprtree.h"

int codeGenArrayAddress(tnode* root, FILE* targetFile);
int codeGenAddressOperand(tnode* root, FILE* targetFile);
int codeGenLeafValue(tnode* root, FILE* targetFile);
int codeGenAddressExpr(tnode* root, FILE* targetFile);
int codeGenBinaryOp(tnode* root, FILE* targetFile);

#endif // EXPR_CODEGEN_H