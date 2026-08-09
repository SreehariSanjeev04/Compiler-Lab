#ifndef SYMBOLTABLE_UTILS_H
#define SYMBOLTABLE_UTILS_H
#include <symboltable.h>

void freeSymbolTable();
void addDimensionSizes(struct Gsymbol* symbol, int dimensions);
int* createStrideArray(struct Gsymbol* symbol);
void assignBindingAddresses();
int returnPointerType(int baseType);

#endif