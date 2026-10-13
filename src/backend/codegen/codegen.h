#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "exprtree.h"
#include "constants.h"
#include "gsymboltable.h"

int codeGen(tnode* root, FILE* targetFile);

void generateProgramStart(FILE* targetFile);
void generateFunctionCode(struct Gsymbol* funcSymbol, tnode* body, FILE* targetFile);

#endif // CODEGEN_H
