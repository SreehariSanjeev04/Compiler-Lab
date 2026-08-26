#include "lsymboltable.h"
#include <string.h>

struct Lsymbol* Lhead = NULL;
struct Lsymbol* Ltail = NULL;

/**
 * Installs a new symbol into the symbol table.
 * @param name The name of the variable.
 * @param type The type of the variable (e.g., TYPE_INT, TYPE_BOOL, etc.).
 * @param binding The pseudo memory offset for the variable
 * @return A pointer to the newly created Lsymbol, or NULL if memory allocation fails.
 */
struct Lsymbol* LInstall(char* name, int type, int binding) {
    struct Lsymbol* newSymbol = (struct Lsymbol*)malloc(sizeof(struct Lsymbol));
    if(!newSymbol) {
        fprintf(stderr, "Memory allocation failed for Lsymbol\n");
        return NULL;
    }
    if(LLookup(name) != NULL) {
        fprintf(stderr, "Error: Variable '%s' is already defined in the local symbol table.\n", name);
        free(newSymbol);
        return NULL;
    }
    newSymbol->name = strdup(name);
    newSymbol->type = type;
    newSymbol->binding = binding;
    newSymbol->next = NULL;

    if(Lhead == NULL) {
        Lhead = Ltail = newSymbol;
    } else {
        Ltail->next = newSymbol;
        Ltail = newSymbol;
    }
    return newSymbol;
}


/**
 * Looks up a symbol in the symbol table by its name.
 * @param name The name of the variable to look up.
 * @return A pointer to the Lsymbol if found, or NULL if not found.
 */
struct Lsymbol* LLookup(char* name) {
    struct Lsymbol* current = Lhead;
    while(current != NULL) {
        if(strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Gets the head of the symbol table.
 * @return A pointer to the head of the symbol table.
 */
struct Lsymbol* LSymbolGetHead() {
    return Lhead;
}

/**
 * Sets the head of the symbol table to a new head.
 */
void LSymbolReset() {
    Lhead = NULL;
    Ltail = NULL;
}

/**
 * Destroys the symbol table and frees all allocated memory.
 * @param head A pointer to the head of the symbol table.
 */
void LSymbolTableDestroy(struct Lsymbol* head) {
    struct Lsymbol* current = head;
    struct Lsymbol* nextSymbol;

    while(current != NULL) {
        nextSymbol = current->next;
        free(current->name);
        free(current);
        current = nextSymbol;
    }
    LSymbolReset();
}

/**
 * Assigns BP-relative positive offsets to local variables. Parameters are
 * installed first with negative bindings; any entry still carrying the
 * unassigned sentinel (0) is a declared local and gets +1, +2, ... in
 * declaration order. Must be called once, after the function's LdeclBlock.
 */
void assignLocalBindingAddresses(void) {
    int binding = 0;
    struct Lsymbol* current = Lhead;
    while (current != NULL) {
        if (current->binding == 0) {
            current->binding = ++binding;
        }
        current = current->next;
    }
}