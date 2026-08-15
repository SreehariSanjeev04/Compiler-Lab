#ifndef SYMBOLTABLE_UTILS_H
#define SYMBOLTABLE_UTILS_H
#include <symboltable.h>

void freeSymbolTable(void);
void addDimensionSizes(struct Gsymbol* symbol, int size);
int* createStrideArray(struct Gsymbol* symbol);
void assignBindingAddresses(void);

#endif