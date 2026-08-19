#ifndef GSYMBOLTABLE_UTILS_H
#define GSYMBOLTABLE_UTILS_H
#include <gsymboltable.h>

void freeSymbolTable(void);
void addDimensionSizes(struct Gsymbol* symbol, int size);
int* createStrideArray(struct Gsymbol* symbol);
void assignBindingAddresses(void);

#endif