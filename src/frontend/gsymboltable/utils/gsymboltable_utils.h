#ifndef GSYMBOLTABLE_UTILS_H
#define GSYMBOLTABLE_UTILS_H
#include <gsymboltable.h>
#include <dimnode.h>

extern int showGlobalSymbolTable;

/**
 * Frees all memory allocated for the symbol table.
 */
void freeSymbolTable(void);
/**
 * Adds a dimension size to the symbol, updating dimensions and total size.
 */
void addDimensionSizes(struct Gsymbol* symbol, int size);
/**
 * Copies the given DimNode sizes into the symbol's dimension sizes.
 */
void handleDimensionSizes(struct Gsymbol* symbol, struct DimNode* DimNode);
/**
 * Computes the stride array for a (multi-dimensional) array symbol.
 */
int* createStrideArray(struct Gsymbol* symbol);
void assignBindingAddresses(void); /* assigns static bindings; tuple vars use their tuple size */
void printGlobalSymbolTable(void);

#endif