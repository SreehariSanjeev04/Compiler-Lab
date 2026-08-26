#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "exprtree.h"
#include "constants.h"
#include "gsymboltable.h"

int codeGen(tnode* root, FILE* targetFile);

/* Orchestration entry points:
 *   generateProgramStart  - lazily emits the XSM header (stack base = end of
 *                           globals), CALL MAIN and the Exit routine. Called
 *                           once the declaration block has been parsed.
 *   generateFunctionCode  - emits label + prologue + body + epilogue for one
 *                           function definition, then frees its AST and local
 *                           table. Called by parser actions as each definition
 *                           reduces. */
void generateProgramStart(FILE* targetFile);
void generateFunctionCode(struct Gsymbol* funcSymbol, tnode* body, FILE* targetFile);

#endif // CODEGEN_H
