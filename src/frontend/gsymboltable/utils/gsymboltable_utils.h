#ifndef GSYMBOLTABLE_UTILS_H
#define GSYMBOLTABLE_UTILS_H
#include <gsymboltable.h>
#include <dimnode.h>

extern int showGlobalSymbolTable;

void freeSymbolTable(void);
void addDimensionSizes(struct Gsymbol* symbol, int size);
void handleDimensionSizes(struct Gsymbol* symbol, struct DimNode* DimNode);
int* createStrideArray(struct Gsymbol* symbol);
void assignBindingAddresses(void);
void printGlobalSymbolTable(void);

#endif