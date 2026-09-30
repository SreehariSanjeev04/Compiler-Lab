#ifndef EXPR_CODEGEN_H
#define EXPR_CODEGEN_H

#include <stdio.h>
#include "exprtree.h"

int codeGenArrayAddress(tnode* root, FILE* targetFile);
void emitVarAddressInto(int reg, const char* varname, FILE* targetFile);
int codeGenAddressOperand(tnode* root, FILE* targetFile); /* value of an expression used as a pointer/address (array decay applied) */
int codeGenFuncCall(tnode* root, FILE* targetFile); /* caller-side sequence for a function invocation */
void codeGenTupleCallInto(tnode* call, tnode* destination, FILE* targetFile);
int codeGenNot(tnode* root, FILE* targetFile);
int codeGenLeafValue(tnode* root, FILE* targetFile);
int codeGenAddressExpr(tnode* root, FILE* targetFile);
int codeGenBinaryOp(tnode* root, FILE* targetFile);
int codeGenTupleAddress(tnode* root, FILE* targetFile); /* address of a tuple field; returns the register holding it */
int codeGenTupleValue(tnode* root, FILE* targetFile); /* loads the value of a tuple field into a register */

#endif // EXPR_CODEGEN_H
