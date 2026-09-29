#ifndef LSYMBOLTABLE_H
#define LSYMBOLTABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <tupletable.h>

typedef struct Lsymbol {
    char* name; // name of the variable
    int type;         // type of the variable
    int binding;      // BP-relative offset: params are negative, locals positive
    int pointerLevel; // Pointer level of the variable
    struct TupleTable* tupleEntry; // Pointer to tuple table entry in case the symbol is for tuple
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
void LSymbolAssignBindingAddresses(void);
/**
 * Destroys the symbol table and frees all allocated memory.
 */
void LSymbolTableDestroy(struct Lsymbol* head);

#endif // LSYMBOLTABLE_H