#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "exprtree.h"
#include "constants.h"

int codeGen(tnode* root, FILE* targetFile);
void generateCode(tnode* root, FILE* targetFile);

#endif // CODEGEN_H