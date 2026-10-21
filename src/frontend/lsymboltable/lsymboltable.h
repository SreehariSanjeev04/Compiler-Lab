#ifndef LSYMBOLTABLE_H
#define LSYMBOLTABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <typetable.h>
#include <classtable.h>

typedef struct Lsymbol {
    char* name; // name of the variable
    int type;         // type of the variable
    struct TypeTable* typeEntry;
    int binding;      // BP-relative offset: params are negative, locals positive
    int pointerLevel; // Pointer level of the variable
    struct Lsymbol *next; // pointer to the next symbol in the list
    struct ClassTable *cPtr; // pointer to the class entry if it is a class variable
} Lsymbol;

extern struct Lsymbol* Lhead;

/**
 * Installs a new symbol into the symbol table.
 * @return the newly created Lsymbol, or NULL if allocation fails
 */
struct Lsymbol* LInstall(char* name, struct TypeTable* type, int pointerLevel, int binding, struct ClassTable *cptr);
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
