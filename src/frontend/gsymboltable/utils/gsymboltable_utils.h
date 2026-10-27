#ifndef GSYMBOLTABLE_UTILS_H
#define GSYMBOLTABLE_UTILS_H
#include <gsymboltable.h>

extern int showGlobalSymbolTable;

/**
 * Frees all memory allocated for the symbol table.
 */
void freeSymbolTable(void);
/**
 * Sets the length of a one-dimensional global array.
 */
void setArrayLength(struct Gsymbol* symbol, int length);
void assignBindingAddresses(void);
void printGlobalSymbolTable(void);

#endif
