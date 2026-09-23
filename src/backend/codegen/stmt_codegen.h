#ifndef STMT_CODEGEN_H
#define STMT_CODEGEN_H

#include <stdio.h>
#include "exprtree.h"

int codeGenControlLeaf(tnode* root, FILE* targetFile);
int codeGenFlowControl(tnode* root, FILE* targetFile);
int codeGenStatement(tnode* root, FILE* targetFile); /* handles whole-tuple copies and tuple field read/writes */

#endif // STMT_CODEGEN_H