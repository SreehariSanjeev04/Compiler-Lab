#ifndef LSYMBOLTABLE_H
#define LSYMBOLTABLE_H
#include <stdio.h>
#include <stdlib.h>

typedef struct Lsymbol {
    char* name; // name of the variable
    int type;         // type of the variable (e.g., TYPE_INT, TYPE_BOOL, etc.)
    int binding;      // BP-relative offset: params are negative (-3, -4, ...), locals positive (+1, +2, ...)
    int pointerLevel; // Pointer level of the variable
    struct Lsymbol *next; // pointer to the next symbol in the list
} Lsymbol;

extern struct Lsymbol* Lhead;

/**
 * Installs a new symbol into the symbol table.
 * @return the newly created Lsymbol, or NULL if allocation fails
 */
struct Lsymbol* LInstall(char* name, int type, int pointerLevel, int binding);
/**
 * Looks up a symbol by name.
 * @return the Lsymbol if found, or NULL if not found
 */
struct Lsymbol* LLookup(char* name);
/**
 * @return the head of the symbol table
 */
struct Lsymbol* LSymbolGetHead();
/**
 * Resets the symbol table (head and tail to NULL).
 */
void LSymbolReset();
/**
 * Assigns BP-relative positive offsets to declared locals (params already have
 * negative bindings). Must be called once, after the function's LdeclBlock.
 */
void assignLocalBindingAddresses(void);
/**
 * Destroys the symbol table and frees all allocated memory.
 */
void LSymbolTableDestroy(struct Lsymbol* head);

#endif // LSYMBOLTABLE_H