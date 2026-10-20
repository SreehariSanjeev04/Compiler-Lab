#ifndef CLASSCODEGEN_H
#define CLASSCODEGEN_H

#include "exprtree.h"

void emitClassFieldAccessAddress(
    int targetReg,
    tnode *root,
    FILE *targetFile
);

int codeGenClassFieldAccess(tnode *root, FILE* targetFile);
int codeGenClassMethodAccess(tnode *root, FILE* targetFile);
int codeGenClassDelete(tnode *root, FILE* targetFile);
int codeGenSelf(FILE* targetFile);

#endif
